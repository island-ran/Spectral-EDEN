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
    h_dist_map_.clear();
}

void MultiDtgPlus::MarkFrontierChanged(){
    ++frontier_version_;
}

bool MultiDtgPlus::HasEffectiveFrontier(
    const h_ptr &h, int region_id) const{
    (void)region_id;
    if(h == nullptr || EROI_ == nullptr) return false;
    for(const hfe_ptr &edge : h->hf_edges_){
        if(edge == nullptr || edge->tail_n_ == nullptr ||
           !std::isfinite(edge->length_) ||
           edge->length_ >= kBlockedEdgeDistance) continue;
        const uint32_t frontier_id = edge->tail_n_->fid_;
        const uint8_t viewpoint_id = edge->tail_n_->vid_;
        if(frontier_id >= EROI_->EROI_.size()) continue;
        const auto &frontier = EROI_->EROI_[frontier_id];
        if(frontier.f_state_ == 1U &&
           viewpoint_id < frontier.local_vps_.size() &&
           frontier.local_vps_[viewpoint_id] == 1U){
            return true;
        }
    }
    return false;
}

void MultiDtgPlus::PublishGlobalPlanDiagnostics(){
    if(global_planner_mode_ == GlobalPlannerMode::SPECTRAL_V4 &&
       !spectral_v4_config_.log_diagnostics) return;

    std_msgs::Float64MultiArray message;
    message.layout.dim.resize(1);
    message.layout.dim[0].label =
        "stamp,status,method,dtg_version,frontier_version,active_anchors,"
        "route_size,selected_path,planner_mode,v4_backend,"
        "worker_graph_version,region_state_version,committed_target_id,"
        "ncut,balance,residual,truncated,graph_delta_ms,planning_ms";
    const double ncut = region_snapshot_ == nullptr
        ? std::numeric_limits<double>::quiet_NaN() : region_snapshot_->ncut;
    const double balance = region_snapshot_ == nullptr
        ? 0.0 : region_snapshot_->balance;
    const double residual = region_snapshot_ == nullptr
        ? std::numeric_limits<double>::quiet_NaN() :
          region_snapshot_->residual;
    message.data = {
        ros::WallTime::now().toSec(),
        static_cast<double>(global_plan_diagnostics_.status),
        static_cast<double>(global_plan_diagnostics_.method),
        static_cast<double>(global_plan_diagnostics_.dtg_version),
        static_cast<double>(global_plan_diagnostics_.frontier_version),
        static_cast<double>(global_plan_diagnostics_.active_anchor_count),
        static_cast<double>(global_plan_diagnostics_.route_size),
        global_plan_diagnostics_.selected_path_cost,
        static_cast<double>(global_plan_diagnostics_.planner_mode),
        static_cast<double>(global_plan_diagnostics_.v4_backend),
        static_cast<double>(global_plan_diagnostics_.worker_graph_version),
        static_cast<double>(global_plan_diagnostics_.region_state_version),
        static_cast<double>(global_plan_diagnostics_.committed_target_id),
        ncut,
        balance,
        residual,
        region_snapshot_ != nullptr && region_snapshot_->truncated ? 1.0 : 0.0,
        global_plan_diagnostics_.graph_delta_ms,
        global_plan_diagnostics_.planning_ms};
    message.layout.dim[0].size = message.data.size();
    message.layout.dim[0].stride = message.data.size();
    spectral_diag_pub_.publish(message);

    if(global_plan_diagnostics_.status != GlobalPlanStatus::SUCCESS){
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

// ── Route context collection (shared by EDEN baseline) ──

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
    if(!populate_missing_pair_distances){
        return GlobalPlanStatus::SUCCESS;
    }
    return BuildActiveDistanceMatrix(context, true);
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

// ── EOHDT fallback (EDEN baseline only) ──

GlobalPlanStatus MultiDtgPlus::BuildOriginalEohdtRoute(const Eigen::Vector3d &ps,
    GlobalRouteContext &context, vector<h_ptr> &route_h,
    vector<Eigen::Vector3d> &path2fh, double &d1){
    route_h.clear();
    path2fh.clear();
    d1 = 0.0;

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

// ── Top-level dispatcher ──

GlobalPlanStatus MultiDtgPlus::PlanGlobalRoute(const Eigen::Vector3d &ps,
    vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1){
    route_h.clear();
    path2fh.clear();
    d1 = 0.0;

    if(!ps.allFinite()){
        global_plan_diagnostics_.status = GlobalPlanStatus::INTERNAL_ERROR;
        global_plan_diagnostics_.detail = "non-finite robot position";
        return GlobalPlanStatus::INTERNAL_ERROR;
    }

    // Log planner mode at start of each cycle
    global_plan_diagnostics_.planner_mode = static_cast<int>(global_planner_mode_);

    switch(global_planner_mode_){
    case GlobalPlannerMode::EDEN_BASELINE:
        return PlanGlobalRouteEden(ps, route_h, path2fh, d1);

    case GlobalPlannerMode::SPECTRAL_V4:
        return PlanGlobalRouteV4(ps, route_h, path2fh, d1);
    }

    global_plan_diagnostics_.status = GlobalPlanStatus::INTERNAL_ERROR;
    global_plan_diagnostics_.detail = "unknown planner mode";
    return GlobalPlanStatus::INTERNAL_ERROR;
}

bool MultiDtgPlus::TspApproxiPlan(const Eigen::Vector3d &ps,
    vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1){
    return PlanGlobalRoute(ps, route_h, path2fh, d1) == GlobalPlanStatus::SUCCESS;
}
