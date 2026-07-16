#ifndef MR_DTG_PLUS_SPECTRAL_V2_POLICY_H_
#define MR_DTG_PLUS_SPECTRAL_V2_POLICY_H_

#include <cstddef>
#include <limits>

namespace DTGPlus {

// Runtime states of the Spectral-EDEN v2 policy.  This header deliberately
// depends only on the C++ standard library so the policy can be tested without
// ROS, mapping, or trajectory-planning dependencies.
enum class SpectralMode {
  DISABLED = 0,
  OBSERVING,
  ACTIVE_SOFT,
  RECOVERY
};

const char* SpectralModeName(SpectralMode mode);

// Invalid/non-finite values are mapped to zero.  Otherwise the result is
// clipped to [0, 1].
double Clamp01(double value);

// g_t = (lambda3 - lambda2) / (lambda3 + epsilon), clipped to [0, 1].
// Invalid inputs return NaN so callers can distinguish missing evidence from
// a genuinely weak eigengap.
double RelativeEigengap(double lambda2, double lambda3,
                        double epsilon = 1.0e-9);

// lambda2_ema(t) = alpha * lambda2_ema(t-1) + (1-alpha) * lambda2(t).
// A missing previous value initializes the EMA from the current sample; a
// missing current sample preserves the previous finite value.
double UpdateLambda2Ema(double lambda2, double previous_ema, double alpha);

// r_t = lambda2 / (lambda2_ema + epsilon).  The ratio is intentionally not
// clipped because values above one are meaningful; confidence computation
// clips it when forming q_lambda.
double RelativeLambda2(double lambda2, double lambda2_ema,
                       double epsilon = 1.0e-9);

struct PartitionConfidenceConfig {
  double ncut_soft_threshold = 0.30;
  double eigengap_soft_threshold = 0.05;

  double weight_ncut = 0.30;
  double weight_eigengap = 0.25;
  double weight_relative_lambda2 = 0.15;
  double weight_bottleneck = 0.20;
  double weight_balance = 0.10;

  double numeric_epsilon = 1.0e-9;
};

struct PartitionEvidence {
  double ncut = std::numeric_limits<double>::quiet_NaN();
  double relative_eigengap = std::numeric_limits<double>::quiet_NaN();
  double relative_lambda2 = std::numeric_limits<double>::quiet_NaN();

  std::size_t cluster0_size = 0U;
  std::size_t cluster1_size = 0U;

  double median_cut_clearance =
      std::numeric_limits<double>::quiet_NaN();
  double median_inside_clearance =
      std::numeric_limits<double>::quiet_NaN();
};

struct PartitionConfidenceResult {
  double q_ncut = 0.0;
  double q_eigengap = 0.0;
  double q_relative_lambda2 = 0.0;
  double q_bottleneck = 0.0;
  double q_balance = 0.0;

  // Sum of the finite, positive configured weights.  The weighted score is
  // normalized by this value, making the result robust to YAML round-off or
  // intentionally disabled (zero-weight) evidence terms.
  double effective_weight_sum = 0.0;
  double confidence = 0.0;
};

PartitionConfidenceResult ComputePartitionConfidence(
    const PartitionEvidence& evidence,
    const PartitionConfidenceConfig& config = PartitionConfidenceConfig());

// Route-level summary used by the dual-route guard.  switch_count is the
// number of owner-region transitions and revisit_distance is the estimated
// distance attributable to leaving and later returning to a region.
struct RouteMetrics {
  double path_cost = std::numeric_limits<double>::infinity();
  std::size_t switch_count = 0U;
  double revisit_distance = 0.0;
};

struct RouteAcceptanceConfig {
  double max_route_regret = 0.05;
  double switch_penalty = 0.0;
  double revisit_penalty = 0.0;
  double numeric_epsilon = 1.0e-9;
};

struct RouteDecision {
  double baseline_combined_cost = std::numeric_limits<double>::infinity();
  double candidate_combined_cost = std::numeric_limits<double>::infinity();
  double path_regret = std::numeric_limits<double>::infinity();
  double path_regret_limit = 0.05;

  bool valid = false;
  bool combined_cost_improved = false;
  bool within_path_regret = false;
  bool accepted = false;
};

// lambda_switch(t) = lambda0 * C_part(t) * P_return(t).
double ComputeDynamicSwitchPenalty(double base_penalty,
                                   double partition_confidence,
                                   double return_probability);

// Accepts the region-aware candidate only when its combined cost is strictly
// lower and its path cost is within the configured regret bound.
RouteDecision EvaluateRegionRoute(
    const RouteMetrics& baseline, const RouteMetrics& candidate,
    const RouteAcceptanceConfig& config = RouteAcceptanceConfig());

struct LockDebtConfig {
  double decay = 0.90;
  double max_debt = 8.0;
};

struct LockDebtUpdate {
  double previous_debt = 0.0;
  double decayed_debt = 0.0;
  double added_detour = 0.0;
  double debt = 0.0;
  bool recovery_required = false;
};

// B(t+1) = eta * B(t) + max(0, selected_path - baseline_path).
LockDebtUpdate UpdateLockDebt(
    double previous_debt, double selected_path_cost,
    double baseline_path_cost,
    const LockDebtConfig& config = LockDebtConfig());

struct SpectralModeConfig {
  double confidence_on = 0.70;
  double confidence_off = 0.40;
  std::size_t on_persistence = 3U;
  std::size_t off_persistence = 5U;
  std::size_t grace_epochs = 8U;
};

struct SpectralModeState {
  SpectralMode mode = SpectralMode::DISABLED;
  std::size_t strong_signal_streak = 0U;
  std::size_t weak_signal_streak = 0U;
  // Full policy updates elapsed since entering ACTIVE_SOFT.  It is reset to
  // zero on activation, so grace_epochs denotes complete protected epochs.
  std::size_t active_epochs = 0U;
};

struct SpectralModeInput {
  // enabled is the operator/config master switch.  eligible represents
  // runtime prerequisites (graph size, compute budget, non-late exploration).
  bool enabled = true;
  bool eligible = true;

  // RECOVERY has explicit entry/exit signals because duration is measured by
  // the ROS-facing integration layer (wall time), not by this pure policy.
  bool recovery_requested = false;
  bool recovery_complete = false;

  // A route rejection falls back to EOHDT for this epoch.  It gates initial
  // activation but does not tear down an already stable partition by itself.
  bool route_acceptable = true;
  double partition_confidence = 0.0;
};

struct SpectralModeUpdate {
  SpectralModeState state;
  bool mode_changed = false;
  bool activated = false;
  bool deactivated = false;
  bool entered_recovery = false;
  bool exited_recovery = false;
};

// Applies on/off hysteresis, asymmetric persistence, and an activation grace
// period.  DISABLED/RECOVERY transitions have priority over confidence.
SpectralModeUpdate UpdateSpectralMode(
    const SpectralModeState& previous, const SpectralModeInput& input,
    const SpectralModeConfig& config = SpectralModeConfig());

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_SPECTRAL_V2_POLICY_H_
