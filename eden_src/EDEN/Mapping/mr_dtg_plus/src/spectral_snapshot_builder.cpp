#include <mr_dtg_plus/spectral_snapshot_builder.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace DTGPlus {
namespace {

using Clock = std::chrono::steady_clock;

double ElapsedMilliseconds(const Clock::time_point& start) {
  return std::chrono::duration<double, std::milli>(Clock::now() - start)
      .count();
}

std::uint64_t UndirectedEdgeKey(std::uint32_t first,
                                std::uint32_t second) {
  const std::uint32_t low = std::min(first, second);
  const std::uint32_t high = std::max(first, second);
  return (static_cast<std::uint64_t>(low) << 32U) |
         static_cast<std::uint64_t>(high);
}

bool IsClearanceMode(SpectralWeightMode mode) {
  return mode == SpectralWeightMode::DISTANCE_CLEARANCE ||
         mode == SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS;
}

bool IsBetweennessMode(SpectralWeightMode mode) {
  return mode ==
         SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS;
}

SpectralSnapshotBuildOutput Failure(
    SpectralSnapshotBuildStatus status, const std::string& reason,
    const RawSpectralSnapshot& raw, const Clock::time_point& total_start) {
  SpectralSnapshotBuildOutput output;
  output.status = status;
  output.reason = reason;
  output.graph_version = raw.graph_version;
  output.frontier_version = raw.frontier_version;
  output.raw_node_count = raw.nodes.size();
  output.raw_edge_count = raw.edges.size();
  output.timings.worker_total_ms = ElapsedMilliseconds(total_start);
  return output;
}

struct AdjacentRawEdge {
  std::size_t node = 0U;
  std::size_t edge = 0U;
};

struct IndexedRawGraph {
  std::unordered_map<std::uint32_t, std::size_t> index_by_id;
  std::vector<std::vector<AdjacentRawEdge>> adjacency;
  std::vector<std::size_t> active_nodes;
};

bool IndexAndValidateRawGraph(
    const RawSpectralSnapshot& raw,
    const SpectralSnapshotBuildConfig& config, IndexedRawGraph* graph,
    SpectralSnapshotBuildStatus* status, std::string* reason) {
  if (graph == nullptr || status == nullptr || reason == nullptr) {
    return false;
  }
  if (!std::isfinite(config.numeric_epsilon) ||
      config.numeric_epsilon <= 0.0 ||
      config.min_spectral_nodes == 0U ||
      (config.max_spectral_nodes != 0U &&
       config.max_spectral_nodes < config.min_spectral_nodes) ||
      (config.support_mode == SpectralSupportMode::SUPPORT_SPARSE &&
       config.knn == 0U)) {
    *status = SpectralSnapshotBuildStatus::INVALID_CONFIG;
    *reason = "invalid support-graph configuration";
    return false;
  }
  if (raw.nodes.empty()) {
    *status = SpectralSnapshotBuildStatus::EMPTY_RAW_GRAPH;
    *reason = "raw spectral snapshot has no nodes";
    return false;
  }

  graph->index_by_id.reserve(raw.nodes.size());
  graph->adjacency.assign(raw.nodes.size(),
                          std::vector<AdjacentRawEdge>());
  for (std::size_t index = 0U; index < raw.nodes.size(); ++index) {
    const std::uint32_t id = raw.nodes[index].h_id;
    if (!graph->index_by_id.emplace(id, index).second) {
      *status = SpectralSnapshotBuildStatus::DUPLICATE_NODE_ID;
      std::ostringstream stream;
      stream << "raw spectral snapshot contains duplicate H-node ID " << id;
      *reason = stream.str();
      return false;
    }
    if (raw.nodes[index].active_anchor) {
      graph->active_nodes.push_back(index);
    }
  }
  if (graph->active_nodes.empty()) {
    *status = SpectralSnapshotBuildStatus::NO_ACTIVE_ANCHOR;
    *reason = "raw spectral snapshot has no active anchor";
    return false;
  }
  if (raw.edges.empty()) {
    *status = SpectralSnapshotBuildStatus::NO_RAW_EDGES;
    *reason = "raw spectral snapshot has no H-H edges";
    return false;
  }

  std::unordered_set<std::uint64_t> unique_edges;
  unique_edges.reserve(raw.edges.size());
  for (std::size_t edge_index = 0U; edge_index < raw.edges.size();
       ++edge_index) {
    const RawSpectralEdgeInput& edge = raw.edges[edge_index];
    const auto from = graph->index_by_id.find(edge.from_h_id);
    const auto to = graph->index_by_id.find(edge.to_h_id);
    if (from == graph->index_by_id.end() ||
        to == graph->index_by_id.end()) {
      *status = SpectralSnapshotBuildStatus::UNKNOWN_EDGE_ENDPOINT;
      std::ostringstream stream;
      stream << "raw edge " << edge_index
             << " references an absent H-node";
      *reason = stream.str();
      return false;
    }
    if (from->second == to->second) {
      *status = SpectralSnapshotBuildStatus::SELF_LOOP;
      std::ostringstream stream;
      stream << "raw edge " << edge_index << " is a self-loop";
      *reason = stream.str();
      return false;
    }
    if (!unique_edges
             .insert(UndirectedEdgeKey(edge.from_h_id, edge.to_h_id))
             .second) {
      *status = SpectralSnapshotBuildStatus::DUPLICATE_EDGE;
      std::ostringstream stream;
      stream << "raw edge " << edge_index
             << " duplicates an undirected endpoint pair";
      *reason = stream.str();
      return false;
    }
    if (!std::isfinite(edge.length) ||
        edge.length <= config.numeric_epsilon) {
      *status = SpectralSnapshotBuildStatus::INVALID_EDGE_LENGTH;
      std::ostringstream stream;
      stream << "raw edge " << edge_index
             << " does not have a finite positive length";
      *reason = stream.str();
      return false;
    }
    graph->adjacency[from->second].push_back({to->second, edge_index});
    graph->adjacency[to->second].push_back({from->second, edge_index});
  }
  return true;
}

struct ShortestPathTree {
  std::vector<double> distance;
  std::vector<std::size_t> predecessor_node;
  std::vector<std::size_t> predecessor_edge;
};

ShortestPathTree RunDijkstra(
    const RawSpectralSnapshot& raw, const IndexedRawGraph& graph,
    std::size_t source, double epsilon) {
  const std::size_t node_count = raw.nodes.size();
  const std::size_t absent = node_count;
  ShortestPathTree tree;
  tree.distance.assign(node_count,
                       std::numeric_limits<double>::infinity());
  tree.predecessor_node.assign(node_count, absent);
  tree.predecessor_edge.assign(node_count, raw.edges.size());

  using QueueEntry = std::pair<double, std::size_t>;
  std::priority_queue<QueueEntry, std::vector<QueueEntry>,
                      std::greater<QueueEntry>>
      open;
  tree.distance[source] = 0.0;
  open.push({0.0, source});

  while (!open.empty()) {
    const QueueEntry current = open.top();
    open.pop();
    const double tolerance =
        epsilon * std::max(1.0, std::abs(tree.distance[current.second]));
    if (current.first > tree.distance[current.second] + tolerance) {
      continue;
    }
    for (const AdjacentRawEdge& adjacent :
         graph.adjacency[current.second]) {
      const double candidate =
          current.first + raw.edges[adjacent.edge].length;
      const double current_distance = tree.distance[adjacent.node];
      const double comparison_tolerance =
          epsilon *
          std::max(1.0, std::max(std::abs(candidate),
                                 std::abs(current_distance)));
      bool replace = !std::isfinite(current_distance) ||
                     candidate + comparison_tolerance < current_distance;
      if (!replace &&
          std::isfinite(current_distance) &&
          std::abs(candidate - current_distance) <= comparison_tolerance) {
        const std::size_t old_predecessor =
            tree.predecessor_node[adjacent.node];
        replace = old_predecessor == absent ||
                  raw.nodes[current.second].h_id <
                      raw.nodes[old_predecessor].h_id;
      }
      if (replace) {
        tree.distance[adjacent.node] = candidate;
        tree.predecessor_node[adjacent.node] = current.second;
        tree.predecessor_edge[adjacent.node] = adjacent.edge;
        open.push({candidate, adjacent.node});
      }
    }
  }
  return tree;
}

bool ReconstructPathEdges(
    const RawSpectralSnapshot& raw, const ShortestPathTree& tree,
    std::size_t source, std::size_t target,
    std::vector<std::size_t>* path_edges, std::string* reason) {
  if (path_edges == nullptr || reason == nullptr) {
    return false;
  }
  path_edges->clear();
  std::size_t current = target;
  std::size_t guard = 0U;
  while (current != source) {
    if (current >= tree.predecessor_node.size() ||
        tree.predecessor_node[current] >= raw.nodes.size() ||
        tree.predecessor_edge[current] >= raw.edges.size() ||
        ++guard > raw.nodes.size()) {
      *reason = "failed to reconstruct an active-anchor shortest path";
      path_edges->clear();
      return false;
    }
    path_edges->push_back(tree.predecessor_edge[current]);
    current = tree.predecessor_node[current];
  }
  std::reverse(path_edges->begin(), path_edges->end());
  return true;
}

bool PathClearance(
    const RawSpectralSnapshot& raw,
    const std::vector<std::size_t>& path_edges, double* clearance,
    std::string* reason) {
  if (clearance == nullptr || reason == nullptr) {
    return false;
  }
  double minimum = std::numeric_limits<double>::infinity();
  for (const std::size_t edge_index : path_edges) {
    const double value = raw.edges[edge_index].cached_clearance;
    if (!std::isfinite(value) || value < 0.0) {
      *reason =
          "a selected support path lacks finite cached edge clearance";
      return false;
    }
    minimum = std::min(minimum, value);
  }
  if (!std::isfinite(minimum)) {
    *reason = "a selected support path contains no edge clearance";
    return false;
  }
  *clearance = minimum;
  return true;
}

bool ComputeEdgeBetweenness(SpectralGraphSnapshot* snapshot,
                            std::string* reason) {
  if (snapshot == nullptr || reason == nullptr) {
    return false;
  }
  struct Adjacent {
    std::size_t node = 0U;
    std::size_t edge = 0U;
  };

  const std::size_t node_count = snapshot->nodes.size();
  std::unordered_map<std::uint32_t, std::size_t> index_by_id;
  index_by_id.reserve(node_count);
  for (std::size_t index = 0U; index < node_count; ++index) {
    index_by_id.emplace(snapshot->nodes[index].h_id, index);
  }
  std::vector<std::vector<Adjacent>> adjacency(node_count);
  for (std::size_t edge_index = 0U;
       edge_index < snapshot->edges.size(); ++edge_index) {
    const SpectralEdgeInput& edge = snapshot->edges[edge_index];
    const auto from = index_by_id.find(edge.from_h_id);
    const auto to = index_by_id.find(edge.to_h_id);
    if (from == index_by_id.end() || to == index_by_id.end() ||
        !std::isfinite(edge.length) || edge.length <= 0.0) {
      *reason = "cannot compute betweenness on an invalid support edge";
      return false;
    }
    adjacency[from->second].push_back({to->second, edge_index});
    adjacency[to->second].push_back({from->second, edge_index});
  }

  std::vector<double> centrality(snapshot->edges.size(), 0.0);
  const double epsilon = 1.0e-10;
  using QueueEntry = std::pair<double, std::size_t>;
  for (std::size_t source = 0U; source < node_count; ++source) {
    std::vector<double> distance(
        node_count, std::numeric_limits<double>::infinity());
    std::vector<double> path_count(node_count, 0.0);
    std::vector<double> dependency(node_count, 0.0);
    std::vector<unsigned char> settled(node_count, 0U);
    std::vector<std::vector<std::pair<std::size_t, std::size_t>>>
        predecessors(node_count);
    std::vector<std::size_t> order;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>,
                        std::greater<QueueEntry>>
        open;
    distance[source] = 0.0;
    path_count[source] = 1.0;
    open.push({0.0, source});

    while (!open.empty()) {
      const QueueEntry current = open.top();
      open.pop();
      if (current.first > distance[current.second] + epsilon ||
          settled[current.second] != 0U) {
        continue;
      }
      settled[current.second] = 1U;
      order.push_back(current.second);
      for (const Adjacent& adjacent : adjacency[current.second]) {
        const double candidate =
            current.first + snapshot->edges[adjacent.edge].length;
        if (candidate + epsilon < distance[adjacent.node]) {
          distance[adjacent.node] = candidate;
          path_count[adjacent.node] = path_count[current.second];
          predecessors[adjacent.node].clear();
          predecessors[adjacent.node].push_back(
              {current.second, adjacent.edge});
          open.push({candidate, adjacent.node});
        } else if (std::abs(candidate - distance[adjacent.node]) <=
                   epsilon) {
          path_count[adjacent.node] += path_count[current.second];
          predecessors[adjacent.node].push_back(
              {current.second, adjacent.edge});
        }
      }
    }

    for (auto node = order.rbegin(); node != order.rend(); ++node) {
      if (path_count[*node] <= epsilon) {
        continue;
      }
      for (const auto& predecessor : predecessors[*node]) {
        const double contribution =
            path_count[predecessor.first] / path_count[*node] *
            (1.0 + dependency[*node]);
        if (!std::isfinite(contribution) || contribution < 0.0) {
          *reason = "edge betweenness produced a non-finite dependency";
          return false;
        }
        centrality[predecessor.second] += contribution;
        dependency[predecessor.first] += contribution;
      }
    }
  }

  for (std::size_t edge_index = 0U;
       edge_index < snapshot->edges.size(); ++edge_index) {
    snapshot->edges[edge_index].betweenness =
        0.5 * centrality[edge_index];
  }
  return true;
}

}  // namespace

const char* SpectralSnapshotBuildStatusName(
    SpectralSnapshotBuildStatus status) {
  switch (status) {
    case SpectralSnapshotBuildStatus::SUCCESS:
      return "SUCCESS";
    case SpectralSnapshotBuildStatus::INVALID_CONFIG:
      return "INVALID_CONFIG";
    case SpectralSnapshotBuildStatus::EMPTY_RAW_GRAPH:
      return "EMPTY_RAW_GRAPH";
    case SpectralSnapshotBuildStatus::DUPLICATE_NODE_ID:
      return "DUPLICATE_NODE_ID";
    case SpectralSnapshotBuildStatus::NO_ACTIVE_ANCHOR:
      return "NO_ACTIVE_ANCHOR";
    case SpectralSnapshotBuildStatus::NO_RAW_EDGES:
      return "NO_RAW_EDGES";
    case SpectralSnapshotBuildStatus::UNKNOWN_EDGE_ENDPOINT:
      return "UNKNOWN_EDGE_ENDPOINT";
    case SpectralSnapshotBuildStatus::SELF_LOOP:
      return "SELF_LOOP";
    case SpectralSnapshotBuildStatus::DUPLICATE_EDGE:
      return "DUPLICATE_EDGE";
    case SpectralSnapshotBuildStatus::INVALID_EDGE_LENGTH:
      return "INVALID_EDGE_LENGTH";
    case SpectralSnapshotBuildStatus::ACTIVE_ANCHORS_DISCONNECTED:
      return "ACTIVE_ANCHORS_DISCONNECTED";
    case SpectralSnapshotBuildStatus::PATH_RECONSTRUCTION_FAILED:
      return "PATH_RECONSTRUCTION_FAILED";
    case SpectralSnapshotBuildStatus::MISSING_EDGE_CLEARANCE:
      return "MISSING_EDGE_CLEARANCE";
    case SpectralSnapshotBuildStatus::EMPTY_SUPPORT_GRAPH:
      return "EMPTY_SUPPORT_GRAPH";
    case SpectralSnapshotBuildStatus::COMPRESSION_FAILED:
      return "COMPRESSION_FAILED";
    case SpectralSnapshotBuildStatus::TOO_FEW_NODES:
      return "TOO_FEW_NODES";
    case SpectralSnapshotBuildStatus::TOO_MANY_NODES:
      return "TOO_MANY_NODES";
    case SpectralSnapshotBuildStatus::BETWEENNESS_FAILED:
      return "BETWEENNESS_FAILED";
  }
  return "UNKNOWN";
}

bool CompressSpectralDegreeTwoChains(
    const SpectralGraphSnapshot& input,
    const std::unordered_set<std::uint32_t>& preserved_h_ids,
    SpectralGraphSnapshot* output, std::string* reason) {
  if (output == nullptr || reason == nullptr) {
    return false;
  }
  *output = SpectralGraphSnapshot();
  output->graph_version = input.graph_version;
  if (input.nodes.empty() || input.edges.empty()) {
    *reason = "cannot compress an empty spectral support graph";
    return false;
  }

  struct Adjacent {
    std::size_t node = 0U;
    std::size_t edge = 0U;
  };
  std::unordered_map<std::uint32_t, std::size_t> index_by_id;
  index_by_id.reserve(input.nodes.size());
  for (std::size_t index = 0U; index < input.nodes.size(); ++index) {
    if (!index_by_id.emplace(input.nodes[index].h_id, index).second) {
      *reason = "corridor compression found a duplicate node ID";
      return false;
    }
  }

  std::vector<std::vector<Adjacent>> adjacency(input.nodes.size());
  std::unordered_set<std::uint64_t> unique_edges;
  for (std::size_t edge_index = 0U; edge_index < input.edges.size();
       ++edge_index) {
    const SpectralEdgeInput& edge = input.edges[edge_index];
    const auto from = index_by_id.find(edge.from_h_id);
    const auto to = index_by_id.find(edge.to_h_id);
    if (from == index_by_id.end() || to == index_by_id.end()) {
      *reason = "corridor compression found an unknown edge endpoint";
      return false;
    }
    if (from->second == to->second ||
        !unique_edges
             .insert(UndirectedEdgeKey(edge.from_h_id, edge.to_h_id))
             .second) {
      *reason = "corridor compression requires unique non-self edges";
      return false;
    }
    if (!std::isfinite(edge.length) || edge.length <= 0.0) {
      *reason = "corridor compression found an invalid edge length";
      return false;
    }
    adjacency[from->second].push_back({to->second, edge_index});
    adjacency[to->second].push_back({from->second, edge_index});
  }

  std::vector<unsigned char> preserve(input.nodes.size(), 0U);
  for (std::size_t index = 0U; index < input.nodes.size(); ++index) {
    if (input.nodes[index].active_anchor ||
        preserved_h_ids.count(input.nodes[index].h_id) != 0U ||
        adjacency[index].size() != 2U) {
      preserve[index] = 1U;
    }
  }

  // An all-degree-two connected component is a cycle.  Three deterministic
  // break points retain that cycle without producing a self-loop or duplicate
  // parallel endpoint pair.
  std::vector<unsigned char> component_seen(input.nodes.size(), 0U);
  for (std::size_t seed = 0U; seed < input.nodes.size(); ++seed) {
    if (component_seen[seed] != 0U) {
      continue;
    }
    std::vector<std::size_t> component;
    std::vector<std::size_t> open(1U, seed);
    component_seen[seed] = 1U;
    bool has_preserved_node = false;
    while (!open.empty()) {
      const std::size_t current = open.back();
      open.pop_back();
      component.push_back(current);
      has_preserved_node =
          has_preserved_node || preserve[current] != 0U;
      for (const Adjacent& adjacent : adjacency[current]) {
        if (component_seen[adjacent.node] == 0U) {
          component_seen[adjacent.node] = 1U;
          open.push_back(adjacent.node);
        }
      }
    }
    if (!has_preserved_node) {
      std::sort(component.begin(), component.end(),
                [&input](std::size_t lhs, std::size_t rhs) {
                  return input.nodes[lhs].h_id <
                         input.nodes[rhs].h_id;
                });
      const std::size_t break_count =
          std::min<std::size_t>(3U, component.size());
      for (std::size_t index = 0U; index < break_count; ++index) {
        preserve[component[index]] = 1U;
      }
    }
  }

  struct CorridorChain {
    std::size_t start = 0U;
    std::size_t end = 0U;
    std::vector<std::size_t> nodes;
    std::vector<std::size_t> edges;
  };
  std::vector<CorridorChain> chains;
  std::vector<unsigned char> visited_edge(input.edges.size(), 0U);
  for (std::size_t start = 0U; start < input.nodes.size(); ++start) {
    if (preserve[start] == 0U) {
      continue;
    }
    for (const Adjacent& first : adjacency[start]) {
      if (visited_edge[first.edge] != 0U) {
        continue;
      }
      CorridorChain chain;
      chain.start = start;
      chain.nodes.push_back(start);
      std::size_t previous = start;
      std::size_t current = first.node;
      std::size_t current_edge = first.edge;

      while (true) {
        if (visited_edge[current_edge] != 0U) {
          *reason =
              "corridor compression encountered an already consumed edge";
          return false;
        }
        visited_edge[current_edge] = 1U;
        chain.edges.push_back(current_edge);
        chain.nodes.push_back(current);
        if (preserve[current] != 0U) {
          break;
        }
        if (adjacency[current].size() != 2U) {
          *reason =
              "a non-preserved corridor node does not have degree two";
          return false;
        }
        const Adjacent next =
            adjacency[current][0].node == previous
                ? adjacency[current][1]
                : adjacency[current][0];
        previous = current;
        current = next.node;
        current_edge = next.edge;
      }
      chain.end = current;
      chains.push_back(std::move(chain));
    }
  }
  if (std::find(visited_edge.begin(), visited_edge.end(), 0U) !=
      visited_edge.end()) {
    *reason = "corridor compression left an unanchored edge";
    return false;
  }

  std::unordered_set<std::uint32_t> output_node_ids;
  const auto add_output_node =
      [&input, output, &output_node_ids](std::size_t index) {
        if (output_node_ids.insert(input.nodes[index].h_id).second) {
          output->nodes.push_back(input.nodes[index]);
        }
      };
  for (std::size_t index = 0U; index < input.nodes.size(); ++index) {
    if (preserve[index] != 0U) {
      add_output_node(index);
    }
  }

  std::map<std::uint64_t, std::vector<std::size_t>> chain_groups;
  for (std::size_t index = 0U; index < chains.size(); ++index) {
    const std::uint32_t from =
        input.nodes[chains[index].start].h_id;
    const std::uint32_t to = input.nodes[chains[index].end].h_id;
    chain_groups[UndirectedEdgeKey(from, to)].push_back(index);
  }

  std::unordered_set<std::uint64_t> output_edge_keys;
  const auto emit_segment =
      [&input, output, &output_edge_keys](
          const CorridorChain& chain, std::size_t begin,
          std::size_t end) {
        if (begin >= end || end > chain.edges.size()) {
          return false;
        }
        const std::uint32_t from =
            input.nodes[chain.nodes[begin]].h_id;
        const std::uint32_t to =
            input.nodes[chain.nodes[end]].h_id;
        if (from == to ||
            !output_edge_keys.insert(UndirectedEdgeKey(from, to)).second) {
          return false;
        }
        double length = 0.0;
        double clearance = std::numeric_limits<double>::infinity();
        double betweenness = 0.0;
        for (std::size_t index = begin; index < end; ++index) {
          const SpectralEdgeInput& edge =
              input.edges[chain.edges[index]];
          length += edge.length;
          if (std::isfinite(edge.clearance)) {
            clearance = std::min(clearance, edge.clearance);
          }
          if (std::isfinite(edge.betweenness)) {
            betweenness = std::max(betweenness, edge.betweenness);
          }
        }
        SpectralEdgeInput compressed(from, to, length);
        compressed.clearance =
            std::isfinite(clearance)
                ? clearance
                : std::numeric_limits<double>::quiet_NaN();
        compressed.betweenness = betweenness;
        output->edges.push_back(compressed);
        return true;
      };

  for (const auto& group : chain_groups) {
    for (const std::size_t chain_index : group.second) {
      const CorridorChain& chain = chains[chain_index];
      const std::size_t edge_count = chain.edges.size();
      bool emitted = false;
      if (chain.start == chain.end) {
        if (edge_count < 3U) {
          *reason = "corridor compression found a degenerate cycle";
          return false;
        }
        const std::size_t first_break =
            std::max<std::size_t>(1U, edge_count / 3U);
        const std::size_t second_break =
            std::min(edge_count - 1U,
                     std::max(first_break + 1U,
                              (2U * edge_count) / 3U));
        add_output_node(chain.nodes[first_break]);
        add_output_node(chain.nodes[second_break]);
        emitted =
            emit_segment(chain, 0U, first_break) &&
            emit_segment(chain, first_break, second_break) &&
            emit_segment(chain, second_break, edge_count);
      } else if (group.second.size() > 1U && edge_count > 1U) {
        const std::size_t split = edge_count / 2U;
        add_output_node(chain.nodes[split]);
        emitted = emit_segment(chain, 0U, split) &&
                  emit_segment(chain, split, edge_count);
      } else {
        emitted = emit_segment(chain, 0U, edge_count);
      }
      if (!emitted) {
        *reason =
            "corridor compression produced duplicate support endpoints";
        return false;
      }
    }
  }

  if (output->edges.empty()) {
    *reason = "corridor compression removed every support edge";
    return false;
  }
  std::sort(output->nodes.begin(), output->nodes.end(),
            [](const SpectralNodeInput& lhs,
               const SpectralNodeInput& rhs) {
              return lhs.h_id < rhs.h_id;
            });
  std::sort(output->edges.begin(), output->edges.end(),
            [](const SpectralEdgeInput& lhs,
               const SpectralEdgeInput& rhs) {
              const std::uint32_t lhs_low =
                  std::min(lhs.from_h_id, lhs.to_h_id);
              const std::uint32_t rhs_low =
                  std::min(rhs.from_h_id, rhs.to_h_id);
              if (lhs_low != rhs_low) {
                return lhs_low < rhs_low;
              }
              return std::max(lhs.from_h_id, lhs.to_h_id) <
                     std::max(rhs.from_h_id, rhs.to_h_id);
            });
  return true;
}

SpectralSnapshotBuildOutput BuildSpectralGraphSnapshot(
    const RawSpectralSnapshot& raw,
    const SpectralSnapshotBuildConfig& config) {
  const Clock::time_point total_start = Clock::now();
  const Clock::time_point support_start = Clock::now();
  IndexedRawGraph graph;
  SpectralSnapshotBuildStatus validation_status =
      SpectralSnapshotBuildStatus::INVALID_CONFIG;
  std::string reason;
  if (!IndexAndValidateRawGraph(raw, config, &graph,
                                &validation_status, &reason)) {
    return Failure(validation_status, reason, raw, total_start);
  }

  std::vector<ShortestPathTree> shortest_paths;
  shortest_paths.reserve(graph.active_nodes.size());
  for (const std::size_t source : graph.active_nodes) {
    shortest_paths.push_back(
        RunDijkstra(raw, graph, source, config.numeric_epsilon));
  }
  for (std::size_t first = 0U; first < graph.active_nodes.size();
       ++first) {
    for (std::size_t second = first + 1U;
         second < graph.active_nodes.size(); ++second) {
      if (!std::isfinite(
              shortest_paths[first]
                  .distance[graph.active_nodes[second]])) {
        return Failure(
            SpectralSnapshotBuildStatus::ACTIVE_ANCHORS_DISCONNECTED,
            "at least two active anchors are disconnected in the raw DTG",
            raw, total_start);
      }
    }
  }

  SpectralSnapshotBuildOutput output;
  output.graph_version = raw.graph_version;
  output.frontier_version = raw.frontier_version;
  output.raw_node_count = raw.nodes.size();
  output.raw_edge_count = raw.edges.size();
  output.snapshot.graph_version = raw.graph_version;

  if (config.support_mode == SpectralSupportMode::ACTIVE_COMPLETE) {
    for (const std::size_t raw_index : graph.active_nodes) {
      output.snapshot.nodes.emplace_back(
          raw.nodes[raw_index].h_id, true);
    }
    std::vector<std::size_t> path_edges;
    for (std::size_t first = 0U; first < graph.active_nodes.size();
         ++first) {
      for (std::size_t second = first + 1U;
           second < graph.active_nodes.size(); ++second) {
        const std::size_t target = graph.active_nodes[second];
        if (!ReconstructPathEdges(
                raw, shortest_paths[first], graph.active_nodes[first],
                target, &path_edges, &reason)) {
          return Failure(
              SpectralSnapshotBuildStatus::PATH_RECONSTRUCTION_FAILED,
              reason, raw, total_start);
        }
        SpectralEdgeInput edge(
            raw.nodes[graph.active_nodes[first]].h_id,
            raw.nodes[target].h_id,
            shortest_paths[first].distance[target]);
        if (IsClearanceMode(config.weight_mode) &&
            !PathClearance(raw, path_edges, &edge.clearance,
                           &reason)) {
          return Failure(
              SpectralSnapshotBuildStatus::MISSING_EDGE_CLEARANCE,
              reason, raw, total_start);
        }
        output.snapshot.edges.push_back(edge);
      }
    }
  } else {
    std::unordered_set<std::uint64_t> selected_anchor_pairs;
    for (std::size_t first = 0U; first < graph.active_nodes.size();
         ++first) {
      std::vector<std::pair<double, std::size_t>> neighbours;
      neighbours.reserve(graph.active_nodes.size() - 1U);
      for (std::size_t second = 0U;
           second < graph.active_nodes.size(); ++second) {
        if (first == second) {
          continue;
        }
        neighbours.push_back(
            {shortest_paths[first]
                 .distance[graph.active_nodes[second]],
             second});
      }
      std::sort(
          neighbours.begin(), neighbours.end(),
          [&raw, &graph](const std::pair<double, std::size_t>& lhs,
                         const std::pair<double, std::size_t>& rhs) {
            if (lhs.first != rhs.first) {
              return lhs.first < rhs.first;
            }
            return raw.nodes[graph.active_nodes[lhs.second]].h_id <
                   raw.nodes[graph.active_nodes[rhs.second]].h_id;
          });
      const std::size_t keep =
          std::min(config.knn, neighbours.size());
      for (std::size_t index = 0U; index < keep; ++index) {
        selected_anchor_pairs.insert(UndirectedEdgeKey(
            raw.nodes[graph.active_nodes[first]].h_id,
            raw.nodes[graph.active_nodes[neighbours[index].second]]
                .h_id));
      }
    }

    // A symmetric kNN union can still split at the most important
    // bottleneck.  Add a deterministic Prim MST over exact raw-graph
    // shortest-path distances.
    const std::size_t active_count = graph.active_nodes.size();
    if (active_count > 1U) {
      std::vector<double> key(
          active_count, std::numeric_limits<double>::infinity());
      std::vector<std::size_t> parent(active_count, active_count);
      std::vector<unsigned char> used(active_count, 0U);
      key[0] = 0.0;
      for (std::size_t iteration = 0U; iteration < active_count;
           ++iteration) {
        std::size_t current = active_count;
        for (std::size_t candidate = 0U; candidate < active_count;
             ++candidate) {
          if (used[candidate] == 0U &&
              (current == active_count ||
               key[candidate] < key[current] ||
               (key[candidate] == key[current] &&
                raw.nodes[graph.active_nodes[candidate]].h_id <
                    raw.nodes[graph.active_nodes[current]].h_id))) {
            current = candidate;
          }
        }
        if (current == active_count ||
            !std::isfinite(key[current])) {
          return Failure(
              SpectralSnapshotBuildStatus::
                  ACTIVE_ANCHORS_DISCONNECTED,
              "active-anchor MST could not span the raw DTG", raw,
              total_start);
        }
        used[current] = 1U;
        if (parent[current] != active_count) {
          selected_anchor_pairs.insert(UndirectedEdgeKey(
              raw.nodes[graph.active_nodes[current]].h_id,
              raw.nodes[graph.active_nodes[parent[current]]].h_id));
        }
        for (std::size_t candidate = 0U; candidate < active_count;
             ++candidate) {
          if (used[candidate] != 0U || candidate == current) {
            continue;
          }
          const double distance =
              shortest_paths[current]
                  .distance[graph.active_nodes[candidate]];
          if (distance < key[candidate] ||
              (distance == key[candidate] &&
               (parent[candidate] == active_count ||
                raw.nodes[graph.active_nodes[current]].h_id <
                    raw.nodes[graph.active_nodes[parent[candidate]]]
                        .h_id))) {
            key[candidate] = distance;
            parent[candidate] = current;
          }
        }
      }
    }

    std::unordered_set<std::size_t> support_nodes;
    std::unordered_set<std::size_t> support_edges;
    for (const std::size_t active : graph.active_nodes) {
      support_nodes.insert(active);
    }
    // Required root-path nodes are accepted only from a component reachable
    // by the current active-anchor set.
    for (std::size_t index = 0U; index < raw.nodes.size(); ++index) {
      if (raw.nodes[index].support_required &&
          std::isfinite(shortest_paths.front().distance[index])) {
        support_nodes.insert(index);
      }
    }

    std::unordered_map<std::uint32_t, std::size_t> active_row_by_id;
    for (std::size_t row = 0U; row < graph.active_nodes.size();
         ++row) {
      active_row_by_id.emplace(
          raw.nodes[graph.active_nodes[row]].h_id, row);
    }
    std::vector<std::size_t> path_edges;
    for (const std::uint64_t pair_key : selected_anchor_pairs) {
      const std::uint32_t from_id =
          static_cast<std::uint32_t>(pair_key >> 32U);
      const std::uint32_t to_id =
          static_cast<std::uint32_t>(pair_key & 0xffffffffU);
      const std::size_t row = active_row_by_id.at(from_id);
      const std::size_t source = graph.active_nodes[row];
      const std::size_t target = graph.index_by_id.at(to_id);
      if (!ReconstructPathEdges(raw, shortest_paths[row], source,
                                target, &path_edges, &reason)) {
        return Failure(
            SpectralSnapshotBuildStatus::PATH_RECONSTRUCTION_FAILED,
            reason, raw, total_start);
      }
      support_nodes.insert(source);
      support_nodes.insert(target);
      for (const std::size_t edge_index : path_edges) {
        support_edges.insert(edge_index);
        support_nodes.insert(
            graph.index_by_id.at(raw.edges[edge_index].from_h_id));
        support_nodes.insert(
            graph.index_by_id.at(raw.edges[edge_index].to_h_id));
      }
    }

    // Match the v2 support semantics: after selecting the path-union nodes,
    // retain every real H-H edge induced by those nodes.  This keeps root
    // branches and real cross-links without introducing all-pairs shortcuts.
    for (std::size_t edge_index = 0U; edge_index < raw.edges.size();
         ++edge_index) {
      const std::size_t from =
          graph.index_by_id.at(raw.edges[edge_index].from_h_id);
      const std::size_t to =
          graph.index_by_id.at(raw.edges[edge_index].to_h_id);
      if (support_nodes.count(from) != 0U &&
          support_nodes.count(to) != 0U) {
        support_edges.insert(edge_index);
      }
    }

    std::vector<std::size_t> ordered_nodes(support_nodes.begin(),
                                           support_nodes.end());
    std::sort(ordered_nodes.begin(), ordered_nodes.end(),
              [&raw](std::size_t lhs, std::size_t rhs) {
                return raw.nodes[lhs].h_id < raw.nodes[rhs].h_id;
              });
    for (const std::size_t index : ordered_nodes) {
      output.snapshot.nodes.emplace_back(
          raw.nodes[index].h_id, raw.nodes[index].active_anchor);
    }

    std::vector<std::size_t> ordered_edges(support_edges.begin(),
                                           support_edges.end());
    std::sort(
        ordered_edges.begin(), ordered_edges.end(),
        [&raw](std::size_t lhs, std::size_t rhs) {
          const RawSpectralEdgeInput& left = raw.edges[lhs];
          const RawSpectralEdgeInput& right = raw.edges[rhs];
          const std::uint32_t left_low =
              std::min(left.from_h_id, left.to_h_id);
          const std::uint32_t right_low =
              std::min(right.from_h_id, right.to_h_id);
          if (left_low != right_low) {
            return left_low < right_low;
          }
          return std::max(left.from_h_id, left.to_h_id) <
                 std::max(right.from_h_id, right.to_h_id);
        });
    for (const std::size_t edge_index : ordered_edges) {
      const RawSpectralEdgeInput& raw_edge = raw.edges[edge_index];
      SpectralEdgeInput edge(raw_edge.from_h_id, raw_edge.to_h_id,
                             raw_edge.length);
      if (std::isfinite(raw_edge.cached_clearance) &&
          raw_edge.cached_clearance >= 0.0) {
        edge.clearance = raw_edge.cached_clearance;
      } else if (IsClearanceMode(config.weight_mode)) {
        return Failure(
            SpectralSnapshotBuildStatus::MISSING_EDGE_CLEARANCE,
            "a selected real support edge lacks finite cached clearance",
            raw, total_start);
      }
      output.snapshot.edges.push_back(edge);
    }
  }

  if (output.snapshot.edges.empty()) {
    return Failure(
        SpectralSnapshotBuildStatus::EMPTY_SUPPORT_GRAPH,
        "support-graph selection produced no usable edge", raw,
        total_start);
  }
  output.support_node_count_before_compression =
      output.snapshot.nodes.size();
  output.support_edge_count_before_compression =
      output.snapshot.edges.size();
  output.timings.support_graph_ms =
      ElapsedMilliseconds(support_start);

  if (config.corridor_compression &&
      config.support_mode == SpectralSupportMode::SUPPORT_SPARSE) {
    std::unordered_set<std::uint32_t> preserved_ids;
    for (const RawSpectralNodeInput& node : raw.nodes) {
      if (node.preserve) {
        preserved_ids.insert(node.h_id);
      }
    }
    const Clock::time_point compression_start = Clock::now();
    SpectralGraphSnapshot compressed;
    if (!CompressSpectralDegreeTwoChains(
            output.snapshot, preserved_ids, &compressed, &reason)) {
      return Failure(
          SpectralSnapshotBuildStatus::COMPRESSION_FAILED, reason, raw,
          total_start);
    }
    output.timings.compression_ms =
        ElapsedMilliseconds(compression_start);
    output.snapshot = std::move(compressed);
  }

  if (output.snapshot.nodes.size() < config.min_spectral_nodes) {
    std::ostringstream stream;
    stream << "support graph has " << output.snapshot.nodes.size()
           << " nodes, fewer than " << config.min_spectral_nodes;
    return Failure(SpectralSnapshotBuildStatus::TOO_FEW_NODES,
                   stream.str(), raw, total_start);
  }
  if (config.max_spectral_nodes != 0U &&
      output.snapshot.nodes.size() > config.max_spectral_nodes) {
    std::ostringstream stream;
    stream << "support graph has " << output.snapshot.nodes.size()
           << " nodes, exceeding " << config.max_spectral_nodes;
    return Failure(SpectralSnapshotBuildStatus::TOO_MANY_NODES,
                   stream.str(), raw, total_start);
  }

  if (config.compute_betweenness ||
      IsBetweennessMode(config.weight_mode)) {
    const Clock::time_point betweenness_start = Clock::now();
    if (!ComputeEdgeBetweenness(&output.snapshot, &reason)) {
      return Failure(
          SpectralSnapshotBuildStatus::BETWEENNESS_FAILED, reason, raw,
          total_start);
    }
    output.timings.betweenness_ms =
        ElapsedMilliseconds(betweenness_start);
  }

  output.status = SpectralSnapshotBuildStatus::SUCCESS;
  output.reason = "immutable raw DTG support graph built successfully";
  output.timings.worker_total_ms =
      ElapsedMilliseconds(total_start);
  return output;
}

SpectralWorkerOutput BuildAndSolveSpectral(
    const RawSpectralSnapshot& raw,
    const SpectralSnapshotBuildConfig& build_config,
    const SpectralRouter& router,
    const FiedlerHistory& previous_fiedler) {
  const Clock::time_point worker_start = Clock::now();
  SpectralWorkerOutput output;
  if (build_config.weight_mode != router.config().weight_mode) {
    output.build.status =
        SpectralSnapshotBuildStatus::INVALID_CONFIG;
    output.build.graph_version = raw.graph_version;
    output.build.frontier_version = raw.frontier_version;
    output.build.raw_node_count = raw.nodes.size();
    output.build.raw_edge_count = raw.edges.size();
    output.build.reason =
        "support-builder and spectral-router weight modes differ";
    output.timings.worker_total_ms =
        ElapsedMilliseconds(worker_start);
    output.build.timings = output.timings;
    return output;
  }
  output.build = BuildSpectralGraphSnapshot(raw, build_config);
  output.timings = output.build.timings;
  if (!output.build.success()) {
    output.timings.worker_total_ms =
        ElapsedMilliseconds(worker_start);
    output.build.timings.worker_total_ms =
        output.timings.worker_total_ms;
    return output;
  }

  output.result =
      router.Solve(output.build.snapshot, previous_fiedler);
  output.timings.solver_ms =
      output.result.diagnostics.solve_time_ms;
  output.timings.ncut_ms =
      output.result.diagnostics.ncut_time_ms;
  output.timings.worker_total_ms =
      ElapsedMilliseconds(worker_start);
  output.build.timings = output.timings;
  return output;
}

}  // namespace DTGPlus
