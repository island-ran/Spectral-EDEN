#include <mr_dtg_plus/spectral_v4_backends.h>
#include <mr_dtg_plus/spectral_router.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

namespace DTGPlus {

bool SolveIncrementalFiedler(
    const ContractedSkeleton& skeleton,
    const SpectralV4Config& config,
    const V4BackendContext& context,
    const FiedlerHistory& warm_start,
    BinaryRegionProposal* proposal,
    FiedlerHistory* next_warm_start,
    std::string* reason) {

    const size_t n = skeleton.node_ids.size();
    if(n < 3){
        *reason = "skeleton too small for Fiedler bisection";
        return false;
    }
    if(n > static_cast<size_t>(config.exact_max_nodes)){
        *reason = "skeleton exceeds exact Fiedler budget";
        return false;
    }

    SpectralGraphSnapshot snapshot;
    snapshot.graph_version = skeleton.graph_version;

    std::unordered_map<uint32_t, size_t> id_to_index;
    for(size_t i = 0; i < n; ++i){
        id_to_index[skeleton.node_ids[i]] = i;
        bool active = false;
        auto it = skeleton.node_is_active_anchor.find(skeleton.node_ids[i]);
        if(it != skeleton.node_is_active_anchor.end()) active = it->second;
        snapshot.nodes.emplace_back(skeleton.node_ids[i], active);
    }

    for(const auto &edge : skeleton.edges){
        if(id_to_index.count(edge.from) && id_to_index.count(edge.to)){
            SpectralEdgeInput input(edge.from, edge.to, edge.length);
            input.clearance = edge.min_clearance;
            snapshot.edges.emplace_back(input);
        }
    }

    if(snapshot.edges.empty()){
        *reason = "skeleton has no edges for spectral solve";
        return false;
    }

    SpectralConfig numeric;
    numeric.weight_mode = SpectralWeightMode::DISTANCE_CLEARANCE;
    numeric.length_decay = config.length_decay;
    numeric.clearance_reference = config.clearance_reference;
    numeric.clearance_power = config.clearance_power;
    numeric.min_edge_weight = config.min_edge_weight;
    numeric.min_spectral_nodes = 3;
    numeric.max_spectral_nodes = static_cast<size_t>(config.exact_max_nodes);
    numeric.dense_solver_max_nodes =
        static_cast<size_t>(std::max(3, std::min(12, config.exact_max_nodes)));
    numeric.iterative_max_iterations =
        static_cast<size_t>(std::max(1, config.exact_max_iterations));
    numeric.iterative_tolerance = config.exact_residual_threshold * 0.1;
    numeric.max_residual_norm = config.exact_residual_threshold;
    numeric.min_cluster_size = 1;
    numeric.min_cluster_volume_fraction =
        std::max(0.0, std::min(0.49, config.min_balance));

    SpectralRouter router(numeric);
    SpectralResult result = router.Solve(snapshot, warm_start);

    if(!result.success()){
        *reason = "Fiedler solve failed: " +
            std::string(SpectralSolveStatusName(result.status));
        return false;
    }

    if(!result.has_valid_cut()){
        *reason = "Fiedler solve found no valid cut";
        return false;
    }

    proposal->backend = CutBackend::INCREMENTAL_FIEDLER;
    proposal->graph_version = skeleton.graph_version;
    proposal->frontier_version = context.frontier_version;
    proposal->parent_region_id = context.parent_region_id;
    proposal->ncut = result.diagnostics.ncut;
    proposal->balance = std::min(
        result.diagnostics.cut_left_volume,
        result.diagnostics.cut_right_volume) /
        std::max(result.diagnostics.cut_left_volume +
                 result.diagnostics.cut_right_volume, 1.0e-12);
    proposal->residual = result.diagnostics.residual_norm;
    proposal->support_coverage = 1.0;
    proposal->truncated = false;
    proposal->valid = std::isfinite(proposal->ncut) &&
        std::isfinite(proposal->residual) &&
        proposal->residual <= config.exact_residual_threshold;

    for(size_t i = 0; i < result.h_ids.size() && i < result.labels.size(); ++i){
        if(result.labels[i] == 0) proposal->side_a.push_back(result.h_ids[i]);
        else if(result.labels[i] == 1) proposal->side_b.push_back(result.h_ids[i]);
    }

    uint64_t sig = 0;
    for(auto id : proposal->side_a)
        sig ^= static_cast<uint64_t>(id) * 0x9E3779B185EBCA87ULL;
    proposal->deterministic_signature = sig;
    *next_warm_start = result.fiedler_by_h_id;

    *reason = "Incremental Fiedler: Ncut=" +
        std::to_string(proposal->ncut) + " balance=" +
        std::to_string(proposal->balance);
    return proposal->valid;
}

}  // namespace DTGPlus
