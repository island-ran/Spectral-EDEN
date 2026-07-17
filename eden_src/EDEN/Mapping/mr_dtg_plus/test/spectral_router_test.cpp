#include <gtest/gtest.h>

#include <mr_dtg_plus/spectral_router.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <unordered_set>
#include <vector>

namespace DTGPlus {
namespace {

SpectralEdgeInput MakeEdge(
    std::uint32_t from, std::uint32_t to, double length,
    double clearance = std::numeric_limits<double>::quiet_NaN(),
    double betweenness = 0.0) {
  SpectralEdgeInput edge(from, to, length);
  edge.clearance = clearance;
  edge.betweenness = betweenness;
  return edge;
}

SpectralGraphSnapshot MakeChain(const std::vector<std::uint32_t>& ids) {
  SpectralGraphSnapshot snapshot;
  for (const std::uint32_t id : ids) {
    snapshot.nodes.emplace_back(id);
  }
  for (std::size_t index = 1; index < ids.size(); ++index) {
    snapshot.edges.emplace_back(ids[index - 1U], ids[index], 1.0);
  }
  return snapshot;
}

SpectralGraphSnapshot MakeTwoCliquesWithWeakBridge() {
  SpectralGraphSnapshot snapshot;
  for (std::uint32_t id = 1U; id <= 8U; ++id) {
    snapshot.nodes.emplace_back(id);
  }
  for (std::uint32_t first = 1U; first <= 4U; ++first) {
    for (std::uint32_t second = first + 1U; second <= 4U; ++second) {
      snapshot.edges.emplace_back(first, second, 1.0);
    }
  }
  for (std::uint32_t first = 5U; first <= 8U; ++first) {
    for (std::uint32_t second = first + 1U; second <= 8U; ++second) {
      snapshot.edges.emplace_back(first, second, 1.0);
    }
  }
  snapshot.edges.emplace_back(4U, 5U, 8.0);
  return snapshot;
}

SpectralGraphSnapshot MakeRing() {
  SpectralGraphSnapshot snapshot;
  constexpr std::uint32_t kNodeCount = 8U;
  for (std::uint32_t id = 1U; id <= kNodeCount; ++id) {
    snapshot.nodes.emplace_back(id);
  }
  // The small deterministic length variation removes the repeated Fiedler
  // eigenvalue of a perfectly uniform cycle without changing its ring
  // topology or introducing a weak bridge.
  for (std::uint32_t id = 1U; id <= kNodeCount; ++id) {
    const std::uint32_t next = id == kNodeCount ? 1U : id + 1U;
    snapshot.edges.emplace_back(id, next, 1.0 + 0.01 * id);
  }
  return snapshot;
}

TEST(SpectralRouterTest, ChainHasExpectedSpectrumOrderAndFiniteResidual) {
  const SpectralGraphSnapshot chain = MakeChain({10U, 20U, 30U, 40U});

  const SpectralResult result = SpectralRouter().Solve(chain);

  ASSERT_TRUE(result.success()) << result.diagnostics.reason;
  ASSERT_EQ(result.h_ids.size(), 4U);
  EXPECT_NEAR(result.diagnostics.lambda1, 0.0, 1.0e-12);
  EXPECT_NEAR(result.diagnostics.lambda2, 0.5, 1.0e-12);
  EXPECT_NEAR(result.diagnostics.lambda3, 1.5, 1.0e-12);
  EXPECT_TRUE(std::isfinite(result.diagnostics.residual_norm));
  EXPECT_LT(result.diagnostics.residual_norm, 1.0e-10);
  EXPECT_TRUE(result.fiedler.allFinite());
  EXPECT_EQ(result.ordered_h_ids,
            (std::vector<std::uint32_t>{10U, 20U, 30U, 40U}));
}

TEST(SpectralRouterTest, WeakBridgeSweepSeparatesCliquesWithLowerNcutThanRing) {
  SpectralConfig config;
  config.min_cluster_size = 3U;
  config.min_cluster_volume_fraction = 0.2;
  const SpectralRouter router(config);

  const SpectralResult bridged = router.Solve(MakeTwoCliquesWithWeakBridge());
  ASSERT_TRUE(bridged.has_valid_cut()) << bridged.diagnostics.reason;
  ASSERT_EQ(bridged.labels.size(), 8U);
  for (std::size_t index = 1U; index < 4U; ++index) {
    EXPECT_EQ(bridged.labels[index], bridged.labels[0]);
  }
  for (std::size_t index = 5U; index < 8U; ++index) {
    EXPECT_EQ(bridged.labels[index], bridged.labels[4]);
  }
  EXPECT_NE(bridged.labels[0], bridged.labels[4]);
  EXPECT_EQ(bridged.diagnostics.cut_left_size, 4U);
  EXPECT_EQ(bridged.diagnostics.cut_right_size, 4U);

  const SpectralResult ring = router.Solve(MakeRing());
  ASSERT_TRUE(ring.has_valid_cut()) << ring.diagnostics.reason;
  EXPECT_LT(bridged.diagnostics.ncut, ring.diagnostics.ncut);
}

TEST(SpectralRouterTest, HistoryAlignsSignByCommonNodeIdAcrossInputReordering) {
  const SpectralRouter router;
  const std::vector<std::uint32_t> path_ids{11U, 22U, 33U, 44U, 55U};
  const SpectralResult first = router.Solve(MakeChain(path_ids));
  ASSERT_TRUE(first.success()) << first.diagnostics.reason;

  SpectralGraphSnapshot reordered;
  for (const std::uint32_t id : {33U, 55U, 11U, 44U, 22U}) {
    reordered.nodes.emplace_back(id);
  }
  for (std::size_t index = 1U; index < path_ids.size(); ++index) {
    reordered.edges.emplace_back(path_ids[index - 1U], path_ids[index], 1.0);
  }

  FiedlerHistory history = first.fiedler_by_h_id;
  history.emplace(999U, -123.0);  // Not present in the new graph.
  const SpectralResult aligned = router.Solve(reordered, history);
  ASSERT_TRUE(aligned.success()) << aligned.diagnostics.reason;
  EXPECT_TRUE(aligned.diagnostics.history_alignment_used);
  EXPECT_EQ(aligned.diagnostics.history_overlap_count, path_ids.size());
  for (const std::uint32_t id : path_ids) {
    ASSERT_NE(aligned.fiedler_by_h_id.find(id),
              aligned.fiedler_by_h_id.end());
    EXPECT_NEAR(aligned.fiedler_by_h_id.at(id),
                first.fiedler_by_h_id.at(id), 1.0e-10)
        << "H-node ID " << id << " was not aligned through the ID mapping";
  }

  // A deliberately reversed history must reverse the output orientation,
  // demonstrating that temporal alignment takes precedence over a fresh
  // eigensolver/canonical sign choice.
  FiedlerHistory reversed_history;
  for (const std::uint32_t id : path_ids) {
    reversed_history.emplace(id, -first.fiedler_by_h_id.at(id));
  }
  const SpectralResult reversed = router.Solve(reordered, reversed_history);
  ASSERT_TRUE(reversed.success()) << reversed.diagnostics.reason;
  EXPECT_TRUE(reversed.diagnostics.history_alignment_used);
  for (const std::uint32_t id : path_ids) {
    EXPECT_NEAR(reversed.fiedler_by_h_id.at(id),
                -first.fiedler_by_h_id.at(id), 1.0e-10);
  }
}

TEST(SpectralRouterTest, ReversedDuplicateUndirectedEdgeIsRejected) {
  SpectralGraphSnapshot snapshot = MakeChain({1U, 2U, 3U});
  snapshot.edges.emplace_back(2U, 1U, 1.0);

  const SpectralResult result = SpectralRouter().Solve(snapshot);

  EXPECT_EQ(result.status, SpectralSolveStatus::DUPLICATE_EDGE);
  EXPECT_EQ(result.diagnostics.duplicate_edge_count, 1U);
}

TEST(SpectralRouterTest, StructurallyIsolatedNodeIsRejected) {
  SpectralGraphSnapshot snapshot;
  snapshot.nodes.emplace_back(1U);
  snapshot.nodes.emplace_back(2U);
  snapshot.nodes.emplace_back(3U);
  snapshot.edges.emplace_back(1U, 2U, 1.0);

  const SpectralResult result = SpectralRouter().Solve(snapshot);

  EXPECT_EQ(result.status, SpectralSolveStatus::ISOLATED_NODE);
  EXPECT_EQ(result.diagnostics.isolated_h_ids,
            (std::vector<std::uint32_t>{3U}));
}

TEST(SpectralRouterTest, DisconnectedGraphWithoutIsolatesIsRejected) {
  SpectralGraphSnapshot snapshot;
  for (std::uint32_t id = 1U; id <= 4U; ++id) {
    snapshot.nodes.emplace_back(id);
  }
  snapshot.edges.emplace_back(1U, 2U, 1.0);
  snapshot.edges.emplace_back(3U, 4U, 1.0);

  const SpectralResult result = SpectralRouter().Solve(snapshot);

  EXPECT_EQ(result.status, SpectralSolveStatus::DISCONNECTED_GRAPH);
  EXPECT_EQ(result.diagnostics.component_count, 2U);
  EXPECT_TRUE(result.diagnostics.isolated_h_ids.empty());
}

TEST(SpectralRouterTest, ClearanceModeAssignsLowerWeightToNarrowEdge) {
  SpectralConfig config;
  config.weight_mode = SpectralWeightMode::DISTANCE_CLEARANCE;
  config.clearance_reference = 1.0;
  config.clearance_power = 1.0;
  config.min_edge_weight = 1.0e-12;

  SpectralGraphSnapshot snapshot;
  for (std::uint32_t id = 1U; id <= 4U; ++id) {
    snapshot.nodes.emplace_back(id);
  }
  snapshot.edges.push_back(MakeEdge(1U, 2U, 1.0, 1.0));
  snapshot.edges.push_back(MakeEdge(2U, 3U, 1.0, 0.2));
  snapshot.edges.push_back(MakeEdge(3U, 4U, 1.0, 1.0));

  const SpectralResult result = SpectralRouter(config).Solve(snapshot);

  ASSERT_TRUE(result.success()) << result.diagnostics.reason;
  ASSERT_EQ(result.edge_weights.size(), 3U);
  EXPECT_LT(result.edge_weights[1], result.edge_weights[0]);
  EXPECT_LT(result.edge_weights[1], result.edge_weights[2]);
  EXPECT_NEAR(result.edge_weights[1] / result.edge_weights[0], 0.2,
              1.0e-12);
}

TEST(SpectralRouterTest, MinimumWeightClampsOnlyExplicitEdges) {
  SpectralConfig config;
  config.length_decay = 1000.0;
  config.min_edge_weight = 0.125;
  const SpectralGraphSnapshot chain = MakeChain({1U, 2U, 3U, 4U});

  const SpectralResult result = SpectralRouter(config).Solve(chain);

  ASSERT_TRUE(result.success()) << result.diagnostics.reason;
  ASSERT_EQ(result.edge_weights.size(), 3U);
  for (const double weight : result.edge_weights) {
    EXPECT_DOUBLE_EQ(weight, config.min_edge_weight);
  }
  EXPECT_DOUBLE_EQ(result.weight_matrix(0, 2), 0.0);
  EXPECT_DOUBLE_EQ(result.weight_matrix(0, 3), 0.0);
  EXPECT_DOUBLE_EQ(result.weight_matrix(1, 3), 0.0);
  EXPECT_DOUBLE_EQ(result.weight_matrix(0, 0), 0.0);
}

TEST(SpectralRouterTest, ActiveAnchorProjectionPreservesSpectralOrder) {
  SpectralGraphSnapshot snapshot;
  for (std::uint32_t id = 1U; id <= 5U; ++id) {
    snapshot.nodes.emplace_back(id, (id & 1U) != 0U);
    if (id > 1U) {
      snapshot.edges.emplace_back(id - 1U, id, 1.0);
    }
  }

  const SpectralResult result = SpectralRouter().Solve(snapshot);

  ASSERT_TRUE(result.success()) << result.diagnostics.reason;
  EXPECT_EQ(result.diagnostics.active_anchor_count, 3U);
  EXPECT_EQ(result.ordered_active_h_ids,
            (std::vector<std::uint32_t>{1U, 3U, 5U}));

  const std::unordered_set<std::uint32_t> active_ids{1U, 3U, 5U};
  std::vector<std::uint32_t> expected_projection;
  for (const std::uint32_t id : result.ordered_h_ids) {
    if (active_ids.count(id) != 0U) {
      expected_projection.push_back(id);
    }
  }
  EXPECT_EQ(result.ordered_active_h_ids, expected_projection);
}

}  // namespace
}  // namespace DTGPlus
