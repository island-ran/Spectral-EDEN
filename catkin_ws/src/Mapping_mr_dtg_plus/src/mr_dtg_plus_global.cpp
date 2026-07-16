#include <mr_dtg_plus/mr_dtg_plus.h>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace DTGPlus;

namespace {
constexpr double kUnreachableDistance = 999998.0;
constexpr double kBlockedEdgeDistance = 200000.0;
}

uint64_t MultiDtgPlus::HPairKey(const uint32_t h1, const uint32_t h2){
    const uint32_t lo = std::min(h1, h2);
    const uint32_t hi = std::max(h1, h2);
    return (static_cast<uint64_t>(lo) << 32U) | static_cast<uint64_t>(hi);
}

void MultiDtgPlus::MarkTopologyChanged(){
    ++dtg_version_;
    ++topology_changes_since_submit_;
    spectral_dirty_ = true;
    // A path is valid only for the exact H-H graph on which it was found.
    // Clearing here also bounds memory after long exploration runs.
    h_dist_map_.clear();
}

void MultiDtgPlus::MarkFrontierChanged(){
    ++frontier_version_;
    ++frontier_changes_since_submit_;
    spectral_dirty_ = true;
}

bool MultiDtgPlus::HasPendingRegions() const{
    for(const auto &h : H_list_){
        if(HasEffectiveFrontier(h, -1)) return true;
    }
    return false;
}

void MultiDtgPlus::PublishGlobalPlanDiagnostics(){
    if(!spectral_exec_config_.log_diagnostics) return;

    std_msgs::Float64MultiArray message;
    message.layout.dim.resize(1);
    message.layout.dim[0].label =
        "stamp,status,method,fallback,dtg_version,frontier_version,spectral_epoch,"
        "lambda2,lambda3,eigengap23,ncut,residual,solve_ms,partition_valid,"
        "active_region,partition_change,active_anchors,route_size,spectral_mode,"
        "route_decision,recovery_reason,partition_confidence,q_ncut,q_eigengap,"
        "q_relative_lambda2,q_bottleneck,q_balance,baseline_path,spectral_path,"
        "selected_path,baseline_combined,spectral_combined,route_regret,"
        "switch_penalty,lock_debt,baseline_switches,spectral_switches,"
        "effective_frontiers,quarantined_frontiers,raw_spectral_nodes,"
        "compressed_spectral_nodes,submitted_jobs,stale_results,timeout_results";
    message.layout.dim[0].label +=
        ",mode_toggles,recovery_count,frontier_reassignments,repeated_targets,"
        "label_change_rate,spectral_route_utilization,solver_type,"
        "solver_iterations,solver_warm_start";
    message.data = {
        ros::WallTime::now().toSec(),
        static_cast<double>(global_plan_diagnostics_.status),
        static_cast<double>(global_plan_diagnostics_.method),
        global_plan_diagnostics_.used_fallback ? 1.0 : 0.0,
        static_cast<double>(global_plan_diagnostics_.dtg_version),
        static_cast<double>(global_plan_diagnostics_.frontier_version),
        static_cast<double>(global_plan_diagnostics_.spectral_epoch),
        global_plan_diagnostics_.lambda2,
        global_plan_diagnostics_.lambda3,
        global_plan_diagnostics_.eigengap_23,
        global_plan_diagnostics_.ncut,
        global_plan_diagnostics_.residual_norm,
        global_plan_diagnostics_.spectral_solve_ms,
        global_plan_diagnostics_.partition_valid ? 1.0 : 0.0,
        static_cast<double>(global_plan_diagnostics_.active_region_id),
        static_cast<double>(global_plan_diagnostics_.partition_change),
        static_cast<double>(global_plan_diagnostics_.active_anchor_count),
        static_cast<double>(global_plan_diagnostics_.route_size),
        static_cast<double>(global_plan_diagnostics_.spectral_mode),
        static_cast<double>(global_plan_diagnostics_.route_decision),
        static_cast<double>(global_plan_diagnostics_.recovery_reason),
        global_plan_diagnostics_.partition_confidence,
        global_plan_diagnostics_.confidence_ncut,
        global_plan_diagnostics_.confidence_eigengap,
        global_plan_diagnostics_.confidence_relative_lambda2,
        global_plan_diagnostics_.confidence_bottleneck,
        global_plan_diagnostics_.confidence_balance,
        global_plan_diagnostics_.baseline_path_cost,
        global_plan_diagnostics_.spectral_path_cost,
        global_plan_diagnostics_.selected_path_cost,
        global_plan_diagnostics_.baseline_combined_cost,
        global_plan_diagnostics_.spectral_combined_cost,
        global_plan_diagnostics_.route_regret,
        global_plan_diagnostics_.dynamic_switch_penalty,
        global_plan_diagnostics_.lock_debt,
        static_cast<double>(global_plan_diagnostics_.baseline_switches),
        static_cast<double>(global_plan_diagnostics_.spectral_switches),
        static_cast<double>(global_plan_diagnostics_.effective_frontier_count),
        static_cast<double>(global_plan_diagnostics_.quarantined_frontier_count),
        static_cast<double>(global_plan_diagnostics_.raw_spectral_nodes),
        static_cast<double>(global_plan_diagnostics_.compressed_spectral_nodes),
        static_cast<double>(global_plan_diagnostics_.submitted_spectral_jobs),
        static_cast<double>(global_plan_diagnostics_.stale_spectral_results),
        static_cast<double>(global_plan_diagnostics_.timed_out_spectral_results),
        static_cast<double>(global_plan_diagnostics_.spectral_mode_toggles),
        static_cast<double>(global_plan_diagnostics_.recovery_count),
        static_cast<double>(global_plan_diagnostics_.frontier_reassignments),
        static_cast<double>(global_plan_diagnostics_.repeated_targets),
        global_plan_diagnostics_.label_change_rate,
        global_plan_diagnostics_.spectral_route_utilization,
        static_cast<double>(global_plan_diagnostics_.spectral_solver_type),
        static_cast<double>(global_plan_diagnostics_.spectral_solver_iterations),
        global_plan_diagnostics_.spectral_warm_start_used ? 1.0 : 0.0};
    message.layout.dim[0].size = message.data.size();
    message.layout.dim[0].stride = message.data.size();
    spectral_diag_pub_.publish(message);

    if(global_plan_diagnostics_.used_fallback ||
       global_plan_diagnostics_.status != GlobalPlanStatus::SUCCESS ||
       last_partition_change_ != PartitionChangeType::NONE){
        std_msgs::String event;
        event.data = global_plan_diagnostics_.detail;
        spectral_event_pub_.publish(event);
    }
}

bool MultiDtgPlus::IsActiveBoundary(const h_ptr &h) const{
    return HasEffectiveFrontier(h, -1);
}

bool MultiDtgPlus::HasAnyActiveBoundary() const{
    for(const auto &h : H_list_){
        if(IsActiveBoundary(h)) return true;
    }
    return false;
}

GlobalPlanStatus MultiDtgPlus::CollectActiveBoundaryRegions(
    const Eigen::Vector3d &ps, GlobalRouteContext &context,
    bool populate_missing_pair_distances){
    context = GlobalRouteContext();

    list<h_ptr> local_hnodes;
    GetHnodesBBX(LRM_->local_map_upbd_, LRM_->local_map_lowbd_, local_hnodes);

    vector<Eigen::Vector3d> targets;
    vector<h_ptr> target_hnodes;
    targets.reserve(local_hnodes.size());
    target_hnodes.reserve(local_hnodes.size());
    for(const auto &h : local_hnodes){
        if(h == nullptr) continue;
        targets.emplace_back(h->pos_);
        target_hnodes.emplace_back(h);
    }

    vector<double> distances;
    vector<vector<Eigen::Vector3d>> origin_paths;
    if(!targets.empty()){
        LRM_->ClearTopo();
        LRM_->GetDists(ps, targets, distances, origin_paths, false);
    }

    const size_t count = std::min(target_hnodes.size(),
        std::min(distances.size(), origin_paths.size()));
    for(size_t i = 0; i < count; ++i){
        if(std::isfinite(distances[i]) && distances[i] < kUnreachableDistance){
            context.seed_hnodes.emplace_back(target_hnodes[i]);
            context.seed_distances.emplace_back(distances[i]);
            context.seed_paths.emplace_back(origin_paths[i]);
        }
    }

    if(context.seed_hnodes.empty()){
        return HasAnyActiveBoundary() ? GlobalPlanStatus::NO_FEASIBLE_SEED
                                      : GlobalPlanStatus::NO_ACTIVE_FRONTIER;
    }

    ParallelDijkstra(context.seed_hnodes, context.seed_distances,
        context.active_hnodes, context.root_paths);
    if(context.active_hnodes.empty()){
        return HasAnyActiveBoundary() ? GlobalPlanStatus::GRAPH_DISCONNECTED
                                      : GlobalPlanStatus::NO_ACTIVE_FRONTIER;
    }

    for(size_t i = 0; i < context.active_hnodes.size(); ++i){
        context.active_index[context.active_hnodes[i]->id_] = i;
    }
    return BuildActiveDistanceMatrix(context, populate_missing_pair_distances);
}

GlobalPlanStatus MultiDtgPlus::BuildActiveDistanceMatrix(
    GlobalRouteContext &context, bool populate_missing_pair_distances){
    const size_t n = context.active_hnodes.size();
    if(n == 0 || context.root_paths.size() != n){
        return GlobalPlanStatus::NO_ACTIVE_FRONTIER;
    }

    context.distance_matrix = Eigen::MatrixXd::Constant(
        static_cast<Eigen::Index>(n + 1), static_cast<Eigen::Index>(n + 1), 999999.0);
    context.distance_matrix.diagonal().setZero();
    // Preserve the directed root convention used by the original EOHDT Prim
    // implementation: root -> region has a cost, region -> root is zero.
    context.distance_matrix.col(0).setZero();
    for(size_t i = 0; i < n; ++i){
        context.distance_matrix(0, static_cast<Eigen::Index>(i + 1)) = context.root_paths[i].first;
    }

    for(size_t i = 0; i < n; ++i){
        vector<h_ptr> missing_targets;
        missing_targets.reserve(n - i - 1);
        for(size_t j = i + 1; j < n; ++j){
            const uint64_t key = HPairKey(context.active_hnodes[i]->id_, context.active_hnodes[j]->id_);
            const auto cached = h_dist_map_.find(key);
            if(cached == h_dist_map_.end() || cached->second.dtg_version != dtg_version_ ||
               !std::isfinite(cached->second.distance) ||
               cached->second.distance >= kBlockedEdgeDistance){
                missing_targets.emplace_back(context.active_hnodes[j]);
            }
        }
        if(populate_missing_pair_distances && !missing_targets.empty()){
            h_ptr start = context.active_hnodes[i];
            MainTainDistMap(start, missing_targets);
        }

        for(size_t j = i + 1; j < n; ++j){
            const uint64_t key = HPairKey(context.active_hnodes[i]->id_, context.active_hnodes[j]->id_);
            const auto cached = h_dist_map_.find(key);
            if(cached == h_dist_map_.end() || cached->second.dtg_version != dtg_version_ ||
               !std::isfinite(cached->second.distance) ||
               cached->second.distance >= kBlockedEdgeDistance){
                context.pairwise_connected = false;
                continue;
            }
            const Eigen::Index r = static_cast<Eigen::Index>(i + 1);
            const Eigen::Index c = static_cast<Eigen::Index>(j + 1);
            context.distance_matrix(r, c) = cached->second.distance;
            context.distance_matrix(c, r) = cached->second.distance;
        }
    }
    return GlobalPlanStatus::SUCCESS;
}

GlobalPlanStatus MultiDtgPlus::BuildPathToFirst(const Eigen::Vector3d &ps,
    const GlobalRouteContext &context, const h_ptr &first_h,
    vector<Eigen::Vector3d> &path2fh, double &d1){
    if(first_h == nullptr || context.seed_hnodes.empty()) return GlobalPlanStatus::PATH_FAILED;
    h_ptr target = first_h;
    if(!Astar(ps, context.seed_hnodes, context.seed_distances, target,
              context.seed_paths, path2fh, d1)){
        return GlobalPlanStatus::PATH_FAILED;
    }
    if(path2fh.empty()){
        if((ps - first_h->pos_).norm() < 1e-3){
            path2fh.emplace_back(ps);
            d1 = 0.0;
        }
        else{
            return GlobalPlanStatus::PATH_FAILED;
        }
    }
    return GlobalPlanStatus::SUCCESS;
}

GlobalPlanStatus MultiDtgPlus::RunEohdtFallback(const Eigen::Vector3d &ps,
    GlobalRouteContext &context, vector<h_ptr> &route_h,
    vector<Eigen::Vector3d> &path2fh, double &d1){
    route_h.clear();
    path2fh.clear();
    d1 = 0.0;

    // This function is the immutable original-EDEN safety baseline.  It must
    // always contain every reachable effective anchor; spectral state is not
    // allowed to filter this set.
    vector<size_t> selected;
    for(size_t i = 0; i < context.active_hnodes.size(); ++i){
        selected.emplace_back(i);
    }
    if(selected.empty()) return GlobalPlanStatus::NO_ACTIVE_FRONTIER;

    const size_t node_count = selected.size() + 1;
    vector<double> key(node_count, std::numeric_limits<double>::infinity());
    vector<int> parent(node_count, -1);
    vector<bool> used(node_count, false);
    key[0] = 0.0;

    for(size_t iteration = 0; iteration < node_count; ++iteration){
        size_t current = node_count;
        for(size_t i = 0; i < node_count; ++i){
            if(!used[i] && (current == node_count || key[i] < key[current])) current = i;
        }
        if(current == node_count || !std::isfinite(key[current])){
            return GlobalPlanStatus::GRAPH_DISCONNECTED;
        }
        used[current] = true;
        for(size_t candidate = 1; candidate < node_count; ++candidate){
            if(used[candidate] || candidate == current) continue;
            double distance = std::numeric_limits<double>::infinity();
            if(current == 0){
                distance = context.root_paths[selected[candidate - 1]].first;
            }
            else{
                distance = context.distance_matrix(
                    static_cast<Eigen::Index>(selected[current - 1] + 1),
                    static_cast<Eigen::Index>(selected[candidate - 1] + 1));
            }
            if(std::isfinite(distance) && distance < kUnreachableDistance && distance < key[candidate]){
                key[candidate] = distance;
                parent[candidate] = static_cast<int>(current);
            }
        }
    }

    vector<vector<size_t>> children(node_count);
    for(size_t i = 1; i < node_count; ++i){
        if(parent[i] < 0 || static_cast<size_t>(parent[i]) >= node_count){
            return GlobalPlanStatus::INTERNAL_ERROR;
        }
        children[static_cast<size_t>(parent[i])].emplace_back(i);
    }

    size_t farthest_leaf = 0;
    double farthest_root_distance = -1.0;
    for(size_t i = 1; i < node_count; ++i){
        if(children[i].empty()){
            const double root_distance = context.root_paths[selected[i - 1]].first;
            if(root_distance > farthest_root_distance){
                farthest_root_distance = root_distance;
                farthest_leaf = i;
            }
        }
    }
    if(farthest_leaf == 0) return GlobalPlanStatus::INTERNAL_ERROR;

    std::unordered_set<uint64_t> terminal_branch;
    for(size_t node = farthest_leaf; node != 0; node = static_cast<size_t>(parent[node])){
        terminal_branch.insert((static_cast<uint64_t>(parent[node]) << 32U) |
            static_cast<uint64_t>(node));
    }

    vector<size_t> stack(1, 0);
    while(!stack.empty()){
        const size_t current = stack.back();
        stack.pop_back();
        if(current != 0) route_h.emplace_back(context.active_hnodes[selected[current - 1]]);

        size_t terminal_child = node_count;
        vector<size_t> ordinary_children;
        for(const size_t child : children[current]){
            const uint64_t edge_key = (static_cast<uint64_t>(current) << 32U) |
                static_cast<uint64_t>(child);
            if(terminal_branch.count(edge_key) != 0U) terminal_child = child;
            else ordinary_children.emplace_back(child);
        }
        if(terminal_child != node_count) stack.emplace_back(terminal_child);
        std::sort(ordinary_children.begin(), ordinary_children.end(),
            [&](size_t lhs, size_t rhs){ return key[lhs] > key[rhs]; });
        for(const size_t child : ordinary_children) stack.emplace_back(child);
    }
    if(route_h.size() != selected.size()) return GlobalPlanStatus::INTERNAL_ERROR;

    const GlobalPlanStatus path_status = BuildPathToFirst(
        ps, context, route_h.front(), path2fh, d1);
    if(path_status == GlobalPlanStatus::SUCCESS) DebugLineStrip(ps, route_h);
    return path_status;
}

GlobalPlanStatus MultiDtgPlus::PlanGlobalRoute(const Eigen::Vector3d &ps,
    vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1){
    global_plan_diagnostics_ = GlobalPlanDiagnostics();
    global_plan_diagnostics_.dtg_version = dtg_version_;
    global_plan_diagnostics_.frontier_version = frontier_version_;
    global_plan_diagnostics_.active_region_id = active_region_id_;
    route_h.clear();
    path2fh.clear();
    d1 = 0.0;
    soft_route_selected_ = false;
    spectral_fallback_this_cycle_ = false;
    const double now = ros::WallTime::now().toSec();

    UpdateFrontierRuntimeStates();
    ReassignFrontierOwners();

    auto finish = [&](GlobalPlanStatus result){
        global_plan_diagnostics_.status = result;
        global_plan_diagnostics_.active_region_id = active_region_id_;
        global_plan_diagnostics_.partition_valid = partition_valid_;
        global_plan_diagnostics_.partition_change = static_cast<int>(last_partition_change_);
        global_plan_diagnostics_.route_size = route_h.size();
        global_plan_diagnostics_.spectral_mode =
            static_cast<int>(spectral_mode_state_.mode);
        global_plan_diagnostics_.recovery_reason =
            static_cast<int>(recovery_reason_);
        global_plan_diagnostics_.partition_confidence =
            partition_confidence_result_.confidence;
        global_plan_diagnostics_.confidence_ncut =
            partition_confidence_result_.q_ncut;
        global_plan_diagnostics_.confidence_eigengap =
            partition_confidence_result_.q_eigengap;
        global_plan_diagnostics_.confidence_relative_lambda2 =
            partition_confidence_result_.q_relative_lambda2;
        global_plan_diagnostics_.confidence_bottleneck =
            partition_confidence_result_.q_bottleneck;
        global_plan_diagnostics_.confidence_balance =
            partition_confidence_result_.q_balance;
        global_plan_diagnostics_.lock_debt = lock_debt_;
        global_plan_diagnostics_.raw_spectral_nodes = last_raw_spectral_node_count_;
        global_plan_diagnostics_.compressed_spectral_nodes =
            last_compressed_spectral_node_count_;
        global_plan_diagnostics_.submitted_spectral_jobs = submitted_spectral_jobs_;
        global_plan_diagnostics_.stale_spectral_results = stale_spectral_results_;
        global_plan_diagnostics_.timed_out_spectral_results =
            timed_out_spectral_results_;
        global_plan_diagnostics_.spectral_mode_toggles =
            spectral_mode_toggle_count_;
        global_plan_diagnostics_.recovery_count = recovery_count_;
        global_plan_diagnostics_.frontier_reassignments =
            frontier_reassignment_count_;
        global_plan_diagnostics_.repeated_targets = repeated_target_count_;
        global_plan_diagnostics_.label_change_rate = last_label_change_rate_;
        global_plan_diagnostics_.spectral_route_utilization =
            evaluated_spectral_routes_ == 0U ? 0.0 :
            static_cast<double>(accepted_spectral_routes_) /
            static_cast<double>(evaluated_spectral_routes_);
        if(last_spectral_result_.success()){
            global_plan_diagnostics_.spectral_solver_type = static_cast<int>(
                last_spectral_result_.diagnostics.solver_type);
            global_plan_diagnostics_.spectral_solver_iterations =
                last_spectral_result_.diagnostics.solver_iterations;
            global_plan_diagnostics_.spectral_warm_start_used =
                last_spectral_result_.diagnostics.iterative_warm_start_used;
        }
        double total_gain = 0.0;
        global_plan_diagnostics_.effective_frontier_count =
            EffectiveFrontierCount(&total_gain);
        global_plan_diagnostics_.quarantined_frontier_count = 0U;
        for(const auto &frontier : frontier_runtime_){
            if(frontier.second.state == FrontierState::QUARANTINED){
                ++global_plan_diagnostics_.quarantined_frontier_count;
            }
        }
        PublishGlobalPlanDiagnostics();
        last_partition_change_ = PartitionChangeType::NONE;
        return result;
    };

    GlobalRouteContext context;
    GlobalPlanStatus status = CollectActiveBoundaryRegions(
        ps, context, true);
    if(status != GlobalPlanStatus::SUCCESS){
        // Region completion is confirmed against distinct DTG update epochs,
        // including the tail phase in which no active anchor remains.
        if(status == GlobalPlanStatus::NO_ACTIVE_FRONTIER && !regions_.empty()){
            UpdateRegionExecutionState(context);
            global_plan_diagnostics_.active_region_id = active_region_id_;
        }
        global_plan_diagnostics_.detail = "failed to collect reachable active boundary regions";
        return finish(status);
    }
    global_plan_diagnostics_.active_anchor_count = context.active_hnodes.size();

    std::string spectral_reason;
    ConsumeSpectralResult(context, now, spectral_reason);
    ReassignFrontierOwners();
    UpdateRegionExecutionState(context);
    ReassignFrontierOwners();
    DetectAndHandleRegionStall(now);

    const bool late_stage = IsLateStage(context);
    if(!spectral_exec_config_.enabled || late_stage ||
       !spectral_graph_eligible_){
        UpdateSpectralRuntimeMode(false, false, now);
    }
    else if(spectral_mode_state_.mode == SpectralMode::RECOVERY){
        if(now >= recovery_until_) UpdateSpectralRuntimeMode(true, false, now);
    }
    if(NeedSpectralUpdate(context, now)){
        SubmitSpectralJobAsync(context, now, spectral_reason);
    }

    if(last_spectral_result_.success()){
        global_plan_diagnostics_.lambda2 = last_spectral_result_.diagnostics.lambda2;
        global_plan_diagnostics_.lambda3 = last_spectral_result_.diagnostics.lambda3;
        global_plan_diagnostics_.eigengap_23 =
            last_spectral_result_.diagnostics.eigengap_23;
        global_plan_diagnostics_.ncut = last_spectral_result_.diagnostics.ncut;
        global_plan_diagnostics_.residual_norm =
            last_spectral_result_.diagnostics.residual_norm;
        global_plan_diagnostics_.spectral_solve_ms =
            last_spectral_result_.diagnostics.total_time_ms;
    }
    global_plan_diagnostics_.spectral_epoch = spectral_epoch_;

    // Safety invariant: build the original EDEN/EOHDT route first on every
    // cycle.  Every v2 decision below can only replace this successful route.
    vector<h_ptr> baseline_route;
    vector<Eigen::Vector3d> baseline_path;
    double baseline_d1 = 0.0;
    status = RunEohdtFallback(ps, context, baseline_route,
                              baseline_path, baseline_d1);
    if(status != GlobalPlanStatus::SUCCESS){
        route_h.clear();
        path2fh.clear();
        d1 = 0.0;
        global_plan_diagnostics_.method = GlobalPlanMethod::NONE;
        global_plan_diagnostics_.detail = "original EOHDT baseline failed";
        return finish(status);
    }
    route_h = baseline_route;
    path2fh = baseline_path;
    d1 = baseline_d1;
    const RouteMetrics baseline_metrics = ComputeRouteMetrics(context,
                                                               baseline_route);
    global_plan_diagnostics_.baseline_path_cost = baseline_metrics.path_cost;
    global_plan_diagnostics_.baseline_switches = baseline_metrics.switch_count;
    global_plan_diagnostics_.selected_path_cost = baseline_metrics.path_cost;

    auto decay_lock_debt = [&](){
        const LockDebtUpdate debt = UpdateLockDebt(lock_debt_,
            baseline_metrics.path_cost, baseline_metrics.path_cost,
            {spectral_exec_config_.lock_debt_decay,
             spectral_exec_config_.lock_debt_max});
        lock_debt_ = debt.debt;
    };
    const uint64_t dtg_age = dtg_version_ >= last_spectral_dtg_version_
        ? dtg_version_ - last_spectral_dtg_version_
        : std::numeric_limits<uint64_t>::max();
    const uint64_t frontier_age = frontier_version_ >= last_spectral_frontier_version_
        ? frontier_version_ - last_spectral_frontier_version_
        : std::numeric_limits<uint64_t>::max();
    const bool stable_partition_fresh = last_spectral_dtg_version_ != 0U &&
        last_spectral_frontier_version_ != 0U &&
        std::max(dtg_age, frontier_age) <= static_cast<uint64_t>(
            spectral_exec_config_.max_spectral_epoch_age);
    bool route_feedback_due =
        last_route_feedback_spectral_epoch_ != spectral_epoch_;
    const bool route_feedback_eligible = spectral_exec_config_.enabled &&
        !late_stage && spectral_graph_eligible_;
    auto apply_route_feedback = [&](bool route_acceptable){
        if(!route_feedback_due) return;
        UpdateSpectralRuntimeMode(route_feedback_eligible,
                                  route_acceptable, now);
        last_route_feedback_spectral_epoch_ = spectral_epoch_;
        route_feedback_due = false;
    };

    const bool confidence_supports_evaluation =
        spectral_mode_state_.mode == SpectralMode::ACTIVE_SOFT ||
        partition_confidence_result_.confidence >=
            spectral_exec_config_.partition_confidence_on;
    const bool route_evaluation_needed =
        spectral_mode_state_.mode == SpectralMode::ACTIVE_SOFT ||
        route_feedback_due;
    const bool can_evaluate_soft_route = spectral_exec_config_.enabled &&
        !late_stage && spectral_graph_eligible_ &&
        !spectral_fallback_this_cycle_ &&
        spectral_mode_state_.mode != SpectralMode::RECOVERY &&
        stable_spectral_result_.has_valid_cut() &&
        last_spectral_result_.has_valid_cut() &&
        stable_partition_fresh && confidence_supports_evaluation &&
        route_evaluation_needed;
    if(!can_evaluate_soft_route){
        apply_route_feedback(false);
        decay_lock_debt();
        global_plan_diagnostics_.method = spectral_exec_config_.enabled
            ? GlobalPlanMethod::EOHDT_FALLBACK
            : GlobalPlanMethod::EOHDT_DISABLED_SPECTRAL;
        global_plan_diagnostics_.used_fallback = spectral_exec_config_.enabled;
        global_plan_diagnostics_.route_decision = static_cast<int>(
            spectral_mode_state_.mode == SpectralMode::RECOVERY
                ? SpectralRouteDecision::RECOVERY_BASELINE
                : (stable_spectral_result_.has_valid_cut()
                    ? SpectralRouteDecision::MODE_INACTIVE
                    : SpectralRouteDecision::NO_STABLE_PARTITION));
        if(!stable_partition_fresh && stable_spectral_result_.has_valid_cut()){
            global_plan_diagnostics_.detail =
                "stable spectral partition exceeded version age; used original EOHDT";
        }
        else{
            global_plan_diagnostics_.detail = spectral_reason.empty()
                ? std::string("Spectral-v2 ") + SpectralModeName(spectral_mode_state_.mode) +
                    "; used original EOHDT"
                : spectral_reason + "; used original EOHDT";
        }
        return finish(GlobalPlanStatus::SUCCESS);
    }

    const double return_probability = EstimateReturnProbability(context);
    const double switch_penalty = ComputeDynamicSwitchPenalty(
        spectral_exec_config_.switch_penalty_base,
        partition_confidence_result_.confidence, return_probability);
    global_plan_diagnostics_.dynamic_switch_penalty = switch_penalty;
    vector<h_ptr> candidate_route;
    if(!BuildRegionAwareRoute(context, candidate_route,
                              switch_penalty, spectral_reason)){
        apply_route_feedback(false);
        decay_lock_debt();
        global_plan_diagnostics_.method = GlobalPlanMethod::EOHDT_FALLBACK;
        global_plan_diagnostics_.used_fallback = true;
        global_plan_diagnostics_.route_decision = static_cast<int>(
            SpectralRouteDecision::CANDIDATE_BUILD_FAILED);
        global_plan_diagnostics_.detail = spectral_reason +
            "; used original EOHDT";
        return finish(GlobalPlanStatus::SUCCESS);
    }

    const RouteMetrics candidate_metrics = ComputeRouteMetrics(context,
                                                                candidate_route);
    ++evaluated_spectral_routes_;
    RouteAcceptanceConfig acceptance;
    acceptance.max_route_regret = spectral_exec_config_.max_route_regret;
    acceptance.switch_penalty = switch_penalty;
    acceptance.revisit_penalty = spectral_exec_config_.revisit_penalty_weight;
    acceptance.numeric_epsilon = spectral_config_.numeric_epsilon;
    const RouteDecision decision = EvaluateRegionRoute(
        baseline_metrics, candidate_metrics, acceptance);
    global_plan_diagnostics_.spectral_path_cost = candidate_metrics.path_cost;
    global_plan_diagnostics_.spectral_switches = candidate_metrics.switch_count;
    global_plan_diagnostics_.baseline_combined_cost =
        decision.baseline_combined_cost;
    global_plan_diagnostics_.spectral_combined_cost =
        decision.candidate_combined_cost;
    global_plan_diagnostics_.route_regret = decision.path_regret;

    if(!decision.accepted){
        apply_route_feedback(false);
        decay_lock_debt();
        global_plan_diagnostics_.method = GlobalPlanMethod::EOHDT_FALLBACK;
        global_plan_diagnostics_.used_fallback = true;
        global_plan_diagnostics_.route_decision = static_cast<int>(
            decision.within_path_regret
                ? SpectralRouteDecision::COMBINED_COST_REJECTED
                : SpectralRouteDecision::REGRET_REJECTED);
        global_plan_diagnostics_.detail = decision.within_path_regret
            ? "region route did not improve combined cost; used original EOHDT"
            : "region route rejected by path-regret guard; used original EOHDT";
        return finish(GlobalPlanStatus::SUCCESS);
    }

    vector<Eigen::Vector3d> candidate_path;
    double candidate_d1 = 0.0;
    const bool candidate_targets_active_region = active_region_id_ >= 0 &&
        !candidate_route.empty() &&
        RegionForHnode(candidate_route.front()) == active_region_id_;
    const GlobalPlanStatus candidate_path_status = BuildPathToFirst(
        ps, context, candidate_route.front(), candidate_path, candidate_d1);
    if(candidate_path_status != GlobalPlanStatus::SUCCESS){
        apply_route_feedback(false);
        if(candidate_targets_active_region) RecordActiveRegionPathResult(false);
        decay_lock_debt();
        global_plan_diagnostics_.method = GlobalPlanMethod::EOHDT_FALLBACK;
        global_plan_diagnostics_.used_fallback = true;
        global_plan_diagnostics_.route_decision = static_cast<int>(
            SpectralRouteDecision::CANDIDATE_PATH_FAILED);
        global_plan_diagnostics_.detail =
            "region-route first path failed; used original EOHDT";
        return finish(GlobalPlanStatus::SUCCESS);
    }

    const bool mode_was_active_before_feedback =
        spectral_mode_state_.mode == SpectralMode::ACTIVE_SOFT;
    apply_route_feedback(true);
    if(!mode_was_active_before_feedback &&
       spectral_mode_state_.mode == SpectralMode::ACTIVE_SOFT){
        // Select the persistent active region first; the next cycle evaluates
        // the candidate with the correct return probability and switch count.
        UpdateRegionExecutionState(context);
        ReassignFrontierOwners();
        decay_lock_debt();
        global_plan_diagnostics_.method = GlobalPlanMethod::EOHDT_FALLBACK;
        global_plan_diagnostics_.used_fallback = true;
        global_plan_diagnostics_.route_decision = static_cast<int>(
            SpectralRouteDecision::MODE_INACTIVE);
        global_plan_diagnostics_.detail =
            "soft mode activated; initialized active region and retained "
            "original EOHDT for this cycle";
        return finish(GlobalPlanStatus::SUCCESS);
    }
    if(spectral_mode_state_.mode != SpectralMode::ACTIVE_SOFT ||
       !partition_valid_){
        decay_lock_debt();
        global_plan_diagnostics_.method = GlobalPlanMethod::EOHDT_FALLBACK;
        global_plan_diagnostics_.used_fallback = true;
        global_plan_diagnostics_.route_decision = static_cast<int>(
            SpectralRouteDecision::MODE_INACTIVE);
        global_plan_diagnostics_.detail =
            "region route passed guards; waiting for activation persistence; "
            "used original EOHDT";
        return finish(GlobalPlanStatus::SUCCESS);
    }

    const LockDebtUpdate debt = UpdateLockDebt(lock_debt_,
        candidate_metrics.path_cost, baseline_metrics.path_cost,
        {spectral_exec_config_.lock_debt_decay,
         spectral_exec_config_.lock_debt_max});
    lock_debt_ = debt.debt;
    if(debt.recovery_required){
        EnterRecovery(RecoveryReason::LOCK_DEBT, now);
        global_plan_diagnostics_.method = GlobalPlanMethod::EOHDT_FALLBACK;
        global_plan_diagnostics_.used_fallback = true;
        global_plan_diagnostics_.route_decision = static_cast<int>(
            SpectralRouteDecision::RECOVERY_BASELINE);
        global_plan_diagnostics_.detail =
            "accumulated detour debt entered recovery; used original EOHDT";
        return finish(GlobalPlanStatus::SUCCESS);
    }

    route_h = std::move(candidate_route);
    path2fh = std::move(candidate_path);
    d1 = candidate_d1;
    soft_route_selected_ = true;
    if(candidate_targets_active_region) RecordActiveRegionPathResult(true);
    DebugLineStrip(ps, route_h);
    global_plan_diagnostics_.selected_path_cost = candidate_metrics.path_cost;
    ++accepted_spectral_routes_;
    global_plan_diagnostics_.method = GlobalPlanMethod::SPECTRAL_V2_SOFT;
    global_plan_diagnostics_.route_decision = static_cast<int>(
        SpectralRouteDecision::ACCEPTED);
    global_plan_diagnostics_.detail =
        "accepted Spectral-v2 soft regional route within regret bound";
    return finish(GlobalPlanStatus::SUCCESS);
}

bool MultiDtgPlus::TspApproxiPlan(const Eigen::Vector3d &ps,
    vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1){
    return PlanGlobalRoute(ps, route_h, path2fh, d1) == GlobalPlanStatus::SUCCESS;
}
