#ifndef MR_DTG_PLUS_STRUCTURES_H_
#define MR_DTG_PLUS_STRUCTURES_H_
#include <ros/ros.h>
#include <thread>
#include <Eigen/Eigen>
#include <vector>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <cstddef>
#include <limits>

#include <eroi/eroi.h>
using namespace std;

namespace DTGPlus{
struct H_node;
struct F_node;
enum Fstate{
    l_active,
    g_active,
    uncoverable
};

enum Hstate{
    L_ACTIVE,
    G_ACTIVE,
    L_FREE,
    BLOCKED
};

/**
 * @brief Result of the global region-routing stage.
 */
enum class GlobalPlanStatus{
    SUCCESS = 0,
    NO_ACTIVE_FRONTIER,
    NO_FEASIBLE_SEED,
    GRAPH_DISCONNECTED,
    MAP_FINISHED,
    PATH_FAILED,
    INTERNAL_ERROR
};

enum class GlobalPlanMethod{
    NONE = 0,
    SPECTRAL,
    EOHDT_BASELINE
};

// ── V4: Global planner mode (startup binary choice) ──
enum class GlobalPlannerMode{
    EDEN_BASELINE = 0,
    SPECTRAL_V4 = 1
};

// ── V4: Cut backend selection ──
enum class CutBackend{
    INCREMENTAL_FIEDLER = 0,
    LANDMARK_NYSTROM,
    LOCAL_APPR,
    UNAVAILABLE
};

// ── V4: Unified budget-bounded configuration ──
struct SpectralV4Config{
    bool    log_diagnostics = true;

    // Skeleton
    int     skeleton_max_nodes = 512;
    int     max_leaf_regions = 6;
    int     max_recursive_depth = 3;
    double  length_decay = 1.0;
    double  clearance_reference = 1.0;
    double  clearance_power = 1.0;
    double  min_edge_weight = 1.0e-6;
    int     clearance_binary_steps = 5;
    int     clearance_max_samples = 64;

    // Incremental Fiedler
    int     exact_max_nodes = 48;
    int     exact_max_iterations = 12;
    double  exact_residual_threshold = 1.0e-4;

    // Nyström
    int     landmark_count = 32;
    int     landmark_max_count = 48;
    int     nearest_landmarks_per_node = 4;
    int     nystrom_max_support = 512;
    double  nystrom_residual_threshold = 5.0e-4;

    // Local APPR
    double  ppr_alpha = 0.15;
    double  ppr_epsilon = 1.0e-4;
    int     ppr_max_pushes = 20000;
    int     ppr_max_support_nodes = 512;

    // Bisection acceptance
    double  max_ncut = 0.30;
    double  min_balance = 0.20;
    int     proposal_confirm_versions = 2;
    double  proposal_jaccard_threshold = 0.80;
    int     max_region_version_lag = 2;
    double  region_invalidation_ratio = 0.30;

    // Target commitment
    int     local_frontier_top_k = 8;
    int     max_adjacent_region_entries = 2;
    double  min_target_commit_sec = 1.5;
    double  target_switch_abs_margin_sec = 0.7;
    double  target_switch_relative_margin = 0.10;
    double  actual_gain_epsilon = 10.0;

    // Worker
    double  main_thread_delta_budget_ms = 2.0;
};

// ── V4: GraphDelta for incremental skeleton updates ──
struct NodeInsert{ uint32_t h_id; Eigen::Vector3d pos; uint32_t degree; };
struct NodeErase{ uint32_t h_id; };
struct EdgeUpsert{ uint32_t from; uint32_t to; double length; double clearance; };
struct EdgeErase{ uint32_t from; uint32_t to; };
struct FrontierAnchorUpdate{
    uint32_t h_id = 0;
    bool active = false;
    double expected_gain = 0.0;
    std::vector<uint32_t> frontier_ids;
};

struct GraphDelta{
    uint64_t graph_version = 0;
    uint64_t frontier_version = 0;
    std::vector<NodeInsert> node_inserts;
    std::vector<NodeErase> node_erases;
    std::vector<EdgeUpsert> edge_upserts;
    std::vector<EdgeErase> edge_erases;
    std::vector<FrontierAnchorUpdate> frontier_updates;
    uint32_t robot_h_id = 0;
    bool robot_changed = false;

    bool empty() const{
        return node_inserts.empty() && node_erases.empty() &&
               edge_upserts.empty() && edge_erases.empty() &&
               frontier_updates.empty() && !robot_changed;
    }
};

// ── V4: Binary region proposal (unified output of all three backends) ──
struct BinaryRegionProposal{
    uint64_t graph_version = 0;
    uint64_t frontier_version = 0;
    CutBackend backend = CutBackend::UNAVAILABLE;

    int parent_region_id = -1;
    std::vector<uint32_t> side_a;
    std::vector<uint32_t> side_b;

    double ncut = std::numeric_limits<double>::infinity();
    double balance = 0.0;
    double residual = std::numeric_limits<double>::infinity();
    double support_coverage = 0.0;
    bool truncated = false;
    bool valid = false;

    uint64_t deterministic_signature = 0;
};

// ── V4: Region tree node ──
struct RegionTreeNode{
    int id = -1;
    int parent_id = -1;
    int child_a = -1;
    int child_b = -1;
    std::unordered_set<uint32_t> h_ids;
    uint64_t accepted_graph_version = 0;
    double cut_quality = 0.0;
    int depth = 0;
    bool leaf = true;
};

// ── V4: Region quotient graph ──
struct QuotientRegion{
    int region_id = -1;
    double total_gain = 0.0;
    std::vector<uint32_t> frontier_ids;
    std::vector<uint32_t> entry_h_ids;
};

struct QuotientEdge{
    int from_region = -1;
    int to_region = -1;
    uint32_t entry_from = 0;
    uint32_t entry_to = 0;
    double traversal_time = 0.0;
};

// ── V4: Immutable region state snapshot ──
struct RegionStateSnapshot{
    uint64_t graph_version = 0;
    uint64_t frontier_version = 0;
    CutBackend source_backend = CutBackend::UNAVAILABLE;
    std::vector<RegionTreeNode> region_tree;
    std::vector<QuotientRegion> quotient_regions;
    std::vector<QuotientEdge> quotient_edges;
    std::unordered_map<uint32_t, int> h_to_region;
    double ncut = std::numeric_limits<double>::infinity();
    double balance = 0.0;
    double residual = std::numeric_limits<double>::infinity();
    bool truncated = false;
    bool valid = false;
};

// ── V4: Committed target ──
enum class MacroTargetType{
    FRONTIER = 0,
    REGION_ENTRY
};

struct CommittedTarget{
    bool valid = false;
    MacroTargetType type = MacroTargetType::FRONTIER;
    uint32_t frontier_id = 0;
    uint8_t viewpoint_id = 0;
    uint32_t anchor_h_id = 0;
    uint32_t entry_from_h_id = 0;
    uint32_t entry_to_h_id = 0;
    int region_id = -1;

    uint64_t selected_graph_version = 0;
    double accepted_time = 0.0;
    double exact_path_time = 0.0;
    double selection_cost = 0.0;
    double expected_gain = 0.0;
    double actual_gain_since_accept = 0.0;
    double initial_unknown_count = 0.0;
    double last_observed_unknown_count = 0.0;
    int consecutive_low_gain = 0;
};

// ── V4: Skeleton structures ──
struct SkeletonEdge{
    uint32_t from = 0;
    uint32_t to = 0;
    double length = 0.0;
    double min_clearance = 0.0;
    uint32_t raw_edge_count = 0;
    uint64_t version = 0;
};

struct ContractedSkeleton{
    uint64_t graph_version = 0;
    std::vector<uint32_t> node_ids;
    std::unordered_map<uint32_t, Eigen::Vector3d> node_positions;
    std::unordered_map<uint32_t, bool> node_is_active_anchor;
    std::unordered_map<uint32_t, double> node_frontier_gain;
    std::unordered_map<uint32_t, std::vector<uint32_t>> node_frontier_ids;
    std::vector<SkeletonEdge> edges;
};

struct GlobalPlanDiagnostics{
    GlobalPlanStatus status = GlobalPlanStatus::INTERNAL_ERROR;
    GlobalPlanMethod method = GlobalPlanMethod::NONE;
    std::string detail;
    uint64_t dtg_version = 0;
    uint64_t frontier_version = 0;
    size_t active_anchor_count = 0;
    size_t route_size = 0;
    double selected_path_cost = 0.0;
    int planner_mode = 0;
    int v4_backend = 0;
    uint64_t worker_graph_version = 0;
    uint64_t region_state_version = 0;
    uint32_t committed_target_id = 0;
    double graph_delta_ms = 0.0;
    double planning_ms = 0.0;
};

template <typename HeadNode, typename TailNode>
struct DTG_edge{
    DTG_edge(){
        length_ = 2e5;
        length_s_ = 1e5;
        clearance_ = std::numeric_limits<double>::quiet_NaN();
        e_flag_ = 0;
    }
    DTG_edge(shared_ptr<HeadNode> &h, shared_ptr<TailNode> &t){
        head_n_ = h;
        tail_n_ = t;
        length_ = 2e5;
        length_s_ = 1e5;
        clearance_ = std::numeric_limits<double>::quiet_NaN();
        e_flag_ = 0;
    }
    double length_, length_s_;
    double clearance_;
    uint8_t e_flag_;
    list<Eigen::Vector3d> path_;
    shared_ptr<HeadNode> head_n_;
    shared_ptr<TailNode> tail_n_;
};
struct DTG_sch_node{
    DTG_sch_node(const float &g, const float &f, const uint32_t &id, const Eigen::Vector3d &p){
        g_ = g;
        f_ = f;
        id_ = id;
        pos_ = p;
        flag_ = 0;
        parent_ = NULL;
    }
    Eigen::Vector3d pos_;
    float g_, f_;
    uint32_t id_;
    uint8_t vid_;
    uint8_t root_id_;
    uint8_t flag_;
    shared_ptr<DTG_sch_node> parent_;
};

class ACompare {
public:
  bool operator()(shared_ptr<DTG_sch_node> node1, shared_ptr<DTG_sch_node> node2) {
    return node1->f_ > node2->f_;
  }
};

class DCompare {
public:
  bool operator()(shared_ptr<DTG_sch_node> node1, shared_ptr<DTG_sch_node> node2) {
    return node1->g_ > node2->g_;
  }
};

typedef priority_queue<shared_ptr<DTG_sch_node>, vector<shared_ptr<DTG_sch_node>>, DCompare> prio_D;
typedef priority_queue<shared_ptr<DTG_sch_node>, vector<shared_ptr<DTG_sch_node>>, ACompare> prio_A;


struct FC_node{
    uint32_t fid_;
    shared_ptr<DTG_sch_node> sch_node_;
    shared_ptr<DTG_edge<H_node, FC_node>> hf_edge_;
    uint8_t f_flag_;
    uint8_t vid_;
};


struct H_node{
    H_node(){
        h_flags_ = 0;
        last_maintain_t_ = 0.0;
    }
    uint32_t id_;
    Eigen::Vector3d pos_;
    list<shared_ptr<DTG_edge<H_node, FC_node>>> hf_edges_;
    list<shared_ptr<DTG_edge<H_node, H_node>>> hh_edges_;
    uint8_t h_flags_;
    shared_ptr<DTG_sch_node> sch_node_;
    double last_maintain_t_;
};
}

#endif
