#include <mr_dtg_plus/committed_target_manager.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace DTGPlus {

namespace {
constexpr double kBetaDefault = 1.0;
}  // namespace

CommittedTargetManager::CommittedTargetManager(const SpectralV4Config& config)
    : config_(config) {}

bool CommittedTargetManager::CanKeepCurrent(
    double now, const CommittedTarget& target,
    bool frontier_valid, bool path_valid, bool trajectory_safe) const {

  // Must have a valid target to keep
  if (!target.valid) {
    return false;
  }

  // Hard events always override the commitment timer.
  if (IsHardInvalidation(
          !frontier_valid, !path_valid, !trajectory_safe,
          target.consecutive_low_gain)) {
    return false;
  }

  const double elapsed = now - target.accepted_time;
  if (elapsed < config_.min_target_commit_sec) return true;

  // Once the mandatory interval expires the target remains a candidate, but
  // the planner may compare it with alternatives using the switching margin.
  return false;
}

bool CommittedTargetManager::IsHardInvalidation(
    bool frontier_invalid, bool path_blocked,
    bool trajectory_unsafe, int consecutive_low_gain) const {
  return frontier_invalid || path_blocked || trajectory_unsafe ||
         consecutive_low_gain >= 2;
}

int CommittedTargetManager::SelectWithHysteresis(
    double /*now*/, const CommittedTarget& current,
    const std::vector<double>& candidate_costs,
    const std::vector<double>& candidate_gains) const {

  // Cost function: J(c) = T_exact - beta * ln(1 + G)
  auto cost_J = [](double T_exact, double gain) {
    return T_exact - kBetaDefault * std::log1p(std::max(0.0, gain));
  };

  const size_t n = std::min(candidate_costs.size(), candidate_gains.size());

  // Find the best candidate (lowest J).  Ties are broken by lowest index
  // because we only update on strict less-than.
  int best_idx = -1;
  double best_J = std::numeric_limits<double>::max();

  for (size_t i = 0; i < n; ++i) {
    const double J = cost_J(candidate_costs[i], candidate_gains[i]);
    if (J < best_J) {
      best_J = J;
      best_idx = static_cast<int>(i);
    }
  }

  if (best_idx < 0) {
    return -1;  // no candidates available
  }

  // If there is no current target, simply take the best candidate.
  if (!current.valid) {
    return best_idx;
  }

  // Compute the cost of the current committed target.
  const double J_current =
      cost_J(current.exact_path_time, current.expected_gain) +
      static_cast<double>(current.consecutive_low_gain) *
          config_.target_switch_abs_margin_sec;

  // Hysteresis margin: new candidate must beat current by either an absolute
  // margin (seconds) or a relative fraction of |J_current|, whichever is larger.
  const double abs_margin = config_.target_switch_abs_margin_sec;
  const double rel_margin =
      config_.target_switch_relative_margin * std::abs(J_current);
  const double switch_margin = std::max(abs_margin, rel_margin);

  if (best_J + switch_margin < J_current) {
    return best_idx;
  }

  return -1;  // keep current target
}

CommittedTarget CommittedTargetManager::CommitAfterPublish(
    const CommittedTarget& candidate,
    uint32_t frontier_id, uint8_t viewpoint_id,
    double now, double initial_unknown_count) const {
  CommittedTarget committed = candidate;
  committed.valid = true;
  if (candidate.type == MacroTargetType::FRONTIER) {
    committed.frontier_id = frontier_id;
    committed.viewpoint_id = viewpoint_id;
  }
  committed.accepted_time = now;
  committed.actual_gain_since_accept = 0.0;
  committed.initial_unknown_count = std::max(0.0, initial_unknown_count);
  committed.last_observed_unknown_count =
      committed.initial_unknown_count;
  committed.consecutive_low_gain = 0;
  return committed;
}

}  // namespace DTGPlus
