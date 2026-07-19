#include <mr_dtg_plus/spectral_v4_backends.h>

#include <Eigen/Eigenvalues>

#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace DTGPlus {
namespace {

using Adj = std::vector<std::vector<std::pair<size_t, double>>>;

Adj BuildAdjacency(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& cfg,
    std::unordered_map<uint32_t, size_t>* index) {
  index->clear();
  for (size_t i = 0; i < skeleton.node_ids.size(); ++i)
    (*index)[skeleton.node_ids[i]] = i;
  Adj adjacency(skeleton.node_ids.size());
  const double length_scale = V4LengthScale(skeleton, cfg);
  for (const auto& edge : skeleton.edges) {
    const auto a = index->find(edge.from);
    const auto b = index->find(edge.to);
    if (a == index->end() || b == index->end() || a->second == b->second)
      continue;
    const double w = V4EdgeWeight(edge, cfg, length_scale);
    adjacency[a->second].emplace_back(b->second, w);
    adjacency[b->second].emplace_back(a->second, w);
  }
  return adjacency;
}

std::vector<double> Dijkstra(const Adj& adjacency, size_t source) {
  const double inf = std::numeric_limits<double>::infinity();
  std::vector<double> distance(adjacency.size(), inf);
  using Item = std::pair<double, size_t>;
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> queue;
  distance[source] = 0.0;
  queue.emplace(0.0, source);
  while (!queue.empty()) {
    const Item item = queue.top();
    queue.pop();
    if (item.first != distance[item.second]) continue;
    for (const auto& neighbor : adjacency[item.second]) {
      const double edge_distance = 1.0 / std::max(neighbor.second, 1.0e-12);
      const double candidate = item.first + edge_distance;
      if (candidate < distance[neighbor.first]) {
        distance[neighbor.first] = candidate;
        queue.emplace(candidate, neighbor.first);
      }
    }
  }
  return distance;
}

void AddPriority(
    const std::vector<uint32_t>& ids,
    size_t target,
    std::unordered_set<uint32_t>* selected,
    std::vector<uint32_t>* landmarks) {
  std::vector<uint32_t> ordered = ids;
  std::sort(ordered.begin(), ordered.end());
  for (uint32_t id : ordered) {
    if (landmarks->size() >= target) break;
    if (selected->insert(id).second) landmarks->push_back(id);
  }
}

bool BuildProposalFromEmbedding(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& cfg,
    const V4BackendContext& context,
    const Adj& adjacency,
    const Eigen::VectorXd& embedding,
    double residual,
    BinaryRegionProposal* proposal,
    std::string* reason) {
  const size_t n = skeleton.node_ids.size();
  std::vector<size_t> order(n);
  for (size_t i = 0; i < n; ++i) order[i] = i;
  std::sort(order.begin(), order.end(), [&](size_t lhs, size_t rhs) {
    if (embedding[lhs] != embedding[rhs])
      return embedding[lhs] < embedding[rhs];
    return skeleton.node_ids[lhs] < skeleton.node_ids[rhs];
  });

  std::vector<double> degree(n, 0.0);
  double total_volume = 0.0;
  for (size_t i = 0; i < n; ++i) {
    for (const auto& edge : adjacency[i]) degree[i] += edge.second;
    total_volume += degree[i];
  }

  std::unordered_set<size_t> left;
  double left_volume = 0.0;
  double cut = 0.0;
  double best_ncut = std::numeric_limits<double>::infinity();
  double best_balance = 0.0;
  size_t best_split = 0;
  for (size_t split = 1; split < n; ++split) {
    const size_t added = order[split - 1];
    left.insert(added);
    left_volume += degree[added];
    for (const auto& edge : adjacency[added]) {
      if (left.count(edge.first) != 0U) cut -= edge.second;
      else cut += edge.second;
    }
    const double right_volume = total_volume - left_volume;
    if (left_volume <= 1.0e-12 || right_volume <= 1.0e-12) continue;
    const double balance =
        std::min(left_volume, right_volume) / total_volume;
    if (balance + 1.0e-12 < cfg.min_balance) continue;
    const double ncut = cut / left_volume + cut / right_volume;
    if (ncut < best_ncut) {
      best_ncut = ncut;
      best_balance = balance;
      best_split = split;
    }
  }
  if (best_split == 0 || !std::isfinite(best_ncut)) {
    *reason = "Nyström sweep found no budget-valid cut";
    return false;
  }

  proposal->graph_version = skeleton.graph_version;
  proposal->frontier_version = context.frontier_version;
  proposal->backend = CutBackend::LANDMARK_NYSTROM;
  proposal->parent_region_id = context.parent_region_id;
  proposal->ncut = best_ncut;
  proposal->balance = best_balance;
  proposal->residual = residual;
  proposal->support_coverage = 1.0;
  proposal->truncated = false;
  proposal->valid = best_ncut <= cfg.max_ncut &&
      residual <= cfg.nystrom_residual_threshold;
  for (size_t i = 0; i < best_split; ++i)
    proposal->side_a.push_back(skeleton.node_ids[order[i]]);
  for (size_t i = best_split; i < n; ++i)
    proposal->side_b.push_back(skeleton.node_ids[order[i]]);
  if (!proposal->side_a.empty() && !proposal->side_b.empty() &&
      *std::min_element(proposal->side_b.begin(), proposal->side_b.end()) <
      *std::min_element(proposal->side_a.begin(), proposal->side_a.end())) {
    proposal->side_a.swap(proposal->side_b);
  }
  uint64_t signature = 1469598103934665603ULL;
  std::sort(proposal->side_a.begin(), proposal->side_a.end());
  for (uint32_t id : proposal->side_a) {
    signature ^= id;
    signature *= 1099511628211ULL;
  }
  proposal->deterministic_signature = signature;
  return proposal->valid;
}

}  // namespace

bool SolveNystromBisection(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    const V4BackendContext& context,
    std::vector<uint32_t>* persistent_landmarks,
    BinaryRegionProposal* proposal,
    std::string* reason) {
  const size_t n = skeleton.node_ids.size();
  if (n < 4 || n > static_cast<size_t>(config.nystrom_max_support)) {
    *reason = "Nyström support is outside configured budget";
    return false;
  }

  std::unordered_map<uint32_t, size_t> index;
  const Adj adjacency = BuildAdjacency(skeleton, config, &index);
  for (const auto& neighbors : adjacency) {
    if (neighbors.empty()) {
      *reason = "Nyström support contains an isolated node";
      return false;
    }
  }

  const size_t target = std::min(
      n, static_cast<size_t>(std::max(
          3, std::min(config.landmark_count, config.landmark_max_count))));
  std::unordered_set<uint32_t> selected;
  std::vector<uint32_t> landmarks;
  for (uint32_t id : *persistent_landmarks) {
    if (landmarks.size() >= target) break;
    if (index.count(id) != 0U && selected.insert(id).second)
      landmarks.push_back(id);
  }

  std::unordered_map<uint32_t, size_t> degree;
  std::vector<uint32_t> forks;
  std::vector<uint32_t> anchors;
  for (size_t i = 0; i < n; ++i) {
    degree[skeleton.node_ids[i]] = adjacency[i].size();
    if (adjacency[i].size() != 2U) forks.push_back(skeleton.node_ids[i]);
    const auto active = skeleton.node_is_active_anchor.find(skeleton.node_ids[i]);
    if (active != skeleton.node_is_active_anchor.end() && active->second)
      anchors.push_back(skeleton.node_ids[i]);
  }
  AddPriority(forks, target, &selected, &landmarks);
  AddPriority(anchors, target, &selected, &landmarks);

  std::vector<std::pair<double, uint32_t>> clearance_priority;
  for (const auto& edge : skeleton.edges) {
    const double clearance = std::isfinite(edge.min_clearance)
        ? edge.min_clearance : config.clearance_reference;
    clearance_priority.emplace_back(clearance, edge.from);
    clearance_priority.emplace_back(clearance, edge.to);
  }
  std::sort(clearance_priority.begin(), clearance_priority.end(),
      [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first) return lhs.first < rhs.first;
        return lhs.second < rhs.second;
      });
  for (const auto& entry : clearance_priority) {
    if (landmarks.size() >= target) break;
    if (selected.insert(entry.second).second)
      landmarks.push_back(entry.second);
  }

  while (landmarks.size() < target) {
    std::vector<double> minimum_distance(
        n, std::numeric_limits<double>::infinity());
    for (uint32_t landmark : landmarks) {
      const std::vector<double> distance = Dijkstra(adjacency, index[landmark]);
      for (size_t i = 0; i < n; ++i)
        minimum_distance[i] = std::min(minimum_distance[i], distance[i]);
    }
    size_t best = n;
    for (size_t i = 0; i < n; ++i) {
      if (selected.count(skeleton.node_ids[i]) != 0U) continue;
      if (best == n || minimum_distance[i] > minimum_distance[best] ||
          (minimum_distance[i] == minimum_distance[best] &&
           skeleton.node_ids[i] < skeleton.node_ids[best]))
        best = i;
    }
    if (best == n) break;
    selected.insert(skeleton.node_ids[best]);
    landmarks.push_back(skeleton.node_ids[best]);
  }
  if (landmarks.size() < 3U) {
    *reason = "Nyström could not select three valid landmarks";
    return false;
  }
  *persistent_landmarks = landmarks;

  const size_t m = landmarks.size();
  Eigen::MatrixXd distance_nm(n, m);
  std::vector<double> finite_distances;
  for (size_t j = 0; j < m; ++j) {
    const auto distance = Dijkstra(adjacency, index[landmarks[j]]);
    for (size_t i = 0; i < n; ++i) {
      distance_nm(i, j) = distance[i];
      if (distance[i] > 0.0 && std::isfinite(distance[i]))
        finite_distances.push_back(distance[i]);
    }
  }
  if (finite_distances.empty()) {
    *reason = "Nyström landmark distances are invalid";
    return false;
  }
  std::nth_element(finite_distances.begin(),
      finite_distances.begin() + finite_distances.size() / 2,
      finite_distances.end());
  const double sigma = std::max(
      finite_distances[finite_distances.size() / 2], 1.0e-6);

  Eigen::MatrixXd W = Eigen::MatrixXd::Zero(m, m);
  for (size_t i = 0; i < m; ++i) {
    for (size_t j = i + 1; j < m; ++j) {
      const double distance = distance_nm(index[landmarks[i]], j);
      if (!std::isfinite(distance)) continue;
      const double affinity = std::exp(-distance / sigma);
      W(i, j) = affinity;
      W(j, i) = affinity;
    }
  }
  Eigen::VectorXd degree_m = W.rowwise().sum();
  Eigen::MatrixXd normalized = Eigen::MatrixXd::Zero(m, m);
  for (size_t i = 0; i < m; ++i) {
    if (degree_m[i] <= 1.0e-12) {
      *reason = "Nyström landmark graph is disconnected";
      return false;
    }
    for (size_t j = 0; j < m; ++j)
      normalized(i, j) = W(i, j) /
          std::sqrt(degree_m[i] * degree_m[j]);
  }
  const Eigen::MatrixXd laplacian =
      Eigen::MatrixXd::Identity(m, m) - normalized;
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> solver(laplacian);
  if (solver.info() != Eigen::Success || solver.eigenvalues().size() < 3) {
    *reason = "Nyström landmark eigensolve failed";
    return false;
  }

  Eigen::MatrixXd C = Eigen::MatrixXd::Zero(n, m);
  const size_t q = std::min(
      m, static_cast<size_t>(std::max(1, config.nearest_landmarks_per_node)));
  for (size_t i = 0; i < n; ++i) {
    std::vector<std::pair<double, size_t>> nearest;
    for (size_t j = 0; j < m; ++j)
      if (std::isfinite(distance_nm(i, j)))
        nearest.emplace_back(distance_nm(i, j), j);
    std::sort(nearest.begin(), nearest.end());
    for (size_t k = 0; k < std::min(q, nearest.size()); ++k)
      C(i, nearest[k].second) = std::exp(-nearest[k].first / sigma);
  }
  const double affinity_eigenvalue =
      std::max(1.0e-6, 1.0 - solver.eigenvalues()[1]);
  Eigen::VectorXd embedding = C *
      degree_m.cwiseInverse().cwiseSqrt().asDiagonal() *
      solver.eigenvectors().col(1) / affinity_eigenvalue;

  Eigen::VectorXd full_degree(n);
  for (size_t i = 0; i < n; ++i) {
    full_degree[i] = 0.0;
    for (const auto& edge : adjacency[i]) full_degree[i] += edge.second;
  }
  const Eigen::VectorXd trivial = full_degree.cwiseSqrt();
  embedding -= trivial * (trivial.dot(embedding) /
      std::max(trivial.squaredNorm(), 1.0e-12));
  const double norm = embedding.norm();
  if (!std::isfinite(norm) || norm <= 1.0e-12) {
    *reason = "Nyström extension collapsed to a zero vector";
    return false;
  }
  embedding /= norm;

  Eigen::VectorXd lap_times = embedding;
  for (size_t i = 0; i < n; ++i) {
    for (const auto& edge : adjacency[i]) {
      lap_times[i] -= edge.second * embedding[edge.first] /
          std::sqrt(full_degree[i] * full_degree[edge.first]);
    }
  }
  const double lambda = embedding.dot(lap_times);
  const double residual = (lap_times - lambda * embedding).norm();
  const bool ok = BuildProposalFromEmbedding(
      skeleton, config, context, adjacency, embedding, residual,
      proposal, reason);
  if (!ok && reason->empty())
    *reason = "Nyström proposal failed quality thresholds";
  return ok;
}

}  // namespace DTGPlus
