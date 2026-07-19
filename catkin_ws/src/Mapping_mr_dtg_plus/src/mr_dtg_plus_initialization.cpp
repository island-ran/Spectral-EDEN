#include <mr_dtg_plus/mr_dtg_plus.h>
using namespace DTGPlus;

void MultiDtgPlus::AlignInit(ros::NodeHandle &nh, ros::NodeHandle &nh_private){
    std::string ns = ros::this_node::getName();
    nh_ = nh;
    nh_private_ = nh_private;
    nh_private.param(ns + "/Exp/UAV_id", uav_id_, 10);
    nh_private.param(ns + "/MR_DTG/show_edge_details", show_e_details_, false);
    nh_private.param(ns + "/MR_DTG/H_thresh", H_thresh_, 5.0);
    nh_private.param(ns + "/MR_DTG/MaintainHdists", maintain_h_dist_, false);
    nh_private.param(ns + "/MR_DTG/PlanDebug", debug_plan_, false);
    nh_private.param(ns + "/MR_DTG/AccGain", acc_gain_, 2.0);
    nh_private.param(ns + "/MR_DTG/YawGain", yaw_gain_, 2.5);
    nh_private.param(ns + "/MR_DTG/YawSliceAng", yaw_slice_ang_, 0.3);
    nh_private.param(ns + "/MR_DTG/RefineNum", refine_num_, 5);
    nh_private.param(ns + "/MR_DTG/GainThreshFactor", g_thr_fac_, 0.9);
    nh_private.param(ns + "/block_map/sensor_max_range", sensor_range_, 5.0);
    nh_private.param(ns + "/Exp/drone_num", drone_num_, 1);
    nh_private.param(ns + "/Exp/lambdaE", lambda_e_, 0.5);
    nh_private.param(ns + "/Exp/lambdaA", lambda_a_, 0.5);
    nh_private.param(ns + "/opt/MaxVel", v_max_, 0.5);
    nh_private.param(ns + "/opt/MaxAcc", a_max_, 0.5);
    nh_private.param(ns + "/opt/YawVel", yv_max_, 0.5);

    // ── Global planner mode (startup binary choice) ──
    std::string planner_mode_str;
    nh_private.param(ns + "/MR_DTG/GlobalPlanner/mode", planner_mode_str,
        std::string("spectral_v4"));

    if(planner_mode_str == "eden"){
        global_planner_mode_ = GlobalPlannerMode::EDEN_BASELINE;
        ROS_WARN("[GlobalPlanner] mode=EDEN_BASELINE  ns=%s", ns.c_str());
    }
    else if(planner_mode_str == "spectral_v4"){
        global_planner_mode_ = GlobalPlannerMode::SPECTRAL_V4;
        ROS_WARN("[GlobalPlanner] mode=SPECTRAL_V4  ns=%s", ns.c_str());
    }
    else{
        ROS_FATAL_STREAM("[GlobalPlanner] Unknown mode: " << planner_mode_str
            << ". Valid modes: eden, spectral_v4");
        throw std::runtime_error("invalid global planner mode: " + planner_mode_str);
    }

    // ── Load V4 params (used only by SPECTRAL_V4 mode) ──
    if(global_planner_mode_ == GlobalPlannerMode::SPECTRAL_V4){
        nh_private.param(ns + "/MR_DTG/SpectralV4/log_diagnostics", spectral_v4_config_.log_diagnostics, true);

        // Skeleton
        nh_private.param(ns + "/MR_DTG/SpectralV4/skeleton_max_nodes", spectral_v4_config_.skeleton_max_nodes, 512);
        nh_private.param(ns + "/MR_DTG/SpectralV4/max_leaf_regions", spectral_v4_config_.max_leaf_regions, 6);
        nh_private.param(ns + "/MR_DTG/SpectralV4/max_recursive_depth", spectral_v4_config_.max_recursive_depth, 3);
        nh_private.param(ns + "/MR_DTG/SpectralV4/length_decay", spectral_v4_config_.length_decay, 1.0);
        nh_private.param(ns + "/MR_DTG/SpectralV4/clearance_reference", spectral_v4_config_.clearance_reference, 1.0);
        nh_private.param(ns + "/MR_DTG/SpectralV4/clearance_power", spectral_v4_config_.clearance_power, 1.0);
        nh_private.param(ns + "/MR_DTG/SpectralV4/min_edge_weight", spectral_v4_config_.min_edge_weight, 1.0e-6);
        nh_private.param(ns + "/MR_DTG/SpectralV4/clearance_binary_steps", spectral_v4_config_.clearance_binary_steps, 5);
        nh_private.param(ns + "/MR_DTG/SpectralV4/clearance_max_samples", spectral_v4_config_.clearance_max_samples, 64);

        // Incremental Fiedler
        nh_private.param(ns + "/MR_DTG/SpectralV4/exact_max_nodes", spectral_v4_config_.exact_max_nodes, 48);
        nh_private.param(ns + "/MR_DTG/SpectralV4/exact_max_iterations", spectral_v4_config_.exact_max_iterations, 12);
        nh_private.param(ns + "/MR_DTG/SpectralV4/exact_residual_threshold", spectral_v4_config_.exact_residual_threshold, 1.0e-4);

        // Nyström
        nh_private.param(ns + "/MR_DTG/SpectralV4/landmark_count", spectral_v4_config_.landmark_count, 32);
        nh_private.param(ns + "/MR_DTG/SpectralV4/landmark_max_count", spectral_v4_config_.landmark_max_count, 48);
        nh_private.param(ns + "/MR_DTG/SpectralV4/nearest_landmarks_per_node", spectral_v4_config_.nearest_landmarks_per_node, 4);
        nh_private.param(ns + "/MR_DTG/SpectralV4/nystrom_max_support", spectral_v4_config_.nystrom_max_support, 512);
        nh_private.param(ns + "/MR_DTG/SpectralV4/nystrom_residual_threshold", spectral_v4_config_.nystrom_residual_threshold, 5.0e-4);

        // Local APPR
        nh_private.param(ns + "/MR_DTG/SpectralV4/ppr_alpha", spectral_v4_config_.ppr_alpha, 0.15);
        nh_private.param(ns + "/MR_DTG/SpectralV4/ppr_epsilon", spectral_v4_config_.ppr_epsilon, 1.0e-4);
        nh_private.param(ns + "/MR_DTG/SpectralV4/ppr_max_pushes", spectral_v4_config_.ppr_max_pushes, 20000);
        nh_private.param(ns + "/MR_DTG/SpectralV4/ppr_max_support_nodes", spectral_v4_config_.ppr_max_support_nodes, 512);

        // Bisection acceptance
        nh_private.param(ns + "/MR_DTG/SpectralV4/max_ncut", spectral_v4_config_.max_ncut, 0.30);
        nh_private.param(ns + "/MR_DTG/SpectralV4/min_balance", spectral_v4_config_.min_balance, 0.20);
        nh_private.param(ns + "/MR_DTG/SpectralV4/proposal_confirm_versions", spectral_v4_config_.proposal_confirm_versions, 2);
        nh_private.param(ns + "/MR_DTG/SpectralV4/proposal_jaccard_threshold", spectral_v4_config_.proposal_jaccard_threshold, 0.80);
        nh_private.param(ns + "/MR_DTG/SpectralV4/max_region_version_lag", spectral_v4_config_.max_region_version_lag, 2);
        nh_private.param(ns + "/MR_DTG/SpectralV4/region_invalidation_ratio", spectral_v4_config_.region_invalidation_ratio, 0.30);

        // Target commitment
        nh_private.param(ns + "/MR_DTG/SpectralV4/local_frontier_top_k", spectral_v4_config_.local_frontier_top_k, 8);
        nh_private.param(ns + "/MR_DTG/SpectralV4/max_adjacent_region_entries", spectral_v4_config_.max_adjacent_region_entries, 2);
        nh_private.param(ns + "/MR_DTG/SpectralV4/min_target_commit_sec", spectral_v4_config_.min_target_commit_sec, 1.5);
        nh_private.param(ns + "/MR_DTG/SpectralV4/target_switch_abs_margin_sec", spectral_v4_config_.target_switch_abs_margin_sec, 0.7);
        nh_private.param(ns + "/MR_DTG/SpectralV4/target_switch_relative_margin", spectral_v4_config_.target_switch_relative_margin, 0.10);
        nh_private.param(ns + "/MR_DTG/SpectralV4/actual_gain_epsilon", spectral_v4_config_.actual_gain_epsilon, 10.0);

        // Worker
        nh_private.param(ns + "/MR_DTG/SpectralV4/main_thread_delta_budget_ms", spectral_v4_config_.main_thread_delta_budget_ms, 2.0);

        if(spectral_v4_config_.skeleton_max_nodes < 4 ||
           spectral_v4_config_.max_leaf_regions < 1 ||
           spectral_v4_config_.max_recursive_depth < 0 ||
           spectral_v4_config_.length_decay < 0.0 ||
           spectral_v4_config_.clearance_reference <= 0.0 ||
           spectral_v4_config_.clearance_power < 0.0 ||
           spectral_v4_config_.min_edge_weight <= 0.0 ||
           spectral_v4_config_.clearance_binary_steps < 0 ||
           spectral_v4_config_.clearance_max_samples < 2 ||
           spectral_v4_config_.exact_max_nodes < 3 ||
           spectral_v4_config_.exact_max_iterations < 1 ||
           spectral_v4_config_.exact_residual_threshold <= 0.0 ||
           spectral_v4_config_.landmark_count < 3 ||
           spectral_v4_config_.landmark_count >
               spectral_v4_config_.landmark_max_count ||
           spectral_v4_config_.nearest_landmarks_per_node < 1 ||
           spectral_v4_config_.nystrom_max_support <
               spectral_v4_config_.exact_max_nodes ||
           spectral_v4_config_.nystrom_residual_threshold <= 0.0 ||
           spectral_v4_config_.ppr_alpha <= 0.0 ||
           spectral_v4_config_.ppr_alpha >= 1.0 ||
           spectral_v4_config_.ppr_epsilon <= 0.0 ||
           spectral_v4_config_.min_balance <= 0.0 ||
           spectral_v4_config_.min_balance >= 0.5 ||
           spectral_v4_config_.max_ncut <= 0.0 ||
           spectral_v4_config_.ppr_max_pushes <= 0 ||
           spectral_v4_config_.ppr_max_support_nodes < 4 ||
           spectral_v4_config_.proposal_confirm_versions < 1 ||
           spectral_v4_config_.proposal_jaccard_threshold < 0.0 ||
           spectral_v4_config_.proposal_jaccard_threshold > 1.0 ||
           spectral_v4_config_.max_region_version_lag < 0 ||
           spectral_v4_config_.region_invalidation_ratio <= 0.0 ||
           spectral_v4_config_.region_invalidation_ratio > 1.0 ||
           spectral_v4_config_.local_frontier_top_k < 1 ||
           spectral_v4_config_.max_adjacent_region_entries < 0 ||
           spectral_v4_config_.min_target_commit_sec < 0.0 ||
           spectral_v4_config_.target_switch_abs_margin_sec < 0.0 ||
           spectral_v4_config_.target_switch_relative_margin < 0.0 ||
           spectral_v4_config_.actual_gain_epsilon < 0.0 ||
           spectral_v4_config_.main_thread_delta_budget_ms <= 0.0){
            ROS_FATAL("[GlobalPlanner] invalid SpectralV4 budget configuration");
            throw std::runtime_error("invalid SpectralV4 configuration");
        }

        // Create and start the permanent spectral worker
        spectral_worker_ = std::make_unique<SpectralWorker>(spectral_v4_config_);
        spectral_worker_->Start();

        // Initialize V4 state
        target_manager_ = CommittedTargetManager(spectral_v4_config_);
        committed_target_ = CommittedTarget();
        pending_target_ = CommittedTarget();
        region_snapshot_ = nullptr;
        v4_bootstrap_pending_ = true;
        frontier_anchor_index_.clear();
        last_robot_h_id_ = 0;

        ROS_WARN("[GlobalPlanner] SpectralV4 Worker started  "
                 "exact_max_nodes=%d  landmark_count=%d  ppr_max_pushes=%d  "
                 "local_frontier_top_k=%d  min_target_commit=%.1fs  "
                 "switch_margin=%.2f/%.0f%%  ns=%s",
                 spectral_v4_config_.exact_max_nodes,
                 spectral_v4_config_.landmark_count,
                 spectral_v4_config_.ppr_max_pushes,
                 spectral_v4_config_.local_frontier_top_k,
                 spectral_v4_config_.min_target_commit_sec,
                 spectral_v4_config_.target_switch_abs_margin_sec,
                 spectral_v4_config_.target_switch_relative_margin * 100.0,
                 ns.c_str());
    }
    else{
        // EDEN mode: no worker
        spectral_worker_ = nullptr;
        ROS_WARN("[GlobalPlanner] EDEN_BASELINE mode: no SpectralWorker created");
    }

    // ── Common initialization ──
    dtg_version_ = 1;
    frontier_version_ = 1;
    global_plan_diagnostics_ = GlobalPlanDiagnostics();

    origin_ = LRM_->origin_;
    vox_scl_ = LRM_->blockscale_;
    map_upbd_ = LRM_->map_upbd_;
    map_lowbd_ = LRM_->map_lowbd_;
    vox_num_ = LRM_->block_num_;
    cur_hid_ = uav_id_;
    cout<<"cur_hid_ 0:"<<cur_hid_<<endl;
    if(drone_num_ > 1){
        use_swarm_ = true;
    }
    topo_pub_ = nh.advertise<visualization_msgs::MarkerArray>(ns + "/MR_DTG/Graph", 10);
    debug_pub_ = nh.advertise<visualization_msgs::Marker>(ns + "/MR_DTG/Debug", 10);
    spectral_diag_pub_ = nh.advertise<std_msgs::Float64MultiArray>(
        ns + "/MR_DTG/SpectralDiagnostics", 10);
    spectral_event_pub_ = nh.advertise<std_msgs::String>(
        ns + "/MR_DTG/SpectralEvent", 10);
    show_timer_ = nh.createTimer(ros::Duration(0.2), &MultiDtgPlus::ShowAll, this);
    if(maintain_h_dist_) {
        maintain_timer_ = nh.createTimer(ros::Duration(0.25), &MultiDtgPlus::DistMaintTimerCallback, this);
        eng_ = default_random_engine(rd_());
    }
    H_depot_.resize(vox_num_(0) * vox_num_(1) * vox_num_(2));
    F_depot_.resize(vox_num_(0) * vox_num_(1) * vox_num_(2));


    time_t now = time(0);
    std::string Time_ = ctime(&now);
    tm* t=localtime(&now);
    string path = "/home/charliedog/rosprojects/DoomSea/debug/"+to_string(t->tm_year+1900)+"_"+to_string(t->tm_mon+1)+"_"+to_string(t->tm_mday)
    +"_"+to_string(t->tm_hour)+"_"+to_string(t->tm_min)+"_"+to_string(t->tm_sec);
    if(debug_plan_) debug_f_.open(path+"_dtg_debug"+".txt", std::ios::out);
}
