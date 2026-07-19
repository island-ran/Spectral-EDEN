#include <gtest/gtest.h>

#include <mr_dtg_plus/spectral_v4_backends.h>

#include <algorithm>
#include <unordered_set>

namespace DTGPlus {
namespace {

ContractedSkeleton MakeWeakBridgeGraph(size_t cluster_size) {
  ContractedSkeleton graph;
  graph.graph_version = 7;
  const size_t n = 2U * cluster_size;
  for (size_t i = 0; i < n; ++i) {
    const uint32_t id = static_cast<uint32_t>(i + 1U);
    graph.node_ids.push_back(id);
    graph.node_positions[id] =
        Eigen::Vector3d(static_cast<double>(i), 0.0, 0.0);
    graph.node_is_active_anchor[id] =
        i == 0U || i + 1U == n;
    graph.node_frontier_gain[id] =
        graph.node_is_active_anchor[id] ? 20.0 : 0.0;
  }
  auto add = [&](uint32_t a, uint32_t b, double length) {
    SkeletonEdge edge;
    edge.from = a;
    edge.to = b;
    edge.length = length;
    edge.min_clearance = 1.0;
    edge.raw_edge_count = 1;
    edge.version = graph.graph_version;
    graph.edges.push_back(edge);
  };
  for (size_t base : std::vector<size_t>{0U, cluster_size}) {
    for (size_t i = 0; i < cluster_size; ++i)
      for (size_t j = i + 1U; j < cluster_size; ++j)
        add(static_cast<uint32_t>(base + i + 1U),
            static_cast<uint32_t>(base + j + 1U), 1.0);
  }
  add(static_cast<uint32_t>(cluster_size),
      static_cast<uint32_t>(cluster_size + 1U), 8.0);
  return graph;
}

void ExpectCompleteBinaryProposal(
    const ContractedSkeleton& graph,
    const BinaryRegionProposal& proposal) {
  EXPECT_TRUE(proposal.valid);
  EXPECT_FALSE(proposal.side_a.empty());
  EXPECT_FALSE(proposal.side_b.empty());
  std::unordered_set<uint32_t> nodes(
      proposal.side_a.begin(), proposal.side_a.end());
  nodes.insert(proposal.side_b.begin(), proposal.side_b.end());
  EXPECT_EQ(graph.node_ids.size(), nodes.size());
}

TEST(SpectralV4BackendsTest, IncrementalFiedlerProducesUnifiedProposal) {
  const ContractedSkeleton graph = MakeWeakBridgeGraph(4);
  SpectralV4Config config;
  config.exact_max_nodes = 16;
  config.exact_residual_threshold = 1.0e-3;
  config.max_ncut = 1.0;
  config.min_balance = 0.1;
  V4BackendContext context;
  context.parent_region_id = 2;
  context.frontier_version = 9;
  BinaryRegionProposal proposal;
  FiedlerHistory next;
  std::string reason;

  ASSERT_TRUE(SolveIncrementalFiedler(
      graph, config, context, FiedlerHistory(),
      &proposal, &next, &reason)) << reason;
  EXPECT_EQ(CutBackend::INCREMENTAL_FIEDLER, proposal.backend);
  EXPECT_EQ(2, proposal.parent_region_id);
  EXPECT_EQ(9U, proposal.frontier_version);
  ExpectCompleteBinaryProposal(graph, proposal);
}

TEST(SpectralV4BackendsTest, NystromUsesPersistentTopologicalLandmarks) {
  const ContractedSkeleton graph = MakeWeakBridgeGraph(8);
  SpectralV4Config config;
  config.landmark_count = 6;
  config.landmark_max_count = 6;
  config.nystrom_max_support = 32;
  config.nystrom_residual_threshold = 10.0;
  config.max_ncut = 2.0;
  config.min_balance = 0.05;
  V4BackendContext context;
  context.parent_region_id = 0;
  context.robot_h_id = 1;
  std::vector<uint32_t> landmarks;
  BinaryRegionProposal first;
  std::string reason;

  ASSERT_TRUE(SolveNystromBisection(
      graph, config, context, &landmarks, &first, &reason)) << reason;
  EXPECT_EQ(6U, landmarks.size());
  const std::vector<uint32_t> stable_landmarks = landmarks;
  BinaryRegionProposal second;
  ASSERT_TRUE(SolveNystromBisection(
      graph, config, context, &landmarks, &second, &reason)) << reason;
  EXPECT_EQ(stable_landmarks, landmarks);
  EXPECT_EQ(first.deterministic_signature,
            second.deterministic_signature);
  EXPECT_EQ(CutBackend::LANDMARK_NYSTROM, first.backend);
  ExpectCompleteBinaryProposal(graph, first);
}

TEST(SpectralV4BackendsTest, LocalApprRunsResidualPushFromRobotSeed) {
  const ContractedSkeleton graph = MakeWeakBridgeGraph(6);
  SpectralV4Config config;
  config.ppr_max_pushes = 20000;
  config.ppr_max_support_nodes = 64;
  config.max_ncut = 2.0;
  config.min_balance = 0.05;
  V4BackendContext context;
  context.parent_region_id = 0;
  context.robot_h_id = 1;
  BinaryRegionProposal proposal;
  std::string reason;

  ASSERT_TRUE(SolveLocalAppr(
      graph, config, context, &proposal, &reason)) << reason;
  EXPECT_EQ(CutBackend::LOCAL_APPR, proposal.backend);
  EXPECT_GT(proposal.support_coverage, 0.0);
  EXPECT_LE(proposal.support_coverage, 1.0);
  ExpectCompleteBinaryProposal(graph, proposal);
}

}  // namespace
}  // namespace DTGPlus
