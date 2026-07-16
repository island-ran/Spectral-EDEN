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

    int spectral_graph_mode = static_cast<int>(spectral_exec_config_.graph_mode);
    int spectral_weight_mode = static_cast<int>(spectral_config_.weight_mode);
    int min_spectral_nodes = static_cast<int>(spectral_config_.min_spectral_nodes);
    int max_spectral_nodes = static_cast<int>(spectral_config_.max_spectral_nodes);
    int min_cluster_size = static_cast<int>(spectral_config_.min_cluster_size);
    int dense_solver_max_nodes = static_cast<int>(spectral_config_.dense_solver_max_nodes);
    int iterative_max_iterations = static_cast<int>(spectral_config_.iterative_max_iterations);
    nh_private.param(ns + "/MR_DTG/Spectral/Enable", spectral_exec_config_.enabled, false);
    nh_private.param(ns + "/MR_DTG/Spectral/FallbackToEOHDT", spectral_exec_config_.fallback_to_eohdt, true);
    nh_private.param(ns + "/MR_DTG/Spectral/PartitionEnable", spectral_exec_config_.partition_enabled, true);
    nh_private.param(ns + "/MR_DTG/Spectral/RegionLockEnable", spectral_exec_config_.region_lock_enabled, true);
    nh_private.param(ns + "/MR_DTG/Spectral/LogDiagnostics", spectral_exec_config_.log_diagnostics, true);
    nh_private.param(ns + "/MR_DTG/Spectral/GraphMode", spectral_graph_mode, 1);
    nh_private.param(ns + "/MR_DTG/Spectral/WeightMode", spectral_weight_mode, 0);
    nh_private.param(ns + "/MR_DTG/Spectral/LengthDecay", spectral_config_.length_decay, 1.0);
    nh_private.param(ns + "/MR_DTG/Spectral/LengthScaleMultiplier", spectral_config_.length_scale_multiplier, 1.0);
    nh_private.param(ns + "/MR_DTG/Spectral/ClearanceReference", spectral_config_.clearance_reference, 1.0);
    nh_private.param(ns + "/MR_DTG/Spectral/ClearancePower", spectral_config_.clearance_power, 1.0);
    nh_private.param(ns + "/MR_DTG/Spectral/BetweennessWeight", spectral_config_.betweenness_weight, 1.0);
    nh_private.param(ns + "/MR_DTG/Spectral/MinEdgeWeight", spectral_config_.min_edge_weight, 1.0e-6);
    nh_private.param(ns + "/MR_DTG/Spectral/MinSpectralNodes", min_spectral_nodes, 3);
    nh_private.param(ns + "/MR_DTG/Spectral/MaxSpectralNodes", max_spectral_nodes, 200);
    nh_private.param(ns + "/MR_DTG/Spectral/MinClusterSize", min_cluster_size, 2);
    nh_private.param(ns + "/MR_DTG/Spectral/MinClusterVolumeFraction", spectral_config_.min_cluster_volume_fraction, 0.05);
    nh_private.param(ns + "/MR_DTG/Spectral/MaxResidualNorm", spectral_config_.max_residual_norm, 1.0e-8);
    nh_private.param(ns + "/MR_DTG/Spectral/MinPartitionNodes", spectral_exec_config_.min_partition_nodes, 6);
    nh_private.param(ns + "/MR_DTG/Spectral/Lambda2Threshold", spectral_exec_config_.lambda2_threshold, 0.15);
    nh_private.param(ns + "/MR_DTG/Spectral/EigengapThreshold", spectral_exec_config_.eigengap_threshold, 0.02);
    nh_private.param(ns + "/MR_DTG/Spectral/NcutThreshold", spectral_exec_config_.ncut_threshold, 0.30);
    nh_private.param(ns + "/MR_DTG/Spectral/Lambda2EmaAlpha", spectral_exec_config_.lambda2_ema_alpha, 0.90);
    nh_private.param(ns + "/MR_DTG/Spectral/Lambda2Ratio", spectral_exec_config_.lambda2_ratio, 1.0);
    nh_private.param(ns + "/MR_DTG/Spectral/TriggerPersistence", spectral_exec_config_.trigger_persistence, 2);
    nh_private.param(ns + "/MR_DTG/Spectral/ReleasePersistence", spectral_exec_config_.release_persistence, 3);
    nh_private.param(ns + "/MR_DTG/Spectral/RegionDoneCycles", spectral_exec_config_.region_done_cycles, 3);
    nh_private.param(ns + "/MR_DTG/Spectral/UnreachableConfirmCycles", spectral_exec_config_.unreachable_confirm_cycles, 3);
    nh_private.param(ns + "/MR_DTG/Spectral/ClearanceBinarySteps", spectral_exec_config_.clearance_binary_steps, 5);
    nh_private.param(ns + "/MR_DTG/Spectral/ClearanceMaxSamples", spectral_exec_config_.clearance_max_samples, 64);
    nh_private.param(ns + "/MR_DTG/Spectral/RegionMatchThreshold", spectral_exec_config_.region_match_threshold, 0.30);
    nh_private.param(ns + "/MR_DTG/Spectral/LabelHysteresis", spectral_exec_config_.label_hysteresis, 0.02);
    nh_private.param(ns + "/MR_DTG/Spectral/RouteJumpWeight", spectral_exec_config_.route_jump_weight, 1.0);
    nh_private.param(ns + "/MR_DTG/Spectral/RouteTerminalBias", spectral_exec_config_.route_terminal_bias, 0.25);
    nh_private.param(ns + "/MR_DTG/Spectral/SpectralViewWeight", spectral_exec_config_.spectral_view_weight, 0.0);
    nh_private.param(ns + "/MR_DTG/Spectral/CrossRegionWeight", spectral_exec_config_.cross_region_weight, 0.0);
    nh_private.param(ns + "/MR_DTG/Spectral/TimeBudgetMs", spectral_exec_config_.spectral_time_budget_ms, 10.0);
    nh_private.param(ns + "/MR_DTG/Spectral/UpdatePeriod", spectral_exec_config_.update_period, 0.20);

    // Spectral-EDEN v2 parameters intentionally coexist with the original
    // Spectral block so old experiment files remain readable.  V2 values are
    // loaded last and therefore take precedence when present.
    nh_private.param(ns + "/MR_DTG/SpectralV2/enabled", spectral_exec_config_.enabled,
        spectral_exec_config_.enabled);
    nh_private.param(ns + "/MR_DTG/SpectralV2/async_solve", spectral_exec_config_.async_solve, true);
    nh_private.param(ns + "/MR_DTG/SpectralV2/corridor_compression",
        spectral_exec_config_.corridor_compression, true);
    nh_private.param(ns + "/MR_DTG/SpectralV2/min_spectral_nodes", min_spectral_nodes, 10);
    nh_private.param(ns + "/MR_DTG/SpectralV2/max_spectral_nodes", max_spectral_nodes, 80);
    nh_private.param(ns + "/MR_DTG/SpectralV2/dense_solver_max_nodes",
        dense_solver_max_nodes, 40);
    nh_private.param(ns + "/MR_DTG/SpectralV2/iterative_max_iterations",
        iterative_max_iterations, 100);
    nh_private.param(ns + "/MR_DTG/SpectralV2/iterative_tolerance",
        spectral_config_.iterative_tolerance, 1.0e-10);
    nh_private.param(ns + "/MR_DTG/SpectralV2/iterative_shift",
        spectral_config_.iterative_shift, 1.0e-4);
    nh_private.param(ns + "/MR_DTG/SpectralV2/spectral_knn", spectral_exec_config_.spectral_knn, 5);
    nh_private.param(ns + "/MR_DTG/SpectralV2/spectral_min_update_interval",
        spectral_exec_config_.spectral_min_update_interval, 0.5);
    nh_private.param(ns + "/MR_DTG/SpectralV2/spectral_max_update_interval",
        spectral_exec_config_.spectral_max_update_interval, 3.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/dirty_node_changes",
        spectral_exec_config_.dirty_node_changes, 3);
    nh_private.param(ns + "/MR_DTG/SpectralV2/dirty_edge_changes",
        spectral_exec_config_.dirty_edge_changes, 5);
    nh_private.param(ns + "/MR_DTG/SpectralV2/spectral_time_budget_ms",
        spectral_exec_config_.spectral_time_budget_ms, 5.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/max_spectral_epoch_age",
        spectral_exec_config_.max_spectral_epoch_age, 2);
    nh_private.param(ns + "/MR_DTG/SpectralV2/spectral_timeout_limit",
        spectral_exec_config_.spectral_timeout_limit, 3);

    nh_private.param(ns + "/MR_DTG/SpectralV2/confidence_weight_ncut",
        spectral_exec_config_.confidence_weight_ncut, 0.30);
    nh_private.param(ns + "/MR_DTG/SpectralV2/confidence_weight_eigengap",
        spectral_exec_config_.confidence_weight_eigengap, 0.25);
    nh_private.param(ns + "/MR_DTG/SpectralV2/confidence_weight_relative_lambda2",
        spectral_exec_config_.confidence_weight_relative_lambda2, 0.15);
    nh_private.param(ns + "/MR_DTG/SpectralV2/confidence_weight_bottleneck",
        spectral_exec_config_.confidence_weight_bottleneck, 0.20);
    nh_private.param(ns + "/MR_DTG/SpectralV2/confidence_weight_balance",
        spectral_exec_config_.confidence_weight_balance, 0.10);
    nh_private.param(ns + "/MR_DTG/SpectralV2/confidence_ncut_soft",
        spectral_exec_config_.confidence_ncut_soft, 0.60);
    nh_private.param(ns + "/MR_DTG/SpectralV2/confidence_eigengap_soft",
        spectral_exec_config_.confidence_eigengap_soft, 0.15);
    nh_private.param(ns + "/MR_DTG/SpectralV2/partition_confidence_on",
        spectral_exec_config_.partition_confidence_on, 0.70);
    nh_private.param(ns + "/MR_DTG/SpectralV2/partition_confidence_off",
        spectral_exec_config_.partition_confidence_off, 0.40);
    nh_private.param(ns + "/MR_DTG/SpectralV2/partition_on_persistence",
        spectral_exec_config_.trigger_persistence, 3);
    nh_private.param(ns + "/MR_DTG/SpectralV2/partition_off_persistence",
        spectral_exec_config_.release_persistence, 5);
    nh_private.param(ns + "/MR_DTG/SpectralV2/partition_grace_epochs",
        spectral_exec_config_.partition_grace_epochs, 8);
    nh_private.param(ns + "/MR_DTG/SpectralV2/label_hysteresis_ratio",
        spectral_exec_config_.label_hysteresis, 0.08);
    nh_private.param(ns + "/MR_DTG/SpectralV2/region_match_threshold",
        spectral_exec_config_.region_match_threshold, 0.50);

    nh_private.param(ns + "/MR_DTG/SpectralV2/max_route_regret",
        spectral_exec_config_.max_route_regret, 0.05);
    nh_private.param(ns + "/MR_DTG/SpectralV2/switch_penalty_base",
        spectral_exec_config_.switch_penalty_base, 2.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/revisit_penalty_weight",
        spectral_exec_config_.revisit_penalty_weight, 1.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/neighbor_override_distance",
        spectral_exec_config_.neighbor_override_distance, 2.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/neighbor_override_margin",
        spectral_exec_config_.neighbor_override_margin, 0.5);
    nh_private.param(ns + "/MR_DTG/SpectralV2/lock_debt_decay",
        spectral_exec_config_.lock_debt_decay, 0.90);
    nh_private.param(ns + "/MR_DTG/SpectralV2/lock_debt_max",
        spectral_exec_config_.lock_debt_max, 8.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/recovery_duration",
        spectral_exec_config_.recovery_duration, 5.0);

    nh_private.param(ns + "/MR_DTG/SpectralV2/late_stage_frontier_count",
        spectral_exec_config_.late_stage_frontier_count, 6);
    nh_private.param(ns + "/MR_DTG/SpectralV2/late_stage_reactivate_frontier_count",
        spectral_exec_config_.late_stage_reactivate_frontier_count, 12);
    nh_private.param(ns + "/MR_DTG/SpectralV2/late_stage_total_gain",
        spectral_exec_config_.late_stage_total_gain, 50.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/late_stage_no_cut_epochs",
        spectral_exec_config_.late_stage_no_cut_epochs, 10);
    nh_private.param(ns + "/MR_DTG/SpectralV2/frontier_low_gain_cycles",
        spectral_exec_config_.frontier_low_gain_cycles, 2);
    nh_private.param(ns + "/MR_DTG/SpectralV2/frontier_actual_gain_eps",
        spectral_exec_config_.frontier_actual_gain_eps, 10);
    nh_private.param(ns + "/MR_DTG/SpectralV2/frontier_expected_gain_eps",
        spectral_exec_config_.frontier_expected_gain_eps, 1.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/frontier_quarantine_time",
        spectral_exec_config_.frontier_quarantine_time, 5.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/region_done_confirm_cycles",
        spectral_exec_config_.region_done_cycles, 3);
    nh_private.param(ns + "/MR_DTG/SpectralV2/region_stall_window",
        spectral_exec_config_.region_stall_window, 8.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/region_stall_timeout",
        spectral_exec_config_.region_stall_timeout, 12.0);
    nh_private.param(ns + "/MR_DTG/SpectralV2/repeat_target_limit",
        spectral_exec_config_.repeat_target_limit, 3);

    spectral_exec_config_.graph_mode = spectral_graph_mode == 0
        ? SpectralGraphMode::ACTIVE_COMPLETE : SpectralGraphMode::SUPPORT_SPARSE;
    if(spectral_weight_mode <= 0){
        spectral_config_.weight_mode = SpectralWeightMode::DISTANCE;
    }
    else if(spectral_weight_mode == 1){
        spectral_config_.weight_mode = SpectralWeightMode::DISTANCE_CLEARANCE;
    }
    else{
        spectral_config_.weight_mode = SpectralWeightMode::DISTANCE_CLEARANCE_BETWEENNESS;
    }
    spectral_config_.min_spectral_nodes = std::max(3, min_spectral_nodes);
    spectral_config_.max_spectral_nodes = std::max(0, max_spectral_nodes);
    spectral_config_.dense_solver_max_nodes = static_cast<size_t>(
        std::max(3, dense_solver_max_nodes));
    spectral_config_.iterative_max_iterations = static_cast<size_t>(
        std::max(1, iterative_max_iterations));
    spectral_config_.iterative_tolerance = std::max(
        spectral_config_.numeric_epsilon, spectral_config_.iterative_tolerance);
    spectral_config_.iterative_shift = std::max(
        spectral_config_.numeric_epsilon, spectral_config_.iterative_shift);
    spectral_config_.min_cluster_size = std::max(1, min_cluster_size);
    spectral_exec_config_.trigger_persistence = std::max(1, spectral_exec_config_.trigger_persistence);
    spectral_exec_config_.release_persistence = std::max(1, spectral_exec_config_.release_persistence);
    spectral_exec_config_.region_done_cycles = std::max(1, spectral_exec_config_.region_done_cycles);
    spectral_exec_config_.unreachable_confirm_cycles = std::max(1, spectral_exec_config_.unreachable_confirm_cycles);
    spectral_exec_config_.clearance_binary_steps = std::max(1, spectral_exec_config_.clearance_binary_steps);
    spectral_exec_config_.clearance_max_samples = std::max(2, spectral_exec_config_.clearance_max_samples);
    spectral_exec_config_.spectral_knn = std::max(1, spectral_exec_config_.spectral_knn);
    spectral_exec_config_.dirty_node_changes = std::max(1, spectral_exec_config_.dirty_node_changes);
    spectral_exec_config_.dirty_edge_changes = std::max(1, spectral_exec_config_.dirty_edge_changes);
    spectral_exec_config_.partition_grace_epochs = std::max(0, spectral_exec_config_.partition_grace_epochs);
    spectral_exec_config_.max_spectral_epoch_age = std::max(
        0, spectral_exec_config_.max_spectral_epoch_age);
    spectral_exec_config_.spectral_timeout_limit = std::max(
        1, spectral_exec_config_.spectral_timeout_limit);
    spectral_exec_config_.lambda2_ema_alpha = std::max(0.0,
        std::min(0.999, spectral_exec_config_.lambda2_ema_alpha));
    spectral_exec_config_.lambda2_ratio = std::max(0.0, spectral_exec_config_.lambda2_ratio);
    spectral_exec_config_.region_match_threshold = std::max(0.0,
        std::min(1.0, spectral_exec_config_.region_match_threshold));
    spectral_exec_config_.label_hysteresis = std::max(0.0, spectral_exec_config_.label_hysteresis);
    spectral_exec_config_.route_jump_weight = std::max(0.0, spectral_exec_config_.route_jump_weight);
    spectral_exec_config_.route_terminal_bias = std::max(0.0, spectral_exec_config_.route_terminal_bias);
    spectral_exec_config_.spectral_view_weight = std::max(0.0, spectral_exec_config_.spectral_view_weight);
    spectral_exec_config_.cross_region_weight = std::max(0.0, spectral_exec_config_.cross_region_weight);
    spectral_exec_config_.spectral_time_budget_ms = std::max(0.0, spectral_exec_config_.spectral_time_budget_ms);
    spectral_exec_config_.update_period = std::max(0.0, spectral_exec_config_.update_period);
    spectral_exec_config_.spectral_min_update_interval = std::max(0.0,
        spectral_exec_config_.spectral_min_update_interval);
    spectral_exec_config_.spectral_max_update_interval = std::max(
        spectral_exec_config_.spectral_min_update_interval,
        spectral_exec_config_.spectral_max_update_interval);
    spectral_exec_config_.partition_confidence_on = Clamp01(
        spectral_exec_config_.partition_confidence_on);
    spectral_exec_config_.partition_confidence_off = Clamp01(
        spectral_exec_config_.partition_confidence_off);
    if(spectral_exec_config_.partition_confidence_on <
       spectral_exec_config_.partition_confidence_off){
        std::swap(spectral_exec_config_.partition_confidence_on,
                  spectral_exec_config_.partition_confidence_off);
    }
    spectral_exec_config_.max_route_regret = std::max(0.0,
        spectral_exec_config_.max_route_regret);
    spectral_exec_config_.switch_penalty_base = std::max(0.0,
        spectral_exec_config_.switch_penalty_base);
    spectral_exec_config_.revisit_penalty_weight = std::max(0.0,
        spectral_exec_config_.revisit_penalty_weight);
    spectral_exec_config_.neighbor_override_distance = std::max(0.0,
        spectral_exec_config_.neighbor_override_distance);
    spectral_exec_config_.neighbor_override_margin = std::max(0.0,
        spectral_exec_config_.neighbor_override_margin);
    spectral_exec_config_.lock_debt_decay = Clamp01(spectral_exec_config_.lock_debt_decay);
    spectral_exec_config_.lock_debt_max = std::max(0.0, spectral_exec_config_.lock_debt_max);
    spectral_exec_config_.recovery_duration = std::max(0.0,
        spectral_exec_config_.recovery_duration);
    spectral_exec_config_.frontier_quarantine_time = std::max(0.0,
        spectral_exec_config_.frontier_quarantine_time);
    spectral_exec_config_.frontier_expected_gain_eps = std::max(0.0,
        spectral_exec_config_.frontier_expected_gain_eps);
    spectral_exec_config_.region_stall_window = std::max(0.0,
        spectral_exec_config_.region_stall_window);
    spectral_exec_config_.region_stall_timeout = std::max(
        spectral_exec_config_.region_stall_window,
        spectral_exec_config_.region_stall_timeout);
    // V2 never uses the legacy hard regional filter.
    spectral_exec_config_.region_lock_enabled = false;

    partition_confidence_config_.ncut_soft_threshold =
        spectral_exec_config_.confidence_ncut_soft;
    partition_confidence_config_.eigengap_soft_threshold =
        spectral_exec_config_.confidence_eigengap_soft;
    partition_confidence_config_.weight_ncut =
        spectral_exec_config_.confidence_weight_ncut;
    partition_confidence_config_.weight_eigengap =
        spectral_exec_config_.confidence_weight_eigengap;
    partition_confidence_config_.weight_relative_lambda2 =
        spectral_exec_config_.confidence_weight_relative_lambda2;
    partition_confidence_config_.weight_bottleneck =
        spectral_exec_config_.confidence_weight_bottleneck;
    partition_confidence_config_.weight_balance =
        spectral_exec_config_.confidence_weight_balance;
    spectral_mode_config_.confidence_on = spectral_exec_config_.partition_confidence_on;
    spectral_mode_config_.confidence_off = spectral_exec_config_.partition_confidence_off;
    spectral_mode_config_.on_persistence = spectral_exec_config_.trigger_persistence;
    spectral_mode_config_.off_persistence = spectral_exec_config_.release_persistence;
    spectral_mode_config_.grace_epochs = spectral_exec_config_.partition_grace_epochs;
    spectral_router_.set_config(spectral_config_);

    dtg_version_ = 1;
    frontier_version_ = 1;
    frontier_epoch_ = 0;
    spectral_epoch_ = 0;
    spectral_dirty_ = true;
    active_region_id_ = -1;
    next_region_id_ = 0;
    lambda2_ema_ = 0.0;
    last_spectral_update_time_ = -1.0;
    last_spectral_dtg_version_ = 0;
    last_spectral_frontier_version_ = 0;
    lambda2_ema_initialized_ = false;
    partition_valid_ = false;
    partition_trigger_streak_ = 0;
    partition_release_streak_ = 0;
    last_partition_change_ = PartitionChangeType::NONE;
    last_region_frontier_epoch_ = std::numeric_limits<uint64_t>::max();
    spectral_job_running_ = false;
    spectral_graph_eligible_ = true;
    spectral_fallback_this_cycle_ = false;
    submitted_spectral_jobs_ = 0;
    stale_spectral_results_ = 0;
    timed_out_spectral_results_ = 0;
    last_submitted_dtg_version_ = 0;
    last_submitted_frontier_version_ = 0;
    topology_changes_since_submit_ = 0;
    frontier_changes_since_submit_ = 0;
    last_submitted_anchor_count_ = 0;
    last_raw_spectral_node_count_ = 0;
    last_compressed_spectral_node_count_ = 0;
    spectral_mode_state_ = SpectralModeState();
    soft_route_selected_ = false;
    lock_debt_ = 0.0;
    recovery_until_ = 0.0;
    recovery_reason_ = RecoveryReason::NONE;
    recovery_requested_ = false;
    late_stage_active_ = false;
    no_cut_epochs_ = 0;
    consecutive_spectral_timeouts_ = 0;
    spectral_mode_toggle_count_ = 0;
    recovery_count_ = 0;
    evaluated_spectral_routes_ = 0;
    accepted_spectral_routes_ = 0;
    last_route_feedback_spectral_epoch_ =
        std::numeric_limits<uint64_t>::max();
    last_label_change_rate_ = 0.0;
    selected_frontier_id_ = std::numeric_limits<uint32_t>::max();
    const double now_wall = ros::WallTime::now().toSec();
    last_frontier_progress_time_ = now_wall;
    last_region_progress_time_ = now_wall;
    watchdog_region_id_ = -1;
    watchdog_blocking_frontiers_ = 0;
    watchdog_unknown_gain_ = 0.0;
    frontier_reassignment_count_ = 0;
    repeated_target_count_ = 0;
    global_plan_diagnostics_ = GlobalPlanDiagnostics();
    
    origin_ = LRM_->origin_;
    vox_scl_ = LRM_->blockscale_;
    map_upbd_ = LRM_->map_upbd_;
    map_lowbd_ = LRM_->map_lowbd_;
    vox_num_ = LRM_->block_num_;
    cur_hid_ = uav_id_;
    cout<<"cur_hid_ 0:"<<cur_hid_<<endl;
    // cout<<"uav_id_ :"<<uav_id_<<endl;
    // cout<<"uav_id_ :"<<uav_id_<<endl;
    if(drone_num_ > 1){
        use_swarm_ = true;
        // swarm_timer_ = nh.createTimer(ros::Duration(SDM_->local_comm_intv_), &MultiDtgPlus::DTGCommunicationCallback, this);
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
