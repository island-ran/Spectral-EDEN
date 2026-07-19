#ifndef MR_DTG_PLUS_SPECTRAL_SNAPSHOT_BUILDER_H_
#define MR_DTG_PLUS_SPECTRAL_SNAPSHOT_BUILDER_H_

#include <mr_dtg_plus/spectral_router.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>

namespace DTGPlus {

// This enum deliberately lives in the pure spectral module instead of using
// the ROS-facing SpectralGraphMode declared by mr_dtg_plus_structures.h.
enum class SpectralSupportMode {
  ACTIVE_COMPLETE = 0,
  SUPPORT_SPARSE = 1
};

enum class SpectralSnapshotBuildStatus {
  SUCCESS = 0,
  INVALID_CONFIG,
  EMPTY_RAW_GRAPH,
  DUPLICATE_NODE_ID,
  NO_ACTIVE_ANCHOR,
  NO_RAW_EDGES,
  UNKNOWN_EDGE_ENDPOINT,
  SELF_LOOP,
  DUPLICATE_EDGE,
  INVALID_EDGE_LENGTH,
  ACTIVE_ANCHORS_DISCONNECTED,
  PATH_RECONSTRUCTION_FAILED,
  MISSING_EDGE_CLEARANCE,
  EMPTY_SUPPORT_GRAPH,
  COMPRESSION_FAILED,
  TOO_FEW_NODES,
  TOO_MANY_NODES,
  BETWEENNESS_FAILED
};

const char* SpectralSnapshotBuildStatusName(
    SpectralSnapshotBuildStatus status);

struct SpectralSnapshotBuildConfig {
  SpectralSupportMode support_mode = SpectralSupportMode::SUPPORT_SPARSE;
  std::size_t knn = 5U;
  bool corridor_compression = true;
  SpectralWeightMode weight_mode = SpectralWeightMode::DISTANCE;
  bool compute_betweenness = false;
  std::size_t min_spectral_nodes = 3U;
  std::size_t max_spectral_nodes = 60U;
  double numeric_epsilon = 1.0e-12;
};

// Timings are kept in a worker-owned value object, including the Ncut sweep
// measured separately by SpectralRouter.
struct SpectralStageTimings {
  double raw_copy_ms = 0.0;
  double support_graph_ms = 0.0;
  double compression_ms = 0.0;
  double betweenness_ms = 0.0;
  double solver_ms = 0.0;
  double ncut_ms = 0.0;
  double worker_total_ms = 0.0;
  double confidence_ms = 0.0;
};

struct SpectralSnapshotBuildOutput {
  SpectralSnapshotBuildStatus status =
      SpectralSnapshotBuildStatus::EMPTY_RAW_GRAPH;
  SpectralGraphSnapshot snapshot;
  std::uint64_t graph_version = 0U;
  std::uint64_t frontier_version = 0U;
  std::size_t raw_node_count = 0U;
  std::size_t raw_edge_count = 0U;
  std::size_t support_node_count_before_compression = 0U;
  std::size_t support_edge_count_before_compression = 0U;
  SpectralStageTimings timings;
  std::string reason;

  bool success() const {
    return status == SpectralSnapshotBuildStatus::SUCCESS;
  }
};

struct SpectralWorkerOutput {
  SpectralSnapshotBuildOutput build;
  SpectralResult result;
  SpectralStageTimings timings;

  bool success() const {
    return build.success() && result.success();
  }
};

// Pure support-graph construction.  It owns no ROS state and reads only the
// immutable raw value passed by the caller.
SpectralSnapshotBuildOutput BuildSpectralGraphSnapshot(
    const RawSpectralSnapshot& raw,
    const SpectralSnapshotBuildConfig& config =
        SpectralSnapshotBuildConfig());

// Pure transform shared by the worker builder and deterministic unit tests.
bool CompressSpectralDegreeTwoChains(
    const SpectralGraphSnapshot& input,
    const std::unordered_set<std::uint32_t>& preserved_h_ids,
    SpectralGraphSnapshot* output, std::string* reason);

// Builds the support graph and invokes a copied pure spectral router.  A
// caller can move RawSpectralSnapshot into its std::async closure and invoke
// this function without capturing any MultiDtgPlus object.
SpectralWorkerOutput BuildAndSolveSpectral(
    const RawSpectralSnapshot& raw,
    const SpectralSnapshotBuildConfig& build_config,
    const SpectralRouter& router,
    const FiedlerHistory& previous_fiedler = FiedlerHistory());

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_SPECTRAL_SNAPSHOT_BUILDER_H_
