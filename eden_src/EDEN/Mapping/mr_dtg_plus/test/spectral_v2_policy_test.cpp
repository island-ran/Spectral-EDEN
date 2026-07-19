#include <gtest/gtest.h>

#include <mr_dtg_plus/spectral_v2_policy.h>

#include <cmath>
#include <limits>

namespace DTGPlus {
namespace {

TEST(SpectralV2PolicyTest, RelativeSpectralMetricsFollowProposalFormulae) {
  EXPECT_NEAR(RelativeEigengap(0.2, 0.5, 1.0e-12), 0.6, 1.0e-9);
  EXPECT_DOUBLE_EQ(RelativeEigengap(0.8, 0.5), 0.0);
  EXPECT_TRUE(std::isnan(RelativeEigengap(-1.0, 0.5)));

  const double initialized = UpdateLambda2Ema(
      0.4, std::numeric_limits<double>::quiet_NaN(), 0.9);
  EXPECT_DOUBLE_EQ(initialized, 0.4);
  const double updated = UpdateLambda2Ema(0.2, initialized, 0.9);
  EXPECT_NEAR(updated, 0.38, 1.0e-12);
  EXPECT_NEAR(RelativeLambda2(0.2, updated, 1.0e-12),
              0.2 / 0.38, 1.0e-9);
}

TEST(SpectralV2PolicyTest, ConfidenceCombinesAllFiveNormalizedQualities) {
  PartitionConfidenceConfig config;
  config.numeric_epsilon = 1.0e-12;

  PartitionEvidence evidence;
  evidence.ncut = 0.15;                  // q_ncut = 0.5
  evidence.relative_eigengap = 0.025;    // q_gap = 0.5
  evidence.relative_lambda2 = 0.25;      // q_lambda = 0.75
  evidence.cluster0_size = 4U;
  evidence.cluster1_size = 6U;           // q_balance = 0.8
  evidence.median_cut_clearance = 0.2;
  evidence.median_inside_clearance = 1.0;  // q_bottleneck ~= 0.8

  const PartitionConfidenceResult result =
      ComputePartitionConfidence(evidence, config);

  EXPECT_NEAR(result.q_ncut, 0.5, 1.0e-12);
  EXPECT_NEAR(result.q_eigengap, 0.5, 1.0e-12);
  EXPECT_NEAR(result.q_relative_lambda2, 0.75, 1.0e-12);
  EXPECT_NEAR(result.q_balance, 0.8, 1.0e-12);
  EXPECT_NEAR(result.q_bottleneck, 0.8, 1.0e-9);
  EXPECT_NEAR(result.effective_weight_sum, 1.0, 1.0e-12);
  EXPECT_NEAR(result.confidence, 0.6275, 1.0e-9);
}

TEST(SpectralV2PolicyTest, ConfidenceClampsEvidenceAndNormalizesWeights) {
  PartitionConfidenceConfig config;
  config.weight_ncut = 2.0;
  config.weight_eigengap = 0.0;
  config.weight_relative_lambda2 = -1.0;
  config.weight_bottleneck = 0.0;
  config.weight_balance = 0.0;

  PartitionEvidence evidence;
  evidence.ncut = 0.15;
  evidence.relative_eigengap = 100.0;
  evidence.relative_lambda2 = 2.0;
  evidence.cluster0_size = 0U;
  evidence.cluster1_size = 10U;
  evidence.median_cut_clearance = 2.0;
  evidence.median_inside_clearance = 1.0;

  const PartitionConfidenceResult result =
      ComputePartitionConfidence(evidence, config);
  EXPECT_NEAR(result.q_ncut, 0.5, 1.0e-12);
  EXPECT_DOUBLE_EQ(result.q_eigengap, 1.0);
  EXPECT_DOUBLE_EQ(result.q_relative_lambda2, 0.0);
  EXPECT_DOUBLE_EQ(result.q_balance, 0.0);
  EXPECT_DOUBLE_EQ(result.q_bottleneck, 0.0);
  EXPECT_DOUBLE_EQ(result.effective_weight_sum, 2.0);
  EXPECT_NEAR(result.confidence, 0.5, 1.0e-12);

  PartitionEvidence missing;
  EXPECT_DOUBLE_EQ(ComputePartitionConfidence(missing).confidence, 0.0);
}

TEST(SpectralV2PolicyTest, DynamicSwitchPenaltyUsesClampedConfidenceAndReturnRisk) {
  EXPECT_NEAR(ComputeDynamicSwitchPenalty(8.0, 0.75, 0.5), 3.0,
              1.0e-12);
  EXPECT_DOUBLE_EQ(ComputeDynamicSwitchPenalty(8.0, 2.0, -1.0), 0.0);
  EXPECT_DOUBLE_EQ(ComputeDynamicSwitchPenalty(-8.0, 0.5, 0.5), 0.0);
}

TEST(SpectralV2PolicyTest, RegionRouteMustPassCombinedCostAndRegretGuards) {
  RouteMetrics baseline;
  baseline.path_cost = 100.0;
  baseline.switch_count = 2U;
  baseline.revisit_distance = 5.0;

  RouteAcceptanceConfig config;
  config.max_route_regret = 0.05;
  config.switch_penalty = 5.0;
  config.revisit_penalty = 2.0;

  RouteMetrics candidate;
  candidate.path_cost = 104.0;
  const RouteDecision accepted =
      EvaluateRegionRoute(baseline, candidate, config);
  EXPECT_TRUE(accepted.valid);
  EXPECT_DOUBLE_EQ(accepted.baseline_combined_cost, 120.0);
  EXPECT_DOUBLE_EQ(accepted.candidate_combined_cost, 104.0);
  EXPECT_TRUE(accepted.combined_cost_improved);
  EXPECT_TRUE(accepted.within_path_regret);
  EXPECT_TRUE(accepted.accepted);
  EXPECT_NEAR(accepted.path_regret, 0.04, 1.0e-9);

  candidate.path_cost = 106.0;
  const RouteDecision excessive_regret =
      EvaluateRegionRoute(baseline, candidate, config);
  EXPECT_TRUE(excessive_regret.combined_cost_improved);
  EXPECT_FALSE(excessive_regret.within_path_regret);
  EXPECT_FALSE(excessive_regret.accepted);

  RouteMetrics plain_baseline;
  plain_baseline.path_cost = 100.0;
  RouteMetrics shorter_but_penalized;
  shorter_but_penalized.path_cost = 99.0;
  shorter_but_penalized.switch_count = 1U;
  const RouteDecision no_combined_improvement =
      EvaluateRegionRoute(plain_baseline, shorter_but_penalized, config);
  EXPECT_TRUE(no_combined_improvement.within_path_regret);
  EXPECT_FALSE(no_combined_improvement.combined_cost_improved);
  EXPECT_FALSE(no_combined_improvement.accepted);
}

TEST(SpectralV2PolicyTest, InvalidRouteNeverPassesGuard) {
  RouteMetrics baseline;
  baseline.path_cost = 10.0;
  RouteMetrics candidate;
  candidate.path_cost = std::numeric_limits<double>::infinity();

  const RouteDecision decision = EvaluateRegionRoute(baseline, candidate);
  EXPECT_FALSE(decision.valid);
  EXPECT_FALSE(decision.accepted);
}

TEST(SpectralV2PolicyTest, LockDebtDecaysAccumulatesOnlyDetourAndTriggersRecovery) {
  LockDebtConfig config;
  config.decay = 0.90;
  config.max_debt = 8.0;

  const LockDebtUpdate increased =
      UpdateLockDebt(5.0, 104.0, 100.0, config);
  EXPECT_DOUBLE_EQ(increased.previous_debt, 5.0);
  EXPECT_DOUBLE_EQ(increased.decayed_debt, 4.5);
  EXPECT_DOUBLE_EQ(increased.added_detour, 4.0);
  EXPECT_DOUBLE_EQ(increased.debt, 8.5);
  EXPECT_TRUE(increased.recovery_required);

  const LockDebtUpdate decayed =
      UpdateLockDebt(increased.debt, 90.0, 100.0, config);
  EXPECT_DOUBLE_EQ(decayed.added_detour, 0.0);
  EXPECT_NEAR(decayed.debt, 7.65, 1.0e-12);
  EXPECT_FALSE(decayed.recovery_required);
}

TEST(SpectralV2PolicyTest, HysteresisUsesAsymmetricPersistenceAndGrace) {
  SpectralModeConfig config;
  config.confidence_on = 0.70;
  config.confidence_off = 0.40;
  config.on_persistence = 3U;
  config.off_persistence = 5U;
  config.grace_epochs = 8U;

  SpectralModeState state;
  SpectralModeInput input;
  input.partition_confidence = 0.8;

  SpectralModeUpdate update = UpdateSpectralMode(state, input, config);
  state = update.state;
  EXPECT_EQ(state.mode, SpectralMode::OBSERVING);
  EXPECT_EQ(state.strong_signal_streak, 1U);

  update = UpdateSpectralMode(state, input, config);
  state = update.state;
  EXPECT_EQ(state.mode, SpectralMode::OBSERVING);
  EXPECT_EQ(state.strong_signal_streak, 2U);

  update = UpdateSpectralMode(state, input, config);
  state = update.state;
  EXPECT_TRUE(update.activated);
  EXPECT_EQ(state.mode, SpectralMode::ACTIVE_SOFT);

  input.partition_confidence = 0.2;
  for (std::size_t epoch = 1U; epoch <= 7U; ++epoch) {
    update = UpdateSpectralMode(state, input, config);
    state = update.state;
    EXPECT_EQ(state.mode, SpectralMode::ACTIVE_SOFT) << "epoch " << epoch;
  }
  EXPECT_EQ(state.weak_signal_streak, 7U);

  update = UpdateSpectralMode(state, input, config);
  EXPECT_TRUE(update.deactivated);
  EXPECT_EQ(update.state.mode, SpectralMode::OBSERVING);
}

TEST(SpectralV2PolicyTest, RouteGuardGatesActivationWithoutDiscardingStrongStreak) {
  SpectralModeConfig config;
  config.on_persistence = 2U;
  SpectralModeState state;

  SpectralModeInput input;
  input.partition_confidence = 0.9;
  input.route_acceptable = false;
  state = UpdateSpectralMode(state, input, config).state;
  state = UpdateSpectralMode(state, input, config).state;
  EXPECT_EQ(state.mode, SpectralMode::OBSERVING);
  EXPECT_EQ(state.strong_signal_streak, 2U);

  input.route_acceptable = true;
  const SpectralModeUpdate update = UpdateSpectralMode(state, input, config);
  EXPECT_TRUE(update.activated);
  EXPECT_EQ(update.state.mode, SpectralMode::ACTIVE_SOFT);
}

TEST(SpectralV2PolicyTest, RecoveryRequiresExplicitCompletionAndDisabledHasPriority) {
  SpectralModeState state;
  state.mode = SpectralMode::ACTIVE_SOFT;

  SpectralModeInput input;
  input.recovery_requested = true;
  SpectralModeUpdate update = UpdateSpectralMode(state, input);
  EXPECT_TRUE(update.entered_recovery);
  EXPECT_TRUE(update.deactivated);
  EXPECT_EQ(update.state.mode, SpectralMode::RECOVERY);

  state = update.state;
  input.recovery_requested = false;
  update = UpdateSpectralMode(state, input);
  EXPECT_EQ(update.state.mode, SpectralMode::RECOVERY);
  EXPECT_FALSE(update.exited_recovery);

  input.recovery_complete = true;
  update = UpdateSpectralMode(update.state, input);
  EXPECT_TRUE(update.exited_recovery);
  EXPECT_EQ(update.state.mode, SpectralMode::OBSERVING);

  input.enabled = false;
  input.recovery_requested = true;
  update = UpdateSpectralMode(update.state, input);
  EXPECT_EQ(update.state.mode, SpectralMode::DISABLED);
  EXPECT_FALSE(update.entered_recovery);
}

}  // namespace
}  // namespace DTGPlus
