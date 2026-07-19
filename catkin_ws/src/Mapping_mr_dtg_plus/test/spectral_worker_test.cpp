#include <gtest/gtest.h>

#include <mr_dtg_plus/spectral_worker.h>

#include <algorithm>
#include <chrono>
#include <thread>

namespace DTGPlus {
namespace {

TEST(SpectralWorkerTest, AppliesEveryRelativeDeltaBeforeLatestSolve) {
  SpectralV4Config config;
  config.log_diagnostics = false;
  config.proposal_confirm_versions = 1;
  config.max_ncut = 2.0;
  SpectralWorker worker(config);

  auto first = std::make_shared<GraphDelta>();
  first->graph_version = 1;
  first->frontier_version = 1;
  for (uint32_t id = 1; id <= 4; ++id) {
    NodeInsert node;
    node.h_id = id;
    node.pos = Eigen::Vector3d(id, 0.0, 0.0);
    first->node_inserts.push_back(node);
  }
  for (uint32_t id = 1; id < 4; ++id) {
    EdgeUpsert edge;
    edge.from = id;
    edge.to = id + 1;
    edge.length = 1.0;
    edge.clearance = 1.0;
    first->edge_upserts.push_back(edge);
  }
  FrontierAnchorUpdate anchor_one;
  anchor_one.h_id = 1;
  anchor_one.active = true;
  anchor_one.expected_gain = 10.0;
  anchor_one.frontier_ids.push_back(100);
  first->frontier_updates.push_back(anchor_one);
  first->robot_h_id = 1;
  first->robot_changed = true;

  auto second = std::make_shared<GraphDelta>();
  second->graph_version = 2;
  second->frontier_version = 2;
  NodeInsert node_five;
  node_five.h_id = 5;
  node_five.pos = Eigen::Vector3d(5.0, 0.0, 0.0);
  second->node_inserts.push_back(node_five);
  EdgeUpsert edge;
  edge.from = 4;
  edge.to = 5;
  edge.length = 1.0;
  edge.clearance = 1.0;
  second->edge_upserts.push_back(edge);
  FrontierAnchorUpdate anchor_five;
  anchor_five.h_id = 5;
  anchor_five.active = true;
  anchor_five.expected_gain = 20.0;
  anchor_five.frontier_ids.push_back(200);
  second->frontier_updates.push_back(anchor_five);

  // Submit before Start so both relative deltas are certainly in the same
  // mailbox batch. Dropping the first one would leave an invalid partial graph.
  worker.Submit(first);
  worker.Submit(second);
  worker.Start();

  std::shared_ptr<const RegionStateSnapshot> snapshot;
  for (int i = 0; i < 100 && snapshot == nullptr; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    snapshot = worker.TryConsume();
  }
  worker.Stop();
  ASSERT_NE(nullptr, snapshot);
  ASSERT_TRUE(snapshot->valid);
  EXPECT_EQ(2U, snapshot->graph_version);
  std::vector<uint32_t> frontier_ids;
  for (const auto& region : snapshot->quotient_regions)
    frontier_ids.insert(
        frontier_ids.end(),
        region.frontier_ids.begin(), region.frontier_ids.end());
  EXPECT_NE(frontier_ids.end(),
            std::find(frontier_ids.begin(), frontier_ids.end(), 100U));
  EXPECT_NE(frontier_ids.end(),
            std::find(frontier_ids.begin(), frontier_ids.end(), 200U));
}

}  // namespace
}  // namespace DTGPlus
