#include <mr_dtg_plus/spectral_router.h>

#include <Eigen/Eigenvalues>
#include <Eigen/SparseCholesky>
#include <Eigen/SparseCore>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <queue>
#include <sstream>
#include <unordered_set>
#include <utility>

namespace DTGPlus {
namespace {

using Clock = std::chrono::steady_clock;

double ElapsedMilliseconds(const Clock::time_point& start) {
  return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
             Clock::now() - start)
      .count();
}

std::uint64_t UndirectedEdgeKey(std::uint32_t a, std::uint32_t b) {
  const std::uint32_t low = std::min(a, b);
  const std::uint32_t high = std::max(a, b);
  return (static_cast<std::uint64_t>(low) << 32U) |
         static_cast<std::uint64_t>(high);
}

double Median(std::vector<double> values) {
  std::sort(values.begin(), values.end());
  const std::size_t middle = values.size() / 2U;
  if ((values.size() & 1U) != 0U) {
    return values[middle];
  }
  return values[middle - 1U] +
         0.5 * (values[middle] - values[middle - 1U]);
}

std::string EdgeDescription(const SpectralEdgeInput& edge,
                            std::size_t edge_index) {
  std::ostringstream stream;
  stream << "edge[" << edge_index << "] (" << edge.from_h_id << ", "
         << edge.to_h_id << ")";
  return stream.str();
}

std::size_t CountComponents(const std::vector<std::vector<std::size_t>>& graph) {
  std::vector<unsigned char> visited(graph.size(), 0U);
  std::size_t components = 0U;
  std::queue<std::size_t> open;

  for (std::size_t root = 0; root < graph.size(); ++root) {
    if (visited[root] != 0U) {
      continue;
    }
    ++components;
    visited[root] = 1U;
    open.push(root);
    while (!open.empty()) {
      const std::size_t current = open.front();
      open.pop();
      for (const std::size_t neighbor : graph[current]) {
        if (visited[neighbor] == 0U) {
          visited[neighbor] = 1U;
          open.push(neighbor);
        }
      }
    }
  }
  return components;
}

void SetFailure(SpectralResult* result, SpectralSolveStatus status,
                const std::string& reason, const Clock::time_point& start) {
  result->status = status;
  result->diagnostics.reason = reason;
  result->diagnostics.total_time_ms = ElapsedMilliseconds(start);
}

bool IsClearanceMode(SpectralWeightMode mode) {
  return mode == SpectralWeightMode::DISTANCE_CLEARANCE ||
         mode == SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS;
}

bool IsBetweennessMode(SpectralWeightMode mode) {
  return mode == SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS;
}

bool AppendComplementVector(Eigen::VectorXd candidate,
                            const Eigen::VectorXd& trivial,
                            Eigen::MatrixXd* basis,
                            Eigen::Index column,
                            double epsilon) {
  // Two modified Gram-Schmidt passes are cheap for n <= 80 and avoid losing
  // the second vector when inverse iteration strongly amplifies lambda2.
  for (int pass = 0; pass < 2; ++pass) {
    candidate.noalias() -= trivial * trivial.dot(candidate);
    for (Eigen::Index previous = 0; previous < column; ++previous) {
      candidate.noalias() -=
          basis->col(previous) * basis->col(previous).dot(candidate);
    }
  }
  const double norm = candidate.norm();
  if (!std::isfinite(norm) || norm <= epsilon) {
    return false;
  }
  basis->col(column) = candidate / norm;
  return basis->col(column).allFinite();
}

bool BuildInitialIterativeBasis(
    const std::vector<std::uint32_t>& h_ids,
    const FiedlerHistory& previous_fiedler,
    const Eigen::VectorXd& trivial,
    std::size_t min_history_overlap,
    double epsilon,
    Eigen::MatrixXd* basis,
    bool* warm_start_used) {
  const Eigen::Index node_count = trivial.size();
  *basis = Eigen::MatrixXd::Zero(node_count, 2);
  *warm_start_used = false;
  Eigen::Index completed = 0;

  Eigen::VectorXd historical = Eigen::VectorXd::Zero(node_count);
  std::size_t history_overlap = 0U;
  for (Eigen::Index index = 0; index < node_count; ++index) {
    const auto found =
        previous_fiedler.find(h_ids[static_cast<std::size_t>(index)]);
    if (found == previous_fiedler.end() || !std::isfinite(found->second)) {
      continue;
    }
    historical(index) = found->second;
    ++history_overlap;
  }
  if (history_overlap >= min_history_overlap &&
      AppendComplementVector(historical, trivial, basis, completed,
                             epsilon)) {
    ++completed;
    *warm_start_used = true;
  }

  // Rank-based seeds make a cold solve deterministic and independent of the
  // input node ordering.  Unit-vector fallbacks guarantee completion of the
  // two-dimensional block for every graph with at least three nodes.
  std::vector<std::size_t> id_order(h_ids.size());
  for (std::size_t index = 0; index < h_ids.size(); ++index) {
    id_order[index] = index;
  }
  std::sort(id_order.begin(), id_order.end(),
            [&h_ids](std::size_t lhs, std::size_t rhs) {
              return h_ids[lhs] < h_ids[rhs];
            });
  std::vector<double> rank(h_ids.size(), 0.0);
  for (std::size_t order = 0; order < id_order.size(); ++order) {
    rank[id_order[order]] = static_cast<double>(order + 1U);
  }

  for (int seed = 0; seed < 4 && completed < 2; ++seed) {
    Eigen::VectorXd candidate(node_count);
    for (Eigen::Index index = 0; index < node_count; ++index) {
      const double value = rank[static_cast<std::size_t>(index)];
      switch (seed) {
        case 0:
          candidate(index) = value;
          break;
        case 1:
          candidate(index) = value * value;
          break;
        case 2:
          candidate(index) = std::sin(0.7548776662466927 * value);
          break;
        default:
          candidate(index) = std::cos(1.3247179572447458 * value);
          break;
      }
    }
    if (AppendComplementVector(candidate, trivial, basis, completed,
                               epsilon)) {
      ++completed;
    }
  }

  for (const std::size_t index : id_order) {
    if (completed >= 2) {
      break;
    }
    Eigen::VectorXd candidate = Eigen::VectorXd::Zero(node_count);
    candidate(static_cast<Eigen::Index>(index)) = 1.0;
    if (AppendComplementVector(candidate, trivial, basis, completed,
                               epsilon)) {
      ++completed;
    }
  }
  return completed == 2 && basis->allFinite();
}

bool OrthonormalizeIterativeBlock(const Eigen::MatrixXd& candidate,
                                  const Eigen::VectorXd& trivial,
                                  double epsilon,
                                  Eigen::MatrixXd* basis) {
  *basis = Eigen::MatrixXd::Zero(candidate.rows(), candidate.cols());
  for (Eigen::Index column = 0; column < candidate.cols(); ++column) {
    if (!AppendComplementVector(candidate.col(column), trivial, basis,
                                column, epsilon)) {
      return false;
    }
  }
  return basis->allFinite();
}

bool SolveSparseSmallestEigenpairs(
    const Eigen::MatrixXd& normalized_laplacian,
    const Eigen::VectorXd& trivial,
    const std::vector<std::uint32_t>& h_ids,
    const FiedlerHistory& previous_fiedler,
    const SpectralConfig& config,
    Eigen::Vector3d* eigenvalues,
    Eigen::MatrixXd* eigenvectors,
    std::size_t* iteration_count,
    bool* warm_start_used,
    std::string* reason) {
  const Eigen::Index node_count = normalized_laplacian.rows();
  using SparseMatrix = Eigen::SparseMatrix<double>;
  std::vector<Eigen::Triplet<double>> triplets;
  triplets.reserve(static_cast<std::size_t>(node_count) * 8U);
  for (Eigen::Index row = 0; row < node_count; ++row) {
    for (Eigen::Index column = 0; column < node_count; ++column) {
      const double value = normalized_laplacian(row, column);
      if (value != 0.0) {
        triplets.emplace_back(row, column, value);
      }
    }
  }
  SparseMatrix shifted_laplacian(node_count, node_count);
  shifted_laplacian.setFromTriplets(triplets.begin(), triplets.end());
  for (Eigen::Index index = 0; index < node_count; ++index) {
    shifted_laplacian.coeffRef(index, index) += config.iterative_shift;
  }
  shifted_laplacian.makeCompressed();

  Eigen::SimplicialLDLT<SparseMatrix> inverse_solver;
  inverse_solver.compute(shifted_laplacian);
  if (inverse_solver.info() != Eigen::Success) {
    *reason = "sparse shifted-Laplacian factorization failed";
    return false;
  }

  Eigen::MatrixXd basis;
  if (!BuildInitialIterativeBasis(
          h_ids, previous_fiedler, trivial, config.min_alignment_overlap,
          config.numeric_epsilon, &basis, warm_start_used)) {
    *reason = "could not construct a two-vector non-trivial initial block";
    return false;
  }

  const double convergence_tolerance =
      std::min(config.iterative_tolerance, 0.5 * config.max_residual_norm);
  *iteration_count = 0U;
  for (std::size_t iteration = 0;
       iteration < config.iterative_max_iterations; ++iteration) {
    const Eigen::MatrixXd inverse_block = inverse_solver.solve(basis);
    if (inverse_solver.info() != Eigen::Success ||
        !inverse_block.allFinite()) {
      *reason = "sparse shifted inverse solve failed";
      return false;
    }

    Eigen::MatrixXd orthonormal_block;
    if (!OrthonormalizeIterativeBlock(
            inverse_block, trivial, config.numeric_epsilon,
            &orthonormal_block)) {
      *reason = "block inverse iteration lost an independent search vector";
      return false;
    }

    Eigen::Matrix2d projected =
        orthonormal_block.transpose() * normalized_laplacian *
        orthonormal_block;
    projected = 0.5 * (projected + projected.transpose()).eval();
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix2d> ritz_solver(
        projected, Eigen::ComputeEigenvectors);
    if (ritz_solver.info() != Eigen::Success ||
        !ritz_solver.eigenvalues().allFinite() ||
        !ritz_solver.eigenvectors().allFinite()) {
      *reason = "the two-dimensional Rayleigh-Ritz solve failed";
      return false;
    }

    basis = orthonormal_block * ritz_solver.eigenvectors();
    const Eigen::Vector2d nontrivial_values = ritz_solver.eigenvalues();
    const Eigen::MatrixXd residual =
        normalized_laplacian * basis -
        basis * nontrivial_values.asDiagonal();
    const double lambda2_residual = residual.col(0).norm();
    const double lambda3_residual = residual.col(1).norm();
    *iteration_count = iteration + 1U;
    if (!std::isfinite(lambda2_residual) ||
        !std::isfinite(lambda3_residual)) {
      *reason = "block inverse iteration produced a non-finite Ritz residual";
      return false;
    }
    if (std::max(lambda2_residual, lambda3_residual) <=
        convergence_tolerance) {
      (*eigenvalues)(0) = trivial.dot(normalized_laplacian * trivial);
      (*eigenvalues)(1) = nontrivial_values(0);
      (*eigenvalues)(2) = nontrivial_values(1);
      eigenvectors->resize(node_count, 3);
      eigenvectors->col(0) = trivial;
      eigenvectors->rightCols(2) = basis;
      if (!eigenvalues->allFinite() || !eigenvectors->allFinite()) {
        *reason = "block inverse iteration produced non-finite eigenpairs";
        return false;
      }
      return true;
    }
  }

  std::ostringstream stream;
  stream << "sparse block inverse iteration did not converge in "
         << config.iterative_max_iterations << " iterations";
  *reason = stream.str();
  return false;
}

}  // namespace

const char* SpectralSolveStatusName(SpectralSolveStatus status) {
  switch (status) {
    case SpectralSolveStatus::SUCCESS:
      return "SUCCESS";
    case SpectralSolveStatus::SUCCESS_NO_VALID_CUT:
      return "SUCCESS_NO_VALID_CUT";
    case SpectralSolveStatus::INVALID_CONFIG:
      return "INVALID_CONFIG";
    case SpectralSolveStatus::EMPTY_GRAPH:
      return "EMPTY_GRAPH";
    case SpectralSolveStatus::TOO_FEW_NODES:
      return "TOO_FEW_NODES";
    case SpectralSolveStatus::TOO_MANY_NODES:
      return "TOO_MANY_NODES";
    case SpectralSolveStatus::DUPLICATE_NODE_ID:
      return "DUPLICATE_NODE_ID";
    case SpectralSolveStatus::NO_EDGES:
      return "NO_EDGES";
    case SpectralSolveStatus::UNKNOWN_EDGE_ENDPOINT:
      return "UNKNOWN_EDGE_ENDPOINT";
    case SpectralSolveStatus::SELF_LOOP:
      return "SELF_LOOP";
    case SpectralSolveStatus::DUPLICATE_EDGE:
      return "DUPLICATE_EDGE";
    case SpectralSolveStatus::INVALID_EDGE_LENGTH:
      return "INVALID_EDGE_LENGTH";
    case SpectralSolveStatus::INVALID_EDGE_CLEARANCE:
      return "INVALID_EDGE_CLEARANCE";
    case SpectralSolveStatus::INVALID_EDGE_BETWEENNESS:
      return "INVALID_EDGE_BETWEENNESS";
    case SpectralSolveStatus::INVALID_EDGE_WEIGHT:
      return "INVALID_EDGE_WEIGHT";
    case SpectralSolveStatus::ISOLATED_NODE:
      return "ISOLATED_NODE";
    case SpectralSolveStatus::DISCONNECTED_GRAPH:
      return "DISCONNECTED_GRAPH";
    case SpectralSolveStatus::NUMERICAL_FAILURE:
      return "NUMERICAL_FAILURE";
    case SpectralSolveStatus::EIGEN_SOLVER_FAILURE:
      return "EIGEN_SOLVER_FAILURE";
    case SpectralSolveStatus::RESIDUAL_TOO_LARGE:
      return "RESIDUAL_TOO_LARGE";
  }
  return "UNKNOWN_STATUS";
}

bool SpectralRouter::ValidateConfig(std::string* reason) const {
  const auto finite = [](double value) { return std::isfinite(value); };

  switch (config_.weight_mode) {
    case SpectralWeightMode::DISTANCE:
    case SpectralWeightMode::DISTANCE_CLEARANCE:
    case SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS:
      break;
    default:
      *reason = "weight_mode is not a supported enum value";
      return false;
  }

  if (!finite(config_.length_decay) || config_.length_decay < 0.0) {
    *reason = "length_decay must be finite and non-negative";
    return false;
  }
  if (!finite(config_.length_scale_multiplier) ||
      config_.length_scale_multiplier <= 0.0) {
    *reason = "length_scale_multiplier must be finite and positive";
    return false;
  }
  if (!finite(config_.clearance_reference) ||
      config_.clearance_reference <= 0.0) {
    *reason = "clearance_reference must be finite and positive";
    return false;
  }
  if (!finite(config_.clearance_power) || config_.clearance_power < 0.0) {
    *reason = "clearance_power must be finite and non-negative";
    return false;
  }
  if (!finite(config_.betweenness_weight) ||
      config_.betweenness_weight < 0.0) {
    *reason = "betweenness_weight must be finite and non-negative";
    return false;
  }
  if (!finite(config_.min_edge_weight) || config_.min_edge_weight < 0.0) {
    *reason = "min_edge_weight must be finite and non-negative";
    return false;
  }
  if (config_.min_spectral_nodes < 3U) {
    *reason = "min_spectral_nodes must be at least 3 to expose lambda3";
    return false;
  }
  if (config_.max_spectral_nodes != 0U &&
      config_.max_spectral_nodes < config_.min_spectral_nodes) {
    *reason = "max_spectral_nodes must be zero or no smaller than the minimum";
    return false;
  }
  if (config_.dense_solver_max_nodes < 3U) {
    *reason = "dense_solver_max_nodes must be at least 3";
    return false;
  }
  if (config_.iterative_max_iterations == 0U) {
    *reason = "iterative_max_iterations must be at least 1";
    return false;
  }
  if (!finite(config_.iterative_tolerance) ||
      config_.iterative_tolerance <= 0.0) {
    *reason = "iterative_tolerance must be finite and positive";
    return false;
  }
  if (!finite(config_.iterative_shift) || config_.iterative_shift <= 0.0) {
    *reason = "iterative_shift must be finite and positive";
    return false;
  }
  if (config_.min_cluster_size == 0U) {
    *reason = "min_cluster_size must be at least 1";
    return false;
  }
  if (!finite(config_.min_cluster_volume) ||
      config_.min_cluster_volume < 0.0) {
    *reason = "min_cluster_volume must be finite and non-negative";
    return false;
  }
  if (!finite(config_.min_cluster_volume_fraction) ||
      config_.min_cluster_volume_fraction < 0.0 ||
      config_.min_cluster_volume_fraction >= 0.5) {
    *reason = "min_cluster_volume_fraction must be in [0, 0.5)";
    return false;
  }
  if (!finite(config_.numeric_epsilon) || config_.numeric_epsilon <= 0.0) {
    *reason = "numeric_epsilon must be finite and positive";
    return false;
  }
  if (!finite(config_.eigenvalue_tolerance) ||
      config_.eigenvalue_tolerance <= 0.0) {
    *reason = "eigenvalue_tolerance must be finite and positive";
    return false;
  }
  if (!finite(config_.max_residual_norm) ||
      config_.max_residual_norm <= 0.0) {
    *reason = "max_residual_norm must be finite and positive";
    return false;
  }
  if (!finite(config_.sweep_value_tolerance) ||
      config_.sweep_value_tolerance < 0.0) {
    *reason = "sweep_value_tolerance must be finite and non-negative";
    return false;
  }
  if (config_.min_alignment_overlap == 0U) {
    *reason = "min_alignment_overlap must be at least 1";
    return false;
  }
  if (!finite(config_.alignment_dot_tolerance) ||
      config_.alignment_dot_tolerance < 0.0) {
    *reason = "alignment_dot_tolerance must be finite and non-negative";
    return false;
  }
  return true;
}

SpectralResult SpectralRouter::Solve(
    const SpectralGraphSnapshot& snapshot,
    const FiedlerHistory& previous_fiedler) const {
  const Clock::time_point total_start = Clock::now();
  SpectralResult result;
  result.diagnostics.graph_version = snapshot.graph_version;
  result.diagnostics.node_count = snapshot.nodes.size();
  result.diagnostics.edge_count = snapshot.edges.size();

  std::string config_reason;
  if (!ValidateConfig(&config_reason)) {
    SetFailure(&result, SpectralSolveStatus::INVALID_CONFIG, config_reason,
               total_start);
    return result;
  }

  const std::size_t node_count = snapshot.nodes.size();
  if (node_count == 0U) {
    SetFailure(&result, SpectralSolveStatus::EMPTY_GRAPH,
               "the spectral graph has no nodes", total_start);
    return result;
  }
  if (node_count < config_.min_spectral_nodes) {
    std::ostringstream reason;
    reason << "the graph has " << node_count << " nodes, fewer than the "
           << config_.min_spectral_nodes << " required nodes";
    SetFailure(&result, SpectralSolveStatus::TOO_FEW_NODES, reason.str(),
               total_start);
    return result;
  }
  if (config_.max_spectral_nodes != 0U &&
      node_count > config_.max_spectral_nodes) {
    std::ostringstream reason;
    reason << "the graph has " << node_count
           << " nodes, exceeding the configured solver limit "
           << config_.max_spectral_nodes;
    SetFailure(&result, SpectralSolveStatus::TOO_MANY_NODES, reason.str(),
               total_start);
    return result;
  }

  result.h_ids.reserve(node_count);
  result.id_to_index.reserve(node_count);
  for (std::size_t index = 0; index < node_count; ++index) {
    const std::uint32_t h_id = snapshot.nodes[index].h_id;
    const auto inserted = result.id_to_index.emplace(h_id, index);
    if (!inserted.second) {
      ++result.diagnostics.duplicate_node_count;
      std::ostringstream reason;
      reason << "duplicate H-node ID " << h_id << " at node index " << index;
      SetFailure(&result, SpectralSolveStatus::DUPLICATE_NODE_ID, reason.str(),
                 total_start);
      return result;
    }
    result.h_ids.push_back(h_id);
    if (snapshot.nodes[index].active_anchor) {
      ++result.diagnostics.active_anchor_count;
    }
  }

  if (snapshot.edges.empty()) {
    SetFailure(&result, SpectralSolveStatus::NO_EDGES,
               "the spectral graph has no edges", total_start);
    return result;
  }

  std::vector<std::vector<std::size_t>> structural_graph(node_count);
  std::vector<std::pair<std::size_t, std::size_t>> edge_indices;
  edge_indices.reserve(snapshot.edges.size());
  std::vector<double> edge_lengths;
  edge_lengths.reserve(snapshot.edges.size());
  std::unordered_set<std::uint64_t> unique_edges;
  unique_edges.reserve(snapshot.edges.size());
  double maximum_betweenness = 0.0;

  for (std::size_t edge_index = 0; edge_index < snapshot.edges.size();
       ++edge_index) {
    const SpectralEdgeInput& edge = snapshot.edges[edge_index];
    const auto from = result.id_to_index.find(edge.from_h_id);
    const auto to = result.id_to_index.find(edge.to_h_id);
    if (from == result.id_to_index.end() || to == result.id_to_index.end()) {
      SetFailure(&result, SpectralSolveStatus::UNKNOWN_EDGE_ENDPOINT,
                 EdgeDescription(edge, edge_index) +
                     " references an H-node absent from snapshot.nodes",
                 total_start);
      return result;
    }
    if (from->second == to->second) {
      SetFailure(&result, SpectralSolveStatus::SELF_LOOP,
                 EdgeDescription(edge, edge_index) + " is a self-loop",
                 total_start);
      return result;
    }
    const std::uint64_t key =
        UndirectedEdgeKey(edge.from_h_id, edge.to_h_id);
    if (!unique_edges.insert(key).second) {
      ++result.diagnostics.duplicate_edge_count;
      SetFailure(&result, SpectralSolveStatus::DUPLICATE_EDGE,
                 EdgeDescription(edge, edge_index) +
                     " duplicates an existing undirected edge",
                 total_start);
      return result;
    }
    if (!std::isfinite(edge.length) || edge.length <= 0.0) {
      SetFailure(&result, SpectralSolveStatus::INVALID_EDGE_LENGTH,
                 EdgeDescription(edge, edge_index) +
                     " must have a finite positive length",
                 total_start);
      return result;
    }
    if (IsClearanceMode(config_.weight_mode) &&
        (!std::isfinite(edge.clearance) || edge.clearance < 0.0)) {
      SetFailure(&result, SpectralSolveStatus::INVALID_EDGE_CLEARANCE,
                 EdgeDescription(edge, edge_index) +
                     " must have finite non-negative clearance in this mode",
                 total_start);
      return result;
    }
    if (IsBetweennessMode(config_.weight_mode) &&
        (!std::isfinite(edge.betweenness) || edge.betweenness < 0.0)) {
      SetFailure(
          &result, SpectralSolveStatus::INVALID_EDGE_BETWEENNESS,
          EdgeDescription(edge, edge_index) +
              " must have finite non-negative betweenness in this mode",
          total_start);
      return result;
    }

    structural_graph[from->second].push_back(to->second);
    structural_graph[to->second].push_back(from->second);
    edge_indices.emplace_back(from->second, to->second);
    edge_lengths.push_back(edge.length);
    if (IsBetweennessMode(config_.weight_mode)) {
      maximum_betweenness = std::max(maximum_betweenness, edge.betweenness);
    }
  }

  for (std::size_t index = 0; index < node_count; ++index) {
    if (structural_graph[index].empty()) {
      result.diagnostics.isolated_h_ids.push_back(result.h_ids[index]);
    }
  }
  result.diagnostics.component_count = CountComponents(structural_graph);
  if (!result.diagnostics.isolated_h_ids.empty()) {
    std::ostringstream reason;
    reason << "the graph contains " << result.diagnostics.isolated_h_ids.size()
           << " structurally isolated node(s)";
    SetFailure(&result, SpectralSolveStatus::ISOLATED_NODE, reason.str(),
               total_start);
    return result;
  }
  if (result.diagnostics.component_count != 1U) {
    std::ostringstream reason;
    reason << "the graph has " << result.diagnostics.component_count
           << " structural connected components";
    SetFailure(&result, SpectralSolveStatus::DISCONNECTED_GRAPH, reason.str(),
               total_start);
    return result;
  }

  result.diagnostics.median_edge_length = Median(edge_lengths);
  const double raw_length_scale =
      config_.length_scale_multiplier * result.diagnostics.median_edge_length;
  if (!std::isfinite(raw_length_scale) || raw_length_scale <= 0.0) {
    SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE,
               "median edge length and scale multiplier produced an invalid "
               "length scale",
               total_start);
    return result;
  }
  result.diagnostics.length_scale =
      std::max(raw_length_scale, config_.numeric_epsilon);
  result.diagnostics.betweenness_scale = maximum_betweenness;

  result.weight_matrix = Eigen::MatrixXd::Zero(node_count, node_count);
  result.edge_weights.reserve(snapshot.edges.size());
  std::vector<std::vector<std::size_t>> weighted_graph(node_count);

  for (std::size_t edge_index = 0; edge_index < snapshot.edges.size();
       ++edge_index) {
    const SpectralEdgeInput& edge = snapshot.edges[edge_index];
    const double exponent = -config_.length_decay * edge.length /
                            result.diagnostics.length_scale;
    double weight = std::exp(exponent);

    if (IsClearanceMode(config_.weight_mode)) {
      double clearance_factor = 1.0;
      if (config_.clearance_power > 0.0) {
        const double clearance_ratio =
            std::min(1.0, edge.clearance / config_.clearance_reference);
        clearance_factor = std::pow(clearance_ratio, config_.clearance_power);
      }
      weight *= clearance_factor;
    }

    if (IsBetweennessMode(config_.weight_mode)) {
      double normalized_betweenness = edge.betweenness;
      if (config_.normalize_betweenness) {
        normalized_betweenness = maximum_betweenness > 0.0
                                     ? edge.betweenness / maximum_betweenness
                                     : 0.0;
      }
      weight /= 1.0 +
                config_.betweenness_weight * normalized_betweenness;
    }

    // This clamp is deliberately inside the explicit-edge loop.  Missing
    // pairs remain exact zeros in weight_matrix.
    weight = std::max(weight, config_.min_edge_weight);
    if (!std::isfinite(weight) || weight < 0.0) {
      SetFailure(&result, SpectralSolveStatus::INVALID_EDGE_WEIGHT,
                 EdgeDescription(edge, edge_index) +
                     " produced a non-finite or negative weight",
                 total_start);
      return result;
    }

    const std::size_t from_index = edge_indices[edge_index].first;
    const std::size_t to_index = edge_indices[edge_index].second;
    result.weight_matrix(from_index, to_index) = weight;
    result.weight_matrix(to_index, from_index) = weight;
    result.edge_weights.push_back(weight);
    if (weight > 0.0) {
      weighted_graph[from_index].push_back(to_index);
      weighted_graph[to_index].push_back(from_index);
    }
  }

  result.diagnostics.min_edge_weight =
      *std::min_element(result.edge_weights.begin(), result.edge_weights.end());
  result.diagnostics.max_edge_weight =
      *std::max_element(result.edge_weights.begin(), result.edge_weights.end());
  result.diagnostics.component_count = CountComponents(weighted_graph);
  if (result.diagnostics.component_count != 1U) {
    std::ostringstream reason;
    reason << "computed positive-weight graph has "
           << result.diagnostics.component_count
           << " components; use a positive min_edge_weight or rescale lengths";
    SetFailure(&result, SpectralSolveStatus::DISCONNECTED_GRAPH, reason.str(),
               total_start);
    return result;
  }

  result.degree = result.weight_matrix.rowwise().sum();
  if (!result.degree.allFinite()) {
    SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE,
               "the weighted degree vector contains NaN or infinity",
               total_start);
    return result;
  }
  result.diagnostics.min_degree = result.degree.minCoeff();
  result.diagnostics.max_degree = result.degree.maxCoeff();
  if (result.diagnostics.min_degree <= config_.numeric_epsilon) {
    for (std::size_t index = 0; index < node_count; ++index) {
      if (result.degree(static_cast<Eigen::Index>(index)) <=
          config_.numeric_epsilon) {
        result.diagnostics.isolated_h_ids.push_back(result.h_ids[index]);
      }
    }
    SetFailure(
        &result, SpectralSolveStatus::NUMERICAL_FAILURE,
        "at least one weighted degree is below numeric_epsilon; increase "
        "min_edge_weight or rescale the graph",
        total_start);
    return result;
  }

  const Eigen::VectorXd inverse_sqrt_degree =
      result.degree.array().sqrt().inverse().matrix();
  result.normalized_laplacian =
      Eigen::MatrixXd::Identity(node_count, node_count) -
      inverse_sqrt_degree.asDiagonal() * result.weight_matrix *
          inverse_sqrt_degree.asDiagonal();
  // Remove harmless asymmetric roundoff before invoking the self-adjoint
  // solver, while retaining the constructed matrix for diagnostics/tests.
  result.normalized_laplacian =
      0.5 * (result.normalized_laplacian +
             result.normalized_laplacian.transpose())
                .eval();
  if (!result.normalized_laplacian.allFinite()) {
    SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE,
               "L_sym contains NaN or infinity", total_start);
    return result;
  }

  Eigen::Vector3d smallest_eigenvalues;
  Eigen::MatrixXd smallest_eigenvectors;
  double maximum_eigenvalue = 2.0;
  const Clock::time_point solve_start = Clock::now();
  if (node_count <= config_.dense_solver_max_nodes) {
    result.diagnostics.solver_type =
        SpectralSolverType::DENSE_SELF_ADJOINT;
    Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eigen_solver(
        result.normalized_laplacian, Eigen::ComputeEigenvectors);
    result.diagnostics.solve_time_ms = ElapsedMilliseconds(solve_start);
    if (eigen_solver.info() != Eigen::Success) {
      SetFailure(&result, SpectralSolveStatus::EIGEN_SOLVER_FAILURE,
                 "Eigen::SelfAdjointEigenSolver did not converge",
                 total_start);
      return result;
    }
    if (!eigen_solver.eigenvalues().allFinite() ||
        !eigen_solver.eigenvectors().allFinite()) {
      SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE,
                 "the dense eigensolver returned NaN or infinity",
                 total_start);
      return result;
    }
    smallest_eigenvalues = eigen_solver.eigenvalues().head<3>();
    smallest_eigenvectors = eigen_solver.eigenvectors().leftCols(3);
    maximum_eigenvalue = eigen_solver.eigenvalues()(node_count - 1U);
  } else {
    result.diagnostics.solver_type =
        SpectralSolverType::SPARSE_BLOCK_INVERSE_ITERATION;
    Eigen::VectorXd trivial = result.degree.array().sqrt().matrix();
    trivial.normalize();
    std::string iterative_reason;
    const bool solved = SolveSparseSmallestEigenpairs(
        result.normalized_laplacian, trivial, result.h_ids, previous_fiedler,
        config_, &smallest_eigenvalues, &smallest_eigenvectors,
        &result.diagnostics.solver_iterations,
        &result.diagnostics.iterative_warm_start_used, &iterative_reason);
    result.diagnostics.solve_time_ms = ElapsedMilliseconds(solve_start);
    if (!solved) {
      SetFailure(&result, SpectralSolveStatus::EIGEN_SOLVER_FAILURE,
                 iterative_reason, total_start);
      return result;
    }
    // For a symmetric non-negative affinity matrix, the normalized
    // Laplacian spectrum is analytically contained in [0, 2].  lambda3 is
    // included here so a bad iterative Ritz value still trips validation.
    maximum_eigenvalue = std::max(2.0, smallest_eigenvalues(2));
  }

  if (!smallest_eigenvalues.allFinite() ||
      !smallest_eigenvectors.allFinite()) {
    SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE,
               "the selected eigensolver returned NaN or infinity",
               total_start);
    return result;
  }
  result.diagnostics.lambda1 = smallest_eigenvalues(0);
  result.diagnostics.lambda2 = smallest_eigenvalues(1);
  result.diagnostics.lambda3 = smallest_eigenvalues(2);
  result.diagnostics.eigengap_23 =
      result.diagnostics.lambda3 - result.diagnostics.lambda2;

  const double eigen_tolerance = config_.eigenvalue_tolerance;
  if (std::abs(result.diagnostics.lambda1) > eigen_tolerance ||
      result.diagnostics.lambda2 < -eigen_tolerance ||
      result.diagnostics.lambda3 < result.diagnostics.lambda2 -
                                       eigen_tolerance ||
      maximum_eigenvalue > 2.0 + eigen_tolerance) {
    std::ostringstream reason;
    reason << "unexpected L_sym eigenvalues (lambda1="
           << result.diagnostics.lambda1 << ", lambda2="
           << result.diagnostics.lambda2 << ", lambda3="
           << result.diagnostics.lambda3 << ", lambda_max="
           << maximum_eigenvalue << ")";
    SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE, reason.str(),
               total_start);
    return result;
  }

  result.fiedler = smallest_eigenvectors.col(1);

  double alignment_dot = 0.0;
  for (std::size_t index = 0; index < node_count; ++index) {
    const auto historical = previous_fiedler.find(result.h_ids[index]);
    if (historical == previous_fiedler.end() ||
        !std::isfinite(historical->second)) {
      continue;
    }
    alignment_dot += historical->second *
                     result.fiedler(static_cast<Eigen::Index>(index));
    ++result.diagnostics.history_overlap_count;
  }
  if (result.diagnostics.history_overlap_count > 0U) {
    result.diagnostics.history_alignment_dot = alignment_dot;
  }

  if (result.diagnostics.history_overlap_count >=
          config_.min_alignment_overlap &&
      std::isfinite(alignment_dot) &&
      std::abs(alignment_dot) > config_.alignment_dot_tolerance) {
    result.diagnostics.history_alignment_used = true;
    if (alignment_dot < 0.0) {
      result.fiedler = -result.fiedler;
      result.diagnostics.sign_flipped = true;
    }
  } else {
    // No robot pose belongs in this pure graph kernel.  For the first solve,
    // use an ID-stable canonical sign: the lowest-ID component with meaningful
    // magnitude is made negative.  Later cycles use temporal alignment above.
    std::vector<std::size_t> id_order(node_count);
    for (std::size_t index = 0; index < node_count; ++index) {
      id_order[index] = index;
    }
    std::sort(id_order.begin(), id_order.end(),
              [&result](std::size_t lhs, std::size_t rhs) {
                return result.h_ids[lhs] < result.h_ids[rhs];
              });
    for (const std::size_t index : id_order) {
      const double value = result.fiedler(static_cast<Eigen::Index>(index));
      if (std::abs(value) <= config_.alignment_dot_tolerance) {
        continue;
      }
      result.diagnostics.canonical_sign_used = true;
      if (value > 0.0) {
        result.fiedler = -result.fiedler;
        result.diagnostics.sign_flipped = true;
      }
      break;
    }
  }

  const Eigen::VectorXd residual =
      result.normalized_laplacian * result.fiedler -
      result.diagnostics.lambda2 * result.fiedler;
  result.diagnostics.residual_norm = residual.norm();
  Eigen::VectorXd trivial = result.degree.array().sqrt().matrix();
  trivial.normalize();
  result.diagnostics.trivial_orthogonality =
      std::abs(trivial.dot(result.fiedler));
  if (!std::isfinite(result.diagnostics.residual_norm) ||
      result.diagnostics.residual_norm > config_.max_residual_norm) {
    std::ostringstream reason;
    reason << "Fiedler residual " << result.diagnostics.residual_norm
           << " exceeds configured limit " << config_.max_residual_norm;
    SetFailure(&result, SpectralSolveStatus::RESIDUAL_TOO_LARGE, reason.str(),
               total_start);
    return result;
  }

  result.fiedler_by_h_id.reserve(node_count);
  result.spectral_order.resize(node_count);
  for (std::size_t index = 0; index < node_count; ++index) {
    result.fiedler_by_h_id.emplace(
        result.h_ids[index],
        result.fiedler(static_cast<Eigen::Index>(index)));
    result.spectral_order[index] = index;
  }
  std::sort(result.spectral_order.begin(), result.spectral_order.end(),
            [&result](std::size_t lhs, std::size_t rhs) {
              const double lhs_value =
                  result.fiedler(static_cast<Eigen::Index>(lhs));
              const double rhs_value =
                  result.fiedler(static_cast<Eigen::Index>(rhs));
              if (lhs_value != rhs_value) {
                return lhs_value < rhs_value;
              }
              return result.h_ids[lhs] < result.h_ids[rhs];
            });
  result.ordered_h_ids.reserve(node_count);
  result.ordered_active_h_ids.reserve(result.diagnostics.active_anchor_count);
  for (const std::size_t index : result.spectral_order) {
    result.ordered_h_ids.push_back(result.h_ids[index]);
    if (snapshot.nodes[index].active_anchor) {
      result.ordered_active_h_ids.push_back(result.h_ids[index]);
    }
  }

  result.labels.assign(node_count, -1);
  const double total_volume = result.degree.sum();
  if (!std::isfinite(total_volume) ||
      total_volume <= config_.numeric_epsilon) {
    SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE,
               "total graph volume is non-finite or numerically zero",
               total_start);
    return result;
  }
  const double required_volume =
      std::max(config_.min_cluster_volume,
               config_.min_cluster_volume_fraction * total_volume);
  double left_volume = 0.0;
  double cut_weight = 0.0;
  double best_ncut = std::numeric_limits<double>::infinity();
  double best_balance = -1.0;
  std::size_t best_left_size = 0U;

  std::vector<unsigned char> in_left(node_count, 0U);
  for (std::size_t left_size = 1U; left_size < node_count; ++left_size) {
    const std::size_t added = result.spectral_order[left_size - 1U];
    double weight_to_old_left = 0.0;
    for (std::size_t neighbor = 0; neighbor < node_count; ++neighbor) {
      if (in_left[neighbor] != 0U) {
        weight_to_old_left +=
            result.weight_matrix(static_cast<Eigen::Index>(added),
                                 static_cast<Eigen::Index>(neighbor));
      }
    }
    cut_weight += result.degree(static_cast<Eigen::Index>(added)) -
                  2.0 * weight_to_old_left;
    left_volume += result.degree(static_cast<Eigen::Index>(added));
    in_left[added] = 1U;

    const std::size_t right_size = node_count - left_size;
    if (left_size < config_.min_cluster_size ||
        right_size < config_.min_cluster_size) {
      continue;
    }
    const double right_volume = total_volume - left_volume;
    if (left_volume + config_.numeric_epsilon < required_volume ||
        right_volume + config_.numeric_epsilon < required_volume ||
        left_volume <= config_.numeric_epsilon ||
        right_volume <= config_.numeric_epsilon) {
      continue;
    }

    const double left_value = result.fiedler(static_cast<Eigen::Index>(
        result.spectral_order[left_size - 1U]));
    const double right_value = result.fiedler(static_cast<Eigen::Index>(
        result.spectral_order[left_size]));
    const double value_scale =
        std::max(1.0, std::max(std::abs(left_value), std::abs(right_value)));
    if (right_value - left_value <=
        config_.sweep_value_tolerance * value_scale) {
      continue;
    }

    double stable_cut_weight = cut_weight;
    const double cut_roundoff_tolerance =
        config_.numeric_epsilon * std::max(1.0, total_volume);
    if (stable_cut_weight < 0.0 &&
        stable_cut_weight >= -cut_roundoff_tolerance) {
      stable_cut_weight = 0.0;
    }
    if (!std::isfinite(stable_cut_weight) || stable_cut_weight < 0.0) {
      SetFailure(&result, SpectralSolveStatus::NUMERICAL_FAILURE,
                 "incremental sweep produced an invalid cut weight",
                 total_start);
      return result;
    }
    const double ncut = stable_cut_weight / left_volume +
                        stable_cut_weight / right_volume;
    if (!std::isfinite(ncut)) {
      continue;
    }
    ++result.diagnostics.sweep_candidate_count;
    const double balance = std::min(left_volume, right_volume);
    const double comparison_tolerance =
        config_.numeric_epsilon * std::max(1.0, std::abs(best_ncut));
    if (best_left_size == 0U ||
        ncut < best_ncut - comparison_tolerance ||
        (std::abs(ncut - best_ncut) <= comparison_tolerance &&
         balance > best_balance)) {
      best_ncut = ncut;
      best_balance = balance;
      best_left_size = left_size;
    }
  }

  if (best_left_size == 0U) {
    result.status = SpectralSolveStatus::SUCCESS_NO_VALID_CUT;
    result.diagnostics.reason =
        "the embedding is valid, but no sweep threshold satisfies the "
        "configured size, volume, and distinct-value constraints";
    result.diagnostics.total_time_ms = ElapsedMilliseconds(total_start);
    return result;
  }

  result.diagnostics.cut_valid = true;
  result.diagnostics.cut_left_size = best_left_size;
  result.diagnostics.cut_right_size = node_count - best_left_size;
  const double threshold_left = result.fiedler(static_cast<Eigen::Index>(
      result.spectral_order[best_left_size - 1U]));
  const double threshold_right = result.fiedler(static_cast<Eigen::Index>(
      result.spectral_order[best_left_size]));
  result.diagnostics.cut_threshold =
      threshold_left + 0.5 * (threshold_right - threshold_left);

  for (std::size_t order_index = 0; order_index < node_count; ++order_index) {
    result.labels[result.spectral_order[order_index]] =
        order_index < best_left_size ? 0 : 1;
  }
  double final_left_volume = 0.0;
  double final_cut_weight = 0.0;
  for (std::size_t i = 0; i < node_count; ++i) {
    if (result.labels[i] == 0) {
      final_left_volume += result.degree(static_cast<Eigen::Index>(i));
      for (std::size_t j = 0; j < node_count; ++j) {
        if (result.labels[j] == 1) {
          final_cut_weight += result.weight_matrix(
              static_cast<Eigen::Index>(i), static_cast<Eigen::Index>(j));
        }
      }
    }
  }
  result.diagnostics.cut_left_volume = final_left_volume;
  result.diagnostics.cut_right_volume = total_volume - final_left_volume;
  result.diagnostics.cut_weight = final_cut_weight;
  result.diagnostics.ncut =
      final_cut_weight / result.diagnostics.cut_left_volume +
      final_cut_weight / result.diagnostics.cut_right_volume;

  result.status = SpectralSolveStatus::SUCCESS;
  result.diagnostics.reason = "spectral embedding and sweep cut succeeded";
  result.diagnostics.total_time_ms = ElapsedMilliseconds(total_start);
  return result;
}

}  // namespace DTGPlus
