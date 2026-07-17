#include <gtest/gtest.h>

#include <mr_dtg_plus/spectral_snapshot_builder.h>

#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace DTGPlus {
namespace {

TEST(SpectralSnapshotCompressionTest, DegreeTwoChainPreservesLengthAndMinimumClearance) {
  SpectralGraphSnapshot input;
  input.graph_version = 42U;
  for (std::uint32_t id = 1U; id <= 4U; ++id) {
    input.nodes.emplace_back(id, false);
  }
  input.edges.emplace_back(1U, 2U, 1.0, 0.8, 0.0);
  input.edges.emplace_back(2U, 3U, 2.0, 0.4, 0.0);
  input.edges.emplace_back(3U, 4U, 3.0, 0.6, 0.0);

  SpectralGraphSnapshot output;
  std::string reason;
  ASSERT_TRUE(CompressSpectralDegreeTwoChains(
      input, std::unordered_set<std::uint32_t>{1U, 4U}, &output, &reason))
      << reason;
  ASSERT_EQ(output.graph_version, 42U);
  ASSERT_EQ(output.nodes.size(), 2U);
  ASSERT_EQ(output.edges.size(), 1U);
  EXPECT_DOUBLE_EQ(output.edges.front().length, 6.0);
  EXPECT_DOUBLE_EQ(output.edges.front().clearance, 0.4);
}

TEST(SpectralSnapshotCompressionTest, ActiveAnchorSplitsCorridorAndIsNeverRemoved) {
  SpectralGraphSnapshot input;
  input.nodes.emplace_back(1U, false);
  input.nodes.emplace_back(2U, true);
  input.nodes.emplace_back(3U, false);
  input.edges.emplace_back(1U, 2U, 1.0, 1.0, 0.0);
  input.edges.emplace_back(2U, 3U, 1.0, 0.5, 0.0);

  SpectralGraphSnapshot output;
  std::string reason;
  ASSERT_TRUE(CompressSpectralDegreeTwoChains(
      input, std::unordered_set<std::uint32_t>{1U, 3U}, &output, &reason))
      << reason;
  ASSERT_EQ(output.nodes.size(), 3U);
  EXPECT_TRUE(output.nodes[1].active_anchor);
  EXPECT_EQ(output.edges.size(), 2U);
}

TEST(SpectralSnapshotCompressionTest, PureRingProducesNoSelfLoopOrDuplicatePair) {
  SpectralGraphSnapshot input;
  for (std::uint32_t id = 1U; id <= 4U; ++id) {
    input.nodes.emplace_back(id, false);
  }
  input.edges.emplace_back(1U, 2U, 1.0, 1.0, 0.0);
  input.edges.emplace_back(2U, 3U, 1.0, 1.0, 0.0);
  input.edges.emplace_back(3U, 4U, 1.0, 1.0, 0.0);
  input.edges.emplace_back(4U, 1U, 1.0, 1.0, 0.0);

  SpectralGraphSnapshot output;
  std::string reason;
  ASSERT_TRUE(CompressSpectralDegreeTwoChains(
      input, std::unordered_set<std::uint32_t>(), &output, &reason))
      << reason;
  ASSERT_EQ(output.nodes.size(), 3U);
  ASSERT_EQ(output.edges.size(), 3U);
  std::unordered_set<std::uint64_t> pairs;
  for (const SpectralEdgeInput& edge : output.edges) {
    EXPECT_NE(edge.from_h_id, edge.to_h_id);
    EXPECT_TRUE(std::isfinite(edge.length));
    EXPECT_GT(edge.length, 0.0);
    const std::uint32_t low = std::min(edge.from_h_id, edge.to_h_id);
    const std::uint32_t high = std::max(edge.from_h_id, edge.to_h_id);
    const std::uint64_t key =
        (static_cast<std::uint64_t>(low) << 32U) | high;
    EXPECT_TRUE(pairs.insert(key).second);
  }
}

}  // namespace
}  // namespace DTGPlus
