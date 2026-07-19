#include <mr_dtg_plus/spectral_v4_backends.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace DTGPlus {
bool SolveLocalAppr(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    const V4BackendContext& context,
    BinaryRegionProposal* proposal,
    std::string* reason) {
  if (skeleton.node_ids.size() < 4U) {
    *reason = "APPR support has fewer than four nodes";
    return false;
  }
  const auto seed_it = std::find(
      skeleton.node_ids.begin(), skeleton.node_ids.end(), context.robot_h_id);
  if (seed_it == skeleton.node_ids.end()) {
    *reason = "APPR seed is not in the selected leaf";
    return false;
  }

  using Neighbor = std::pair<uint32_t, double>;
  std::unordered_map<uint32_t, std::vector<Neighbor>> adjacency;
  std::unordered_map<uint32_t, double> degree;
  const double length_scale = V4LengthScale(skeleton, config);
  for (const auto& edge : skeleton.edges) {
    const double weight =
        V4EdgeWeight(edge, config, length_scale);
    adjacency[edge.from].emplace_back(edge.to, weight);
    adjacency[edge.to].emplace_back(edge.from, weight);
    degree[edge.from] += weight;
    degree[edge.to] += weight;
  }
  for (uint32_t id : skeleton.node_ids) {
    if (degree[id] <= 1.0e-12) {
      *reason = "APPR support contains an isolated node";
      return false;
    }
  }

  std::unordered_map<uint32_t, double> page_rank;
  std::unordered_map<uint32_t, double> residual;
  std::unordered_set<uint32_t> support;
  residual[context.robot_h_id] = 1.0;
  support.insert(context.robot_h_id);

  using QueueItem = std::pair<double, uint32_t>;
  std::priority_queue<QueueItem> queue;
  queue.emplace(1.0 / degree[context.robot_h_id], context.robot_h_id);
  const size_t max_pushes =
      static_cast<size_t>(std::max(1, config.ppr_max_pushes));
  const size_t max_support =
      static_cast<size_t>(std::max(4, config.ppr_max_support_nodes));
  size_t pushes = 0;

  while (!queue.empty() && pushes < max_pushes) {
    const uint32_t node = queue.top().second;
    queue.pop();
    const double value = residual[node];
    if (value / degree[node] <= config.ppr_epsilon) continue;

    page_rank[node] += config.ppr_alpha * value;
    const double remain = (1.0 - config.ppr_alpha) * value;
    residual[node] = 0.5 * remain;
    for (const auto& neighbor : adjacency[node]) {
      if (support.count(neighbor.first) == 0U &&
          support.size() >= max_support)
        continue;
      const double amount =
          0.5 * remain * neighbor.second / degree[node];
      residual[neighbor.first] += amount;
      if (residual[neighbor.first] / degree[neighbor.first] >
          config.ppr_epsilon) {
        support.insert(neighbor.first);
        queue.emplace(
            residual[neighbor.first] / degree[neighbor.first],
            neighbor.first);
      }
    }
    ++pushes;
  }

  std::vector<std::pair<double, uint32_t>> order;
  for (uint32_t id : support) {
    const double value = page_rank[id] / degree[id];
    if (std::isfinite(value) && value > 0.0)
      order.emplace_back(value, id);
  }
  std::sort(order.begin(), order.end(),
      [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first) return lhs.first > rhs.first;
        return lhs.second < rhs.second;
      });
  if (order.empty()) {
    *reason = "APPR push produced an empty support";
    return false;
  }

  double total_volume = 0.0;
  for (uint32_t id : skeleton.node_ids) total_volume += degree[id];
  std::unordered_set<uint32_t> left;
  double left_volume = 0.0;
  double cut = 0.0;
  double best_conductance = std::numeric_limits<double>::infinity();
  double best_balance = 0.0;
  size_t best_split = 0;
  for (size_t split = 1; split <= order.size(); ++split) {
    const uint32_t added = order[split - 1].second;
    left.insert(added);
    left_volume += degree[added];
    for (const auto& neighbor : adjacency[added]) {
      if (left.count(neighbor.first) != 0U) cut -= neighbor.second;
      else cut += neighbor.second;
    }
    const double right_volume = total_volume - left_volume;
    if (left_volume <= 1.0e-12 || right_volume <= 1.0e-12) continue;
    const double balance =
        std::min(left_volume, right_volume) / total_volume;
    if (balance + 1.0e-12 < config.min_balance) continue;
    const double conductance = cut /
        std::min(left_volume, right_volume);
    if (conductance < best_conductance) {
      best_conductance = conductance;
      best_balance = balance;
      best_split = split;
    }
  }
  if (best_split == 0 || !std::isfinite(best_conductance)) {
    *reason = "APPR conductance sweep found no valid cut";
    return false;
  }

  std::unordered_set<uint32_t> side_a;
  for (size_t i = 0; i < best_split; ++i)
    side_a.insert(order[i].second);
  proposal->graph_version = skeleton.graph_version;
  proposal->frontier_version = context.frontier_version;
  proposal->backend = CutBackend::LOCAL_APPR;
  proposal->parent_region_id = context.parent_region_id;
  proposal->ncut = best_conductance;
  proposal->balance = best_balance;
  double remaining_residual = 0.0;
  for (const auto& entry : residual)
    remaining_residual += std::abs(entry.second);
  proposal->residual = remaining_residual;
  proposal->support_coverage =
      static_cast<double>(support.size()) /
      static_cast<double>(skeleton.node_ids.size());
  proposal->truncated = pushes >= max_pushes ||
      support.size() >= max_support;
  proposal->valid = best_conductance <= config.max_ncut;
  for (uint32_t id : skeleton.node_ids) {
    if (side_a.count(id) != 0U) proposal->side_a.push_back(id);
    else proposal->side_b.push_back(id);
  }
  if (!proposal->side_a.empty() && !proposal->side_b.empty() &&
      *std::min_element(proposal->side_b.begin(), proposal->side_b.end()) <
      *std::min_element(proposal->side_a.begin(), proposal->side_a.end()))
    proposal->side_a.swap(proposal->side_b);
  std::sort(proposal->side_a.begin(), proposal->side_a.end());
  uint64_t signature = 1469598103934665603ULL;
  for (uint32_t id : proposal->side_a) {
    signature ^= id;
    signature *= 1099511628211ULL;
  }
  proposal->deterministic_signature = signature;
  *reason = "Local APPR pushes=" + std::to_string(pushes) +
      " support=" + std::to_string(support.size()) +
      " conductance=" + std::to_string(best_conductance);
  return proposal->valid;
}

}  // namespace DTGPlus
