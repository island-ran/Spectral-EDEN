#include <gtest/gtest.h>

#include <mr_dtg_plus/spectral_snapshot_builder.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace DTGPlus {
namespace {

RawSpectralSnapshot MakeRawChain(std::uint32_t node_count) {
  RawSpectralSnapshot raw;
  raw.graph_version = 17U;
  raw.frontier_version = 23U;
  for (std::uint32_t id = 1U; id <= node_count; ++id) {
    raw.nodes.emplace_back(id);
    if (id > 1U) {
      raw.edges.emplace_back(id - 1U, id, 1.0);
    }
  }
  return raw;
}

bool HasUndirectedEdge(const SpectralGraphSnapshot& snapshot,
                       std::uint32_t first, std::uint32_t second) {
  for (const SpectralEdgeInput& edge : snapshot.edges) {
    if ((edge.from_h_id == first && edge.to_h_id == second) ||
        (edge.from_h_id == second && edge.to_h_id == first)) {
      return true;
    }
  }
  return false;
}

const SpectralEdgeInput* FindUndirectedEdge(
    const SpectralGraphSnapshot& snapshot, std::uint32_t first,
    std::uint32_t second) {
  for (const SpectralEdgeInput& edge : snapshot.edges) {
    if ((edge.from_h_id == first && edge.to_h_id == second) ||
        (edge.from_h_id == second && edge.to_h_id == first)) {
      return &edge;
    }
  }
  return nullptr;
}

TEST(SpectralSnapshotBuilderTest,
     SparseSupportUsesRealShortestPathUnionWithoutShortcuts) {
  RawSpectralSnapshot raw = MakeRawChain(5U);
  raw.nodes[0].active_anchor = true;
  raw.nodes[2].active_anchor = true;
  raw.nodes[4].active_anchor = true;
  raw.nodes[0].preserve = true;
  raw.nodes[2].preserve = true;
  raw.nodes[4].preserve = true;

  SpectralSnapshotBuildConfig config;
  config.knn = 1U;
  config.corridor_compression = false;
  config.min_spectral_nodes = 3U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  ASSERT_TRUE(output.success()) << output.reason;
  EXPECT_EQ(output.graph_version, raw.graph_version);
  EXPECT_EQ(output.frontier_version, raw.frontier_version);
  EXPECT_EQ(output.snapshot.graph_version, raw.graph_version);
  EXPECT_EQ(output.snapshot.nodes.size(), 5U);
  EXPECT_EQ(output.snapshot.edges.size(), 4U);
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 1U, 2U));
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 2U, 3U));
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 3U, 4U));
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 4U, 5U));
  EXPECT_FALSE(HasUndirectedEdge(output.snapshot, 1U, 3U));
  EXPECT_TRUE(std::isfinite(output.timings.support_graph_ms));
  EXPECT_GE(output.timings.support_graph_ms, 0.0);
}

TEST(SpectralSnapshotBuilderTest,
     ActiveMstReconnectsASeparatedSymmetricKnnUnion) {
  RawSpectralSnapshot raw;
  for (const std::uint32_t id : {1U, 2U, 10U, 11U}) {
    raw.nodes.emplace_back(id, true, true);
  }
  raw.edges.emplace_back(1U, 2U, 1.0);
  raw.edges.emplace_back(2U, 10U, 100.0);
  raw.edges.emplace_back(10U, 11U, 1.0);

  SpectralSnapshotBuildConfig config;
  config.knn = 1U;
  config.corridor_compression = false;
  config.min_spectral_nodes = 4U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  ASSERT_TRUE(output.success()) << output.reason;
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 1U, 2U));
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 2U, 10U));
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 10U, 11U));
}

TEST(SpectralSnapshotBuilderTest,
     RequiredRootBranchIsRetainedInTheInducedSupportGraph) {
  RawSpectralSnapshot raw = MakeRawChain(3U);
  raw.nodes[0].active_anchor = true;
  raw.nodes[2].active_anchor = true;
  raw.nodes.emplace_back(20U, false, false, true);
  raw.nodes.emplace_back(21U, false, true, true);
  raw.edges.emplace_back(2U, 20U, 2.0);
  raw.edges.emplace_back(20U, 21U, 2.0);

  SpectralSnapshotBuildConfig config;
  config.knn = 1U;
  config.corridor_compression = false;
  config.min_spectral_nodes = 3U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  ASSERT_TRUE(output.success()) << output.reason;
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 2U, 20U));
  EXPECT_TRUE(HasUndirectedEdge(output.snapshot, 20U, 21U));
}

TEST(SpectralSnapshotBuilderTest,
     DegreeTwoCompressionPreservesLengthAndActiveEndpoints) {
  RawSpectralSnapshot raw = MakeRawChain(5U);
  raw.nodes.front().active_anchor = true;
  raw.nodes.front().preserve = true;
  raw.nodes.back().active_anchor = true;
  raw.nodes.back().preserve = true;

  SpectralSnapshotBuildConfig config;
  config.knn = 1U;
  config.corridor_compression = true;
  config.min_spectral_nodes = 2U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  ASSERT_TRUE(output.success()) << output.reason;
  ASSERT_EQ(output.snapshot.nodes.size(), 2U);
  ASSERT_EQ(output.snapshot.edges.size(), 1U);
  EXPECT_EQ(output.snapshot.nodes.front().h_id, 1U);
  EXPECT_EQ(output.snapshot.nodes.back().h_id, 5U);
  EXPECT_TRUE(output.snapshot.nodes.front().active_anchor);
  EXPECT_TRUE(output.snapshot.nodes.back().active_anchor);
  EXPECT_DOUBLE_EQ(output.snapshot.edges.front().length, 4.0);
  EXPECT_TRUE(std::isfinite(output.timings.compression_ms));
  EXPECT_GE(output.timings.compression_ms, 0.0);
}

TEST(SpectralSnapshotBuilderTest,
     ClearanceModeFailsOpenWhenSelectedEdgeHasNoImmutableValue) {
  RawSpectralSnapshot raw = MakeRawChain(3U);
  raw.nodes.front().active_anchor = true;
  raw.nodes.back().active_anchor = true;
  raw.edges.front().cached_clearance = 0.8;
  raw.edges.back().cached_clearance =
      std::numeric_limits<double>::quiet_NaN();

  SpectralSnapshotBuildConfig config;
  config.weight_mode = SpectralWeightMode::DISTANCE_CLEARANCE;
  config.corridor_compression = false;
  config.min_spectral_nodes = 3U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  EXPECT_FALSE(output.success());
  EXPECT_EQ(output.status,
            SpectralSnapshotBuildStatus::MISSING_EDGE_CLEARANCE);
}

TEST(SpectralSnapshotBuilderTest,
     ActiveCompleteUsesExactRawGraphDistanceAndPathClearance) {
  RawSpectralSnapshot raw = MakeRawChain(3U);
  raw.nodes.front().active_anchor = true;
  raw.nodes.back().active_anchor = true;
  raw.edges.front().cached_clearance = 0.7;
  raw.edges.back().cached_clearance = 0.3;

  SpectralSnapshotBuildConfig config;
  config.support_mode = SpectralSupportMode::ACTIVE_COMPLETE;
  config.weight_mode = SpectralWeightMode::DISTANCE_CLEARANCE;
  config.corridor_compression = false;
  config.min_spectral_nodes = 2U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  ASSERT_TRUE(output.success()) << output.reason;
  ASSERT_EQ(output.snapshot.nodes.size(), 2U);
  ASSERT_EQ(output.snapshot.edges.size(), 1U);
  EXPECT_DOUBLE_EQ(output.snapshot.edges.front().length, 2.0);
  EXPECT_DOUBLE_EQ(output.snapshot.edges.front().clearance, 0.3);
}

TEST(SpectralSnapshotBuilderTest,
     OptionalWeightedBetweennessIsFiniteAndHighlightsChainCenter) {
  RawSpectralSnapshot raw = MakeRawChain(4U);
  for (RawSpectralNodeInput& node : raw.nodes) {
    node.active_anchor = true;
    node.preserve = true;
  }
  for (RawSpectralEdgeInput& edge : raw.edges) {
    edge.cached_clearance = 1.0;
  }

  SpectralSnapshotBuildConfig config;
  config.knn = 1U;
  config.corridor_compression = false;
  config.weight_mode =
      SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS;
  config.min_spectral_nodes = 4U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  ASSERT_TRUE(output.success()) << output.reason;
  const SpectralEdgeInput* left =
      FindUndirectedEdge(output.snapshot, 1U, 2U);
  const SpectralEdgeInput* middle =
      FindUndirectedEdge(output.snapshot, 2U, 3U);
  const SpectralEdgeInput* right =
      FindUndirectedEdge(output.snapshot, 3U, 4U);
  ASSERT_NE(left, nullptr);
  ASSERT_NE(middle, nullptr);
  ASSERT_NE(right, nullptr);
  EXPECT_TRUE(std::isfinite(left->betweenness));
  EXPECT_TRUE(std::isfinite(middle->betweenness));
  EXPECT_TRUE(std::isfinite(right->betweenness));
  EXPECT_GT(middle->betweenness, left->betweenness);
  EXPECT_GT(middle->betweenness, right->betweenness);
}

TEST(SpectralSnapshotBuilderTest,
     PureWorkerBuildsAndSolvesWithoutLivePlannerState) {
  RawSpectralSnapshot raw = MakeRawChain(5U);
  for (RawSpectralNodeInput& node : raw.nodes) {
    node.active_anchor = true;
    node.preserve = true;
  }

  SpectralSnapshotBuildConfig build_config;
  build_config.knn = 2U;
  build_config.corridor_compression = false;
  build_config.min_spectral_nodes = 3U;

  SpectralConfig solver_config;
  solver_config.min_spectral_nodes = 3U;
  solver_config.min_cluster_size = 1U;
  const SpectralWorkerOutput output = BuildAndSolveSpectral(
      raw, build_config, SpectralRouter(solver_config));

  ASSERT_TRUE(output.build.success()) << output.build.reason;
  ASSERT_TRUE(output.result.success())
      << output.result.diagnostics.reason;
  EXPECT_TRUE(std::isfinite(output.timings.solver_ms));
  EXPECT_GE(output.timings.solver_ms, 0.0);
  EXPECT_TRUE(std::isfinite(output.timings.worker_total_ms));
  EXPECT_GE(output.timings.worker_total_ms,
            output.timings.solver_ms);
  EXPECT_TRUE(std::isfinite(output.timings.ncut_ms));
  EXPECT_GE(output.timings.ncut_ms, 0.0);
}

TEST(SpectralSnapshotBuilderTest,
     DisconnectedActiveAnchorsAreRejectedBeforeTheSolver) {
  RawSpectralSnapshot raw;
  raw.nodes.emplace_back(1U, true);
  raw.nodes.emplace_back(2U, false);
  raw.nodes.emplace_back(3U, true);
  raw.nodes.emplace_back(4U, false);
  raw.edges.emplace_back(1U, 2U, 1.0);
  raw.edges.emplace_back(3U, 4U, 1.0);

  SpectralSnapshotBuildConfig config;
  config.corridor_compression = false;
  config.min_spectral_nodes = 2U;

  const SpectralSnapshotBuildOutput output =
      BuildSpectralGraphSnapshot(raw, config);

  EXPECT_FALSE(output.success());
  EXPECT_EQ(
      output.status,
      SpectralSnapshotBuildStatus::ACTIVE_ANCHORS_DISCONNECTED);
}

TEST(SpectralSnapshotBuilderTest,
     WorkerRejectsBuilderAndRouterWeightModeMismatch) {
  RawSpectralSnapshot raw = MakeRawChain(3U);
  raw.nodes.front().active_anchor = true;
  raw.nodes.back().active_anchor = true;

  SpectralSnapshotBuildConfig build_config;
  build_config.weight_mode =
      SpectralWeightMode::DISTANCE_CLEARANCE;
  build_config.corridor_compression = false;
  build_config.min_spectral_nodes = 3U;

  const SpectralWorkerOutput output = BuildAndSolveSpectral(
      raw, build_config, SpectralRouter());

  EXPECT_FALSE(output.success());
  EXPECT_EQ(output.build.status,
            SpectralSnapshotBuildStatus::INVALID_CONFIG);
}

}  // namespace
}  // namespace DTGPlus
