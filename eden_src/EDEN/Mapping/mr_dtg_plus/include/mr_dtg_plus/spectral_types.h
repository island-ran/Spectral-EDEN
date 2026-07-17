#ifndef MR_DTG_PLUS_SPECTRAL_TYPES_H_
#define MR_DTG_PLUS_SPECTRAL_TYPES_H_

#include <Eigen/Core>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

namespace DTGPlus {

// The three weighting levels described by the Spectral-EDEN design.  All
// modes use the length term.  The latter two additionally require the
// corresponding edge attributes to be finite and non-negative.
enum class SpectralWeightMode {
  DISTANCE = 0,
  DISTANCE_CLEARANCE = 1,
  DISTANCE_CLEARANCE_BETWEENNESS = 2
};

enum class SpectralSolveStatus {
  SUCCESS = 0,
  // The embedding and ordering are valid, but the configured sweep-cut
  // constraints admit no binary partition.  Callers may still use the
  // Fiedler order, but must not enable a regional preference from this result.
  SUCCESS_NO_VALID_CUT,
  INVALID_CONFIG,
  EMPTY_GRAPH,
  TOO_FEW_NODES,
  TOO_MANY_NODES,
  DUPLICATE_NODE_ID,
  NO_EDGES,
  UNKNOWN_EDGE_ENDPOINT,
  SELF_LOOP,
  DUPLICATE_EDGE,
  INVALID_EDGE_LENGTH,
  INVALID_EDGE_CLEARANCE,
  INVALID_EDGE_BETWEENNESS,
  INVALID_EDGE_WEIGHT,
  ISOLATED_NODE,
  DISCONNECTED_GRAPH,
  NUMERICAL_FAILURE,
  EIGEN_SOLVER_FAILURE,
  RESIDUAL_TOO_LARGE
};

// Solver selected from the support-graph size.  Keeping this in the pure
// kernel diagnostics makes it possible to verify that large snapshots do not
// silently fall back to the cubic dense path.
enum class SpectralSolverType {
  NONE = 0,
  DENSE_SELF_ADJOINT,
  SPARSE_BLOCK_INVERSE_ITERATION
};

struct SpectralConfig {
  SpectralWeightMode weight_mode = SpectralWeightMode::DISTANCE;

  // exp(-length_decay * length / (length_scale_multiplier * median_length))
  double length_decay = 1.0;
  double length_scale_multiplier = 1.0;

  // min(1, clearance / clearance_reference)^clearance_power
  double clearance_reference = 1.0;
  double clearance_power = 1.0;

  // 1 / (1 + betweenness_weight * normalized_betweenness).  When enabled,
  // raw non-negative input betweenness values are divided by their maximum.
  double betweenness_weight = 1.0;
  bool normalize_betweenness = true;

  // Applied only to explicitly supplied graph edges.  A zero entry in W for
  // a missing edge is never raised to min_edge_weight.
  double min_edge_weight = 1.0e-6;

  std::size_t min_spectral_nodes = 3;
  // Zero disables the upper bound.  Spectral-EDEN v2 compresses the support
  // graph before this kernel and skips a solve if it still exceeds 60 nodes.
  std::size_t max_spectral_nodes = 60;

  // Small support graphs use Eigen's dense self-adjoint eigensolver.  Larger
  // graphs use a sparse shifted block inverse iteration for lambda2/lambda3;
  // lambda1 and its vector are known analytically for a connected L_sym.
  std::size_t dense_solver_max_nodes = 40;
  std::size_t iterative_max_iterations = 100;
  double iterative_tolerance = 1.0e-10;
  double iterative_shift = 1.0e-4;

  std::size_t min_cluster_size = 1;
  double min_cluster_volume = 0.0;
  // Fraction of total graph volume required on each side of the cut.
  double min_cluster_volume_fraction = 0.0;

  double numeric_epsilon = 1.0e-12;
  double eigenvalue_tolerance = 1.0e-8;
  double max_residual_norm = 1.0e-8;
  // Adjacent Fiedler values closer than this relative tolerance do not form
  // a representable scalar threshold and are skipped by the sweep.
  double sweep_value_tolerance = 1.0e-12;

  std::size_t min_alignment_overlap = 1;
  double alignment_dot_tolerance = 1.0e-12;
};

struct SpectralNodeInput {
  std::uint32_t h_id = 0;
  bool active_anchor = false;

  SpectralNodeInput() = default;
  explicit SpectralNodeInput(std::uint32_t id, bool active = false)
      : h_id(id), active_anchor(active) {}
};

struct SpectralEdgeInput {
  std::uint32_t from_h_id = 0;
  std::uint32_t to_h_id = 0;
  double length = std::numeric_limits<double>::quiet_NaN();
  double clearance = std::numeric_limits<double>::quiet_NaN();
  double betweenness = 0.0;

  SpectralEdgeInput() = default;
  SpectralEdgeInput(std::uint32_t from, std::uint32_t to, double edge_length)
      : from_h_id(from), to_h_id(to), length(edge_length) {}
  SpectralEdgeInput(std::uint32_t from, std::uint32_t to, double edge_length,
                    double edge_clearance, double edge_betweenness)
      : from_h_id(from),
        to_h_id(to),
        length(edge_length),
        clearance(edge_clearance),
        betweenness(edge_betweenness) {}
};

struct SpectralGraphSnapshot {
  std::uint64_t graph_version = 0;
  std::vector<SpectralNodeInput> nodes;
  // Undirected sparse topology.  Each unordered endpoint pair must occur
  // exactly once; reversed duplicates are rejected.
  std::vector<SpectralEdgeInput> edges;
};

// Immutable, pointer-free copy of the live DTG used by the asynchronous
// spectral worker.  The planning thread is responsible only for filling these
// value types.  In particular, the worker must never retain H-node pointers,
// edge pointers, map pointers, or references to a GlobalRouteContext.
struct RawSpectralNodeInput {
  std::uint32_t h_id = 0;
  bool active_anchor = false;
  // Prevents a semantically important node (frontier attachment, seed, or
  // other execution anchor) from being removed by degree-two compression.
  bool preserve = false;
  // Includes a node in the initial support set even when it is not on a
  // selected active-anchor shortest path.  Robot-to-DTG root-path members use
  // this flag so the worker can retain the live approach branch.
  bool support_required = false;

  RawSpectralNodeInput() = default;
  explicit RawSpectralNodeInput(std::uint32_t id, bool active = false,
                                bool preserved = false,
                                bool required = false)
      : h_id(id),
        active_anchor(active),
        preserve(preserved),
        support_required(required) {}
};

struct RawSpectralEdgeInput {
  std::uint32_t from_h_id = 0;
  std::uint32_t to_h_id = 0;
  double length = std::numeric_limits<double>::quiet_NaN();
  // The main thread may copy an already-computed immutable scalar.  It must
  // not query the mutable occupancy map merely to fill this field.  Distance-
  // only builds accept NaN; clearance-weighted builds fail open when a
  // selected support edge has no finite cached value.
  double cached_clearance = std::numeric_limits<double>::quiet_NaN();

  RawSpectralEdgeInput() = default;
  RawSpectralEdgeInput(std::uint32_t from, std::uint32_t to,
                       double edge_length,
                       double edge_clearance =
                           std::numeric_limits<double>::quiet_NaN())
      : from_h_id(from),
        to_h_id(to),
        length(edge_length),
        cached_clearance(edge_clearance) {}
};

struct RawSpectralSnapshot {
  std::uint64_t graph_version = 0;
  std::uint64_t frontier_version = 0;
  std::vector<RawSpectralNodeInput> nodes;
  std::vector<RawSpectralEdgeInput> edges;
};

using FiedlerHistory = std::unordered_map<std::uint32_t, double>;

struct SpectralDiagnostics {
  std::uint64_t graph_version = 0;
  std::size_t node_count = 0;
  std::size_t edge_count = 0;
  std::size_t active_anchor_count = 0;
  std::size_t component_count = 0;
  std::size_t duplicate_node_count = 0;
  std::size_t duplicate_edge_count = 0;
  std::vector<std::uint32_t> isolated_h_ids;

  double median_edge_length = std::numeric_limits<double>::quiet_NaN();
  double length_scale = std::numeric_limits<double>::quiet_NaN();
  double betweenness_scale = std::numeric_limits<double>::quiet_NaN();
  double min_edge_weight = std::numeric_limits<double>::quiet_NaN();
  double max_edge_weight = std::numeric_limits<double>::quiet_NaN();
  double min_degree = std::numeric_limits<double>::quiet_NaN();
  double max_degree = std::numeric_limits<double>::quiet_NaN();

  double lambda1 = std::numeric_limits<double>::quiet_NaN();
  double lambda2 = std::numeric_limits<double>::quiet_NaN();
  double lambda3 = std::numeric_limits<double>::quiet_NaN();
  double eigengap_23 = std::numeric_limits<double>::quiet_NaN();
  double residual_norm = std::numeric_limits<double>::quiet_NaN();
  double trivial_orthogonality = std::numeric_limits<double>::quiet_NaN();

  SpectralSolverType solver_type = SpectralSolverType::NONE;
  std::size_t solver_iterations = 0;
  bool iterative_warm_start_used = false;

  std::size_t history_overlap_count = 0;
  double history_alignment_dot = std::numeric_limits<double>::quiet_NaN();
  bool history_alignment_used = false;
  bool canonical_sign_used = false;
  bool sign_flipped = false;

  bool cut_valid = false;
  std::size_t sweep_candidate_count = 0;
  std::size_t cut_left_size = 0;
  std::size_t cut_right_size = 0;
  double cut_threshold = std::numeric_limits<double>::quiet_NaN();
  double cut_weight = std::numeric_limits<double>::quiet_NaN();
  double cut_left_volume = std::numeric_limits<double>::quiet_NaN();
  double cut_right_volume = std::numeric_limits<double>::quiet_NaN();
  double ncut = std::numeric_limits<double>::quiet_NaN();

  double solve_time_ms = 0.0;
  double ncut_time_ms = 0.0;
  double total_time_ms = 0.0;
  std::string reason;
};

struct SpectralResult {
  SpectralSolveStatus status = SpectralSolveStatus::EMPTY_GRAPH;
  SpectralDiagnostics diagnostics;

  // Node-indexed outputs preserve snapshot.nodes order.
  std::vector<std::uint32_t> h_ids;
  std::unordered_map<std::uint32_t, std::size_t> id_to_index;
  Eigen::MatrixXd weight_matrix;
  Eigen::VectorXd degree;
  Eigen::MatrixXd normalized_laplacian;
  Eigen::VectorXd fiedler;
  FiedlerHistory fiedler_by_h_id;

  // One computed weight per snapshot.edges entry, in the same order.
  std::vector<double> edge_weights;

  // labels are 0/1 when diagnostics.cut_valid, otherwise -1.  The two order
  // vectors contain the same Fiedler sort expressed as indices and H-node IDs.
  std::vector<int> labels;
  std::vector<std::size_t> spectral_order;
  std::vector<std::uint32_t> ordered_h_ids;
  // Convenience projection of ordered_h_ids onto nodes marked active_anchor.
  std::vector<std::uint32_t> ordered_active_h_ids;

  bool success() const {
    return status == SpectralSolveStatus::SUCCESS ||
           status == SpectralSolveStatus::SUCCESS_NO_VALID_CUT;
  }

  bool has_valid_cut() const {
    return success() && diagnostics.cut_valid;
  }
};

const char* SpectralSolveStatusName(SpectralSolveStatus status);

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_SPECTRAL_TYPES_H_
