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
 *
 * Keep completion separate from transient planning failures.  In particular,
 * a missing robot-to-DTG seed or a disconnected active frontier must never be
 * interpreted as the end of exploration.
 */
enum class GlobalPlanStatus{
    SUCCESS = 0,
    NO_ACTIVE_FRONTIER,
    NO_FEASIBLE_SEED,
    GRAPH_DISCONNECTED,
    SPECTRAL_FAILED,
    SPECTRAL_TIMEOUT,
    SPECTRAL_UNSTABLE,
    ROUTE_REJECTED_BY_REGRET,
    FRONTIER_STALLED,
    MAP_FINISHED,
    PATH_FAILED,
    INTERNAL_ERROR
};

enum class GlobalPlanMethod{
    NONE = 0,
    SPECTRAL,
    SPECTRAL_V2_SOFT,
    EOHDT_FALLBACK,
    EOHDT_DISABLED_SPECTRAL
};

enum class FrontierState{
    NEW = 0,
    ACTIVE,
    SUSPECT,
    DEFERRED,
    QUARANTINED,
    // Eigen's sparse ordering headers expose a legacy DEAD macro, so the
    // proposal's DEAD lifecycle state uses an unambiguous C++ identifier.
    DEAD_FRONTIER
};

enum class RecoveryReason{
    NONE = 0,
    LOCK_DEBT,
    REGION_STALL,
    PARTITION_CHANGE,
    SPECTRAL_TIMEOUTS,
    REPEATED_TARGET
};

enum class SpectralRouteDecision{
    NOT_EVALUATED = 0,
    ACCEPTED,
    MODE_INACTIVE,
    NO_STABLE_PARTITION,
    CANDIDATE_BUILD_FAILED,
    REGRET_REJECTED,
    COMBINED_COST_REJECTED,
    CANDIDATE_PATH_FAILED,
    RECOVERY_BASELINE
};

enum class RegionExecState{
    CANDIDATE = 0,
    ACTIVE,
    DORMANT,
    DONE,
    STALE,
    UNREACHABLE
};

enum class PartitionChangeType{
    NONE = 0,
    CREATED,
    SPLIT,
    MERGE,
    RESET
};

enum class SpectralGraphMode{
    ACTIVE_COMPLETE = 0,
    SUPPORT_SPARSE = 1
};

/**
 * @brief Online execution policy around the pure spectral eigensolver.
 *
 * Numerical edge-weight and eigensolver settings live in SpectralConfig
 * (spectral_types.h).  This structure controls when a valid embedding is
 * allowed to become a persistent exploration-region decision.
 */
struct SpectralExecutionConfig{
    bool enabled = false;
    bool fallback_to_eohdt = true;
    bool partition_enabled = true;
    bool region_lock_enabled = true;
    bool log_diagnostics = true;
    bool async_solve = true;
    bool corridor_compression = true;
    SpectralGraphMode graph_mode = SpectralGraphMode::SUPPORT_SPARSE;

    int min_partition_nodes = 6;
    int trigger_persistence = 5;
    int release_persistence = 3;
    int region_done_cycles = 3;
    int unreachable_confirm_cycles = 3;
    int max_spectral_epoch_age = 2;
    int clearance_binary_steps = 5;
    int clearance_max_samples = 64;
    int spectral_knn = 5;
    int dirty_node_changes = 3;
    int dirty_edge_changes = 5;
    int partition_grace_epochs = 8;
    int late_stage_frontier_count = 6;
    int late_stage_reactivate_frontier_count = 12;
    int late_stage_no_cut_epochs = 10;
    int frontier_low_gain_cycles = 2;
    int frontier_actual_gain_eps = 10;
    int repeat_target_limit = 3;
    int spectral_timeout_limit = 3;

    double lambda2_threshold = 0.15;
    double eigengap_threshold = 0.02;
    double ncut_threshold = 0.30;
    double lambda2_ema_alpha = 0.90;
    double lambda2_ratio = 1.0;
    double region_match_threshold = 0.30;
    double label_hysteresis = 0.02;
    double route_jump_weight = 1.0;
    double route_terminal_bias = 0.25;
    double spectral_view_weight = 0.0;
    double cross_region_weight = 0.3;
    double spectral_time_budget_ms = 10.0;
    double update_period = 0.20;
    double spectral_min_update_interval = 0.5;
    double spectral_max_update_interval = 3.0;

    // Continuous partition confidence.  The five weights are normalized at
    // start-up, so experiments may tune them without maintaining an exact
    // floating-point sum of one.
    double confidence_weight_ncut = 0.30;
    double confidence_weight_eigengap = 0.25;
    double confidence_weight_relative_lambda2 = 0.15;
    double confidence_weight_bottleneck = 0.20;
    double confidence_weight_balance = 0.10;
    double confidence_ncut_soft = 0.60;
    double confidence_eigengap_soft = 0.15;
    double partition_confidence_on = 0.70;
    double partition_confidence_off = 0.40;

    // Spectral-v2 route protection and recovery policy.
    double max_route_regret = 0.05;
    double switch_penalty_base = 2.0;
    double revisit_penalty_weight = 1.0;
    double neighbor_override_distance = 2.0;
    double neighbor_override_margin = 0.5;
    double lock_debt_decay = 0.90;
    double lock_debt_max = 8.0;
    double recovery_duration = 5.0;

    // Late-stage and effective-frontier watchdogs.  unknown_num is used as a
    // cheap online gain proxy; actual executed-target gain is tracked
    // separately by FrontierRuntimeState.
    double late_stage_total_gain = 50.0;
    double frontier_expected_gain_eps = 1.0;
    double frontier_quarantine_time = 5.0;
    double region_stall_window = 8.0;
    double region_stall_timeout = 12.0;

    // Late-stage auto-disable: when the fraction of solves producing valid
    // cuts drops below this threshold for a sustained window, disable spectral.
    int    auto_disable_min_epochs = 30;
    int    auto_disable_window = 50;
    int    auto_disable_min_window = 20;
    int    auto_disable_cut_pct = 15;  // percentage
};

struct RegionState{
    int id = -1;
    RegionExecState exec_state = RegionExecState::CANDIDATE;
    std::unordered_set<uint32_t> node_ids;
    std::unordered_set<uint32_t> active_anchor_ids;
    double spectral_center = 0.0;
    uint64_t last_seen_spectral_epoch = 0;
    uint64_t last_counted_frontier_epoch = 0;
    uint64_t last_unreachable_frontier_epoch = 0;
    uint64_t unreachable_spectral_epoch = 0;
    int empty_streak = 0;
    int unreachable_streak = 0;
    int age_epochs = 0;
    int weak_epochs = 0;
    double confidence = 0.0;
};

struct FrontierRuntimeState{
    FrontierState state = FrontierState::NEW;
    int owner_region = -1;
    int low_gain_streak = 0;
    int path_failure_streak = 0;
    int repeat_count = 0;
    int last_unknown_num = -1;
    int selected_unknown_before = -1;
    double last_actual_gain = 0.0;
    double last_seen_time = 0.0;
    double last_selected_time = -1.0;
    double last_repeat_count_time = -1.0;
    double quarantine_until = 0.0;
    uint64_t last_seen_frontier_epoch = 0;
    uint64_t last_gain_check_frontier_epoch = 0;
};

struct GlobalPlanDiagnostics{
    GlobalPlanStatus status = GlobalPlanStatus::INTERNAL_ERROR;
    GlobalPlanMethod method = GlobalPlanMethod::NONE;
    bool used_fallback = false;
    bool partition_valid = false;
    std::string detail;
    double lambda2 = 0.0;
    double lambda3 = 0.0;
    double eigengap_23 = 0.0;
    double ncut = 0.0;
    double residual_norm = 0.0;
    double spectral_solve_ms = 0.0;
    int spectral_solver_type = 0;
    size_t spectral_solver_iterations = 0;
    bool spectral_warm_start_used = false;
    uint64_t spectral_epoch = 0;
    uint64_t dtg_version = 0;
    uint64_t frontier_version = 0;
    int active_region_id = -1;
    int partition_change = static_cast<int>(PartitionChangeType::NONE);
    size_t active_anchor_count = 0;
    size_t route_size = 0;
    int spectral_mode = 0;
    int route_decision = static_cast<int>(SpectralRouteDecision::NOT_EVALUATED);
    int recovery_reason = static_cast<int>(RecoveryReason::NONE);
    double partition_confidence = 0.0;
    double confidence_ncut = 0.0;
    double confidence_eigengap = 0.0;
    double confidence_relative_lambda2 = 0.0;
    double confidence_bottleneck = 0.0;
    double confidence_balance = 0.0;
    double baseline_path_cost = 0.0;
    double spectral_path_cost = 0.0;
    double selected_path_cost = 0.0;
    double baseline_combined_cost = 0.0;
    double spectral_combined_cost = 0.0;
    double route_regret = 0.0;
    double dynamic_switch_penalty = 0.0;
    double lock_debt = 0.0;
    size_t baseline_switches = 0;
    size_t spectral_switches = 0;
    size_t effective_frontier_count = 0;
    size_t quarantined_frontier_count = 0;
    size_t raw_spectral_nodes = 0;
    size_t compressed_spectral_nodes = 0;
    uint64_t submitted_spectral_jobs = 0;
    uint64_t stale_spectral_results = 0;
    uint64_t timed_out_spectral_results = 0;
    uint64_t spectral_mode_toggles = 0;
    uint64_t recovery_count = 0;
    uint64_t frontier_reassignments = 0;
    uint64_t repeated_targets = 0;
    double label_change_rate = 0.0;
    double spectral_route_utilization = 0.0;
};

template <typename HeadNode, typename TailNode>
struct DTG_edge{
    DTG_edge(){
        length_ = 2e5;
        length_s_ = 1e5;
        clearance_ = std::numeric_limits<double>::quiet_NaN();
        betweenness_ = 0.0;
        // head_ = 0;
        // tail_ = 0;
        // head_s_ = 0;
        // tail_s_ = 0;
        e_flag_ = 0;
    }
    DTG_edge(shared_ptr<HeadNode> &h, shared_ptr<TailNode> &t){
        // head_ = h->id_;
        // head_s_ = h->id_;
        // tail_ = t->id_;
        // tail_s_ = t->id_;
        head_n_ = h;
        // head_n_s_ = h;
        tail_n_ = t;
        // tail_n_s_ = t;

        length_ = 2e5;
        length_s_ = 1e5;
        clearance_ = std::numeric_limits<double>::quiet_NaN();
        betweenness_ = 0.0;
        e_flag_ = 0;
    }
    double length_, length_s_;          // to be checked
    double clearance_, betweenness_;
    uint8_t e_flag_;                     //0(to be erased)(have global)(have local) (fake edge)(all in local)(showed)(checked)
    // uint32_t head_, tail_;
    // uint32_t head_swarm_, tail_swarm_;
    list<Eigen::Vector3d> path_;//, path_swarm_;    //head--->tail
    shared_ptr<HeadNode> head_n_;//, head_n_s_;
    shared_ptr<TailNode> tail_n_;//, tail_n_s_;
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
    uint32_t id_; // (fid or hid)
    uint8_t vid_; // only for fnodes
    uint8_t root_id_;
    uint8_t flag_;                      //0000 00(close)(h:1 f:0)
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
    uint8_t f_flag_;     //0000 0(local gvp send)(local gvp)0
    uint8_t vid_;   // also the id of center vp

    // list<uint8_t> vps_;
};

// struct F_node{
//     F_node(){
//         f_flag_ = 0;
//         exploring_id_ = 0;
//     }
//     EROIStruct::EroiNode *eroi_;
//     uint8_t exploring_id_;
//     // Eigen::Vector3d upbd_, lowbd_;
//     float g_;
//     shared_ptr<DTG_edge<H_node, F_node>> hf_edge_;
//     uint8_t f_flag_;     //0000 0(local gvp send)(local gvp)0
//     shared_ptr<DTG_sch_node> sch_node_;
// };


struct H_node{
    H_node(){
        h_flags_ = 0;
        last_maintain_t_ = 0.0;
        fiedler_val_ = 0.0;
        spectral_region_id_ = -1;
        spectral_epoch_ = 0;
    }
    uint32_t id_;
    Eigen::Vector3d pos_;
    list<shared_ptr<DTG_edge<H_node, FC_node>>> hf_edges_;
    list<shared_ptr<DTG_edge<H_node, H_node>>> hh_edges_;
    //for path search
    uint8_t h_flags_;     //000(search target) (in tsp update list)(global gvp)(local gvp)(close)
    shared_ptr<DTG_sch_node> sch_node_;
    double last_maintain_t_;
    // Spectral values are valid only when spectral_epoch_ matches the latest
    // published spectral snapshot.
    double fiedler_val_;
    int spectral_region_id_;
    uint64_t spectral_epoch_;
};
}



#endif

//                 @@@@@
//                 ##**#%@@
//           @@@@%*-:::--+%
//         @@%#*%#-::::=:.+@%
//        @#=:..-#-.:::+++#@%
//       @#:...:.:#-...:*@@%
//     @%=........-#:++=+%@%
//    @%- . =:.....+*-...:#@%
//    @%: . =#......#: ... +@%
//    @%:   -:.....:#..... :%@
//      @*: ...... -* ..... =@%
//       @%=:... . -= ..... +@%
//        @@*==--::--:...  .%@%
//        @+=++++=======--=*@%
//       @#++++==========+@@
//      @#++++====-=======*%
//     %*+++===--===----===+%
//    #*++===--=*%%%#=-=====*@
//  *++==--+%@@%%%@%*--====#@
// #*====-=#@        @#=--===%@
// #*+=--+##          @@*=--=+#@
// #*+++=*#              @%+=+++%
// #*++=+#*                #++++*#
// #*++=+#                  #=+++#
// #*+++#                   %*=++*#
// #++=*%                   #%+=++#
