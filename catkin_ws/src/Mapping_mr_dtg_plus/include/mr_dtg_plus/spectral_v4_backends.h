#ifndef MR_DTG_PLUS_SPECTRAL_V4_BACKENDS_H_
#define MR_DTG_PLUS_SPECTRAL_V4_BACKENDS_H_

#include <mr_dtg_plus/mr_dtg_plus_structures.h>
#include <mr_dtg_plus/spectral_types.h>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace DTGPlus {

struct V4BackendContext {
  int parent_region_id = -1;
  uint64_t frontier_version = 0;
  uint32_t robot_h_id = 0;
};

inline double V4LengthScale(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config) {
  if (skeleton.edges.empty() || config.length_decay <= 0.0)
    return std::numeric_limits<double>::infinity();
  std::vector<double> lengths;
  lengths.reserve(skeleton.edges.size());
  for (const auto& edge : skeleton.edges)
    if (std::isfinite(edge.length) && edge.length > 0.0)
      lengths.push_back(edge.length);
  if (lengths.empty())
    return std::numeric_limits<double>::infinity();
  const size_t middle = lengths.size() / 2U;
  std::nth_element(
      lengths.begin(), lengths.begin() + middle, lengths.end());
  return std::max(
      lengths[middle] / config.length_decay, 1.0e-9);
}

inline double V4EdgeWeight(
    const SkeletonEdge& edge,
    const SpectralV4Config& config,
    double length_scale) {
  const double length_term = std::isfinite(length_scale)
      ? std::exp(-std::max(0.0, edge.length) / length_scale)
      : 1.0;
  const double clearance = std::isfinite(edge.min_clearance)
      ? std::max(0.0, edge.min_clearance)
      : config.clearance_reference;
  const double clearance_term = std::pow(
      std::min(1.0, (clearance + 1.0e-9) /
                        (config.clearance_reference + 1.0e-9)),
      config.clearance_power);
  return std::max(
      config.min_edge_weight, length_term * clearance_term);
}

bool SolveIncrementalFiedler(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    const V4BackendContext& context,
    const FiedlerHistory& warm_start,
    BinaryRegionProposal* proposal,
    FiedlerHistory* next_warm_start,
    std::string* reason);

bool SolveNystromBisection(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    const V4BackendContext& context,
    std::vector<uint32_t>* persistent_landmarks,
    BinaryRegionProposal* proposal,
    std::string* reason);

bool SolveLocalAppr(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    const V4BackendContext& context,
    BinaryRegionProposal* proposal,
    std::string* reason);

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_SPECTRAL_V4_BACKENDS_H_
