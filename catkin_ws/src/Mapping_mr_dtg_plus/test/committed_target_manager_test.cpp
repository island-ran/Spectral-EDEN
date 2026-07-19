#include <gtest/gtest.h>

#include <mr_dtg_plus/committed_target_manager.h>

namespace DTGPlus {
namespace {

TEST(CommittedTargetManagerTest, CommitmentTimerKeepsOnlyValidTarget) {
  SpectralV4Config config;
  config.min_target_commit_sec = 1.5;
  CommittedTargetManager manager(config);
  CommittedTarget target;
  target.valid = true;
  target.accepted_time = 10.0;

  EXPECT_TRUE(manager.CanKeepCurrent(
      11.0, target, true, true, true));
  EXPECT_FALSE(manager.CanKeepCurrent(
      11.0, target, false, true, true));
  EXPECT_FALSE(manager.CanKeepCurrent(
      12.0, target, true, true, true));
}

TEST(CommittedTargetManagerTest, HysteresisRejectsNoisyAlternation) {
  SpectralV4Config config;
  config.target_switch_abs_margin_sec = 0.7;
  config.target_switch_relative_margin = 0.1;
  CommittedTargetManager manager(config);
  CommittedTarget current;
  current.valid = true;
  current.exact_path_time = 10.0;
  current.expected_gain = 10.0;

  EXPECT_EQ(-1, manager.SelectWithHysteresis(
      20.0, current, {10.0, 9.8}, {10.0, 10.0}));
  EXPECT_EQ(1, manager.SelectWithHysteresis(
      20.0, current, {10.0, 7.0}, {10.0, 10.0}));
}

TEST(CommittedTargetManagerTest, CommitPreservesPreparedMacroTarget) {
  SpectralV4Config config;
  CommittedTargetManager manager(config);
  CommittedTarget candidate;
  candidate.valid = true;
  candidate.type = MacroTargetType::REGION_ENTRY;
  candidate.anchor_h_id = 8;
  candidate.entry_from_h_id = 7;
  candidate.entry_to_h_id = 8;
  candidate.region_id = 3;
  candidate.expected_gain = 42.0;

  const CommittedTarget committed = manager.CommitAfterPublish(
      candidate, 0, 0, 12.0, 0.0);
  EXPECT_TRUE(committed.valid);
  EXPECT_EQ(MacroTargetType::REGION_ENTRY, committed.type);
  EXPECT_EQ(7U, committed.entry_from_h_id);
  EXPECT_EQ(8U, committed.entry_to_h_id);
  EXPECT_EQ(3, committed.region_id);
  EXPECT_DOUBLE_EQ(42.0, committed.expected_gain);
  EXPECT_DOUBLE_EQ(12.0, committed.accepted_time);
}

}  // namespace
}  // namespace DTGPlus
