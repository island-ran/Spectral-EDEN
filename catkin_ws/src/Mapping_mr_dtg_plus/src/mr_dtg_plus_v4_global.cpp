#include <mr_dtg_plus/mr_dtg_plus.h>

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace DTGPlus;

namespace {
constexpr double kBlockedEdgeDistance = 200000.0;
constexpr double kRegionEntryArrivalToleranceMeters = 0.05;
constexpr double kPathSafetySampleSpacingMeters = 0.10;

double FrontierGainForAnchor(
    const h_ptr& anchor, const EroiGrid* eroi) {
  if (anchor == nullptr || eroi == nullptr) return 0.0;
  double gain = 0.0;
  std::unordered_set<uint32_t> counted;
  for (const auto& edge : anchor->hf_edges_) {
    if (edge == nullptr || edge->tail_n_ == nullptr ||
        !std::isfinite(edge->length_) ||
        edge->length_ >= kBlockedEdgeDistance)
      continue;
    const uint32_t frontier_id = edge->tail_n_->fid_;
    const uint8_t viewpoint_id = edge->tail_n_->vid_;
    if (!counted.insert(frontier_id).second ||
        frontier_id >= eroi->EROI_.size())
      continue;
    const auto& frontier = eroi->EROI_[frontier_id];
    if (frontier.f_state_ != 1U ||
        viewpoint_id >= frontier.local_vps_.size() ||
        frontier.local_vps_[viewpoint_id] != 1U)
      continue;
    gain += std::max(0, frontier.unknown_num_);
  }
  return gain;
}

struct V4Candidate {
  CommittedTarget target;
  h_ptr anchor;
  double gain = 0.0;
  double travel_time = std::numeric_limits<double>::infinity();
  std::vector<Eigen::Vector3d> path;
};

bool SameMacroTarget(const CommittedTarget& lhs,
                     const CommittedTarget& rhs) {
  if (!lhs.valid || !rhs.valid || lhs.type != rhs.type ||
      lhs.anchor_h_id != rhs.anchor_h_id)
    return false;
  if (lhs.type == MacroTargetType::REGION_ENTRY)
    return lhs.entry_from_h_id == rhs.entry_from_h_id &&
        lhs.entry_to_h_id == rhs.entry_to_h_id;
  return lhs.frontier_id == rhs.frontier_id;
}

bool PathBlockedForKnownSpace(
    const std::vector<Eigen::Vector3d>& path,
    lowres_lite::LowResMap* lowres_map) {
  if (path.empty() || lowres_map == nullptr)
    return true;
  auto point_blocked = [&](const Eigen::Vector3d& point) {
    return !point.allFinite() ||
        !lowres_map->IsFeasible(point, false);
  };
  if (point_blocked(path.front())) return true;
  for (size_t index = 1; index < path.size(); ++index) {
    const Eigen::Vector3d delta = path[index] - path[index - 1];
    const double length = delta.norm();
    if (!std::isfinite(length)) return true;
    const int sample_count = std::max(
        1, static_cast<int>(
               std::ceil(length / kPathSafetySampleSpacingMeters)));
    for (int sample = 1; sample <= sample_count; ++sample) {
      const Eigen::Vector3d point =
          path[index - 1] +
          delta * (static_cast<double>(sample) /
                   static_cast<double>(sample_count));
      if (point_blocked(point)) return true;
    }
  }
  return false;
}

}  // namespace

double MultiDtgPlus::EstimatePathClearance(
    const list<Eigen::Vector3d>& path) const {
  if (path.empty() || BM_ == nullptr || LRM_ == nullptr)
    return spectral_v4_config_.clearance_reference;
  const double maximum =
      std::max(1.0e-3, spectral_v4_config_.clearance_reference);
  const Eigen::Vector3d robot_size = LRM_->GetRobotSize();
  const size_t max_samples = static_cast<size_t>(
      std::max(2, spectral_v4_config_.clearance_max_samples));
  const size_t stride = std::max<size_t>(
      1, (path.size() + max_samples - 1) / max_samples);
  double clearance = maximum;
  size_t index = 0;
  for (auto point = path.begin(); point != path.end(); ++point, ++index) {
    const bool endpoint =
        point == path.begin() || std::next(point) == path.end();
    if (!endpoint && index % stride != 0U) continue;
    if (BM_->PosBBXOccupied(*point, robot_size)) return 0.0;
    double low = 0.0;
    double high = maximum;
    for (int step = 0;
         step < spectral_v4_config_.clearance_binary_steps; ++step) {
      const double radius = 0.5 * (low + high);
      const Eigen::Vector3d expanded =
          robot_size + Eigen::Vector3d::Ones() * (2.0 * radius);
      if (BM_->PosBBXOccupied(*point, expanded)) high = radius;
      else low = radius;
    }
    clearance = std::min(clearance, low);
  }
  return std::max(0.0, clearance);
}

double MultiDtgPlus::EdgeClearance(const hhe_ptr& edge) {
  if (edge == nullptr) return 0.0;
  if (!std::isfinite(edge->clearance_))
    edge->clearance_ = EstimatePathClearance(edge->path_);
  return std::max(0.0, edge->clearance_);
}

h_ptr MultiDtgPlus::FindHnodeById(uint32_t h_id) const {
  for (const auto& node : H_list_)
    if (node != nullptr && node->id_ == h_id) return node;
  return h_ptr();
}

bool MultiDtgPlus::IsV4FrontierValid(
    uint32_t frontier_id, uint8_t viewpoint_id) const {
  if (EROI_ == nullptr || frontier_id >= EROI_->EROI_.size())
    return false;
  const auto& frontier = EROI_->EROI_[frontier_id];
  return frontier.f_state_ == 1U &&
      viewpoint_id < frontier.local_vps_.size() &&
      frontier.local_vps_[viewpoint_id] == 1U;
}

FrontierAnchorUpdate MultiDtgPlus::BuildV4FrontierUpdate(
    const h_ptr& node) const {
  FrontierAnchorUpdate update;
  if (node == nullptr || EROI_ == nullptr) return update;
  update.h_id = node->id_;
  std::unordered_set<uint32_t> counted;
  for (const auto& edge : node->hf_edges_) {
    if (edge == nullptr || edge->tail_n_ == nullptr ||
        !std::isfinite(edge->length_) ||
        edge->length_ >= kBlockedEdgeDistance)
      continue;
    const uint32_t frontier_id = edge->tail_n_->fid_;
    const uint8_t viewpoint_id = edge->tail_n_->vid_;
    if (!IsV4FrontierValid(frontier_id, viewpoint_id) ||
        !counted.insert(frontier_id).second)
      continue;
    update.active = true;
    update.frontier_ids.push_back(frontier_id);
    update.expected_gain +=
        std::max(0, EROI_->EROI_[frontier_id].unknown_num_);
  }
  std::sort(update.frontier_ids.begin(), update.frontier_ids.end());
  return update;
}

void MultiDtgPlus::RecordV4NodeUpsert(const h_ptr& node) {
  if (global_planner_mode_ != GlobalPlannerMode::SPECTRAL_V4 ||
      node == nullptr)
    return;
  NodeInsert insert;
  insert.h_id = node->id_;
  insert.pos = node->pos_;
  insert.degree = static_cast<uint32_t>(node->hh_edges_.size());
  std::lock_guard<std::mutex> lock(v4_delta_mutex_);
  pending_node_erases_.erase(node->id_);
  pending_node_upserts_[node->id_] = insert;
}

void MultiDtgPlus::RecordV4EdgeUpsert(const hhe_ptr& edge) {
  if (global_planner_mode_ != GlobalPlannerMode::SPECTRAL_V4 ||
      edge == nullptr || edge->head_n_ == nullptr ||
      edge->tail_n_ == nullptr || !std::isfinite(edge->length_) ||
      edge->length_ >= kBlockedEdgeDistance)
    return;
  EdgeUpsert update;
  update.from = edge->head_n_->id_;
  update.to = edge->tail_n_->id_;
  update.length = edge->length_;
  update.clearance = EdgeClearance(edge);
  const uint64_t key = HPairKey(update.from, update.to);
  std::lock_guard<std::mutex> lock(v4_delta_mutex_);
  pending_edge_erases_.erase(key);
  pending_edge_upserts_[key] = update;
}

void MultiDtgPlus::RecordV4EdgeErase(uint32_t from, uint32_t to) {
  if (global_planner_mode_ != GlobalPlannerMode::SPECTRAL_V4 ||
      from == to)
    return;
  const uint64_t key = HPairKey(from, to);
  std::lock_guard<std::mutex> lock(v4_delta_mutex_);
  pending_edge_upserts_.erase(key);
  pending_edge_erases_.insert(key);
}

void MultiDtgPlus::RecordV4FrontierAnchor(const h_ptr& node) {
  if (global_planner_mode_ != GlobalPlannerMode::SPECTRAL_V4 ||
      node == nullptr)
    return;
  FrontierAnchorUpdate update = BuildV4FrontierUpdate(node);
  std::lock_guard<std::mutex> lock(v4_delta_mutex_);
  for (const auto& edge : node->hf_edges_) {
    if (edge != nullptr && edge->tail_n_ != nullptr)
      frontier_anchor_index_[edge->tail_n_->fid_].insert(node->id_);
  }
  pending_frontier_updates_[node->id_] = std::move(update);
}

void MultiDtgPlus::NotifyFrontierGainChanges(
    const vector<uint32_t>& frontier_ids) {
  if (global_planner_mode_ != GlobalPlannerMode::SPECTRAL_V4 ||
      frontier_ids.empty())
    return;
  std::unordered_set<uint32_t> anchor_ids;
  {
    std::lock_guard<std::mutex> lock(v4_delta_mutex_);
    for (uint32_t frontier_id : frontier_ids) {
      const auto found = frontier_anchor_index_.find(frontier_id);
      if (found != frontier_anchor_index_.end())
        anchor_ids.insert(found->second.begin(), found->second.end());
    }
  }
  for (uint32_t anchor_id : anchor_ids) {
    const h_ptr anchor = FindHnodeById(anchor_id);
    if (anchor != nullptr) RecordV4FrontierAnchor(anchor);
  }
  if (!anchor_ids.empty()) MarkFrontierChanged();
}

GraphDelta MultiDtgPlus::BuildGraphDelta(
    const Eigen::Vector3d& robot_pos) {
  GraphDelta delta;
  delta.graph_version = dtg_version_;
  delta.frontier_version = frontier_version_;

  if (v4_bootstrap_pending_) {
    std::unordered_set<uint64_t> seen_edges;
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>>
        bootstrap_frontier_index;
    for (const auto& node : H_list_) {
      if (node == nullptr) continue;
      NodeInsert insert;
      insert.h_id = node->id_;
      insert.pos = node->pos_;
      insert.degree = static_cast<uint32_t>(node->hh_edges_.size());
      delta.node_inserts.push_back(insert);
      delta.frontier_updates.push_back(BuildV4FrontierUpdate(node));
      for (const auto& frontier_edge : node->hf_edges_) {
        if (frontier_edge != nullptr &&
            frontier_edge->tail_n_ != nullptr) {
          bootstrap_frontier_index[
              frontier_edge->tail_n_->fid_].insert(node->id_);
        }
      }
      for (const auto& edge : node->hh_edges_) {
        if (edge == nullptr || edge->head_n_ == nullptr ||
            edge->tail_n_ == nullptr || !std::isfinite(edge->length_) ||
            edge->length_ >= kBlockedEdgeDistance)
          continue;
        const uint64_t key =
            HPairKey(edge->head_n_->id_, edge->tail_n_->id_);
        if (!seen_edges.insert(key).second) continue;
        EdgeUpsert update;
        update.from = edge->head_n_->id_;
        update.to = edge->tail_n_->id_;
        update.length = edge->length_;
        update.clearance = EdgeClearance(edge);
        delta.edge_upserts.push_back(update);
      }
    }
    v4_bootstrap_pending_ = false;
    std::lock_guard<std::mutex> lock(v4_delta_mutex_);
    pending_node_upserts_.clear();
    pending_node_erases_.clear();
    pending_edge_upserts_.clear();
    pending_edge_erases_.clear();
    pending_frontier_updates_.clear();
    frontier_anchor_index_ = std::move(bootstrap_frontier_index);
  } else {
    std::lock_guard<std::mutex> lock(v4_delta_mutex_);
    for (const auto& entry : pending_node_upserts_)
      delta.node_inserts.push_back(entry.second);
    for (uint32_t id : pending_node_erases_)
      delta.node_erases.push_back(NodeErase{id});
    for (const auto& entry : pending_edge_upserts_)
      delta.edge_upserts.push_back(entry.second);
    for (uint64_t key : pending_edge_erases_) {
      EdgeErase erase;
      erase.from = static_cast<uint32_t>(key >> 32U);
      erase.to = static_cast<uint32_t>(key & 0xFFFFFFFFU);
      delta.edge_erases.push_back(erase);
    }
    for (const auto& entry : pending_frontier_updates_)
      delta.frontier_updates.push_back(entry.second);
    pending_node_upserts_.clear();
    pending_node_erases_.clear();
    pending_edge_upserts_.clear();
    pending_edge_erases_.clear();
    pending_frontier_updates_.clear();
  }

  h_ptr closest;
  double closest_distance = std::numeric_limits<double>::infinity();
  list<h_ptr> local_nodes;
  if (LRM_ != nullptr) {
    GetHnodesBBX(
        LRM_->local_map_upbd_, LRM_->local_map_lowbd_, local_nodes);
  }
  const list<h_ptr>& search_nodes =
      local_nodes.empty() ? H_list_ : local_nodes;
  for (const auto& node : search_nodes) {
    if (node == nullptr) continue;
    const double distance = (node->pos_ - robot_pos).squaredNorm();
    if (distance < closest_distance ||
        (distance == closest_distance &&
         (closest == nullptr || node->id_ < closest->id_))) {
      closest = node;
      closest_distance = distance;
    }
  }
  const uint32_t robot_h_id = closest == nullptr ? 0U : closest->id_;
  if (robot_h_id != last_robot_h_id_) {
    delta.robot_h_id = robot_h_id;
    delta.robot_changed = true;
    last_robot_h_id_ = robot_h_id;
  }

  auto by_node = [](const auto& lhs, const auto& rhs) {
    return lhs.h_id < rhs.h_id;
  };
  std::sort(delta.node_inserts.begin(), delta.node_inserts.end(), by_node);
  std::sort(delta.node_erases.begin(), delta.node_erases.end(), by_node);
  std::sort(
      delta.frontier_updates.begin(), delta.frontier_updates.end(), by_node);
  auto by_edge = [](const auto& lhs, const auto& rhs) {
    const auto a = std::minmax(lhs.from, lhs.to);
    const auto b = std::minmax(rhs.from, rhs.to);
    return a < b;
  };
  std::sort(delta.edge_upserts.begin(), delta.edge_upserts.end(), by_edge);
  std::sort(delta.edge_erases.begin(), delta.edge_erases.end(), by_edge);
  return delta;
}

bool MultiDtgPlus::ValidateRegionSnapshot(
    const RegionStateSnapshot& snapshot) const {
  if (!snapshot.valid || snapshot.graph_version > dtg_version_ ||
      snapshot.frontier_version > frontier_version_)
    return false;
  if (dtg_version_ - snapshot.graph_version >
      static_cast<uint64_t>(
          std::max(0, spectral_v4_config_.max_region_version_lag)))
    return false;
  for (const auto& edge : snapshot.quotient_edges) {
    const h_ptr from = FindHnodeById(edge.entry_from);
    const h_ptr to = FindHnodeById(edge.entry_to);
    if (from == nullptr || to == nullptr) return false;
    bool connected = false;
    for (const auto& raw_edge : from->hh_edges_) {
      if (raw_edge == nullptr) continue;
      const h_ptr other =
          raw_edge->head_n_ == from ? raw_edge->tail_n_ :
          raw_edge->head_n_;
      if (other == to && std::isfinite(raw_edge->length_) &&
          raw_edge->length_ < kBlockedEdgeDistance) {
        connected = true;
        break;
      }
    }
    if (!connected) return false;
  }
  return true;
}

bool MultiDtgPlus::CurrentMacroTargetIsRegionEntry() const {
  return global_planner_mode_ == GlobalPlannerMode::SPECTRAL_V4 &&
      pending_target_.valid &&
      pending_target_.type == MacroTargetType::REGION_ENTRY;
}

void MultiDtgPlus::CommitTargetAfterPublish(
    const CommittedTarget& executed) {
  if (global_planner_mode_ != GlobalPlannerMode::SPECTRAL_V4 ||
      !pending_target_.valid)
    return;
  const bool same_target = committed_target_.valid &&
      committed_target_.type == pending_target_.type &&
      committed_target_.anchor_h_id == pending_target_.anchor_h_id &&
      (pending_target_.type == MacroTargetType::REGION_ENTRY ||
       committed_target_.frontier_id == executed.frontier_id);
  if (same_target) {
    committed_target_.viewpoint_id = executed.viewpoint_id;
    if (committed_target_.type == MacroTargetType::FRONTIER &&
        EROI_ != nullptr &&
        committed_target_.frontier_id < EROI_->EROI_.size()) {
      const double current_unknown = std::max(
          0, EROI_->EROI_[committed_target_.frontier_id].unknown_num_);
      const double interval_gain = std::max(
          0.0, committed_target_.last_observed_unknown_count -
                   current_unknown);
      committed_target_.actual_gain_since_accept += interval_gain;
      if (interval_gain < spectral_v4_config_.actual_gain_epsilon)
        ++committed_target_.consecutive_low_gain;
      else
        committed_target_.consecutive_low_gain = 0;
      committed_target_.last_observed_unknown_count = current_unknown;
    }
    pending_target_ = committed_target_;
    return;
  }
  double initial_unknown = 0.0;
  if (pending_target_.type == MacroTargetType::FRONTIER &&
      EROI_ != nullptr && executed.frontier_id < EROI_->EROI_.size())
    initial_unknown =
        std::max(0, EROI_->EROI_[executed.frontier_id].unknown_num_);
  committed_target_ = target_manager_.CommitAfterPublish(
      pending_target_, executed.frontier_id, executed.viewpoint_id,
      ros::WallTime::now().toSec(), initial_unknown);
  pending_target_ = committed_target_;
}

void MultiDtgPlus::RejectPendingTargetAfterPlanningFailure(
    bool hard_invalidation) {
  if (global_planner_mode_ != GlobalPlannerMode::SPECTRAL_V4)
    return;
  if (hard_invalidation &&
      SameMacroTarget(pending_target_, committed_target_)) {
    ROS_WARN(
        "[SpectralV4] invalidating committed target after local planning "
        "failure anchor=%u",
        committed_target_.anchor_h_id);
    committed_target_ = CommittedTarget();
  }
  pending_target_ = CommittedTarget();
}

GlobalPlanStatus MultiDtgPlus::PlanGlobalRouteV4(
    const Eigen::Vector3d& ps,
    vector<h_ptr>& route_h,
    vector<Eigen::Vector3d>& path2fh,
    double& d1) {
  global_plan_diagnostics_ = GlobalPlanDiagnostics();
  global_plan_diagnostics_.dtg_version = dtg_version_;
  global_plan_diagnostics_.frontier_version = frontier_version_;
  global_plan_diagnostics_.planner_mode =
      static_cast<int>(GlobalPlannerMode::SPECTRAL_V4);
  route_h.clear();
  path2fh.clear();
  d1 = 0.0;
  const double planning_start = ros::WallTime::now().toSec();
  const double now = ros::WallTime::now().toSec();

  auto finish = [&](GlobalPlanStatus status, const std::string& detail) {
    global_plan_diagnostics_.planning_ms =
        1000.0 * (ros::WallTime::now().toSec() - planning_start);
    if (status == GlobalPlanStatus::SUCCESS)
      global_plan_diagnostics_.selected_path_cost = d1;
    global_plan_diagnostics_.status = status;
    global_plan_diagnostics_.detail = detail;
    global_plan_diagnostics_.method = GlobalPlanMethod::SPECTRAL;
    global_plan_diagnostics_.route_size = route_h.size();
    global_plan_diagnostics_.v4_backend = region_snapshot_
        ? static_cast<int>(region_snapshot_->source_backend)
        : static_cast<int>(CutBackend::UNAVAILABLE);
    global_plan_diagnostics_.worker_graph_version = region_snapshot_
        ? region_snapshot_->graph_version : 0;
    global_plan_diagnostics_.region_state_version = region_snapshot_
        ? region_snapshot_->frontier_version : 0;
    global_plan_diagnostics_.committed_target_id =
        committed_target_.valid ? committed_target_.anchor_h_id : 0;
    PublishGlobalPlanDiagnostics();
    return status;
  };

  if (spectral_worker_ != nullptr) {
    const auto snapshot = spectral_worker_->TryConsume();
    if (snapshot != nullptr && ValidateRegionSnapshot(*snapshot))
      region_snapshot_ = snapshot;
    if (region_snapshot_ != nullptr &&
        !ValidateRegionSnapshot(*region_snapshot_))
      region_snapshot_.reset();
    const double delta_start = ros::WallTime::now().toSec();
    GraphDelta delta = BuildGraphDelta(ps);
    global_plan_diagnostics_.graph_delta_ms =
        1000.0 * (ros::WallTime::now().toSec() - delta_start);
    if (global_plan_diagnostics_.graph_delta_ms >
        spectral_v4_config_.main_thread_delta_budget_ms) {
      ROS_WARN_THROTTLE(
          2.0, "[SpectralV4] GraphDelta %.3f ms exceeds %.3f ms budget",
          global_plan_diagnostics_.graph_delta_ms,
          spectral_v4_config_.main_thread_delta_budget_ms);
    }
    if (!delta.empty())
      spectral_worker_->Submit(
          std::make_shared<const GraphDelta>(std::move(delta)));
  }

  GlobalRouteContext context;
  const GlobalPlanStatus collect =
      CollectActiveBoundaryRegions(ps, context, false);
  if (collect != GlobalPlanStatus::SUCCESS)
    return finish(collect, "V4: no reachable actionable frontier");
  global_plan_diagnostics_.active_anchor_count =
      context.active_hnodes.size();

  bool committed_semantically_valid = committed_target_.valid;
  if (committed_semantically_valid &&
      committed_target_.type == MacroTargetType::FRONTIER) {
    committed_semantically_valid = IsV4FrontierValid(
        committed_target_.frontier_id,
        committed_target_.viewpoint_id);
    if (committed_semantically_valid && EROI_ != nullptr) {
      const double current_unknown = std::max(
          0, EROI_->EROI_[committed_target_.frontier_id].unknown_num_);
      committed_target_.actual_gain_since_accept =
          std::max(0.0,
              committed_target_.initial_unknown_count - current_unknown);
      committed_target_.expected_gain = current_unknown;
    }
  } else if (committed_semantically_valid) {
    const h_ptr from =
        FindHnodeById(committed_target_.entry_from_h_id);
    const h_ptr to =
        FindHnodeById(committed_target_.entry_to_h_id);
    committed_semantically_valid = false;
    if (from != nullptr && to != nullptr) {
      for (const auto& edge : from->hh_edges_) {
        if (edge == nullptr) continue;
        const h_ptr other =
            edge->head_n_ == from ? edge->tail_n_ : edge->head_n_;
        if (other == to && std::isfinite(edge->length_) &&
            edge->length_ < kBlockedEdgeDistance) {
          committed_semantically_valid = true;
          break;
        }
      }
    }
    // Region IDs are derived state. Keep following the same physical entry
    // edge and only refresh its owner when a stable repartition arrives.
    if (committed_semantically_valid && region_snapshot_ != nullptr) {
      const auto owner =
          region_snapshot_->h_to_region.find(committed_target_.anchor_h_id);
      if (owner != region_snapshot_->h_to_region.end())
        committed_target_.region_id = owner->second;
    }
  }

  h_ptr committed_anchor = committed_semantically_valid
      ? FindHnodeById(committed_target_.anchor_h_id) : h_ptr();
  std::vector<Eigen::Vector3d> committed_path;
  double committed_distance = 0.0;
  bool committed_path_valid =
      committed_anchor != nullptr &&
      BuildPathToFirst(
          ps, context, committed_anchor,
          committed_path, committed_distance) ==
          GlobalPlanStatus::SUCCESS;
  if (committed_path_valid &&
      committed_target_.type == MacroTargetType::REGION_ENTRY &&
      PathBlockedForKnownSpace(committed_path, LRM_))
    committed_path_valid = false;
  const bool committed_region_entry_reached =
      committed_target_.valid &&
      committed_target_.type == MacroTargetType::REGION_ENTRY &&
      committed_path_valid &&
      committed_distance <= kRegionEntryArrivalToleranceMeters;
  if (committed_region_entry_reached) {
    committed_path_valid = false;
    ROS_INFO(
        "[SpectralV4] completed region entry edge=%u->%u region=%d "
        "distance=%.3f m",
        committed_target_.entry_from_h_id,
        committed_target_.entry_to_h_id,
        committed_target_.region_id,
        committed_distance);
  }

  if (committed_target_.valid && target_manager_.CanKeepCurrent(
          now, committed_target_, committed_semantically_valid,
          committed_path_valid, true)) {
    pending_target_ = committed_target_;
    route_h.push_back(committed_anchor);
    path2fh = std::move(committed_path);
    d1 = committed_distance;
    return finish(
        GlobalPlanStatus::SUCCESS, "V4: KEEP_COMMITTED");
  }
  if (!committed_semantically_valid || !committed_path_valid ||
      committed_target_.consecutive_low_gain >= 2)
    committed_target_ = CommittedTarget();

  std::vector<V4Candidate> candidates;
  if (committed_target_.valid && committed_anchor != nullptr) {
    committed_target_.exact_path_time = committed_distance;
    V4Candidate current;
    current.target = committed_target_;
    current.anchor = committed_anchor;
    current.gain = committed_target_.expected_gain;
    current.travel_time = committed_distance;
    current.path = committed_path;
    candidates.push_back(std::move(current));
  }

  int robot_region = -1;
  if (region_snapshot_ != nullptr && last_robot_h_id_ != 0U) {
    const auto owner = region_snapshot_->h_to_region.find(last_robot_h_id_);
    if (owner != region_snapshot_->h_to_region.end())
      robot_region = owner->second;
  }

  std::vector<std::pair<double, h_ptr>> local;
  auto collect_local = [&](bool restrict_to_robot_region) {
    local.clear();
    for (const auto& anchor : context.active_hnodes) {
      if (anchor == nullptr) continue;
      if (restrict_to_robot_region &&
          robot_region >= 0 && region_snapshot_ != nullptr) {
        const auto owner =
            region_snapshot_->h_to_region.find(anchor->id_);
        if (owner != region_snapshot_->h_to_region.end() &&
            owner->second != robot_region)
          continue;
      }
      local.emplace_back(
          FrontierGainForAnchor(anchor, EROI_), anchor);
    }
  };
  collect_local(true);
  const bool has_actionable_frontier_in_robot_region = !local.empty();
  // Fail open: a partition may rank raw reachable frontiers, but it must
  // never hide all of them.
  if (local.empty() && !context.active_hnodes.empty())
    collect_local(false);
  std::sort(local.begin(), local.end(),
      [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first) return lhs.first > rhs.first;
        return lhs.second->id_ < rhs.second->id_;
      });
  if (local.size() > static_cast<size_t>(
          std::max(1, spectral_v4_config_.local_frontier_top_k)))
    local.resize(static_cast<size_t>(
        spectral_v4_config_.local_frontier_top_k));
  for (const auto& entry : local) {
    bool duplicate = false;
    for (const auto& candidate : candidates)
      if (candidate.target.type == MacroTargetType::FRONTIER &&
          candidate.anchor == entry.second)
        duplicate = true;
    if (duplicate) continue;
    V4Candidate candidate;
    candidate.target.valid = true;
    candidate.target.type = MacroTargetType::FRONTIER;
    candidate.target.anchor_h_id = entry.second->id_;
    candidate.target.region_id = robot_region;
    candidate.target.selected_graph_version = dtg_version_;
    candidate.target.expected_gain = entry.first;
    candidate.anchor = entry.second;
    candidate.gain = entry.first;
    candidates.push_back(std::move(candidate));
  }

  // A region entry is a macro transition, not an alternative to an
  // actionable frontier in the region the UAV has just entered. Comparing
  // an adjacent region's aggregate gain with one local frontier's gain
  // otherwise makes the short reverse entry win and causes A-B-A-B motion.
  if (!has_actionable_frontier_in_robot_region &&
      robot_region >= 0 && region_snapshot_ != nullptr) {
    std::unordered_map<int, double> active_region_gain;
    for (const auto& anchor : context.active_hnodes) {
      if (anchor == nullptr) continue;
      const auto owner =
          region_snapshot_->h_to_region.find(anchor->id_);
      if (owner == region_snapshot_->h_to_region.end()) continue;
      active_region_gain[owner->second] +=
          FrontierGainForAnchor(anchor, EROI_);
    }

    std::unordered_map<int, std::vector<std::pair<int, double>>>
        region_adjacency;
    for (const auto& edge : region_snapshot_->quotient_edges) {
      if (!std::isfinite(edge.traversal_time) ||
          edge.traversal_time < 0.0)
        continue;
      const double weight = std::max(1.0e-6, edge.traversal_time);
      region_adjacency[edge.from_region].emplace_back(
          edge.to_region, weight);
      region_adjacency[edge.to_region].emplace_back(
          edge.from_region, weight);
    }

    using RegionQueueEntry = std::pair<double, int>;
    std::priority_queue<
        RegionQueueEntry,
        std::vector<RegionQueueEntry>,
        std::greater<RegionQueueEntry>> region_queue;
    std::unordered_map<int, double> distance_to_active_region;
    std::unordered_map<int, double> routed_region_gain;
    for (const auto& region : region_snapshot_->quotient_regions)
      distance_to_active_region[region.region_id] =
          std::numeric_limits<double>::infinity();
    for (const auto& gain : active_region_gain) {
      if (gain.second <= 0.0 ||
          distance_to_active_region.find(gain.first) ==
              distance_to_active_region.end())
        continue;
      distance_to_active_region[gain.first] = 0.0;
      routed_region_gain[gain.first] = gain.second;
      region_queue.emplace(0.0, gain.first);
    }
    while (!region_queue.empty()) {
      const double distance = region_queue.top().first;
      const int region = region_queue.top().second;
      region_queue.pop();
      const auto known = distance_to_active_region.find(region);
      if (known == distance_to_active_region.end() ||
          distance > known->second + 1.0e-9)
        continue;
      const auto neighbors = region_adjacency.find(region);
      if (neighbors == region_adjacency.end()) continue;
      for (const auto& neighbor : neighbors->second) {
        const double candidate_distance =
            distance + neighbor.second;
        auto next =
            distance_to_active_region.find(neighbor.first);
        if (next == distance_to_active_region.end() ||
            candidate_distance + 1.0e-9 >= next->second)
          continue;
        next->second = candidate_distance;
        routed_region_gain[neighbor.first] =
            routed_region_gain[region];
        region_queue.emplace(candidate_distance, neighbor.first);
      }
    }

    const auto robot_route =
        distance_to_active_region.find(robot_region);
    const double robot_route_distance =
        robot_route == distance_to_active_region.end()
            ? std::numeric_limits<double>::infinity()
            : robot_route->second;
    std::vector<QuotientEdge> adjacent;
    for (const auto& edge : region_snapshot_->quotient_edges)
      if (edge.from_region == robot_region ||
          edge.to_region == robot_region)
        adjacent.push_back(edge);
    std::sort(adjacent.begin(), adjacent.end(),
        [](const QuotientEdge& lhs, const QuotientEdge& rhs) {
          if (lhs.traversal_time != rhs.traversal_time)
            return lhs.traversal_time < rhs.traversal_time;
          if (lhs.entry_from != rhs.entry_from)
            return lhs.entry_from < rhs.entry_from;
          return lhs.entry_to < rhs.entry_to;
        });
    int added = 0;
    for (const auto& edge : adjacent) {
      if (added >= spectral_v4_config_.max_adjacent_region_entries)
        break;
      const bool leave_from = edge.from_region == robot_region;
      const uint32_t arrival =
          leave_from ? edge.entry_to : edge.entry_from;
      const int arrival_region =
          leave_from ? edge.to_region : edge.from_region;
      const auto arrival_route =
          distance_to_active_region.find(arrival_region);
      if (!std::isfinite(robot_route_distance) ||
          arrival_route == distance_to_active_region.end() ||
          !std::isfinite(arrival_route->second) ||
          arrival_route->second + 1.0e-6 >=
              robot_route_distance)
        continue;
      h_ptr anchor = FindHnodeById(arrival);
      if (anchor == nullptr) continue;
      bool duplicate = false;
      for (const auto& candidate : candidates)
        if (candidate.target.type == MacroTargetType::REGION_ENTRY &&
            candidate.target.anchor_h_id == arrival)
          duplicate = true;
      if (duplicate) continue;
      V4Candidate candidate;
      candidate.target.valid = true;
      candidate.target.type = MacroTargetType::REGION_ENTRY;
      candidate.target.anchor_h_id = arrival;
      candidate.target.entry_from_h_id =
          leave_from ? edge.entry_from : edge.entry_to;
      candidate.target.entry_to_h_id = arrival;
      candidate.target.region_id = arrival_region;
      candidate.target.selected_graph_version = dtg_version_;
      candidate.anchor = anchor;
      const auto routed_gain =
          routed_region_gain.find(arrival_region);
      if (routed_gain != routed_region_gain.end())
        candidate.gain = routed_gain->second;
      candidate.target.expected_gain = candidate.gain;
      candidates.push_back(std::move(candidate));
      ++added;
    }
  }

  for (auto& candidate : candidates) {
    if (std::isfinite(candidate.travel_time)) continue;
    double distance = 0.0;
    std::vector<Eigen::Vector3d> path;
    if (BuildPathToFirst(
            ps, context, candidate.anchor, path, distance) !=
        GlobalPlanStatus::SUCCESS)
      continue;
    candidate.travel_time = distance;
    candidate.path = std::move(path);
  }
  candidates.erase(
      std::remove_if(candidates.begin(), candidates.end(),
          [&](const V4Candidate& candidate) {
            return !std::isfinite(candidate.travel_time) ||
                candidate.path.empty() ||
                (candidate.target.type == MacroTargetType::REGION_ENTRY &&
                 candidate.travel_time <=
                     kRegionEntryArrivalToleranceMeters) ||
                (candidate.target.type == MacroTargetType::REGION_ENTRY &&
                 PathBlockedForKnownSpace(candidate.path, LRM_));
          }),
      candidates.end());
  if (candidates.empty())
    return finish(
        GlobalPlanStatus::PATH_FAILED, "V4: no reachable small candidate");

  std::vector<double> costs;
  std::vector<double> gains;
  for (const auto& candidate : candidates) {
    costs.push_back(candidate.travel_time);
    gains.push_back(candidate.gain);
  }
  int selected = target_manager_.SelectWithHysteresis(
      now, committed_target_, costs, gains);
  if (selected < 0 && committed_target_.valid) {
    for (size_t i = 0; i < candidates.size(); ++i)
      if (candidates[i].target.type == committed_target_.type &&
          candidates[i].target.anchor_h_id ==
              committed_target_.anchor_h_id) {
        selected = static_cast<int>(i);
        break;
      }
  }
  if (selected < 0) {
    selected = 0;
    for (size_t i = 1; i < candidates.size(); ++i) {
      const double lhs =
          costs[i] - std::log1p(std::max(0.0, gains[i]));
      const double rhs =
          costs[static_cast<size_t>(selected)] -
          std::log1p(std::max(
              0.0, gains[static_cast<size_t>(selected)]));
      if (lhs < rhs) selected = static_cast<int>(i);
    }
  }

  V4Candidate& choice = candidates[static_cast<size_t>(selected)];
  choice.target.exact_path_time = choice.travel_time;
  choice.target.selection_cost =
      choice.travel_time - std::log1p(std::max(0.0, choice.gain));
  choice.target.expected_gain = choice.gain;
  pending_target_ = choice.target;
  route_h.push_back(choice.anchor);
  path2fh = choice.path;
  d1 = choice.travel_time;
  DebugLineStrip(ps, route_h);

  std::ostringstream detail;
  detail << "V4: SELECT_"
         << (choice.target.type == MacroTargetType::FRONTIER
                 ? "FRONTIER" : "REGION_ENTRY")
         << " anchor=" << choice.target.anchor_h_id
         << " region=" << choice.target.region_id
         << " candidates=" << candidates.size();
  return finish(GlobalPlanStatus::SUCCESS, detail.str());
}
