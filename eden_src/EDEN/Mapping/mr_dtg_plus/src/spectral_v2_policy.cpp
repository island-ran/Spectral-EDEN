#include <mr_dtg_plus/spectral_v2_policy.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace DTGPlus {
namespace {

double QuietNaN() {
  return std::numeric_limits<double>::quiet_NaN();
}

double PositiveEpsilon(double epsilon) {
  return std::isfinite(epsilon) && epsilon > 0.0 ? epsilon : 1.0e-9;
}

double NonNegativeOrZero(double value) {
  return std::isfinite(value) && value > 0.0 ? value : 0.0;
}

std::size_t SaturatingIncrement(std::size_t value) {
  return value == std::numeric_limits<std::size_t>::max() ? value : value + 1U;
}

void ResetCounters(SpectralModeState* state) {
  state->strong_signal_streak = 0U;
  state->weak_signal_streak = 0U;
  state->active_epochs = 0U;
}

bool ValidRouteMetrics(const RouteMetrics& metrics) {
  return std::isfinite(metrics.path_cost) && metrics.path_cost >= 0.0 &&
         std::isfinite(metrics.revisit_distance) &&
         metrics.revisit_distance >= 0.0;
}

double CombinedRouteCost(const RouteMetrics& metrics, double switch_penalty,
                         double revisit_penalty) {
  if (!ValidRouteMetrics(metrics)) {
    return std::numeric_limits<double>::infinity();
  }
  const double cost =
      metrics.path_cost + switch_penalty * metrics.switch_count +
      revisit_penalty * metrics.revisit_distance;
  return std::isfinite(cost) ? cost : std::numeric_limits<double>::infinity();
}

}  // namespace

const char* SpectralModeName(SpectralMode mode) {
  switch (mode) {
    case SpectralMode::DISABLED:
      return "DISABLED";
    case SpectralMode::OBSERVING:
      return "OBSERVING";
    case SpectralMode::ACTIVE_SOFT:
      return "ACTIVE_SOFT";
    case SpectralMode::RECOVERY:
      return "RECOVERY";
  }
  return "UNKNOWN";
}

double Clamp01(double value) {
  if (!std::isfinite(value)) {
    return 0.0;
  }
  return std::max(0.0, std::min(1.0, value));
}

double RelativeEigengap(double lambda2, double lambda3, double epsilon) {
  if (!std::isfinite(lambda2) || !std::isfinite(lambda3) || lambda2 < 0.0 ||
      lambda3 < 0.0) {
    return QuietNaN();
  }
  const double denominator = lambda3 + PositiveEpsilon(epsilon);
  if (!std::isfinite(denominator) || denominator <= 0.0) {
    return QuietNaN();
  }
  return Clamp01((lambda3 - lambda2) / denominator);
}

double UpdateLambda2Ema(double lambda2, double previous_ema, double alpha) {
  const bool current_valid = std::isfinite(lambda2) && lambda2 >= 0.0;
  const bool previous_valid =
      std::isfinite(previous_ema) && previous_ema >= 0.0;
  if (!current_valid) {
    return previous_valid ? previous_ema : QuietNaN();
  }
  if (!previous_valid) {
    return lambda2;
  }
  const double clipped_alpha = Clamp01(alpha);
  return clipped_alpha * previous_ema + (1.0 - clipped_alpha) * lambda2;
}

double RelativeLambda2(double lambda2, double lambda2_ema, double epsilon) {
  if (!std::isfinite(lambda2) || !std::isfinite(lambda2_ema) ||
      lambda2 < 0.0 || lambda2_ema < 0.0) {
    return QuietNaN();
  }
  const double denominator = lambda2_ema + PositiveEpsilon(epsilon);
  if (!std::isfinite(denominator) || denominator <= 0.0) {
    return QuietNaN();
  }
  return lambda2 / denominator;
}

double ExpectedGainRankingScale(double expected_gain,
                                double low_gain_epsilon,
                                double low_gain_scale) {
  if (!std::isfinite(expected_gain) ||
      !std::isfinite(low_gain_epsilon) || low_gain_epsilon < 0.0 ||
      !std::isfinite(low_gain_scale)) {
    return 1.0;
  }
  return expected_gain <= low_gain_epsilon
             ? std::max(1.0e-3, Clamp01(low_gain_scale))
             : 1.0;
}

RepeatPlanningObservation ObserveRepeatedPlanning(
    bool same_frontier, int previous_repeat_count,
    double previous_count_time, std::uint64_t previous_total_count,
    double now, double minimum_count_interval) {
  RepeatPlanningObservation observation;
  observation.repeat_count = std::max(0, previous_repeat_count);
  observation.last_count_time = previous_count_time;
  observation.total_repeat_count = previous_total_count;
  if (!std::isfinite(now)) {
    return observation;
  }
  const double interval =
      std::isfinite(minimum_count_interval)
          ? std::max(0.0, minimum_count_interval)
          : 0.5;
  if (!same_frontier) {
    observation.repeat_count = 1;
    observation.last_count_time = now;
    return observation;
  }
  if (!std::isfinite(previous_count_time) ||
      previous_count_time < 0.0 ||
      now - previous_count_time >= interval) {
    if (observation.repeat_count < std::numeric_limits<int>::max()) {
      ++observation.repeat_count;
    }
    if (observation.total_repeat_count <
        std::numeric_limits<std::uint64_t>::max()) {
      ++observation.total_repeat_count;
    }
    observation.last_count_time = now;
  }
  return observation;
}

FirstTargetChoice ChooseFirstTargetFromEohdtPrefix(
    const std::vector<double>& robot_distances,
    const std::vector<int>& region_ids, std::size_t top_k,
    int reference_region, double switch_penalty,
    double switch_penalty_scale, double maximum_regret,
    double numeric_epsilon) {
  FirstTargetChoice choice;
  if (robot_distances.empty() ||
      region_ids.size() != robot_distances.size() ||
      !std::isfinite(robot_distances.front()) ||
      robot_distances.front() < 0.0) {
    return choice;
  }

  const double epsilon = PositiveEpsilon(numeric_epsilon);
  choice.baseline_distance = robot_distances.front();
  const std::size_t candidate_count =
      std::min(robot_distances.size(), std::max<std::size_t>(1U, top_k));
  const double penalty =
      NonNegativeOrZero(switch_penalty) *
      NonNegativeOrZero(switch_penalty_scale);
  double best_score = std::numeric_limits<double>::infinity();
  for (std::size_t index = 0U; index < candidate_count; ++index) {
    const double distance = robot_distances[index];
    if (!std::isfinite(distance) || distance < 0.0) {
      continue;
    }
    const bool crosses_region =
        reference_region >= 0 && region_ids[index] >= 0 &&
        region_ids[index] != reference_region;
    const double score = distance + (crosses_region ? penalty : 0.0);
    if (score + epsilon < best_score) {
      best_score = score;
      choice.baseline_index = index;
      choice.selected_distance = distance;
      choice.valid = true;
    }
  }
  if (!choice.valid) {
    return choice;
  }

  choice.regret =
      (choice.selected_distance - choice.baseline_distance) /
      std::max(choice.baseline_distance, epsilon);
  const double regret_limit = NonNegativeOrZero(maximum_regret);
  choice.within_regret =
      choice.selected_distance <=
      choice.baseline_distance * (1.0 + regret_limit) + epsilon;
  return choice;
}

std::size_t ChooseNearestFailOpenCandidate(
    const std::vector<double>& distances,
    const std::vector<unsigned char>& restorable) {
  if (distances.size() != restorable.size()) {
    return distances.size();
  }
  std::size_t best = distances.size();
  double best_distance = std::numeric_limits<double>::infinity();
  for (std::size_t index = 0U; index < distances.size(); ++index) {
    if (restorable[index] == 0U || !std::isfinite(distances[index]) ||
        distances[index] < 0.0) {
      continue;
    }
    if (distances[index] < best_distance) {
      best = index;
      best_distance = distances[index];
    }
  }
  return best;
}

PartitionConfidenceResult ComputePartitionConfidence(
    const PartitionEvidence& evidence,
    const PartitionConfidenceConfig& config) {
  PartitionConfidenceResult result;
  const double epsilon = PositiveEpsilon(config.numeric_epsilon);

  if (std::isfinite(evidence.ncut) && evidence.ncut >= 0.0 &&
      std::isfinite(config.ncut_soft_threshold) &&
      config.ncut_soft_threshold > epsilon) {
    result.q_ncut =
        1.0 - Clamp01(evidence.ncut / config.ncut_soft_threshold);
  }

  if (std::isfinite(evidence.relative_eigengap) &&
      evidence.relative_eigengap >= 0.0 &&
      std::isfinite(config.eigengap_soft_threshold) &&
      config.eigengap_soft_threshold > epsilon) {
    result.q_eigengap = Clamp01(evidence.relative_eigengap /
                                config.eigengap_soft_threshold);
  }

  if (std::isfinite(evidence.relative_lambda2) &&
      evidence.relative_lambda2 >= 0.0) {
    result.q_relative_lambda2 =
        1.0 - Clamp01(evidence.relative_lambda2);
  }

  const double cluster0 = static_cast<double>(evidence.cluster0_size);
  const double cluster1 = static_cast<double>(evidence.cluster1_size);
  const double total_size = cluster0 + cluster1;
  if (std::isfinite(total_size) && total_size > 0.0) {
    result.q_balance =
        Clamp01(2.0 * std::min(cluster0, cluster1) / total_size);
  }

  if (std::isfinite(evidence.median_cut_clearance) &&
      std::isfinite(evidence.median_inside_clearance) &&
      evidence.median_cut_clearance >= 0.0 &&
      evidence.median_inside_clearance > epsilon) {
    result.q_bottleneck = Clamp01(
        1.0 - evidence.median_cut_clearance /
                  (evidence.median_inside_clearance + epsilon));
  }

  double weighted_sum = 0.0;
  const auto add_term = [&weighted_sum, &result](double weight,
                                                  double quality) {
    if (std::isfinite(weight) && weight > 0.0) {
      result.effective_weight_sum += weight;
      weighted_sum += weight * quality;
    }
  };
  add_term(config.weight_ncut, result.q_ncut);
  add_term(config.weight_eigengap, result.q_eigengap);
  add_term(config.weight_relative_lambda2, result.q_relative_lambda2);
  add_term(config.weight_bottleneck, result.q_bottleneck);
  add_term(config.weight_balance, result.q_balance);

  if (result.effective_weight_sum > epsilon) {
    result.confidence =
        Clamp01(weighted_sum / result.effective_weight_sum);
  }
  return result;
}

double ComputeDynamicSwitchPenalty(double base_penalty,
                                   double partition_confidence,
                                   double return_probability) {
  if (!std::isfinite(base_penalty) || base_penalty <= 0.0) {
    return 0.0;
  }
  return base_penalty * Clamp01(partition_confidence) *
         Clamp01(return_probability);
}

RouteDecision EvaluateRegionRoute(const RouteMetrics& baseline,
                                  const RouteMetrics& candidate,
                                  const RouteAcceptanceConfig& config) {
  RouteDecision decision;
  const double epsilon = PositiveEpsilon(config.numeric_epsilon);
  decision.path_regret_limit = NonNegativeOrZero(config.max_route_regret);

  const double switch_penalty = NonNegativeOrZero(config.switch_penalty);
  const double revisit_penalty = NonNegativeOrZero(config.revisit_penalty);
  decision.baseline_combined_cost =
      CombinedRouteCost(baseline, switch_penalty, revisit_penalty);
  decision.candidate_combined_cost =
      CombinedRouteCost(candidate, switch_penalty, revisit_penalty);
  decision.valid = ValidRouteMetrics(baseline) &&
                   ValidRouteMetrics(candidate) &&
                   std::isfinite(decision.baseline_combined_cost) &&
                   std::isfinite(decision.candidate_combined_cost);
  if (!decision.valid) {
    return decision;
  }

  decision.path_regret =
      (candidate.path_cost - baseline.path_cost) /
      (baseline.path_cost + epsilon);
  decision.combined_cost_improved =
      decision.candidate_combined_cost + epsilon <
      decision.baseline_combined_cost;

  const double maximum_candidate_path =
      baseline.path_cost * (1.0 + decision.path_regret_limit);
  decision.within_path_regret =
      candidate.path_cost <= maximum_candidate_path + epsilon;
  decision.accepted =
      decision.combined_cost_improved && decision.within_path_regret;
  return decision;
}

LockDebtUpdate UpdateLockDebt(double previous_debt, double selected_path_cost,
                              double baseline_path_cost,
                              const LockDebtConfig& config) {
  LockDebtUpdate update;
  update.previous_debt = NonNegativeOrZero(previous_debt);
  const double decay = Clamp01(config.decay);
  update.decayed_debt = update.previous_debt * decay;

  if (std::isfinite(selected_path_cost) && selected_path_cost >= 0.0 &&
      std::isfinite(baseline_path_cost) && baseline_path_cost >= 0.0) {
    update.added_detour =
        std::max(0.0, selected_path_cost - baseline_path_cost);
  }
  update.debt = update.decayed_debt + update.added_detour;
  if (!std::isfinite(update.debt)) {
    update.debt = std::numeric_limits<double>::infinity();
  }

  const double maximum_debt =
      std::isfinite(config.max_debt)
          ? std::max(0.0, config.max_debt)
          : std::numeric_limits<double>::infinity();
  update.recovery_required = update.debt > maximum_debt;
  return update;
}

SpectralModeUpdate UpdateSpectralMode(const SpectralModeState& previous,
                                      const SpectralModeInput& input,
                                      const SpectralModeConfig& config) {
  SpectralModeUpdate update;
  update.state = previous;

  const auto set_mode = [&update](SpectralMode mode) {
    if (update.state.mode != mode) {
      update.mode_changed = true;
      update.state.mode = mode;
    }
  };
  const auto mark_active_exit = [&update, &previous]() {
    if (previous.mode == SpectralMode::ACTIVE_SOFT) {
      update.deactivated = true;
    }
  };

  if (!input.enabled) {
    mark_active_exit();
    set_mode(SpectralMode::DISABLED);
    ResetCounters(&update.state);
    return update;
  }

  if (input.recovery_requested) {
    mark_active_exit();
    update.entered_recovery = previous.mode != SpectralMode::RECOVERY;
    set_mode(SpectralMode::RECOVERY);
    ResetCounters(&update.state);
    return update;
  }

  if (previous.mode == SpectralMode::RECOVERY) {
    ResetCounters(&update.state);
    if (!input.recovery_complete) {
      return update;
    }
    update.exited_recovery = true;
    set_mode(input.eligible ? SpectralMode::OBSERVING
                            : SpectralMode::DISABLED);
    return update;
  }

  if (!input.eligible) {
    mark_active_exit();
    set_mode(SpectralMode::DISABLED);
    ResetCounters(&update.state);
    return update;
  }

  if (update.state.mode != SpectralMode::OBSERVING &&
      update.state.mode != SpectralMode::ACTIVE_SOFT) {
    set_mode(SpectralMode::OBSERVING);
    ResetCounters(&update.state);
  }

  double confidence_on = Clamp01(config.confidence_on);
  double confidence_off = Clamp01(config.confidence_off);
  if (confidence_on < confidence_off) {
    std::swap(confidence_on, confidence_off);
  }
  const std::size_t on_persistence =
      std::max<std::size_t>(1U, config.on_persistence);
  const std::size_t off_persistence =
      std::max<std::size_t>(1U, config.off_persistence);
  const double confidence = Clamp01(input.partition_confidence);

  if (update.state.mode == SpectralMode::OBSERVING) {
    update.state.weak_signal_streak = 0U;
    update.state.active_epochs = 0U;
    if (confidence >= confidence_on) {
      update.state.strong_signal_streak =
          SaturatingIncrement(update.state.strong_signal_streak);
    } else {
      update.state.strong_signal_streak = 0U;
    }

    if (update.state.strong_signal_streak >= on_persistence &&
        input.route_acceptable) {
      set_mode(SpectralMode::ACTIVE_SOFT);
      update.activated = true;
      update.state.strong_signal_streak = 0U;
      update.state.weak_signal_streak = 0U;
      update.state.active_epochs = 0U;
    }
    return update;
  }

  update.state.strong_signal_streak = 0U;
  update.state.active_epochs =
      SaturatingIncrement(update.state.active_epochs);
  if (confidence <= confidence_off) {
    update.state.weak_signal_streak =
        SaturatingIncrement(update.state.weak_signal_streak);
  } else {
    update.state.weak_signal_streak = 0U;
  }

  if (update.state.active_epochs >= config.grace_epochs &&
      update.state.weak_signal_streak >= off_persistence) {
    set_mode(SpectralMode::OBSERVING);
    update.deactivated = true;
    ResetCounters(&update.state);
  }
  return update;
}

}  // namespace DTGPlus
