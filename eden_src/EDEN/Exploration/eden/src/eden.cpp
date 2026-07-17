#include <eden/eden.h>
#include <eden/eden_safety.h>

namespace {

constexpr double kMinTrajectoryDuration = 1.0e-6;

bool ValidateCorridorGroup(const vector<Eigen::MatrixX4d> &corridors,
                           const vector<Eigen::Matrix3Xd> &vertices,
                           const char *name,
                           std::string &reason){
    if(corridors.empty()){
        reason = std::string(name) + " corridor is empty";
        return false;
    }
    if(corridors.size() != vertices.size()){
        reason = std::string(name) + " corridor/vertex count mismatch";
        return false;
    }
    for(std::size_t i = 0; i < corridors.size(); ++i){
        if(corridors[i].rows() < 6 || corridors[i].cols() != 4 ||
           !corridors[i].allFinite()){
            reason = std::string(name) + " corridor contains invalid coefficients";
            return false;
        }
        if(vertices[i].rows() != 3 || vertices[i].cols() <= 0 ||
           !vertices[i].allFinite()){
            reason = std::string(name) + " corridor vertices are invalid";
            return false;
        }
    }
    return true;
}

bool PopulateCorridorBounds(
    const vector<Eigen::Matrix3Xd> &vertices,
    vector<pair<Eigen::Vector3d, Eigen::Vector3d>> &bounds,
    std::string &reason){
    bounds.clear();
    bounds.reserve(vertices.size());
    for(std::size_t i = 0; i < vertices.size(); ++i){
        if(vertices[i].rows() != 3 || vertices[i].cols() <= 0 ||
           !vertices[i].allFinite()){
            reason = "cannot compute bounds for invalid corridor vertices";
            return false;
        }
        Eigen::Vector3d upper = vertices[i].rowwise().maxCoeff();
        Eigen::Vector3d lower = vertices[i].rowwise().minCoeff();
        if(!upper.allFinite() || !lower.allFinite() ||
           (upper.array() < lower.array()).any()){
            reason = "computed corridor bounds are invalid";
            return false;
        }
        bounds.emplace_back(upper, lower);
    }
    return true;
}

}  // namespace


void SingleExp::init(ros::NodeHandle &nh, ros::NodeHandle &nh_private){
    nh_ = nh;
    nh_private_ = nh_private;
    std::string ns = ros::this_node::getName();
    double theta, psi;
    nh_private.param(ns + "/Exp/traj_length", traj_length_, 10.0);
    nh_private.param(ns + "/Exp/traj_sub_length", traj_sub_length_, 3.0);
    nh_private.param(ns + "/Frontier/FOV_ver_num", 
        psi, 0.5 * M_PI); 
    nh_private.param(ns + "/Frontier/cam_hor", 
        theta, 0.5 * M_PI);
    run_branch_ = false;
    // nh_private.param(ns + "/Exp/local_path_search", local_max_search_iter_, 1000);
    // nh_private.param(ns + "/Exp/global_path_search", global_max_search_iter_, 3000);
    // nh_private.param(ns + "/Exp/strong_check_interval", strong_check_interval_, 3.0);
    nh_private.param(ns + "/Exp/replan_duration", replan_duration_, 1.5);
    nh_private.param(ns + "/Exp/check_duration", check_duration_, 3.5);
    // nh_private.param(ns + "/Exp/exc_duration", exc_duration_, 0.5);
    // nh_private.param(ns + "/Exp/takeoff_x", init_pose_(0), 0.0);
    // nh_private.param(ns + "/Exp/takeoff_y", init_pose_(1), 0.0);
    // nh_private.param(ns + "/Exp/takeoff_z", init_pose_(2), 1.0);
    // nh_private.param(ns + "/Exp/takeoff_yaw", init_pose_(3), 0.0);
    nh_private.param(ns + "/Exp/reach_out_t", reach_out_t_, 0.1);
    nh_private.param(ns + "/Exp/statistic", stat_, false);
    nh_private.param(ns + "/Exp/traj_replan_duration", traj_replan_duration_, 0.5);
    
    // nh_private.param(ns + "/opt/YawVel", dyaw_max_, 2.0);
    // nh_private.param(ns + "/opt/YawAcc", ddyaw_max_, 2.0);
    // nh_private.param(ns + "/Exp/UseMotion", use_motion_, true);
    // nh_private.param(ns + "/Exp/UseTerminalCorridor", use_terminal_corridor_, true);
    // nh_private.param(ns + "/Exp/UseDirCorridor", use_dir_corridor_, true);
    // nh_private.param(ns + "/Exp/UseCoverTrajectory", use_cover_trajectory_, true);
    // nh_private.param(ns + "/Exp/VolumeTest", volume_test_, false);
    nh_private.param(ns + "/Exp/UpdateInterval", update_interval_, 0.2);
    nh_private.param(ns + "/Exp/FinishConfirmCycles", finish_confirm_cycles_, 5);
    nh_private.param(ns + "/Exp/FinishStableDuration", finish_stable_duration_, 2.0);
    nh_private.param(ns + "/Exp/FinishTotalGainThreshold", finish_total_gain_threshold_, 10.0);
    nh_private.param(ns + "/Exp/SensorFreshnessTimeout", sensor_freshness_timeout_, 1.0);
    nh_private.param(ns + "/Exp/VolumeCoverageEnabled", volume_coverage_enabled_, false);
    nh_private.param(ns + "/Exp/VolumeCoverageThreshold", volume_coverage_threshold_, 0.99);
    nh_private.param(ns + "/Exp/TotalExplorableVolume", total_explorable_volume_, 0.0);
    if(volume_coverage_enabled_ && !stat_){
        ROS_WARN("VolumeCoverageEnabled requires Exp/statistic; volume coverage will not "
            "be used for completion");
    }

    // use_dir_corridor_ = use_dir_corridor_ & use_terminal_corridor_;
    // cout<<"=======------"<<use_dir_corridor_<<endl;
    // cout<<use_terminal_corridor_<<endl;
    // cout<<use_motion_<<endl;
    // Eigen::Vector4d fovangle;
    // nh_private.param(ns + "/EXP/FovHorizontalUp", fovangle(0), 0.9);
    // nh_private.param(ns + "/EXP/FovHorizontalDown", fovangle(1), -0.9);
    // nh_private.param(ns + "/EXP/FovVerticalUp", fovangle(2), 0.65);
    // nh_private.param(ns + "/EXP/FovVerticalDown", fovangle(3), -0.65);
    // double sensor_range;
    // nh_private.param(ns + "/block_map/sensor_max_range", sensor_range, 5.0);

    // volume_t_ = ros::WallTime::now().toSec() - 100;
    v_num_ = 0;
    len_ = 0.0;

    // LRM_->AlignInit();
    camFov_.resize(4, 4);
    camFov_ << -sin(theta * 0.5), cos(theta * 0.5), 0, 0,
                -sin(theta * 0.5), -cos(theta * 0.5), 0, 0,
                -sin(psi * 0.5), 0, cos(psi * 0.5), 0, 
                -sin(psi * 0.5), 0, -cos(psi * 0.5), 0;
    camV_.resize(3,5);
    camV_<<0.0, 1.0 , 1.0, 1.0, 1.0,
            0.0, 1.0 / tan(theta * 0.5), 1.0 / tan(theta * 0.5), -1.0 / tan(theta * 0.5), -1.0 / tan(theta * 0.5),
            0.0, 1.0 / tan(psi * 0.5), -1.0 / tan(psi * 0.5), 1.0 / tan(psi * 0.5), -1.0 / tan(psi * 0.5);

    Eigen::Vector3d origin;
    Eigen::Vector3i block_size, block_num, local_block_num;
    LRM_.SetMap(&BM_);
    LRM_.AlignInit(nh, nh_private, origin, block_size, block_num, local_block_num);
    if(stat_) CS_.init(nh, nh_private);
    // SDM_.init(nh, nh_private);
    // BM_.SetSwarmDataManager(&SDM_);
    BM_.AlignInit(nh, nh_private, origin, block_size, block_num, local_block_num);
    if(stat_) BM_.SetDataStatistic(&CS_);
    CM_.init(nh, nh_private);

    EROI_.SetLowresMap(LRM_);
    EROI_.SetMap(BM_);
    EROI_.SetColorManager(CM_);
    EROI_.AlignInit(nh, nh_private, origin, block_size, block_num, local_block_num);

    DTG_.SetLowresMap(&LRM_);
    DTG_.SetBlockMap(&BM_);
    DTG_.SetFrontierMap(&EROI_);
    DTG_.SetColorManager(&CM_);
    // DTG_.SetSwarmDataManager(&SDM_);
    DTG_.AlignInit(nh, nh_private);

    TrajOpt_.Init(nh, nh_private);

    finish_confirm_cycles_ = std::max(finish_confirm_cycles_, 1);
    finish_stable_duration_ = std::max(finish_stable_duration_, 0.0);
    finish_total_gain_threshold_ = std::max(finish_total_gain_threshold_, 0.0);
    sensor_freshness_timeout_ = std::max(sensor_freshness_timeout_, 0.0);
    if(!std::isfinite(volume_coverage_threshold_)){
        volume_coverage_threshold_ = 0.99;
    }
    volume_coverage_threshold_ =
        std::max(0.0, std::min(volume_coverage_threshold_, 1.0));
    if(!std::isfinite(total_explorable_volume_) ||
       total_explorable_volume_ <= 0.0){
        total_explorable_volume_ = 0.0;
    }

    last_map_update_t_ = ros::WallTime::now().toSec();
    last_sensor_update_t_ = -std::numeric_limits<double>::infinity();
    traj_end_t_ = last_map_update_t_ - 0.1;
    have_odom_ = false;
    target_f_id_ = -1;
    target_v_id_ = -1;
    dtg_flag_ = false;
    hold_position_valid_ = false;
    finish_confirm_count_ = 0;
    last_finish_frontier_epoch_ = 0;
    finish_stable_since_ = -1.0;
    volume_finish_stable_since_ = -1.0;
    last_global_plan_status_ = DTGPlus::GlobalPlanStatus::INTERNAL_ERROR;
    traj_start_t_ = last_map_update_t_ - 0.2;
    if(EROI_.sensor_type_ == SensorType::CAMERA){
        depth_sub_.reset(new message_filters::Subscriber<sensor_msgs::Image>(nh_, "/depth", 10));
        vi_odom_sub_.reset(new message_filters::Subscriber<nav_msgs::Odometry>(nh_, "/vi_odom", 10));
        sync_image_odom_.reset(new message_filters::Synchronizer<SyncPolicyImageOdom>(
            SyncPolicyImageOdom(100), *depth_sub_, *vi_odom_sub_));
        sync_image_odom_->registerCallback(boost::bind(&SingleExp::ImgOdomCallback, this,  _1, _2));
    }
    else{
        pcl_sub_.reset(new message_filters::Subscriber<sensor_msgs::PointCloud2>(nh_, "/pointcloud", 10));
        vi_odom_sub_.reset(new message_filters::Subscriber<nav_msgs::Odometry>(nh_, "/vi_odom", 10));
        sync_pointcloud_odom_.reset(new message_filters::Synchronizer<SyncPolicyPCLOdom>(
            SyncPolicyPCLOdom(100), *pcl_sub_, *vi_odom_sub_));
        sync_pointcloud_odom_->registerCallback(boost::bind(&SingleExp::PCLOdomCallback, this,  _1, _2));
    }
    if(stat_){
        stat_timer_ = nh_private_.createTimer(ros::Duration(0.5), &SingleExp::DataStatistic, this);
    }
    odom_sub_ = nh_.subscribe("/odom", 10, &SingleExp::BodyOdomCallback, this);
    vis_pub_ = nh_.advertise<visualization_msgs::MarkerArray>(ns + "/Planner/Vis", 10);
    traj_pub_ = nh_.advertise<swarm_exp_msgs::LocalTraj>(ns + "/Planner/Traj", 1);
    debug_pub_ = nh_.advertise<visualization_msgs::Marker>(ns + "/Planner/Debug", 1);


    reload_map_timer_ = nh_private_.createTimer(ros::Duration(0.2), &SingleExp::ReloadMap, this);
     dtg_timer_ = nh.createTimer(ros::Duration(0.4), &SingleExp::UpdateDTG, this);

}
void SingleExp::SetPlanInterval(const double &intv){
    plan_t_ = ros::WallTime::now().toSec() + intv;
}

void SingleExp::ResetFinishConfirmation(){
    finish_confirm_count_ = 0;
    finish_stable_since_ = -1.0;
    last_finish_frontier_epoch_ = DTG_.frontier_epoch();
}

void SingleExp::ResetExplorationState(){
    ResetFinishConfirmation();
    volume_finish_stable_since_ = -1.0;
    last_global_plan_status_ = DTGPlus::GlobalPlanStatus::INTERNAL_ERROR;
    target_f_id_ = -1;
    target_v_id_ = -1;
    dtg_flag_ = false;
    hold_position_valid_ = false;
}

bool SingleExp::ValidateTrajectory(const Trajectory4<5> &trajectory,
                                   std::string &reason) const{
    return EdenSafety::ValidateTrajectory(trajectory, &reason);
}

bool SingleExp::ValidateTrajectoryInput(const TrajOptInput &input,
                                        uint8_t plan_res,
                                        std::string &reason) const{
    if(input.initState.rows() != 4 || input.initState.cols() != 3 ||
       !input.initState.allFinite()){
        reason = "initial state is invalid";
        return false;
    }
    if(input.camFov.rows() <= 0 || input.camFov.cols() != 4 ||
       !input.camFov.allFinite() ||
       input.camV.rows() != 3 || input.camV.cols() <= 0 ||
       !input.camV.allFinite()){
        reason = "camera geometry is invalid";
        return false;
    }

    if(plan_res == 2){
        if(!input.endState_stem.allFinite() ||
           !input.endState_main.allFinite() ||
           !input.endState_sub.allFinite()){
            reason = "branch terminal state is invalid";
            return false;
        }
        return ValidateCorridorGroup(
                   input.corridors_stem, input.corridorVs_stem,
                   "stem", reason) &&
               ValidateCorridorGroup(
                   input.corridors_main, input.corridorVs_main,
                   "main", reason) &&
               ValidateCorridorGroup(
                   input.corridors_sub, input.corridorVs_sub,
                   "sub", reason);
    }
    if(plan_res == 3){
        if(!input.endState_norm.allFinite()){
            reason = "normal terminal state is invalid";
            return false;
        }
        return ValidateCorridorGroup(
            input.corridors_norm, input.corridorVs_norm, "normal", reason);
    }

    reason = "unsupported trajectory plan type";
    return false;
}

bool SingleExp::ValidatePath(const vector<Eigen::Vector3d> &path,
                             bool allow_unknown,
                             std::string &reason){
    if(path.empty()){
        reason = "path is empty";
        return false;
    }
    for(std::size_t i = 0; i < path.size(); ++i){
        if(!path[i].allFinite()){
            reason = "path contains NaN or Inf at index " +
                std::to_string(i);
            return false;
        }
        if(!LRM_.IsFeasible(path[i], allow_unknown)){
            reason = "path contains an infeasible point at index " +
                std::to_string(i);
            return false;
        }
    }
    reason.clear();
    return true;
}

bool SingleExp::VolumeCoverageReached(){
    const double explored_volume = stat_
        ? CS_.GetVolume(0)
        : std::numeric_limits<double>::quiet_NaN();
    const double now = ros::WallTime::now().toSec();
    double ratio = std::numeric_limits<double>::quiet_NaN();
    const bool reached = EdenSafety::UpdateKnownVolumeCompletion(
        volume_coverage_enabled_, stat_, explored_volume,
        total_explorable_volume_, volume_coverage_threshold_,
        finish_stable_duration_, now, &volume_finish_stable_since_, &ratio);
    if(volume_coverage_enabled_ && stat_ && !std::isfinite(ratio)){
        ROS_ERROR_THROTTLE(1.0, "[EDEN] invalid known-volume coverage measurement");
        return false;
    }
    if(!std::isfinite(ratio) || ratio < volume_coverage_threshold_){
        return false;
    }

    const double stable_duration = now - volume_finish_stable_since_;
    ROS_WARN_THROTTLE(
        1.0,
        "[EDEN] known-volume finish pending: %.2f%%/%.2f%%, stable %.2f/%.2fs",
        ratio * 100.0, volume_coverage_threshold_ * 100.0,
        stable_duration, finish_stable_duration_);
    return reached;
}

double SingleExp::ComputeRemainingInformationGain() const{
    double total_gain = 0.0;
    for(std::size_t f_id = 0; f_id < EROI_.EROI_.size(); ++f_id){
        const auto &frontier = EROI_.EROI_[f_id];
        if(frontier.f_state_ != 1) continue;

        std::unordered_set<int> visited_viewpoints;
        for(const auto raw_v_id : frontier.valid_vps_){
            const int v_id = static_cast<int>(raw_v_id);
            if(v_id < 0 || static_cast<std::size_t>(v_id) >= frontier.local_vps_.size()) continue;
            if(frontier.local_vps_[v_id] != 1 || !visited_viewpoints.insert(v_id).second) continue;

            const double gain = EROI_.GetGain(static_cast<int>(f_id), v_id);
            if(std::isfinite(gain) && gain > 0.0) total_gain += gain;
        }
    }
    return total_gain;
}

void SingleExp::PublishHoldPosition(){
    if(!have_odom_) return;
    if(!p_.allFinite()){
        ROS_ERROR_THROTTLE(1.0, "[EDEN] refusing to publish a non-finite hold position");
        return;
    }

    if(!hold_position_valid_){
        hold_position_ = p_;
        hold_position_(2) += 1e-4;
        hold_position_valid_ = true;
    }

    swarm_exp_msgs::LocalTraj hold;
    hold.state = 1;
    hold.start_t = ros::WallTime::now().toSec();
    hold.recover_pt.x = hold_position_(0);
    hold.recover_pt.y = hold_position_(1);
    // The trajectory executor normalizes the displacement to recover_pt.  A
    // tiny non-zero offset avoids normalizing an exact zero vector while still
    // behaving as a position hold.
    hold.recover_pt.z = hold_position_(2);
    traj_pub_.publish(hold);
}

bool SingleExp::TrajCheck(){
    std::string reason;
    if(!ValidateTrajectory(exc_traj_, reason)){
        ROS_ERROR_STREAM("[EDEN] invalid active trajectory in TrajCheck: " << reason);
        dtg_flag_ = false;
        return false;
    }
    if(!std::isfinite(traj_start_t_) || !std::isfinite(traj_end_t_) ||
       !std::isfinite(replan_t_) || !std::isfinite(check_duration_) ||
       check_duration_ <= 0.0 || !std::isfinite(BM_.resolution_) ||
       BM_.resolution_ <= 0.0){
        ROS_ERROR("[EDEN] invalid trajectory timing in TrajCheck");
        dtg_flag_ = false;
        return false;
    }

    const double total_duration = exc_traj_.getTotalDuration();
    const double cur_t = std::max(ros::WallTime::now().toSec(), traj_start_t_);

    if(cur_t > traj_end_t_ - 1e-3 || cur_t > replan_t_) { // traj time out
        ROS_WARN("traj time up");
        // dtg_flag_ = false; // need to update map
        return false;
    }

    const double end_t = std::min(cur_t + check_duration_, replan_t_ - 1e-3);
    const double relative_cur_t = EdenSafety::RelativeTrajectoryTime(
        cur_t, traj_start_t_, total_duration);
    const Eigen::Vector4d last_state = exc_traj_.getPos(relative_cur_t);
    if(!last_state.allFinite()){
        ROS_ERROR("[EDEN] non-finite trajectory state in TrajCheck");
        dtg_flag_ = false;
        return false;
    }

    Eigen::Vector3d last_p = last_state.head(3);
    Eigen::Vector3d p, r_size;
    r_size = LRM_.GetRobotSize();
    if(!r_size.allFinite() || (r_size.array() <= 0.0).any()){
        ROS_ERROR("[EDEN] invalid robot size in TrajCheck");
        dtg_flag_ = false;
        return false;
    }
    // vector<Eigen::Vector3d> debug_pts;
    for(double t = cur_t; t < end_t; t += 0.05){
        const double relative_t = EdenSafety::RelativeTrajectoryTime(
            t, traj_start_t_, total_duration);
        const Eigen::Vector4d state = exc_traj_.getPos(relative_t);
        if(!state.allFinite()){
            ROS_ERROR("[EDEN] non-finite sampled trajectory state in TrajCheck");
            dtg_flag_ = false;
            return false;
        }
        p = state.head(3);
        // debug_pts.emplace_back(p.head(3));
        for(int dim = 0; dim < 3; dim ++){
            if(abs(p(dim) - last_p(dim)) > BM_.resolution_){ 
                if(BM_.PosBBXOccupied(p, r_size)) { // collide
                    dtg_flag_ = false; // need to update map
                    ROS_WARN("collide");
                    return false;
                }
                last_p = p;
                break;
            }
        }
    }


    const double relative_end_t = EdenSafety::RelativeTrajectoryTime(
        end_t - 1.0e-4, traj_start_t_, total_duration);
    const Eigen::Vector4d end_state = exc_traj_.getPos(relative_end_t);
    if(!end_state.allFinite()){
        ROS_ERROR("[EDEN] non-finite trajectory end state in TrajCheck");
        dtg_flag_ = false;
        return false;
    }
    p = end_state.head(3);
    if(BM_.PosBBXOccupied(p, r_size)) { // collide
        dtg_flag_ = false; // need to update map
        ROS_WARN("collide end");
        return false;
    }

    // Debug(debug_pts);
    return true;
}

int SingleExp::AllowPlan(const double &T){

    /* not satisfy plan interval */
    if(T - plan_t_ < 0){
        return 1;
    }

    if(!have_odom_){
        return 3;
    }

    /* not in free space */
    if(!LRM_.IsFeasible(p_)){
        return 2;
    }
    // last_safe_ = p_;

    // /* sensor not update */
    // if(!dtg_flag_ || EROI_.vp_update_){
    //     return 3;
    // }

    /* traj time up */
    if(T - replan_t_ > 0){
        return 5;
    }

    return 0;
}


EdenPlanStatus SingleExp::EdenPlan(){
    if(VolumeCoverageReached()){
        const double ratio = CS_.GetVolume(0) / total_explorable_volume_;
        ROS_WARN("[EDEN] known-volume coverage reached and stable (%.2f%% >= %.2f%%)",
                 ratio * 100.0, volume_coverage_threshold_ * 100.0);
        return EdenPlanStatus::FINISHED;
    }

    Eigen::Vector3d ps, vs, as, pe, ve, ae;
    double ys, yds, ydds, ye, yde, ydde;
    double hand_t = max(ros::WallTime::now().toSec() + reach_out_t_, traj_start_t_); 
    double cur_t = ros::WallTime::now().toSec();
    if(!std::isfinite(hand_t) || !std::isfinite(cur_t) ||
       !std::isfinite(traj_start_t_) || !std::isfinite(traj_end_t_)){
        ROS_ERROR("[EDEN] invalid absolute trajectory timing before planning");
        return EdenPlanStatus::TRAJECTORY_FAILED;
    }
    if(cur_t <= traj_end_t_){
        std::string active_reason;
        if(!ValidateTrajectory(exc_traj_, active_reason)){
            ROS_ERROR_STREAM("[EDEN] cannot sample active trajectory: "
                             << active_reason);
            return EdenPlanStatus::TRAJECTORY_FAILED;
        }
    }
    if(hand_t > traj_end_t_){
        if(cur_t - traj_end_t_ > 0.0){
            hand_t = ros::WallTime::now().toSec();
            ps = p_;
            vs = v_;
            as.setZero();
            ve.setZero();
            ae.setZero();
            ys = yaw_;
            yds = yaw_v_;
            ydds = 0;
            yde = 0; 
            ydde = 0;
            ROS_ERROR("start 0==================");
            cout<<"t cur_t0:"<<cur_t - traj_start_t_<<endl;
            cout<<"t plan0:"<<hand_t - traj_start_t_<<endl;
            cout<<"t total0:"<<exc_traj_.getTotalDuration()<<endl;
        }
        else{
            Eigen::Vector4d p4s, v4s, a4s;
            hand_t = max(traj_end_t_ - 1e-3 - cur_t, 0.0) + cur_t;
            p4s = exc_traj_.getPos(hand_t - traj_start_t_);
            v4s = exc_traj_.getVel(hand_t - traj_start_t_);
            a4s = exc_traj_.getAcc(hand_t - traj_start_t_);
            ROS_WARN("start 1");
            cout<<"t cur_t1:"<<cur_t - traj_start_t_<<endl;
            cout<<"t plan1:"<<hand_t - traj_start_t_<<endl;
            cout<<"t total1:"<<exc_traj_.getTotalDuration()<<endl;
            ps = p4s.head(3);
            vs = v4s.head(3);
            as = a4s.head(3);
            ys = p4s(3);
            yds = v4s(3);
            ydds = a4s(3);
            ve.setZero();
            ae.setZero();
            // YawP_.GetCmd(cur_t - traj_start_t_, ys, yds, ydds);
            yde = 0; 
            ydde = 0;
        }
    }
    else{
        Eigen::Vector4d p4s, v4s, a4s;
        p4s = exc_traj_.getPos(hand_t - traj_start_t_);
        ps = p4s.head(3);
        while(!LRM_.IsFeasible(ps) && hand_t > cur_t){
            hand_t -= reach_out_t_ / 10;
            p4s = exc_traj_.getPos(hand_t - traj_start_t_);
            ps = p4s.head(3);
        }
        if(!LRM_.IsFeasible(ps)){
            hand_t = cur_t;
            ps = p_;
        }
        ROS_WARN("start 2");
        cout<<"t plan2:"<<hand_t - traj_start_t_<<endl;
        cout<<"t cur_t2:"<<cur_t - traj_start_t_<<endl;
        cout<<"t total2:"<<exc_traj_.getTotalDuration()<<endl;
        v4s = exc_traj_.getVel(hand_t - traj_start_t_);
        a4s = exc_traj_.getAcc(hand_t - traj_start_t_);
        vs = v4s.head(3);
        as = a4s.head(3);
        ys = p4s(3);
        yds = v4s(3);
        ydds = a4s(3);
        ve.setZero();
        ae.setZero();
        // YawP_.GetCmd(hand_t - traj_start_t_, ys, yds, ydds);
        yde = 0; 
        ydde = 0;
    }
    if(!ps.allFinite() || !vs.allFinite() || !as.allFinite() ||
       !std::isfinite(ys) || !std::isfinite(yds) ||
       !std::isfinite(ydds)){
        ROS_ERROR("[EDEN] invalid initial state for trajectory optimization");
        return EdenPlanStatus::TRAJECTORY_FAILED;
    }
    EROI_.YawNorm(ys);
    if(!std::isfinite(ys)){
        ROS_ERROR("[EDEN] yaw normalization produced a non-finite value");
        return EdenPlanStatus::TRAJECTORY_FAILED;
    }

    for(auto &f : EROI_.dead_fnodes_){
        cout<<"0erase dtg fn:"<<f.first<<"  v:"<<int(f.second)<<endl;
        DTG_.EraseFnodeFromGraph(f.first, f.second);
    }
    // EROI_.sam


    TrajOptInput toi;
    toi.initState.resize(4, 3);
    toi.initState.col(0).head(3) = ps;
    toi.initState(3, 0) = ys;
    toi.initState.col(1).head(3) = vs;
    toi.initState(3, 1) = yds;
    toi.initState.col(2).head(3) = as;
    toi.initState(3, 2) = ydds;

    uint8_t plan_res;
    vector<Eigen::Vector3d> path_stem, path_main, path_sub, path_norm, pp; 
    pair<uint32_t, uint8_t> tar_stem, tar_main, tar_sub, tar_norm; 
    double y_stem, y_main, y_sub, y_norm;

    Eigen::Vector4d ps_y, vs_dy;
    ps_y.head(3) = ps;
    ps_y(3) = ys;
    vs_dy.head(3) = vs;
    vs_dy(3) = yds;

    double target_duration, corridor_duration, traj_duration;
    const auto log_plan_timings = [&](){
        double coverage_percent =
            std::numeric_limits<double>::quiet_NaN();
        if(stat_ && total_explorable_volume_ > 0.0){
            coverage_percent =
                100.0 * CS_.GetVolume(0) / total_explorable_volume_;
        }
        ROS_INFO_THROTTLE(
            1.0,
            "[EDEN] local_target=%.2fms corridor=%.2fms "
            "trajectory=%.2fms coverage=%.2f%%",
            target_duration * 1000.0,
            corridor_duration * 1000.0,
            traj_duration * 1000.0,
            coverage_percent);
    };
    double plan_ts = ros::WallTime::now().toSec();
    last_global_plan_status_ = TargetPlanning(ps_y, vs_dy, plan_res,
        path_stem, path_main, path_sub, path_norm, 
        tar_stem, tar_main, tar_sub, tar_norm, 
        y_stem, y_main, y_sub, y_norm);
    target_duration = ros::WallTime::now().toSec() - plan_ts;

    if(last_global_plan_status_ != DTGPlus::GlobalPlanStatus::SUCCESS){
        if(last_global_plan_status_ == DTGPlus::GlobalPlanStatus::NO_ACTIVE_FRONTIER){
            const auto summary = EROI_.GetExplorationSummary(false);
            const double now = ros::WallTime::now().toSec();
            const double remaining_gain = ComputeRemainingInformationGain();
            const double sensor_age = now - last_sensor_update_t_;
            const bool sensor_fresh = sensor_freshness_timeout_ <= 0.0 ||
                (std::isfinite(last_sensor_update_t_) && sensor_age <= sensor_freshness_timeout_);
            const bool no_effective_frontier = summary.alive_valid_vp_num_ == 0;
            const bool no_pending_region = !DTG_.HasPendingRegions();
            const bool low_total_gain = remaining_gain <= finish_total_gain_threshold_;
            const bool finish_candidate = sensor_fresh && low_total_gain &&
                no_effective_frontier && no_pending_region;
            const uint64_t frontier_epoch = DTG_.frontier_epoch();

            if(finish_candidate){
                if(finish_stable_since_ < 0.0) finish_stable_since_ = now;
                if(frontier_epoch != last_finish_frontier_epoch_){
                    finish_confirm_count_ = std::min(
                        finish_confirm_count_ + 1, finish_confirm_cycles_);
                    last_finish_frontier_epoch_ = frontier_epoch;
                }
            }
            else{
                ResetFinishConfirmation();
            }

            const double stable_duration = finish_stable_since_ < 0.0 ? 0.0 :
                now - finish_stable_since_;
            if(finish_candidate && finish_confirm_count_ >= finish_confirm_cycles_ &&
                stable_duration >= finish_stable_duration_){
                ROS_WARN("Exploration finish confirmed after %d DTG updates and %.2fs "
                    "(remaining gain %.2f)", finish_confirm_count_, stable_duration,
                    remaining_gain);
                return EdenPlanStatus::FINISHED;
            }

            ROS_WARN_THROTTLE(1.0, "Finish pending: active=%zu alive_vp=%zu pending_region=%d "
                "gain=%.2f/%.2f sensor_age=%.2f stable=%d/%d,%.2f/%.2fs",
                summary.active_eroi_num_, summary.alive_valid_vp_num_, no_pending_region ? 0 : 1,
                remaining_gain, finish_total_gain_threshold_, sensor_age,
                finish_confirm_count_, finish_confirm_cycles_, stable_duration,
                finish_stable_duration_);
            return EdenPlanStatus::RETRY_AFTER_UPDATE;
        }
        // Spectral timeout/instability/route rejection and ordinary path
        // failures are all recoverable here.  Completion is only emitted by
        // the guarded NO_ACTIVE_FRONTIER branch above.
        ResetFinishConfirmation();
        return EdenPlanStatus::GLOBAL_PLAN_FAILED;
    }
    ResetFinishConfirmation();
    
    cout<<"plan_res:"<<int(plan_res)<<endl;
    cout<<"ps_y:"<<ps_y.transpose()<<endl;
    cout<<"p_:"<<p_.transpose()<<" "<<yaw_<<endl;

    // if((p_ - ps_y.head(3)).norm() > 0.5){
    //     cout<<"p_ hand:"<<exc_traj_.getPos(hand_t - traj_start_t_).transpose()<<endl;
    //     cout<<"p_ cur:"<<exc_traj_.getPos(cur_t - traj_start_t_).transpose()<<endl;
    //     ROS_ERROR("?????????????");
    // }
    // cout<<"y_stem:"<<y_stem<<endl;
    // cout<<"y_main:"<<y_main<<endl;
    // cout<<"y_sub:"<<y_sub<<endl;
    // cout<<"y_norm:"<<y_norm<<endl;
    // cout<<"ydiff:"<<abs(EROI_.YawDiff(y_stem, y_main))<<endl;
    // cout<<"ydiff:"<<abs(EROI_.YawDiff(y_main, y_stem))<<endl;
    // cout<<"tar_stem:"<<tar_stem.first<<"  "<<int(tar_stem.second)<<endl;
    // cout<<"tar_sub:"<<tar_sub.first<<"  "<<int(tar_sub.second)<<endl;

    // if(path_stem.size() == 2){
    //     Eigen::Vector3d max_p, min_p;
    //     for(int dim = 0; dim < 3; dim++){
    //         max_p(dim) = max(path_stem[0](dim), path_stem[1](dim));
    //         min_p(dim) = min(path_stem[0](dim), path_stem[1](dim));
    //     }
    //     for(double x = min_p(0); x < max_p(0) + 1e-3; x += 0.7){
    //         for(double y = min_p(1); y < max_p(1) + 1e-3; y += 0.7){
    //             for(double z = min_p(2); z < max_p(2) + 1e-3; z += 0.7){
    //                 Eigen::Vector3d pc(x, y, z);
    //                 cout<<"pc:"<<pc.transpose()<<"  fea:"<<LRM_.IsFeasible(pc)<<endl;
    //                 if(!LRM_.IsFeasible(pc)){
    //                     ROS_ERROR("error connect");
    //                     for(auto ps : path_stem){
    //                         cout<<"ps:"<<ps.transpose()<<"  fea:"<<LRM_.IsFeasible(ps)<<endl;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
    if(plan_res != 2 && plan_res != 3){
        ROS_WARN("[EDEN] target planner returned no executable target (plan_res=%u)",
                 static_cast<unsigned int>(plan_res));
        return EdenPlanStatus::TARGET_INVALID;
    }

    plan_ts = ros::WallTime::now().toSec();
    std::string validation_reason;
    if(plan_res == 2){
        if(!ValidatePath(path_stem, false, validation_reason)){
            ROS_ERROR_STREAM("[EDEN] rejected stem path: " << validation_reason);
            return EdenPlanStatus::TARGET_INVALID;
        }
        int s = LRM_.FindCorridors(path_stem, toi.corridors_stem, toi.corridorVs_stem, pp, false, traj_length_);
        if(s == 0 || pp.empty()){
            ROS_ERROR("[EDEN] stem corridor construction failed");
            return EdenPlanStatus::TRAJECTORY_FAILED;
        }
        toi.endState_stem.head(3) = LRM_.GetStdPos(pp.back());
        toi.endState_stem(3) = y_stem;

        if(s == 2){
            plan_res = 3;
        }
        else if(s == 1){
            ROS_WARN("FindCorridors main");
            if(!ValidatePath(path_main, true, validation_reason) ||
               LRM_.FindCorridors(path_main, toi.corridors_main,
                                  toi.corridorVs_main, pp, true, INFINITY) != 1 ||
               pp.empty()){
                plan_res = 3;
                ROS_ERROR_STREAM("[EDEN] main corridor unavailable; falling back to "
                                 "normal trajectory: " << validation_reason);
            }
            else{
                toi.endState_main.head(3) =
                    LRM_.GetStdPos(pp.back()) - Eigen::Vector3d::Ones() * 0.05;
                toi.endState_main(3) = y_main;
            }

            if(plan_res == 2){
                ROS_WARN("FindCorridors sub");
                validation_reason.clear();
                if(!ValidatePath(path_sub, false, validation_reason) ||
                   LRM_.FindCorridors(path_sub, toi.corridors_sub,
                                      toi.corridorVs_sub, pp, false,
                                      traj_sub_length_) == 0 ||
                   pp.empty()){
                    plan_res = 3;
                    ROS_ERROR_STREAM("[EDEN] sub corridor unavailable; falling back to "
                                     "normal trajectory: " << validation_reason);
                }
                else{
                    toi.endState_sub.head(3) =
                        LRM_.GetStdPos(pp.back()) + Eigen::Vector3d::Ones() * 0.05;
                    toi.endState_sub(3) = y_sub;
                }
            }
        }
        else{
            ROS_ERROR("[EDEN] unexpected stem corridor status");
            return EdenPlanStatus::TRAJECTORY_FAILED;
        }
    }

    if(plan_res == 3){
        ROS_WARN("FindCorridors norm");
        if(!ValidatePath(path_norm, false, validation_reason)){
            ROS_ERROR_STREAM("[EDEN] rejected normal path: " << validation_reason);
            return EdenPlanStatus::TARGET_INVALID;
        }
        int s = LRM_.FindCorridors(path_norm, toi.corridors_norm, toi.corridorVs_norm, pp, false, traj_length_);
        if(s == 0 || pp.empty()){
            ROS_ERROR("[EDEN] normal corridor construction failed");
            return EdenPlanStatus::TRAJECTORY_FAILED;
        }
        toi.endState_norm.head(3) = LRM_.GetStdPos(pp.back());
        toi.endState_norm(3) = y_norm;

        if(s == 2){
            if((pp.back().head(2)- path_norm.back().head(2)).norm() > 1e-3){
                y_norm = atan2(path_norm.back()(1) - pp.back()(1), path_norm.back()(0) - pp.back()(0));
                toi.endState_norm(3) = y_norm;
            }
        }
    }
    corridor_duration = ros::WallTime::now().toSec() - plan_ts;

    toi.camFov = camFov_;
    toi.camV = camV_;
    if(!PopulateCorridorBounds(toi.corridorVs_main, toi.traj_bbxs_main,
                               validation_reason) ||
       !PopulateCorridorBounds(toi.corridorVs_sub, toi.traj_bbxs_sub,
                               validation_reason) ||
       !PopulateCorridorBounds(toi.corridorVs_stem, toi.traj_bbxs_stem,
                               validation_reason) ||
       !PopulateCorridorBounds(toi.corridorVs_norm, toi.traj_bbxs_norm,
                               validation_reason)){
        ROS_ERROR_STREAM("[EDEN] rejected invalid corridor bounds: "
                         << validation_reason);
        return EdenPlanStatus::TRAJECTORY_FAILED;
    }

    validation_reason.clear();
    if(!ValidateTrajectoryInput(toi, plan_res, validation_reason)){
        ROS_ERROR_STREAM("[EDEN] rejected trajectory optimizer input: "
                         << validation_reason);
        return EdenPlanStatus::TRAJECTORY_FAILED;
    }

    if(plan_res == 2){
        Eigen::Vector4d target_pose;
        if(!EROI_.GetVp(static_cast<int>(tar_stem.first),
                        static_cast<int>(tar_stem.second), target_pose) ||
           !target_pose.allFinite()){
            ROS_ERROR("[EDEN] branch target is invalid");
            return EdenPlanStatus::TARGET_INVALID;
        }
        double dp = (ps - target_pose.head(3)).norm();
        double dyaw = abs(EROI_.YawDiff(target_pose(3), ys));
        if(!std::isfinite(dp) || !std::isfinite(dyaw)){
            ROS_ERROR("[EDEN] branch target check produced NaN or Inf");
            return EdenPlanStatus::TARGET_INVALID;
        }
        if((dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25) || !EROI_.StrongCheckViewpoint(tar_stem.first, tar_stem.second, true)){
            DTG_.RemoveVp(tar_stem.first, tar_stem.second);
            cout<<"too close branch!"<<endl;
            return EdenPlanStatus::TARGET_INVALID;
        }
        plan_ts = ros::WallTime::now().toSec();

        if(TrajOpt_.AggressiveBranchTrajOptimize(toi)){
            const Trajectory4<5> candidate = TrajOpt_.stem_traj_;
            validation_reason.clear();
            if(!ValidateTrajectory(candidate, validation_reason)){
                ROS_ERROR_STREAM("[EDEN] rejected invalid branch trajectory: "
                                 << validation_reason);
                return EdenPlanStatus::TRAJECTORY_FAILED;
            }
            if(!ValidateTrajectory(TrajOpt_.main_traj_, validation_reason) ||
               !ValidateTrajectory(TrajOpt_.sub_traj_, validation_reason)){
                ROS_ERROR_STREAM("[EDEN] rejected invalid branch continuation: "
                                 << validation_reason);
                return EdenPlanStatus::TRAJECTORY_FAILED;
            }
            traj_duration = ros::WallTime::now().toSec() - plan_ts;
            if(stat_){
                CS_.SetVolume(target_duration, 3);
                CS_.SetVolume(corridor_duration, 4);
                CS_.SetVolume(traj_duration, 5);
            }

            exc_traj_ = candidate;
            run_branch_ = true;
            traj_start_t_ = hand_t;
            replan_t_ = min(replan_duration_, max(exc_traj_.getTotalDuration() - 0.15, 0.0)) + traj_start_t_;
            traj_end_t_ = exc_traj_.getTotalDuration() + traj_start_t_;
            const bool early_replan =
                !TrajOpt_.upboundVec_.empty() &&
                std::isfinite(TrajOpt_.upboundVec_[0]) &&
                TrajOpt_.upboundVec_[0] > 0.0 &&
                TrajOpt_.main_traj_[0].getCoeffMat().col(4).head(3).norm() <
                    TrajOpt_.upboundVec_[0] * 0.6;
            if(early_replan){
                traj_replan_t_ = traj_start_t_ + traj_replan_duration_;
            }
            else{
                traj_replan_t_ = traj_start_t_ + exc_traj_.getTotalDuration() * 2.0;
            }
            target_f_id_ = tar_stem.first;
            target_v_id_ = tar_stem.second;
            path_stem_ = path_stem;
            path_sub_ = path_sub;
            path_main_ = path_main;
            tar_stem_ = toi.endState_stem;
            tar_sub_ = toi.endState_sub;
            tar_main_ = toi.endState_main;
            traj_type_ = 1;
            // cout<<"duration:"<<exc_traj_.getTotalDuration()<<endl;
            // cout<<"replan_t_1:"<<replan_t_ - cur_t<<endl;
            // cout<<"eroi state:"<<int(EROI_.EROI_[target_f_id_].f_state_)<<"  vp state:"<<int(EROI_.EROI_[target_f_id_].local_vps_[target_v_id_])<<endl;

            DTG_.RecordExecutedFrontier(tar_stem.first);
            if(!PublishTraj()){
                return EdenPlanStatus::TRAJECTORY_FAILED;
            }
            log_plan_timings();
            ShowTraj();
            ShowPathCorridor(path_stem, toi.corridors_stem);

            // cout<<"ter v:"<<TrajOpt_.main_traj_[0].getCoeffMat().col(4).transpose()<<"    "<<TrajOpt_.main_traj_[0].getCoeffMat().col(4).head(3).norm()<<endl;
            // cout<<"ter a:"<<TrajOpt_.main_traj_[0].getCoeffMat().col(3).transpose() * 2<<"    "<<TrajOpt_.main_traj_[0].getCoeffMat().col(3).head(3).norm() * 2<<endl;
            Eigen::Vector3d u_stem, d_stem, u_sub, d_sub, u_main, d_main;
            u_main(0) = -toi.corridors_main[0](0, 3);
            u_main(1) = -toi.corridors_main[0](2, 3);
            u_main(2) = -toi.corridors_main[0](4, 3);
            d_main(0) = toi.corridors_main[0](1, 3);
            d_main(1) = toi.corridors_main[0](3, 3);
            d_main(2) = toi.corridors_main[0](5, 3);
            // cout<<"main diff u:"<<(u_main - toi.endState_stem.head(3)).transpose()<<endl;
            // cout<<"main diff d:"<<(d_main - toi.endState_stem.head(3)).transpose()<<endl;
            u_sub(0) = -toi.corridors_sub[0](0, 3);
            u_sub(1) = -toi.corridors_sub[0](2, 3);
            u_sub(2) = -toi.corridors_sub[0](4, 3);
            d_sub(0) = toi.corridors_sub[0](1, 3);
            d_sub(1) = toi.corridors_sub[0](3, 3);
            d_sub(2) = toi.corridors_sub[0](5, 3);
            // cout<<"sub diff u:"<<(u_sub - toi.endState_stem.head(3)).transpose()<<endl;
            // cout<<"sub diff d:"<<(d_sub - toi.endState_stem.head(3)).transpose()<<endl;
            // cout<<"d1:"<< (toi.endState_stem.head(3) - ps).norm()<<endl;
            // cout<<"d2:"<< (toi.endState_main.head(3) - toi.endState_stem.head(3)).norm()<<endl;
            // cout<<"plan cost:"<<ros::WallTime::now().toSec() - cur_t<<endl;



            // if(EROI_.EROI_[target_f_id_].f_state_ != 1){
            //     cout<<"f:"<<target_f_id_<<"  vp:"<<int(target_v_id_)<<endl;
            // }
            return EdenPlanStatus::SUCCESS;
        }
        else{
            return EdenPlanStatus::TRAJECTORY_FAILED;
        }
    }
    else if(plan_res == 3){
        Eigen::Vector4d target_pose;
        if(!EROI_.GetVp(static_cast<int>(tar_norm.first),
                        static_cast<int>(tar_norm.second), target_pose) ||
           !target_pose.allFinite()){
            ROS_ERROR("[EDEN] normal target is invalid");
            return EdenPlanStatus::TARGET_INVALID;
        }
        double dp = (ps - target_pose.head(3)).norm();
        double dyaw = abs(EROI_.YawDiff(target_pose(3), ys));
        if(!std::isfinite(dp) || !std::isfinite(dyaw)){
            ROS_ERROR("[EDEN] normal target check produced NaN or Inf");
            return EdenPlanStatus::TARGET_INVALID;
        }
        if(!EROI_.StrongCheckViewpoint(tar_norm.first, tar_norm.second, true) || (dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25)){
            DTG_.RemoveVp(tar_norm.first, tar_norm.second);
            cout<<"too close norm!"<<endl;
            return EdenPlanStatus::TARGET_INVALID;
        }

        plan_ts = ros::WallTime::now().toSec();
        if(TrajOpt_.NormTrajOptimize(toi)){
            const Trajectory4<5> candidate = TrajOpt_.norm_traj_;
            validation_reason.clear();
            if(!ValidateTrajectory(candidate, validation_reason)){
                ROS_ERROR_STREAM("[EDEN] rejected invalid normal trajectory: "
                                 << validation_reason);
                return EdenPlanStatus::TRAJECTORY_FAILED;
            }
            traj_duration = ros::WallTime::now().toSec() - plan_ts;
            if(stat_){
                CS_.SetVolume(target_duration, 3);
                CS_.SetVolume(corridor_duration, 4);
                CS_.SetVolume(traj_duration, 5);
            }
            exc_traj_ = candidate;
            run_branch_ = false;
            traj_start_t_ = hand_t;
            replan_t_ = min(replan_duration_ * 2.5, max(exc_traj_.getTotalDuration() - 0.15, 0.0)) + traj_start_t_;
            const bool early_replan =
                TrajOpt_.main_traj_.getPieceNum() > 0 &&
                !TrajOpt_.upboundVec_.empty() &&
                std::isfinite(TrajOpt_.upboundVec_[0]) &&
                TrajOpt_.upboundVec_[0] > 0.0 &&
                TrajOpt_.main_traj_[0].getCoeffMat().allFinite() &&
                TrajOpt_.main_traj_[0].getCoeffMat().col(4).head(3).norm() <
                    TrajOpt_.upboundVec_[0] * 0.6;
            if(early_replan){
                traj_replan_t_ = traj_start_t_ + traj_replan_duration_;
            }
            else{
                traj_replan_t_ = traj_start_t_ + exc_traj_.getTotalDuration() * 2.0;
            }
            traj_end_t_ = exc_traj_.getTotalDuration() + traj_start_t_;
            target_f_id_ = tar_norm.first;
            target_v_id_ = tar_norm.second;
            path_norm_ = path_norm;
            tar_norm_ = toi.endState_norm;
            traj_type_ = 2;

            cout<<"duration:"<<exc_traj_.getTotalDuration()<<endl;
            cout<<"replan_t_2:"<<replan_t_ - cur_t<<endl;
            
            DTG_.RecordExecutedFrontier(tar_norm.first);
            if(!PublishTraj()){
                return EdenPlanStatus::TRAJECTORY_FAILED;
            }
            log_plan_timings();
            ShowTraj();
            ShowPathCorridor(path_norm, toi.corridors_norm);
            cout<<"plan cost:"<<ros::WallTime::now().toSec() - cur_t<<endl;

            return EdenPlanStatus::SUCCESS;
        }
        else{
            return EdenPlanStatus::TRAJECTORY_FAILED;
        }
    }
    else{
        return EdenPlanStatus::TARGET_INVALID;
    }
    
}

bool SingleExp::TrajReplan(){
    Eigen::Vector3d ps, vs, as, pe, ve, ae;
    double ys, yds, ydds, ye, yde, ydde;
    double hand_t = max(ros::WallTime::now().toSec() + reach_out_t_ * 0.5, traj_start_t_); 
    double cur_t = ros::WallTime::now().toSec();
    if(!std::isfinite(hand_t) || !std::isfinite(cur_t) ||
       !std::isfinite(traj_start_t_) || !std::isfinite(traj_end_t_)){
        ROS_ERROR("[EDEN] invalid absolute trajectory timing before replanning");
        return false;
    }
    if(cur_t <= traj_end_t_){
        std::string active_reason;
        if(!ValidateTrajectory(exc_traj_, active_reason)){
            ROS_ERROR_STREAM("[EDEN] cannot sample active trajectory for replanning: "
                             << active_reason);
            return false;
        }
    }
    if(hand_t > traj_end_t_){
        if(cur_t - traj_end_t_ > 0.0){
            hand_t = ros::WallTime::now().toSec();
            ps = p_;
            vs = v_;
            as.setZero();
            ve.setZero();
            ae.setZero();
            ys = yaw_;
            yds = yaw_v_;
            ydds = 0;
            yde = 0; 
            ydde = 0;
            // ROS_ERROR("start 0==================");
            // cout<<"t cur_t0:"<<cur_t - traj_start_t_<<endl;
            // cout<<"t plan0:"<<hand_t - traj_start_t_<<endl;
            // cout<<"t total0:"<<exc_traj_.getTotalDuration()<<endl;
        }
        else{
            Eigen::Vector4d p4s, v4s, a4s;
            hand_t = max(traj_end_t_ - 1e-3 - cur_t, 0.0) + cur_t;
            p4s = exc_traj_.getPos(hand_t - traj_start_t_);
            v4s = exc_traj_.getVel(hand_t - traj_start_t_);
            a4s = exc_traj_.getAcc(hand_t - traj_start_t_);
            // ROS_WARN("start 1");
            // cout<<"t cur_t1:"<<cur_t - traj_start_t_<<endl;
            // cout<<"t plan1:"<<hand_t - traj_start_t_<<endl;
            // cout<<"t total1:"<<exc_traj_.getTotalDuration()<<endl;
            ps = p4s.head(3);
            vs = v4s.head(3);
            as = a4s.head(3);
            ys = p4s(3);
            yds = v4s(3);
            ydds = a4s(3);
            ve.setZero();
            ae.setZero();
            // YawP_.GetCmd(cur_t - traj_start_t_, ys, yds, ydds);
            yde = 0; 
            ydde = 0;
        }
    }
    else{
        Eigen::Vector4d p4s, v4s, a4s;
        p4s = exc_traj_.getPos(hand_t - traj_start_t_);
        ps = p4s.head(3);
        while(!LRM_.IsFeasible(ps) && hand_t > cur_t){
            hand_t -= reach_out_t_ * 0.5 / 10;
            p4s = exc_traj_.getPos(hand_t - traj_start_t_);
            ps = p4s.head(3);
        }
        if(!LRM_.IsFeasible(ps)){
            hand_t = cur_t;
            ps = p_;
        }
        // ROS_WARN("start 2");
        // cout<<"t plan2:"<<hand_t - traj_start_t_<<endl;
        // cout<<"t cur_t2:"<<cur_t - traj_start_t_<<endl;
        // cout<<"t total2:"<<exc_traj_.getTotalDuration()<<endl;
        v4s = exc_traj_.getVel(hand_t - traj_start_t_);
        a4s = exc_traj_.getAcc(hand_t - traj_start_t_);
        vs = v4s.head(3);
        as = a4s.head(3);
        ys = p4s(3);
        yds = v4s(3);
        ydds = a4s(3);
        ve.setZero();
        ae.setZero();
        // YawP_.GetCmd(hand_t - traj_start_t_, ys, yds, ydds);
        yde = 0; 
        ydde = 0;
    }
    if(!ps.allFinite() || !vs.allFinite() || !as.allFinite() ||
       !std::isfinite(ys) || !std::isfinite(yds) ||
       !std::isfinite(ydds)){
        ROS_ERROR("[EDEN] invalid initial state for trajectory replanning");
        return false;
    }
    EROI_.YawNorm(ys);
    if(!std::isfinite(ys)){
        ROS_ERROR("[EDEN] yaw normalization failed during trajectory replanning");
        return false;
    }

    TrajOptInput toi;
    vector<Eigen::Vector3d> path_stem_temp, path_main_temp, path_sub_temp, path_norm_temp, pp; 
    std::string validation_reason;
    toi.initState.resize(4, 3);
    toi.initState.col(0).head(3) = ps;
    toi.initState(3, 0) = ys;
    toi.initState.col(1).head(3) = vs;
    toi.initState(3, 1) = yds;
    toi.initState.col(2).head(3) = as;
    toi.initState(3, 2) = ydds;
    if(traj_type_ == 1){
        pe = tar_stem_.head(3);
        if(!pe.allFinite() || !LRM_.IsFeasible(pe)){
            ROS_WARN("[EDEN] invalid branch terminal position for replanning");
            return false;
        }
        if(!ValidatePath(path_sub_, false, validation_reason)){
            ROS_WARN_STREAM("[EDEN] rejected replanning sub path: "
                            << validation_reason);
            return false;
        }
        if(!ValidatePath(path_main_, true, validation_reason)){
            ROS_WARN_STREAM("[EDEN] rejected replanning main path: "
                            << validation_reason);
            return false;
        }

        LRM_.ClearTopo();
        double dist;
        if(!LRM_.GetPath(ps, pe, path_stem_temp, dist)) {
            ROS_WARN("stem path search failed");
            return false;
        }
        if(!std::isfinite(dist) ||
           !ValidatePath(path_stem_temp, false, validation_reason)){
            ROS_WARN_STREAM("[EDEN] invalid replanning stem path: "
                            << validation_reason);
            return false;
        }

        ROS_WARN("stem");
        int s = LRM_.FindCorridors(path_stem_temp, toi.corridors_stem, toi.corridorVs_stem, pp, false, INFINITY);
        if(s != 1){
            ROS_ERROR("path_stem corridor failed");
            return false;
        }

        if(pp.size() == 0){
            ROS_ERROR("path_stem size == 0");
            return false;
        }
        // for(auto p : path_stem_temp){
        //     cout<<"pstem:"<<p.transpose()<<endl;
        // }
        toi.endState_stem.head(3) = pp.back();
        toi.endState_stem(3) = tar_stem_(3);

        ROS_WARN("main");
        if(LRM_.FindCorridors(path_main_, toi.corridors_main, toi.corridorVs_main, pp, true, INFINITY) != 1) {
            ROS_ERROR("path_main corridor failed");
            return false;
        }

        if(pp.size() == 0){
            ROS_ERROR("path_main size == 0");
            return false;
        }
        toi.endState_main.head(3) = pp.back() - Eigen::Vector3d::Ones() * 0.05;;
        toi.endState_main(3) = tar_main_(3);

        // ROS_WARN("FindCorridors sub");
        // cout<<"path_sub size:"<<path_sub.size()<<endl;
        // for(auto ps : path_sub){
        //     cout<<"ps:"<<ps.transpose()<<endl;
        // }
        ROS_WARN("sub");
        if(LRM_.FindCorridors(path_sub_, toi.corridors_sub, toi.corridorVs_sub, pp, false, traj_sub_length_) == 0) {
            ROS_ERROR("path_sub corridor failed");
            return false;
        }
        if(pp.size() == 0){
            ROS_ERROR("path_sub size == 0");
            return false;
        }
        toi.endState_sub.head(3) = pp.back() + Eigen::Vector3d::Ones() * 0.05;
        toi.endState_sub(3) = tar_sub_(3);

        toi.camFov = camFov_;
        toi.camV = camV_;
        if(!PopulateCorridorBounds(toi.corridorVs_main, toi.traj_bbxs_main,
                                   validation_reason) ||
           !PopulateCorridorBounds(toi.corridorVs_sub, toi.traj_bbxs_sub,
                                   validation_reason) ||
           !PopulateCorridorBounds(toi.corridorVs_stem, toi.traj_bbxs_stem,
                                   validation_reason) ||
           !PopulateCorridorBounds(toi.corridorVs_norm, toi.traj_bbxs_norm,
                                   validation_reason)){
            ROS_ERROR_STREAM("[EDEN] rejected invalid replan corridor bounds: "
                             << validation_reason);
            return false;
        }

        validation_reason.clear();
        if(!ValidateTrajectoryInput(toi, 2, validation_reason)){
            ROS_ERROR_STREAM("[EDEN] rejected trajectory replan input: "
                             << validation_reason);
            return false;
        }

        if(TrajOpt_.AggressiveBranchTrajOptimize(toi)){
            const Trajectory4<5> candidate = TrajOpt_.stem_traj_;
            validation_reason.clear();
            if(!ValidateTrajectory(candidate, validation_reason) ||
               !ValidateTrajectory(TrajOpt_.main_traj_, validation_reason) ||
               !ValidateTrajectory(TrajOpt_.sub_traj_, validation_reason)){
                ROS_ERROR_STREAM("[EDEN] rejected invalid replanned trajectory: "
                                 << validation_reason);
                return false;
            }
            exc_traj_ = candidate;
            run_branch_ = true;
            traj_start_t_ = hand_t;
            replan_t_ = min(replan_t_, min(replan_duration_, max(exc_traj_.getTotalDuration() - 0.15, 0.0)) + traj_start_t_);
            traj_end_t_ = exc_traj_.getTotalDuration() + traj_start_t_;
            const bool early_replan =
                !TrajOpt_.upboundVec_.empty() &&
                std::isfinite(TrajOpt_.upboundVec_[0]) &&
                TrajOpt_.upboundVec_[0] > 0.0 &&
                TrajOpt_.main_traj_[0].getCoeffMat().col(4).head(3).norm() <
                    TrajOpt_.upboundVec_[0] * 0.6;
            if(early_replan){
                traj_replan_t_ = traj_start_t_ + traj_replan_duration_;
            }
            else{
                traj_replan_t_ = traj_start_t_ + exc_traj_.getTotalDuration() * 2.0;
            }

            // target_f_id_ = tar_stem.first;
            // target_v_id_ = tar_stem.second;
            path_stem_ = path_stem_temp;
            // path_sub_ = path_sub;
            // path_main_ = path_main;
            // cout<<"replan_t_1:"<<replan_t_ - cur_t<<endl;
            // cout<<"eroi state:"<<int(EROI_.EROI_[target_f_id_].f_state_)<<"  vp state:"<<int(EROI_.EROI_[target_f_id_].local_vps_[target_v_id_])<<endl;

            if(target_f_id_ >= 0){
                DTG_.RecordExecutedFrontier(static_cast<uint32_t>(target_f_id_));
            }
            if(!PublishTraj()){
                return false;
            }
            ShowTraj();
            ShowPathCorridor(path_stem_temp, toi.corridors_stem);

            cout<<"replan ter v:"<<TrajOpt_.main_traj_[0].getCoeffMat().col(4).transpose()<<"    "<<TrajOpt_.main_traj_[0].getCoeffMat().col(4).head(3).norm()<<endl;
            cout<<"replan ter a:"<<TrajOpt_.main_traj_[0].getCoeffMat().col(3).transpose() * 2<<"    "<<TrajOpt_.main_traj_[0].getCoeffMat().col(3).head(3).norm() * 2<<endl;
            Eigen::Vector3d u_stem, d_stem, u_sub, d_sub, u_main, d_main;
            u_main(0) = -toi.corridors_main[0](0, 3);
            u_main(1) = -toi.corridors_main[0](2, 3);
            u_main(2) = -toi.corridors_main[0](4, 3);
            d_main(0) = toi.corridors_main[0](1, 3);
            d_main(1) = toi.corridors_main[0](3, 3);
            d_main(2) = toi.corridors_main[0](5, 3);
            cout<<"main diff u:"<<(u_main - toi.endState_stem.head(3)).transpose()<<endl;
            cout<<"main diff d:"<<(d_main - toi.endState_stem.head(3)).transpose()<<endl;
            u_sub(0) = -toi.corridors_sub[0](0, 3);
            u_sub(1) = -toi.corridors_sub[0](2, 3);
            u_sub(2) = -toi.corridors_sub[0](4, 3);
            d_sub(0) = toi.corridors_sub[0](1, 3);
            d_sub(1) = toi.corridors_sub[0](3, 3);
            d_sub(2) = toi.corridors_sub[0](5, 3);
            cout<<"sub diff u:"<<(u_sub - toi.endState_stem.head(3)).transpose()<<endl;
            cout<<"sub diff d:"<<(d_sub - toi.endState_stem.head(3)).transpose()<<endl;
            cout<<"d1:"<< (toi.endState_stem.head(3) - ps).norm()<<endl;
            cout<<"d2:"<< (toi.endState_main.head(3) - toi.endState_stem.head(3)).norm()<<endl;
            cout<<"plan cost:"<<ros::WallTime::now().toSec() - cur_t<<endl;
            return true;
        }
        else{
            ROS_WARN("opt failed");
            return false;    
        }
    }
    else if(traj_type_ == 2){
        return false;
    }
    else{
        ROS_WARN("unknown traj type");
        return false;
    }
    
}

// bool SingleExp::GoHome(){

// }

int SingleExp::ViewPointsCheck(const double &t){
    if(target_f_id_ == -1 || target_v_id_ == -1) return 0;

    // uint32_t tfid = target_f_id_;

    // EROI_.SampleTargetEroi(tfid);

    Eigen::Vector4d tar_pose;
    if(!EROI_.GetVp(target_f_id_, target_v_id_, tar_pose)) {
        ROS_WARN_STREAM("[EDEN] cannot retrieve target viewpoint f="
                        << target_f_id_ << " vp=" << target_v_id_);
        return 1;
    }
    if(!tar_pose.allFinite() || !p_.allFinite() || !std::isfinite(yaw_)){
        ROS_ERROR("[EDEN] invalid state in viewpoint check");
        return 1;
    }
    double dp, dyaw;
    dyaw = abs(EROI_.YawDiff(yaw_, tar_pose(3)));
    dp = (tar_pose.head(3) - p_).norm();
    if(!std::isfinite(dp) || !std::isfinite(dyaw)){
        ROS_ERROR("[EDEN] non-finite viewpoint distance or yaw difference");
        return 1;
    }
    if((dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25) || !EROI_.StrongCheckViewpoint(target_f_id_, target_v_id_, true)){
        cout<<"too close:"<<(dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25)<<endl;
        if(target_f_id_ >= 0 &&
           static_cast<std::size_t>(target_f_id_) < EROI_.EROI_.size() &&
           target_v_id_ >= 0 &&
           static_cast<std::size_t>(target_v_id_) <
               EROI_.EROI_[target_f_id_].local_vps_.size()){
            DTG_.RemoveVp(target_f_id_, target_v_id_);
        }
        cout<<"erase vp:"<<target_f_id_<<"  "<<target_v_id_<<endl;
        dtg_flag_ = false;
        return 1;
    }

    return 0;
}

DTGPlus::GlobalPlanStatus SingleExp::TargetPlanning(const Eigen::Vector4d &p, const Eigen::Vector4d &v,
    uint8_t &plan_res, vector<Eigen::Vector3d> &path_stem, vector<Eigen::Vector3d> &path_main, 
    vector<Eigen::Vector3d> &path_sub, vector<Eigen::Vector3d> &path_norm, 
    pair<uint32_t, uint8_t> &tar_stem, pair<uint32_t, uint8_t> &tar_main, 
    pair<uint32_t, uint8_t> &tar_sub, pair<uint32_t, uint8_t> &tar_norm, 
    double &y_stem, double &y_main, double &y_sub, double &y_norm){
    // routing
    Eigen::Vector3d ps = p.head(3);
    vector<DTGPlus::h_ptr> route_h;
    vector<Eigen::Vector3d> path1;
    double d1;
    ros::WallTime t1 = ros::WallTime::now();
    const auto global_status = DTG_.PlanGlobalRoute(ps, route_h, path1, d1);
    if(global_status != DTGPlus::GlobalPlanStatus::SUCCESS){
        plan_res = 1; // route failed
        // Never terminate the process from a routing outcome.  The caller
        // applies the guarded completion test; every other status, including
        // spectral timeout/instability/rejection, remains recoverable.
        return global_status;
    }
   
    // local exp sequence
    DTG_.FindFastExpTarget(route_h, p, v, path1, d1, 
        plan_res, path_stem, path_main, path_sub, path_norm, 
        tar_stem, tar_main, tar_sub, tar_norm,
        y_stem, y_main, y_sub, y_norm);
    return DTGPlus::GlobalPlanStatus::SUCCESS;

}

bool SingleExp::PublishTraj(){
    std::string reason;
    if(!ValidateTrajectory(exc_traj_, reason)){
        ROS_ERROR_STREAM("[EDEN] refusing to publish invalid trajectory: " << reason);
        return false;
    }
    if(!std::isfinite(traj_start_t_)){
        ROS_ERROR("[EDEN] refusing to publish trajectory with invalid start time");
        return false;
    }

    swarm_exp_msgs::LocalTraj traj;
    traj.state = 2;
    traj.start_t = traj_start_t_;
    traj.coef_p.resize(exc_traj_.getPieceNum() * 6);
    traj.t_p.resize(exc_traj_.getPieceNum());
    traj.order_p = 5;
    
    traj.order_yaw = 5;
    traj.t_yaw.resize(exc_traj_.getPieceNum());
    traj.coef_yaw.resize(exc_traj_.getPieceNum() * 6);

    for(int i = 0; i < exc_traj_.getPieceNum(); i++){
        auto &cur_p = exc_traj_[i];
        Eigen::MatrixXd cM;
        cM = cur_p.getCoeffMat();
        traj.t_p[i] = cur_p.getDuration();
        traj.t_yaw[i] = cur_p.getDuration();
        for(int j = 0; j < cM.cols(); j++){
            if(!std::isfinite(cM(0, j)) || !std::isfinite(cM(1, j)) ||
               !std::isfinite(cM(2, j)) || !std::isfinite(cM(3, j))){
                ROS_ERROR("[EDEN] refusing to publish non-finite polynomial coefficients");
                return false;
            }
            traj.coef_p[j + i * 6].x = cM(0, j);
            traj.coef_p[j + i * 6].y = cM(1, j);
            traj.coef_p[j + i * 6].z = cM(2, j);
            traj.coef_yaw[(i+1) * 6 - j - 1] = cM(3, j);
        }
    }

    traj_pub_.publish(traj);
    return true;
}

void SingleExp::ReloadMap(const ros::TimerEvent &e){
    if(!have_odom_) return;
    BM_.ResetLocalMap(p_);

    // BM_->ChangePtsDebug();
    LRM_.ReloadNodesBf(p_);

    vector<Eigen::Vector3d> new_free_nodes;
    LRM_.SetNodeStatesBf(new_free_nodes); // new_free_nodes_ not used
    // ROS_WARN("ReloadMap!");
    // LRM_->CheckThorough();
    BM_.changed_pts_.clear();
}

void SingleExp::UpdateDTG(const ros::TimerEvent &e){
    if(!have_odom_) return;
    ros::WallTime t1 = ros::WallTime::now();
    if(target_f_id_ != -1){
        uint32_t tfid = target_f_id_;
        EROI_.SampleTargetEroi(tfid);
        for(auto &f : EROI_.dead_fnodes_){
            // cout<<"4erase dtg fn:"<<f.first<<"  v:"<<int(f.second)<<endl;
            DTG_.EraseFnodeFromGraph(f.first, f.second);
        }
        EROI_.dead_fnodes_.clear();
    }
    DTG_.BfUpdate(p_);
    dtg_flag_ = true;
    EROI_.vp_update_ = false;
    // double c = (ros::WallTime::now() - t1).toSec();
    ROS_WARN("UpdateDTG!");
    // cout<<"DTG update time:"<<c<<endl;
}

void SingleExp::DataStatistic(const ros::TimerEvent &e){
    if(!have_odom_) return;
    CS_.SetVolume(len_ / (ros::WallTime::now().toSec() - ts_), 2);
    CS_.SetVolume(len_, 1);
    struct rusage usage;

    getrusage(RUSAGE_SELF, &usage);

    CS_.SetVolume(usage.ru_maxrss / 1024.0, 6);
}

void SingleExp::ForceUpdateEroiDtg(){
    if(!have_odom_) return;
    ros::WallTime t1 = ros::WallTime::now();
    if(target_f_id_ != -1){
        uint32_t tfid = target_f_id_;
        EROI_.SampleTargetEroi(tfid);
        for(auto &f : EROI_.dead_fnodes_){
            cout<<"3erase dtg fn:"<<f.first<<"  v:"<<int(f.second)<<endl;
            DTG_.EraseFnodeFromGraph(f.first, f.second);
        }
        EROI_.dead_fnodes_.clear();
    }
    // if(EROI_.single_sample_){

    // }
    // if(EROI_.EROI_[63904].f_state_ == 2){
    //     vector<Eigen::Vector3d> debug_pts;

    //     debug_pts.emplace_back(EROI_.EROI_[63905].center_);
    //     debug_pts.emplace_back(EROI_.EROI_[63904].center_);
    //     Debug(debug_pts);
    //     EROI_.DebugFunc();
    // }
    EROI_.SampleVps();
    EROI_.SampleSingleVpsCallback();
    DTG_.BfUpdate(p_);

    ROS_WARN("ForceUpdateEroiDtg!");
    // double c = (ros::WallTime::now() - t1).toSec();
    // cout<<"DTG update time1:"<<c<<endl;

    dtg_flag_ = true;
    EROI_.vp_update_ = false;
    EROI_.single_sample_ = false;
    EROI_.rough_sample_ = false;
}

// bool SingleExp::FindShorterPath(){

//     Eigen::Vector3d ps, pe;
//     double t = max(min(ros::WallTime::now().toSec() + replan_t_ * 0.5, traj_end_t_ - 1e-3), traj_start_t_ + 1e-3);
//     if(t < traj_start_t_ || t > traj_end_t_) return false;
//     ps = exc_traj_.getPos(t - traj_start_t_).head(3);
//     if(traj_type_ == 1){
//         pe = tar_stem_.head(3);
//     }
//     else{
//         pe = tar_main_.head(3);
//     }
//     vector<Eigen::Vector3d> path_temp;
//     double dist1, dist2;
//     LRM_.ClearTopo();
//     if(!LRM_.GetPath(ps, pe, path_temp, dist1)){
//         return false;
//     }
//     ps = p_;
//     if(!LRM_.GetPath(ps, pe, path_temp, dist2)){
//         return false;
//     }
//     if(dist1 / TrajOpt_.upboundVec_[0] > dist2 / TrajOpt_.upboundVec_[0] + replan_t_ * 0.5){
//         return true;
//     }
//     return false;
// }

void SingleExp::ImgOdomCallback(const sensor_msgs::ImageConstPtr& img,
    const nav_msgs::OdometryConstPtr& odom){
    double tc = ros::WallTime::now().toSec();

    if(!have_odom_) return;
    last_sensor_update_t_ = tc;
    if(tc - last_map_update_t_ < update_interval_) return;
    last_map_update_t_ = tc; 
    ros::WallTime t1 = ros::WallTime::now();

    BM_.OdomCallback(odom);
    BM_.CastInsertImg(img);
    // BM_.InsertImg(img);
    // BM_->VisTotalMap();
    // BM_->ChangePtsDebug();
    // have_odom_ = true;
    // p_(0) = odom->pose.pose.position.x;
    // p_(1) = odom->pose.pose.position.y;
    // p_(2) = odom->pose.pose.position.z;
    if(!BM_.changed_pts_.empty()){
        vector<Eigen::Vector3d> new_free_nodes;
        LRM_.SetNodeStatesBf(new_free_nodes);
        EROI_.UpdateFrontier(BM_.newly_register_idx_);
        EROI_.LoadNewFeasibleLrmVox(new_free_nodes);
        EROI_.Robot_pos_ = p_;
        for(auto &f : EROI_.dead_fnodes_){
            DTG_.EraseFnodeFromGraph(f.first, f.second);
        }
        EROI_.dead_fnodes_.clear();
        // ROS_WARN("check1!");
        // LRM_->CheckThorough();
        // new_free_nodes_.clear();
        BM_.changed_pts_.clear();
    }
    Eigen::Vector4d cur_pose(p_(0), p_(1), p_(2), yaw_);
    list<pair<uint32_t, uint8_t>> clear_list;
    EROI_.GetReachVps(cur_pose, LRM_.node_scale_.norm(), 0.5, clear_list);
    for(auto i : clear_list){
        EROI_.RemoveVp(i.first, i.second, false);
        DTG_.EraseFnodeFromGraph(i.first, i.second);
    }
    // ROS_WARN("map update!");
    // cout<<"vel mean:"<<len_ / (ros::WallTime::now().toSec() - ts_)<<endl;
    // cout<<"len:"<<len_<<endl;
    // cout<<"t:"<<ros::WallTime::now().toSec() - ts_<<endl;
    // double c = (ros::WallTime::now() - t1).toSec();

    // cout<<"DTG update time1:"<<c<<endl;

}

void SingleExp::PCLOdomCallback(const sensor_msgs::PointCloud2ConstPtr& pcl,
    const nav_msgs::OdometryConstPtr& odom){
    if(!have_odom_) return;
    last_sensor_update_t_ = ros::WallTime::now().toSec();
}

void SingleExp::BodyOdomCallback(const nav_msgs::OdometryConstPtr& odom){
    Quaterniond qua;
    qua.x() = odom->pose.pose.orientation.x;
    qua.y() = odom->pose.pose.orientation.y;
    qua.z() = odom->pose.pose.orientation.z;
    qua.w() = odom->pose.pose.orientation.w;

    Eigen::Vector3d position(
        odom->pose.pose.position.x,
        odom->pose.pose.position.y,
        odom->pose.pose.position.z);
    Eigen::Vector3d body_velocity(
        odom->twist.twist.linear.x,
        odom->twist.twist.linear.y,
        odom->twist.twist.linear.z);
    Eigen::Vector3d angular_velocity(
        odom->twist.twist.angular.x,
        odom->twist.twist.angular.y,
        odom->twist.twist.angular.z);
    const double quaternion_norm = qua.norm();
    if(!position.allFinite() || !body_velocity.allFinite() ||
       !angular_velocity.allFinite() || !qua.coeffs().allFinite() ||
       !std::isfinite(quaternion_norm) ||
       quaternion_norm <= kMinTrajectoryDuration){
        ROS_ERROR_THROTTLE(1.0, "[EDEN] rejected non-finite or invalid odometry");
        return;
    }

    qua.normalize();
    const Eigen::Matrix3d rotation = qua.toRotationMatrix();
    const Eigen::Vector3d world_velocity = rotation * body_velocity;
    const double next_yaw = atan2(rotation(1, 0), rotation(0, 0));
    const double next_yaw_velocity =
        angular_velocity.z() * rotation(2, 2) +
        angular_velocity.y() * rotation(2, 1) +
        angular_velocity.x() * rotation(2, 0);
    if(!rotation.allFinite() || !world_velocity.allFinite() ||
       !std::isfinite(next_yaw) || !std::isfinite(next_yaw_velocity)){
        ROS_ERROR_THROTTLE(1.0, "[EDEN] rejected odometry with invalid derived state");
        return;
    }

    if(have_odom_){
        const double displacement = (p_ - position).norm();
        if(std::isfinite(displacement)) len_ += displacement;
    }
    else{
        ts_ = ros::WallTime::now().toSec();
    }

    robot_pose_.setZero();
    have_odom_ = true;
    p_ = position;
    robot_pose_.block(0, 0, 3, 3) = rotation;
    robot_pose_.block(0, 3, 3, 1) = p_;

    v_ = world_velocity;
    yaw_ = next_yaw;
    yaw_v_ = next_yaw_velocity;

    v_total_ += v_.norm();
    v_num_++;
    // SDM_.SetPose(*odom);

    // LoadShowPose(odom->pose.pose);
    // posevis_pub_.publish(vis_model_);
}

void SingleExp::ShowTraj(){
    visualization_msgs::MarkerArray mka;
    mka.markers.resize(7);
    mka.markers[0].header.frame_id = "world";
    mka.markers[0].header.stamp = ros::Time::now();
    mka.markers[0].id = 0;
    mka.markers[0].action = visualization_msgs::Marker::ADD;
    mka.markers[0].type = visualization_msgs::Marker::LINE_STRIP;
    mka.markers[0].scale.x = 0.05;
    mka.markers[0].scale.y = 0.05;
    mka.markers[0].scale.z = 0.05;
    mka.markers[0].color.a = 0.8;
    mka.markers[0].color.r = 0.8;
    mka.markers[0].color.g = 0.8;
    mka.markers[0].color.b = 0.0;

    mka.markers[1] = mka.markers[0];
    mka.markers[1].id = 1;
    mka.markers[1].color.r = 0.8;
    mka.markers[1].color.g = 0.2;
    mka.markers[1].color.b = 0.6;

    mka.markers[2] = mka.markers[0];
    mka.markers[2].id = 2;
    mka.markers[2].color.r = 0.8;
    mka.markers[2].color.g = 0.0;
    mka.markers[2].color.b = 0.0;
    
    mka.markers[3] = mka.markers[0];
    mka.markers[3].id = 3;
    mka.markers[3].color.r = 0.0;
    mka.markers[3].color.g = 0.8;
    mka.markers[3].color.b = 0.8;

    mka.markers[4] = mka.markers[0];
    mka.markers[4].id = 4;
    mka.markers[4].color.r = 0.8;
    mka.markers[4].color.g = 0.2;
    mka.markers[4].color.b = 0.6;
    mka.markers[4].type = visualization_msgs::Marker::LINE_LIST;

    mka.markers[5] = mka.markers[0];
    mka.markers[5].id = 5;
    mka.markers[5].color.r = 0.8;
    mka.markers[5].color.g = 0.0;
    mka.markers[5].color.b = 0.0;
    mka.markers[5].type = visualization_msgs::Marker::LINE_LIST;

    mka.markers[6] = mka.markers[0];
    mka.markers[6].id = 6;
    mka.markers[6].color.r = 0.0;
    mka.markers[6].color.g = 0.8;
    mka.markers[6].color.b = 0.8;
    mka.markers[6].type = visualization_msgs::Marker::LINE_LIST;
    geometry_msgs::Point pt;
    Eigen::Vector4d p;
    if(run_branch_){
        double tt = TrajOpt_.stem_traj_.getTotalDuration();
        for(double t = 0; t < tt; t += 0.05){
            p = TrajOpt_.stem_traj_.getPos(t);
            pt.x = p(0);
            pt.y = p(1);
            pt.z = p(2);
            mka.markers[1].points.emplace_back(pt);
            if(t + 0.05 > tt){
                EROI_.LoadVpLines(mka.markers[4], p);
            }
        }
        tt = TrajOpt_.main_traj_.getTotalDuration();
        for(double t = 0; t < tt; t += 0.05){
            p = TrajOpt_.main_traj_.getPos(t);
            pt.x = p(0);
            pt.y = p(1);
            pt.z = p(2);
            mka.markers[2].points.emplace_back(pt);
            if(t + 0.05 >= tt){
                EROI_.LoadVpLines(mka.markers[5], p);
            }
        }
        tt = TrajOpt_.sub_traj_.getTotalDuration();
        for(double t = 0; t < tt; t += 0.05){
            p = TrajOpt_.sub_traj_.getPos(t);
            pt.x = p(0);
            pt.y = p(1);
            pt.z = p(2);
            mka.markers[3].points.emplace_back(pt);
            if(t + 0.05 >= tt){
                EROI_.LoadVpLines(mka.markers[6], p);
            }
        }
    }
    else{
        double tt = TrajOpt_.norm_traj_.getTotalDuration();
        for(double t = 0; t < tt; t += 0.05){
            p = TrajOpt_.norm_traj_.getPos(t);
            pt.x = p(0);
            pt.y = p(1);
            pt.z = p(2);
            mka.markers[3].points.emplace_back(pt);
        }
    }

    vis_pub_.publish(mka);
}


void SingleExp::ShowPathCorridor(vector<Eigen::Vector3d> &path, vector<Eigen::MatrixX4d> &h){
    visualization_msgs::MarkerArray mka;
    mka.markers.resize(1 + h.size());
    mka.markers[0].header.frame_id = "world";
    mka.markers[0].header.stamp = ros::Time::now();
    mka.markers[0].id = -10;
    mka.markers[0].action = visualization_msgs::Marker::ADD;
    mka.markers[0].type = visualization_msgs::Marker::LINE_STRIP;
    mka.markers[0].scale.x = 0.07;
    mka.markers[0].scale.y = 0.07;
    mka.markers[0].scale.z = 0.07;
    mka.markers[0].color.a = 1.0;
    mka.markers[0].color.r = 0.9;
    mka.markers[0].color.g = 0.9;
    mka.markers[0].color.b = 0.9;
    // if(dangerous_path_){
    //     mka.markers[0].color.a = 1.0;
    //     mka.markers[0].color.r = 0.99;
    //     mka.markers[0].color.g = 0.0;
    //     mka.markers[0].color.b = 0.0;
    // }

    // for(double delta = 0; delta < TrajOpt_.norm_traj_.getTotalDuration(); delta += 0.025){
    //     Eigen::Vector3d p;
    //     geometry_msgs::Point pt;
    //     p = TrajOpt_.norm_traj_.getPos(delta);
    //     pt.x = p(0);
    //     pt.y = p(1);
    //     pt.z = p(2);
    //     mka.markers[0].points.emplace_back(pt);
    // }
    for(auto &p : path){
        geometry_msgs::Point pt;
        pt.x = p(0);
        pt.y = p(1);
        pt.z = p(2);
        mka.markers[0].points.emplace_back(pt);
    }
    for(int i = 0; i < h.size(); i++){
        mka.markers[i + 1].pose.position.x = (h[i](1, 3) - h[i](0, 3)) / 2;
        mka.markers[i + 1].pose.position.y = (h[i](3, 3) - h[i](2, 3)) / 2;
        mka.markers[i + 1].pose.position.z = (h[i](5, 3) - h[i](4, 3)) / 2;
        mka.markers[i + 1].pose.orientation.w = 1.0;

        mka.markers[i + 1].scale.x = (- h[i](1, 3) - h[i](0, 3));
        mka.markers[i + 1].scale.y = (- h[i](3, 3) - h[i](2, 3));
        mka.markers[i + 1].scale.z = (- h[i](5, 3) - h[i](4, 3));
        mka.markers[i + 1].type = visualization_msgs::Marker::CUBE;
        mka.markers[i+1].header.frame_id = "world";
        mka.markers[i+1].header.stamp = ros::Time::now();
        mka.markers[i+1].id = 10+i;
        mka.markers[i+1].action = visualization_msgs::Marker::ADD;
        mka.markers[i+1].lifetime = ros::Duration(1.0);
        mka.markers[i + 1].color.a = 0.2;
        mka.markers[i + 1].color.g = 0.7;
        mka.markers[i + 1].color.b = 0.5;
        // if(dangerous_path_){
        //     mka.markers[i + 1].color.a = 0.2;
        //     mka.markers[i + 1].color.r = 0.9;
        //     mka.markers[i + 1].color.g = 0.2;
        //     mka.markers[i + 1].color.b = 0.2;
        // }
    }
    vis_pub_.publish(mka);
}

void SingleExp::Debug(vector<Eigen::Vector3d> &pts){
    visualization_msgs::Marker mk, mkr1, mkr2;
    mk.header.frame_id = "world";
    mk.header.stamp = ros::Time::now();
    mk.id = 0;
    // mk.id = scan_count_;
    // scan_count_++;
    mk.action = visualization_msgs::Marker::ADD;
    mk.type = visualization_msgs::Marker::SPHERE_LIST;
    mk.scale.x = 0.1;
    mk.scale.y = 0.1;
    mk.scale.z = 0.1;
    mk.color.a = 1.0;
    mk.color.b = 1.0;
    mk.pose.position.x = 0;
    mk.pose.position.y = 0;
    mk.pose.position.z = 0;
    mk.pose.orientation.x = 0;
    mk.pose.orientation.y = 0;
    mk.pose.orientation.z = 0;
    mk.pose.orientation.w = 1;

    geometry_msgs::Point pt;
    for(auto &p : pts){
        pt.x = p.x();
        pt.y = p.y();
        pt.z = p.z();
        mk.points.push_back(pt);
    }
    debug_pub_.publish(mk);
}


void SingleExp::StopDebugFunc(){
    vector<Eigen::Vector3d> debug_pts;
    cout<<"EROI_.EROI_:"<<EROI_.EROI_.size()<<endl;
    cout<<"center:"<<EROI_.EROI_[63905].center_.transpose()<<endl;
    // EROI_.DebugFunc();
    debug_pts.emplace_back(EROI_.EROI_[63905].center_);
    debug_pts.emplace_back(EROI_.EROI_[63904].center_);
    Debug(debug_pts);
}
