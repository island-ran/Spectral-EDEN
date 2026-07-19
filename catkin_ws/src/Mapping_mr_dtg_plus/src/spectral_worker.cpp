#include <mr_dtg_plus/spectral_worker.h>
#include <mr_dtg_plus/spectral_v4_backends.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace DTGPlus {
namespace {

uint64_t EdgeKey(uint32_t a, uint32_t b) {
  const uint32_t low = std::min(a, b);
  const uint32_t high = std::max(a, b);
  return (static_cast<uint64_t>(low) << 32U) | high;
}

struct EdgeData {
  double length = 0.0;
  double clearance = 0.0;
};

struct AnchorData {
  bool active = false;
  double gain = 0.0;
  std::vector<uint32_t> frontier_ids;
};

struct SparseGraph {
  std::unordered_map<uint32_t, Eigen::Vector3d> positions;
  std::unordered_map<uint64_t, EdgeData> edges;
  std::unordered_map<uint32_t, AnchorData> anchors;
  uint32_t robot_h_id = 0;
  uint64_t graph_version = 0;
  uint64_t frontier_version = 0;
};

void ApplyDelta(const GraphDelta& delta, SparseGraph* graph) {
  for (const auto& edge : delta.edge_erases)
    graph->edges.erase(EdgeKey(edge.from, edge.to));

  for (const auto& node : delta.node_erases) {
    graph->positions.erase(node.h_id);
    graph->anchors.erase(node.h_id);
    for (auto it = graph->edges.begin(); it != graph->edges.end();) {
      const uint32_t a = static_cast<uint32_t>(it->first >> 32U);
      const uint32_t b = static_cast<uint32_t>(it->first & 0xFFFFFFFFU);
      if (a == node.h_id || b == node.h_id) it = graph->edges.erase(it);
      else ++it;
    }
  }

  for (const auto& node : delta.node_inserts)
    graph->positions[node.h_id] = node.pos;
  for (const auto& edge : delta.edge_upserts) {
    if (edge.from == edge.to || !std::isfinite(edge.length) ||
        edge.length <= 0.0)
      continue;
    EdgeData data;
    data.length = edge.length;
    data.clearance = std::isfinite(edge.clearance)
        ? std::max(0.0, edge.clearance) : 0.0;
    graph->edges[EdgeKey(edge.from, edge.to)] = data;
  }
  for (const auto& update : delta.frontier_updates) {
    AnchorData data;
    data.active = update.active;
    data.gain = std::max(0.0, update.expected_gain);
    data.frontier_ids = update.frontier_ids;
    std::sort(data.frontier_ids.begin(), data.frontier_ids.end());
    data.frontier_ids.erase(
        std::unique(data.frontier_ids.begin(), data.frontier_ids.end()),
        data.frontier_ids.end());
    graph->anchors[update.h_id] = std::move(data);
  }
  if (delta.robot_changed) graph->robot_h_id = delta.robot_h_id;
  graph->graph_version = std::max(graph->graph_version, delta.graph_version);
  graph->frontier_version =
      std::max(graph->frontier_version, delta.frontier_version);
}

bool RequiresRecontraction(
    const GraphDelta& delta, const SparseGraph& graph) {
  if (!delta.node_inserts.empty() || !delta.node_erases.empty() ||
      !delta.edge_upserts.empty() || !delta.edge_erases.empty() ||
      delta.robot_changed)
    return true;
  for (const auto& update : delta.frontier_updates) {
    const auto old = graph.anchors.find(update.h_id);
    const bool old_active =
        old != graph.anchors.end() && old->second.active;
    if (old_active != update.active) return true;
  }
  return false;
}

void RefreshSkeletonAttributes(
    const SparseGraph& graph, ContractedSkeleton* skeleton) {
  skeleton->graph_version = graph.graph_version;
  for (uint32_t id : skeleton->node_ids) {
    const auto position = graph.positions.find(id);
    if (position != graph.positions.end())
      skeleton->node_positions[id] = position->second;
    const auto anchor = graph.anchors.find(id);
    const bool active =
        anchor != graph.anchors.end() && anchor->second.active;
    skeleton->node_is_active_anchor[id] = active;
    skeleton->node_frontier_gain[id] =
        active ? anchor->second.gain : 0.0;
    if (active)
      skeleton->node_frontier_ids[id] = anchor->second.frontier_ids;
    else
      skeleton->node_frontier_ids.erase(id);
  }
}

ContractedSkeleton ContractChains(
    const SparseGraph& graph, const SpectralV4Config& config) {
  ContractedSkeleton skeleton;
  skeleton.graph_version = graph.graph_version;

  using Incident = std::pair<uint32_t, uint64_t>;
  std::unordered_map<uint32_t, std::vector<Incident>> adjacency;
  for (const auto& node : graph.positions) adjacency[node.first];
  for (const auto& edge : graph.edges) {
    const uint32_t a = static_cast<uint32_t>(edge.first >> 32U);
    const uint32_t b = static_cast<uint32_t>(edge.first & 0xFFFFFFFFU);
    if (graph.positions.count(a) == 0U ||
        graph.positions.count(b) == 0U)
      continue;
    adjacency[a].emplace_back(b, edge.first);
    adjacency[b].emplace_back(a, edge.first);
  }

  // Region planning is meaningful only on the robot-reachable DTG component.
  // If the projection is temporarily unavailable, deterministically retain
  // the largest component until a robot-node delta arrives.
  std::unordered_set<uint32_t> allowed;
  auto collect_component = [&](uint32_t seed) {
    std::unordered_set<uint32_t> component;
    std::queue<uint32_t> queue;
    queue.push(seed);
    component.insert(seed);
    while (!queue.empty()) {
      const uint32_t current = queue.front();
      queue.pop();
      for (const auto& incident : adjacency[current])
        if (component.insert(incident.first).second)
          queue.push(incident.first);
    }
    return component;
  };
  if (graph.positions.count(graph.robot_h_id) != 0U) {
    allowed = collect_component(graph.robot_h_id);
  } else {
    std::unordered_set<uint32_t> assigned;
    uint32_t best_min_id = std::numeric_limits<uint32_t>::max();
    for (const auto& node : graph.positions) {
      if (assigned.count(node.first) != 0U) continue;
      std::unordered_set<uint32_t> component =
          collect_component(node.first);
      assigned.insert(component.begin(), component.end());
      const uint32_t minimum_id = *std::min_element(
          component.begin(), component.end());
      if (component.size() > allowed.size() ||
          (component.size() == allowed.size() &&
           minimum_id < best_min_id)) {
        allowed = std::move(component);
        best_min_id = minimum_id;
      }
    }
  }

  std::unordered_set<uint32_t> preserved;
  for (const auto& node : graph.positions) {
    const uint32_t id = node.first;
    if (allowed.count(id) == 0U) continue;
    const auto anchor = graph.anchors.find(id);
    const bool active = anchor != graph.anchors.end() && anchor->second.active;
    if (adjacency[id].size() != 2U || active || id == graph.robot_h_id)
      preserved.insert(id);
  }
  for (const auto& edge : graph.edges) {
    const uint32_t edge_from =
        static_cast<uint32_t>(edge.first >> 32U);
    const uint32_t edge_to = static_cast<uint32_t>(
        edge.first & 0xFFFFFFFFU);
    if (allowed.count(edge_from) == 0U ||
        allowed.count(edge_to) == 0U)
      continue;
    if (edge.second.clearance <
        0.5 * std::max(config.clearance_reference, 1.0e-6)) {
      preserved.insert(edge_from);
      preserved.insert(edge_to);
    }
  }

  // A pure degree-two cycle otherwise has no endpoint. Preserve the smallest
  // ID and its two neighbours so the loop remains a triangle in the skeleton.
  std::unordered_set<uint32_t> visited_nodes;
  for (const auto& node : graph.positions) {
    if (allowed.count(node.first) == 0U) continue;
    if (visited_nodes.count(node.first) != 0U) continue;
    std::vector<uint32_t> component;
    std::queue<uint32_t> queue;
    queue.push(node.first);
    visited_nodes.insert(node.first);
    bool has_endpoint = false;
    while (!queue.empty()) {
      const uint32_t current = queue.front();
      queue.pop();
      component.push_back(current);
      if (preserved.count(current) != 0U) has_endpoint = true;
      for (const auto& incident : adjacency[current]) {
        if (visited_nodes.insert(incident.first).second)
          queue.push(incident.first);
      }
    }
    if (!has_endpoint && !component.empty()) {
      const uint32_t first =
          *std::min_element(component.begin(), component.end());
      preserved.insert(first);
      for (const auto& incident : adjacency[first])
        preserved.insert(incident.first);
    }
  }

  skeleton.node_ids.assign(preserved.begin(), preserved.end());
  std::sort(skeleton.node_ids.begin(), skeleton.node_ids.end());
  for (uint32_t id : skeleton.node_ids) {
    skeleton.node_positions[id] = graph.positions.at(id);
    const auto anchor = graph.anchors.find(id);
    const bool active = anchor != graph.anchors.end() && anchor->second.active;
    skeleton.node_is_active_anchor[id] = active;
    skeleton.node_frontier_gain[id] =
        active ? anchor->second.gain : 0.0;
    if (active)
      skeleton.node_frontier_ids[id] = anchor->second.frontier_ids;
  }

  std::unordered_set<uint64_t> visited_edges;
  for (uint32_t start : skeleton.node_ids) {
    for (const auto& first : adjacency[start]) {
      if (visited_edges.count(first.second) != 0U) continue;
      visited_edges.insert(first.second);
      uint32_t previous = start;
      uint32_t current = first.first;
      double length = graph.edges.at(first.second).length;
      double clearance = graph.edges.at(first.second).clearance;
      uint32_t raw_count = 1;

      while (preserved.count(current) == 0U) {
        bool advanced = false;
        for (const auto& next : adjacency[current]) {
          if (next.first == previous ||
              visited_edges.count(next.second) != 0U)
            continue;
          visited_edges.insert(next.second);
          length += graph.edges.at(next.second).length;
          clearance = std::min(
              clearance, graph.edges.at(next.second).clearance);
          ++raw_count;
          previous = current;
          current = next.first;
          advanced = true;
          break;
        }
        if (!advanced) break;
      }
      if (preserved.count(current) == 0U || current == start) continue;
      SkeletonEdge edge;
      edge.from = start;
      edge.to = current;
      edge.length = length;
      edge.min_clearance = clearance;
      edge.raw_edge_count = raw_count;
      edge.version = graph.graph_version;
      skeleton.edges.push_back(edge);
    }
  }
  std::sort(skeleton.edges.begin(), skeleton.edges.end(),
      [](const SkeletonEdge& lhs, const SkeletonEdge& rhs) {
        const auto lhs_pair = std::minmax(lhs.from, lhs.to);
        const auto rhs_pair = std::minmax(rhs.from, rhs.to);
        if (lhs_pair.first != rhs_pair.first)
          return lhs_pair.first < rhs_pair.first;
        if (lhs_pair.second != rhs_pair.second)
          return lhs_pair.second < rhs_pair.second;
        return lhs.length < rhs.length;
      });
  return skeleton;
}

ContractedSkeleton InducedSkeleton(
    const ContractedSkeleton& source,
    const std::unordered_set<uint32_t>& nodes) {
  ContractedSkeleton output;
  output.graph_version = source.graph_version;
  for (uint32_t id : source.node_ids) {
    if (nodes.count(id) == 0U) continue;
    output.node_ids.push_back(id);
    output.node_positions[id] = source.node_positions.at(id);
    output.node_is_active_anchor[id] =
        source.node_is_active_anchor.at(id);
    const auto gain = source.node_frontier_gain.find(id);
    output.node_frontier_gain[id] =
        gain == source.node_frontier_gain.end() ? 0.0 : gain->second;
    const auto frontiers = source.node_frontier_ids.find(id);
    if (frontiers != source.node_frontier_ids.end())
      output.node_frontier_ids[id] = frontiers->second;
  }
  for (const auto& edge : source.edges)
    if (nodes.count(edge.from) != 0U && nodes.count(edge.to) != 0U)
      output.edges.push_back(edge);
  return output;
}

void ResetRegionTree(
    const ContractedSkeleton& skeleton,
    std::vector<RegionTreeNode>* tree) {
  tree->clear();
  RegionTreeNode root;
  root.id = 0;
  root.h_ids.insert(
      skeleton.node_ids.begin(), skeleton.node_ids.end());
  root.accepted_graph_version = skeleton.graph_version;
  tree->push_back(std::move(root));
}

void ReconcileRegionTree(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    std::vector<RegionTreeNode>* tree) {
  const std::unordered_set<uint32_t> live(
      skeleton.node_ids.begin(), skeleton.node_ids.end());
  if (tree->empty()) {
    ResetRegionTree(skeleton, tree);
    return;
  }

  std::unordered_set<uint32_t> previous;
  for (const auto& region : *tree)
    if (region.leaf)
      previous.insert(region.h_ids.begin(), region.h_ids.end());
  size_t removed = 0;
  for (uint32_t id : previous)
    if (live.count(id) == 0U) ++removed;
  if (!previous.empty() &&
      static_cast<double>(removed) /
          static_cast<double>(previous.size()) >
              config.region_invalidation_ratio) {
    ResetRegionTree(skeleton, tree);
    return;
  }

  std::unordered_map<uint32_t, int> current_owner;
  for (auto& region : *tree) {
    for (auto it = region.h_ids.begin(); it != region.h_ids.end();) {
      if (live.count(*it) == 0U) it = region.h_ids.erase(it);
      else ++it;
    }
    if (region.leaf)
      for (uint32_t id : region.h_ids) current_owner[id] = region.id;
  }
  for (const auto& region : *tree) {
    if (region.leaf && region.h_ids.empty()) {
      ResetRegionTree(skeleton, tree);
      return;
    }
  }

  std::unordered_map<uint32_t, std::vector<uint32_t>> adjacency;
  for (const auto& edge : skeleton.edges) {
    adjacency[edge.from].push_back(edge.to);
    adjacency[edge.to].push_back(edge.from);
  }
  for (uint32_t id : skeleton.node_ids) {
    if (current_owner.count(id) != 0U) continue;
    int owner = -1;
    for (uint32_t neighbor : adjacency[id]) {
      const auto found = current_owner.find(neighbor);
      if (found != current_owner.end() &&
          (owner < 0 || found->second < owner))
        owner = found->second;
    }
    if (owner < 0) {
      for (const auto& region : *tree)
        if (region.leaf && (owner < 0 || region.id < owner))
          owner = region.id;
    }
    if (owner >= 0 && static_cast<size_t>(owner) < tree->size()) {
      (*tree)[static_cast<size_t>(owner)].h_ids.insert(id);
      current_owner[id] = owner;
    }
  }

  // A deleted physical edge may split a previously accepted leaf. Such a
  // tree no longer represents connected macro regions and is rebuilt from
  // the live skeleton instead of being silently kept.
  for (const auto& region : *tree) {
    if (!region.leaf || region.h_ids.empty()) continue;
    std::unordered_set<uint32_t> reached;
    std::queue<uint32_t> queue;
    queue.push(*std::min_element(
        region.h_ids.begin(), region.h_ids.end()));
    reached.insert(queue.front());
    while (!queue.empty()) {
      const uint32_t current = queue.front();
      queue.pop();
      for (uint32_t neighbor : adjacency[current]) {
        if (region.h_ids.count(neighbor) != 0U &&
            reached.insert(neighbor).second)
          queue.push(neighbor);
      }
    }
    if (reached.size() != region.h_ids.size()) {
      ResetRegionTree(skeleton, tree);
      return;
    }
  }

  for (auto it = tree->rbegin(); it != tree->rend(); ++it) {
    if (it->leaf) continue;
    it->h_ids.clear();
    if (it->child_a >= 0 &&
        static_cast<size_t>(it->child_a) < tree->size())
      it->h_ids.insert(
          (*tree)[static_cast<size_t>(it->child_a)].h_ids.begin(),
          (*tree)[static_cast<size_t>(it->child_a)].h_ids.end());
    if (it->child_b >= 0 &&
        static_cast<size_t>(it->child_b) < tree->size())
      it->h_ids.insert(
          (*tree)[static_cast<size_t>(it->child_b)].h_ids.begin(),
          (*tree)[static_cast<size_t>(it->child_b)].h_ids.end());
  }
}

double RegionGain(
    const RegionTreeNode& region,
    const ContractedSkeleton& skeleton) {
  double gain = 0.0;
  for (uint32_t id : region.h_ids) {
    const auto found = skeleton.node_frontier_gain.find(id);
    if (found != skeleton.node_frontier_gain.end())
      gain += std::max(0.0, found->second);
  }
  return gain;
}

double BottleneckStrength(
    const RegionTreeNode& region,
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config) {
  double minimum = std::numeric_limits<double>::infinity();
  for (const auto& edge : skeleton.edges) {
    if (region.h_ids.count(edge.from) == 0U ||
        region.h_ids.count(edge.to) == 0U)
      continue;
    minimum = std::min(minimum, edge.min_clearance);
  }
  if (!std::isfinite(minimum))
    return 1.0 / std::max(config.clearance_reference, 1.0e-6);
  return 1.0 / std::max(minimum, 1.0e-6);
}

int LeafCount(const std::vector<RegionTreeNode>& tree) {
  int count = 0;
  for (const auto& region : tree) if (region.leaf) ++count;
  return count;
}

int SelectBestLeaf(
    const std::vector<RegionTreeNode>& tree,
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    uint32_t robot_h_id) {
  if (LeafCount(tree) >= config.max_leaf_regions) return -1;
  int best = -1;
  double best_score = -1.0;
  for (const auto& region : tree) {
    if (!region.leaf || region.depth >= config.max_recursive_depth ||
        region.h_ids.size() < 4U)
      continue;
    const double score =
        RegionGain(region, skeleton) *
        BottleneckStrength(region, skeleton, config);
    if (score > best_score ||
        (score == best_score && (best < 0 || region.id < best))) {
      best = region.id;
      best_score = score;
    }
  }
  if (best_score <= 0.0) {
    for (const auto& region : tree)
      if (region.leaf && region.h_ids.count(robot_h_id) != 0U &&
          region.h_ids.size() >= 4U &&
          region.depth < config.max_recursive_depth)
        return region.id;
  }
  return best;
}

CutBackend SelectBackend(
    size_t node_count,
    bool robot_in_leaf,
    int exact_failure_streak,
    int nystrom_failure_streak,
    const SpectralV4Config& config) {
  if (node_count <= static_cast<size_t>(config.exact_max_nodes) &&
      exact_failure_streak == 0)
    return CutBackend::INCREMENTAL_FIEDLER;
  if (node_count <= static_cast<size_t>(config.nystrom_max_support) &&
      nystrom_failure_streak < 2)
    return CutBackend::LANDMARK_NYSTROM;
  if (robot_in_leaf) return CutBackend::LOCAL_APPR;
  return CutBackend::UNAVAILABLE;
}

double Jaccard(
    const std::vector<uint32_t>& lhs,
    const std::vector<uint32_t>& rhs) {
  if (lhs.empty() && rhs.empty()) return 1.0;
  const std::unordered_set<uint32_t> left(lhs.begin(), lhs.end());
  size_t intersection = 0;
  for (uint32_t id : rhs)
    if (left.count(id) != 0U) ++intersection;
  const size_t union_size = left.size() + rhs.size() - intersection;
  return union_size == 0U ? 0.0 :
      static_cast<double>(intersection) / union_size;
}

bool ProposalCoversParent(
    const BinaryRegionProposal& proposal,
    const RegionTreeNode& parent) {
  std::unordered_set<uint32_t> combined;
  for (uint32_t id : proposal.side_a)
    if (!combined.insert(id).second) return false;
  for (uint32_t id : proposal.side_b)
    if (!combined.insert(id).second) return false;
  return combined == parent.h_ids;
}

bool SideActionable(
    const std::vector<uint32_t>& side,
    const ContractedSkeleton& skeleton) {
  const std::unordered_set<uint32_t> members(side.begin(), side.end());
  for (uint32_t id : side) {
    const auto active = skeleton.node_is_active_anchor.find(id);
    if (active != skeleton.node_is_active_anchor.end() && active->second)
      return true;
  }
  for (const auto& edge : skeleton.edges) {
    const bool from = members.count(edge.from) != 0U;
    const bool to = members.count(edge.to) != 0U;
    if (from != to) return true;
  }
  return false;
}

struct Confirmation {
  BinaryRegionProposal proposal;
  uint64_t last_graph_version = 0;
  uint64_t last_frontier_version = 0;
  int count = 0;
};

bool ConfirmProposal(
    const BinaryRegionProposal& proposal,
    const RegionTreeNode& parent,
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    std::unordered_map<int, Confirmation>* confirmations,
    std::string* reason) {
  if (!proposal.valid || proposal.side_a.empty() ||
      proposal.side_b.empty() || !ProposalCoversParent(proposal, parent)) {
    *reason = "proposal does not form a valid partition of its parent";
    return false;
  }
  if (!std::isfinite(proposal.ncut) || proposal.ncut > config.max_ncut ||
      !std::isfinite(proposal.balance) ||
      proposal.balance < config.min_balance) {
    *reason = "proposal failed Ncut or balance threshold";
    return false;
  }
  if (!SideActionable(proposal.side_a, skeleton) ||
      !SideActionable(proposal.side_b, skeleton)) {
    *reason = "one proposal side has neither frontier nor valid entrance";
    return false;
  }

  Confirmation& state = (*confirmations)[parent.id];
  if (state.count == 0) {
    state.proposal = proposal;
    state.last_graph_version = proposal.graph_version;
    state.last_frontier_version = proposal.frontier_version;
    state.count = 1;
  } else if (proposal.graph_version > state.last_graph_version ||
             (proposal.graph_version == state.last_graph_version &&
              proposal.frontier_version >
                  state.last_frontier_version)) {
    const double direct = Jaccard(
        state.proposal.side_a, proposal.side_a);
    const double flipped = Jaccard(
        state.proposal.side_a, proposal.side_b);
    if (std::max(direct, flipped) >= config.proposal_jaccard_threshold) {
      ++state.count;
      state.proposal = proposal;
    } else {
      state.count = 1;
      state.proposal = proposal;
    }
    state.last_graph_version = proposal.graph_version;
    state.last_frontier_version = proposal.frontier_version;
  }
  if (state.count < std::max(1, config.proposal_confirm_versions)) {
    *reason = "proposal awaiting deterministic version confirmation";
    return false;
  }
  confirmations->erase(parent.id);
  *reason = "proposal accepted";
  return true;
}

void AcceptCut(
    const BinaryRegionProposal& proposal,
    std::vector<RegionTreeNode>* tree) {
  const size_t parent_index =
      static_cast<size_t>(proposal.parent_region_id);
  const int child_a_id = static_cast<int>(tree->size());
  const int child_b_id = child_a_id + 1;
  const int depth = (*tree)[parent_index].depth + 1;

  RegionTreeNode child_a;
  child_a.id = child_a_id;
  child_a.parent_id = proposal.parent_region_id;
  child_a.accepted_graph_version = proposal.graph_version;
  child_a.cut_quality = proposal.ncut;
  child_a.depth = depth;
  child_a.h_ids.insert(proposal.side_a.begin(), proposal.side_a.end());

  RegionTreeNode child_b;
  child_b.id = child_b_id;
  child_b.parent_id = proposal.parent_region_id;
  child_b.accepted_graph_version = proposal.graph_version;
  child_b.cut_quality = proposal.ncut;
  child_b.depth = depth;
  child_b.h_ids.insert(proposal.side_b.begin(), proposal.side_b.end());

  tree->push_back(std::move(child_a));
  tree->push_back(std::move(child_b));
  (*tree)[parent_index].leaf = false;
  (*tree)[parent_index].child_a = child_a_id;
  (*tree)[parent_index].child_b = child_b_id;
}

void BuildQuotientGraph(
    const std::vector<RegionTreeNode>& tree,
    const ContractedSkeleton& skeleton,
    std::vector<QuotientRegion>* regions,
    std::vector<QuotientEdge>* edges,
    std::unordered_map<uint32_t, int>* owner) {
  regions->clear();
  edges->clear();
  owner->clear();
  std::unordered_map<int, size_t> region_index;
  for (const auto& region : tree) {
    if (!region.leaf) continue;
    QuotientRegion quotient;
    quotient.region_id = region.id;
    for (uint32_t id : region.h_ids) {
      (*owner)[id] = region.id;
      const auto gain = skeleton.node_frontier_gain.find(id);
      if (gain != skeleton.node_frontier_gain.end())
        quotient.total_gain += std::max(0.0, gain->second);
      const auto frontiers = skeleton.node_frontier_ids.find(id);
      if (frontiers != skeleton.node_frontier_ids.end())
        quotient.frontier_ids.insert(
            quotient.frontier_ids.end(),
            frontiers->second.begin(), frontiers->second.end());
    }
    std::sort(
        quotient.frontier_ids.begin(), quotient.frontier_ids.end());
    quotient.frontier_ids.erase(std::unique(
        quotient.frontier_ids.begin(), quotient.frontier_ids.end()),
        quotient.frontier_ids.end());
    region_index[region.id] = regions->size();
    regions->push_back(std::move(quotient));
  }

  std::unordered_set<uint64_t> physical_entries;
  for (const auto& skeleton_edge : skeleton.edges) {
    const auto from = owner->find(skeleton_edge.from);
    const auto to = owner->find(skeleton_edge.to);
    if (from == owner->end() || to == owner->end() ||
        from->second == to->second)
      continue;
    const uint64_t physical =
        EdgeKey(skeleton_edge.from, skeleton_edge.to);
    if (!physical_entries.insert(physical).second) continue;
    QuotientEdge edge;
    edge.from_region = from->second;
    edge.to_region = to->second;
    edge.entry_from = skeleton_edge.from;
    edge.entry_to = skeleton_edge.to;
    edge.traversal_time = skeleton_edge.length;
    edges->push_back(edge);
    (*regions)[region_index[from->second]].entry_h_ids.push_back(
        skeleton_edge.from);
    (*regions)[region_index[to->second]].entry_h_ids.push_back(
        skeleton_edge.to);
  }
  for (auto& region : *regions) {
    std::sort(region.entry_h_ids.begin(), region.entry_h_ids.end());
    region.entry_h_ids.erase(std::unique(
        region.entry_h_ids.begin(), region.entry_h_ids.end()),
        region.entry_h_ids.end());
  }
  std::sort(regions->begin(), regions->end(),
      [](const QuotientRegion& lhs, const QuotientRegion& rhs) {
        return lhs.region_id < rhs.region_id;
      });
  std::sort(edges->begin(), edges->end(),
      [](const QuotientEdge& lhs, const QuotientEdge& rhs) {
        if (lhs.from_region != rhs.from_region)
          return lhs.from_region < rhs.from_region;
        if (lhs.to_region != rhs.to_region)
          return lhs.to_region < rhs.to_region;
        if (lhs.entry_from != rhs.entry_from)
          return lhs.entry_from < rhs.entry_from;
        return lhs.entry_to < rhs.entry_to;
      });
}

}  // namespace

SpectralWorker::SpectralWorker(const SpectralV4Config& config)
    : config_(config) {}

SpectralWorker::~SpectralWorker() { Stop(); }

void SpectralWorker::Start() {
  std::lock_guard<std::mutex> lock(mailbox_mutex_);
  if (running_) return;
  running_ = true;
  thread_ = std::thread(&SpectralWorker::Run, this);
}

void SpectralWorker::Stop() {
  {
    std::lock_guard<std::mutex> lock(mailbox_mutex_);
    if (!running_ && !thread_.joinable()) return;
    running_ = false;
  }
  cv_.notify_all();
  if (thread_.joinable()) thread_.join();
}

void SpectralWorker::Submit(std::shared_ptr<const GraphDelta> delta) {
  if (!delta || delta->empty()) return;
  {
    std::lock_guard<std::mutex> lock(mailbox_mutex_);
    pending_deltas_.push_back(std::move(delta));
  }
  cv_.notify_one();
}

std::shared_ptr<const RegionStateSnapshot> SpectralWorker::TryConsume() {
  std::lock_guard<std::mutex> lock(mailbox_mutex_);
  std::shared_ptr<const RegionStateSnapshot> result;
  result.swap(latest_result_);
  return result;
}

void SpectralWorker::Run() {
  SparseGraph graph;
  std::vector<RegionTreeNode> region_tree;
  std::unordered_map<int, Confirmation> confirmations;
  FiedlerHistory warm_start;
  std::vector<uint32_t> landmarks;
  int exact_failure_streak = 0;
  int nystrom_failure_streak = 0;
  ContractedSkeleton skeleton;
  bool skeleton_initialized = false;

  while (true) {
    std::vector<std::shared_ptr<const GraphDelta>> deltas;
    {
      std::unique_lock<std::mutex> lock(mailbox_mutex_);
      cv_.wait(lock, [this]() {
        return !pending_deltas_.empty() || !running_;
      });
      if (!running_) break;
      deltas.swap(pending_deltas_);
    }
    bool recontract = !skeleton_initialized;
    for (const auto& delta : deltas) {
      if (!delta) continue;
      recontract = recontract || RequiresRecontraction(*delta, graph);
      ApplyDelta(*delta, &graph);
    }
    if (graph.positions.empty()) continue;

    if (recontract) {
      skeleton = ContractChains(graph, config_);
      skeleton_initialized = true;
    } else {
      RefreshSkeletonAttributes(graph, &skeleton);
    }
    if (skeleton.node_ids.size() >
        static_cast<size_t>(config_.skeleton_max_nodes)) {
      // The global backends are bounded by skeleton_max_nodes. APPR may still
      // operate on the robot leaf once the persistent tree has one.
      ROS_WARN_THROTTLE(2.0,
          "[SpectralWorker] skeleton budget exceeded: %zu > %d",
          skeleton.node_ids.size(), config_.skeleton_max_nodes);
    }
    ReconcileRegionTree(skeleton, config_, &region_tree);

    int leaf_id = SelectBestLeaf(
        region_tree, skeleton, config_, graph.robot_h_id);
    if (leaf_id >= 0 &&
        static_cast<size_t>(leaf_id) < region_tree.size()) {
      const RegionTreeNode& preferred =
          region_tree[static_cast<size_t>(leaf_id)];
      const bool robot_in_preferred =
          preferred.h_ids.count(graph.robot_h_id) != 0U;
      if (SelectBackend(
              preferred.h_ids.size(), robot_in_preferred,
              exact_failure_streak, nystrom_failure_streak,
              config_) == CutBackend::UNAVAILABLE) {
        for (const auto& region : region_tree) {
          if (!region.leaf ||
              region.h_ids.count(graph.robot_h_id) == 0U ||
              region.h_ids.size() < 4U ||
              region.depth >= config_.max_recursive_depth)
            continue;
          leaf_id = region.id;
          break;
        }
      }
    }
    CutBackend backend = CutBackend::UNAVAILABLE;
    BinaryRegionProposal proposal;
    std::string reason = "no leaf eligible for bisection";

    if (leaf_id >= 0 &&
        static_cast<size_t>(leaf_id) < region_tree.size()) {
      const RegionTreeNode& leaf =
          region_tree[static_cast<size_t>(leaf_id)];
      ContractedSkeleton support =
          InducedSkeleton(skeleton, leaf.h_ids);
      const bool robot_in_leaf =
          leaf.h_ids.count(graph.robot_h_id) != 0U;
      backend = SelectBackend(
          support.node_ids.size(), robot_in_leaf,
          exact_failure_streak, nystrom_failure_streak, config_);
      V4BackendContext context;
      context.parent_region_id = leaf_id;
      context.frontier_version = graph.frontier_version;
      context.robot_h_id = graph.robot_h_id;

      bool solved = false;
      if (backend == CutBackend::INCREMENTAL_FIEDLER) {
        FiedlerHistory next_warm_start;
        solved = SolveIncrementalFiedler(
            support, config_, context, warm_start,
            &proposal, &next_warm_start, &reason);
        if (solved) {
          warm_start = std::move(next_warm_start);
          exact_failure_streak = 0;
        } else {
          ++exact_failure_streak;
        }
      } else if (backend == CutBackend::LANDMARK_NYSTROM) {
        solved = SolveNystromBisection(
            support, config_, context, &landmarks, &proposal, &reason);
        if (solved) nystrom_failure_streak = 0;
        else ++nystrom_failure_streak;
      } else if (backend == CutBackend::LOCAL_APPR) {
        solved = SolveLocalAppr(
            support, config_, context, &proposal, &reason);
      }

      if (solved && ConfirmProposal(
              proposal, leaf, skeleton, config_,
              &confirmations, &reason)) {
        AcceptCut(proposal, &region_tree);
      }
    }

    auto snapshot = std::make_shared<RegionStateSnapshot>();
    snapshot->graph_version = graph.graph_version;
    snapshot->frontier_version = graph.frontier_version;
    snapshot->source_backend = backend;
    snapshot->region_tree = region_tree;
    BuildQuotientGraph(
        region_tree, skeleton,
        &snapshot->quotient_regions,
        &snapshot->quotient_edges,
        &snapshot->h_to_region);
    snapshot->ncut = proposal.ncut;
    snapshot->balance = proposal.balance;
    snapshot->residual = proposal.residual;
    snapshot->truncated = proposal.truncated;
    snapshot->valid = !snapshot->quotient_regions.empty();

    if (config_.log_diagnostics) {
      ROS_INFO_STREAM(
          "[SpectralWorker] graph=" << graph.graph_version
          << " frontier=" << graph.frontier_version
          << " skeleton=" << skeleton.node_ids.size()
          << " backend=" << static_cast<int>(backend)
          << " leaves=" << LeafCount(region_tree)
          << " result=" << reason);
    }
    {
      std::lock_guard<std::mutex> lock(mailbox_mutex_);
      latest_result_ = std::move(snapshot);
    }
  }
}

}  // namespace DTGPlus
