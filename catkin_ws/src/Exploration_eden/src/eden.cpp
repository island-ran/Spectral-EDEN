#include <eden/eden.h>


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
    nh_private.param(
        ns + "/Exp/VolumeCoverageEnabled",
        volume_coverage_enabled_, false);
    nh_private.param(
        ns + "/Exp/TotalExplorableVolume",
        total_explorable_volume_, 0.0);
    nh_private.param(
        ns + "/Exp/VolumeCoverageThreshold",
        volume_coverage_threshold_, 1.0);
    nh_private.param(
        ns + "/Exp/FinishStableDuration",
        finish_stable_duration_, 2.0);
    volume_coverage_threshold_ = std::max(
        0.0, std::min(1.0, volume_coverage_threshold_));
    finish_stable_duration_ =
        std::max(0.0, finish_stable_duration_);
    finish_stable_since_ = -1.0;
    exploration_finished_ = false;
    
    // nh_private.param(ns + "/opt/YawVel", dyaw_max_, 2.0);
    // nh_private.param(ns + "/opt/YawAcc", ddyaw_max_, 2.0);
    // nh_private.param(ns + "/Exp/UseMotion", use_motion_, true);
    // nh_private.param(ns + "/Exp/UseTerminalCorridor", use_terminal_corridor_, true);
    // nh_private.param(ns + "/Exp/UseDirCorridor", use_dir_corridor_, true);
    // nh_private.param(ns + "/Exp/UseCoverTrajectory", use_cover_trajectory_, true);
    // nh_private.param(ns + "/Exp/VolumeTest", volume_test_, false);
    nh_private.param(ns + "/Exp/UpdateInterval", update_interval_, 0.2);
    p_.setZero();
    v_.setZero();
    yaw_ = 0.0;
    yaw_v_ = 0.0;

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
    v_total_ = 0.0;
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

    last_map_update_t_ = ros::WallTime::now().toSec();
    traj_end_t_ = last_map_update_t_ - 0.1;
    replan_t_ = traj_end_t_;
    plan_t_ = last_map_update_t_;
    have_odom_ = false;
    target_f_id_ = -1;
    target_v_id_ = -1;
    dtg_flag_ = false;
    traj_start_t_ = last_map_update_t_ - 0.2;
    if(EROI_.sensor_type_ == SensorType::CAMERA){
        depth_sub_.reset(new message_filters::Subscriber<sensor_msgs::Image>(nh_, "/depth", 10));
        vi_odom_sub_.reset(new message_filters::Subscriber<nav_msgs::Odometry>(nh_, "/vi_odom", 10));
        sync_image_odom_.reset(new message_filters::Synchronizer<SyncPolicyImageOdom>(
            SyncPolicyImageOdom(100), *depth_sub_, *vi_odom_sub_));
        sync_image_odom_->registerCallback(boost::bind(&SingleExp::ImgOdomCallback, this,  _1, _2));
    }
    else{
        ROS_FATAL(
            "eden currently requires Frontier/sensor_type=Depth_Camera");
        throw std::runtime_error(
            "unsupported frontier sensor type");
    }
    if(stat_ || volume_coverage_enabled_){
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
    const double safe_interval =
        EdenSafety::SanitizePlanInterval(intv);
    if(!std::isfinite(intv) ||
       std::abs(safe_interval - intv) > 1.0e-9){
        ROS_ERROR(
            "invalid planner interval %.6f; clamped to %.3f s",
            intv, safe_interval);
    }
    plan_t_ = ros::WallTime::now().toSec() + safe_interval;
}

bool SingleExp::TrajCheck(){
    double cur_t = max(ros::WallTime::now().toSec(), traj_start_t_);

    if(cur_t > traj_end_t_ - 1e-3 || cur_t > replan_t_) { // traj time out
        ROS_WARN("traj time up");
        // dtg_flag_ = false; // need to update map
        return false;
    }

    const double total_duration = exc_traj_.getTotalDuration();
    if(exc_traj_.getPieceNum() <= 0 || !std::isfinite(total_duration) ||
       total_duration <= EdenSafety::kMinTrajectoryDuration){
        ROS_ERROR("current trajectory is numerically invalid");
        return false;
    }
    double end_t = min(cur_t + check_duration_, replan_t_ - 1e-3);
    if(end_t <= cur_t) return true;
    const Eigen::Vector3d start_position =
        exc_traj_.getPos(EdenSafety::RelativeTrajectoryTime(
            cur_t, traj_start_t_, total_duration)).head(3);
    if(!start_position.allFinite()) return false;
    Eigen::Vector3d last_checked_position = start_position;
    const double current_footprint_radius = std::max(
        BM_.resolution_, 0.5 * LRM_.GetRobotSize().norm());
    for(double t = cur_t + 0.05; t < end_t; t += 0.05){
        const double relative_t = EdenSafety::RelativeTrajectoryTime(
            t, traj_start_t_, total_duration);
        const Eigen::Vector3d p =
            exc_traj_.getPos(relative_t).head(3);
        if(!p.allFinite()) return false;
        if((p - start_position).norm() < current_footprint_radius ||
           (p - last_checked_position).norm() < BM_.resolution_)
            continue;
        last_checked_position = p;
        if(BM_.PosBBXOccupied(p, LRM_.GetRobotSize())) {
            dtg_flag_ = false;
            ROS_WARN("confirmed occupied voxel on active trajectory");
            return false;
        }
    }

    const Eigen::Vector3d end_position =
        exc_traj_.getPos(EdenSafety::RelativeTrajectoryTime(
        end_t - 1e-4, traj_start_t_, total_duration)).head(3);
    if(!end_position.allFinite()) return false;
    if((end_position - start_position).norm() >=
           current_footprint_radius &&
       BM_.PosBBXOccupied(end_position, LRM_.GetRobotSize())) {
        dtg_flag_ = false;
        ROS_WARN("confirmed occupied endpoint on active trajectory");
        return false;
    }
    return true;
}

int SingleExp::AllowPlan(const double &T){

    /* not satisfy plan interval */
    const double deadline_remaining = plan_t_ - T;
    if(!std::isfinite(T) || !std::isfinite(plan_t_) ||
       deadline_remaining >
           EdenSafety::kMaxPlanInterval + 0.1){
        ROS_ERROR_THROTTLE(
            1.0,
            "planner deadline corrupted (now=%.6f deadline=%.6f "
            "remaining=%.3f); forcing immediate recovery",
            T, plan_t_, deadline_remaining);
        plan_t_ = std::isfinite(T)
                      ? T
                      : ros::WallTime::now().toSec();
    }
    else if(deadline_remaining > 0.0){
        return 1;
    }

    /* not in free space */
    if(!LRM_.IsFeasible(p_)){
        return 2;
    }
    // last_safe_ = p_;

    /* traj time up */
    if(T - replan_t_ > 0){
        return 5;
    }

    return 0;
}

void SingleExp::EscalatePlanningFailure(){
    DTG_.RejectPendingTargetAfterPlanningFailure(true);
    dtg_flag_ = false;
}


bool SingleExp::EdenPlan(){
    Eigen::Vector3d ps, vs, as, pe, ve, ae;
    double ys, yds, ydds, ye, yde, ydde;
    double hand_t = max(ros::WallTime::now().toSec() + reach_out_t_, traj_start_t_); 
    double cur_t = ros::WallTime::now().toSec();
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
            hand_t = max(cur_t, hand_t - reach_out_t_ / 10);
            p4s = exc_traj_.getPos(hand_t - traj_start_t_);
            ps = p4s.head(3);
        }
        const bool use_measured_state = !LRM_.IsFeasible(ps);
        if(use_measured_state){
            hand_t = cur_t;
            ps = p_;
        }
        ROS_WARN("start 2");
        cout<<"t plan2:"<<hand_t - traj_start_t_<<endl;
        cout<<"t cur_t2:"<<cur_t - traj_start_t_<<endl;
        cout<<"t total2:"<<exc_traj_.getTotalDuration()<<endl;
        if(use_measured_state){
            vs = v_;
            as.setZero();
            ys = yaw_;
            yds = yaw_v_;
            ydds = 0.0;
        }
        else{
            v4s = exc_traj_.getVel(hand_t - traj_start_t_);
            a4s = exc_traj_.getAcc(hand_t - traj_start_t_);
            vs = v4s.head(3);
            as = a4s.head(3);
            ys = p4s(3);
            yds = v4s(3);
            ydds = a4s(3);
        }
        ve.setZero();
        ae.setZero();
        // YawP_.GetCmd(hand_t - traj_start_t_, ys, yds, ydds);
        yde = 0; 
        ydde = 0;
    }
    EROI_.YawNorm(ys);

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

    uint8_t plan_res = 1;
    vector<Eigen::Vector3d> path_stem, path_main, path_sub, path_norm, pp; 
    pair<uint32_t, uint8_t> tar_stem{0U, 0U};
    pair<uint32_t, uint8_t> tar_main{0U, 0U};
    pair<uint32_t, uint8_t> tar_sub{0U, 0U};
    pair<uint32_t, uint8_t> tar_norm{0U, 0U};
    double y_stem = ys, y_main = ys, y_sub = ys, y_norm = ys;

    Eigen::Vector4d ps_y, vs_dy;
    ps_y.head(3) = ps;
    ps_y(3) = ys;
    vs_dy.head(3) = vs;
    vs_dy(3) = yds;

    double target_duration, corridor_duration, traj_duration;
    double plan_ts = ros::WallTime::now().toSec();
    TargetPlanning(ps_y, vs_dy, plan_res, 
        path_stem, path_main, path_sub, path_norm, 
        tar_stem, tar_main, tar_sub, tar_norm, 
        y_stem, y_main, y_sub, y_norm);
    target_duration = ros::WallTime::now().toSec() - plan_ts;
    auto planning_failure = [&](bool hard_invalidation,
                                const char *stage) {
        DTG_.RejectPendingTargetAfterPlanningFailure(hard_invalidation);
        ROS_WARN("planning failed at %s", stage);
        return false;
    };
    
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
    // if(abs(EROI_.YawDiff(y_main, y_stem)) > M_PI * 0.5) getchar();
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
    //                     getchar();
    //                 }
    //             }
    //         }
    //     }
    // }
    plan_ts = ros::WallTime::now().toSec();
    if(plan_res == 2){
        // ROS_WARN("FindCorridors stem");
        // cout<<"path_stem size:"<<path_stem.size()<<endl;
        // for(auto ps : path_stem){
        //     cout<<"ps:"<<ps.transpose()<<"  fea:"<<LRM_.IsFeasible(ps)<<endl;
        // }
        int s = LRM_.FindCorridors(path_stem, toi.corridors_stem, toi.corridorVs_stem, pp, false, traj_length_);


        if(pp.size() == 0){
            ROS_ERROR("path_stem size == 0");
            return planning_failure(true, "stem corridor empty");
        }
        toi.endState_stem.head(3) = LRM_.GetStdPos(pp.back());
        toi.endState_stem(3) = y_stem;


        if(s == 2){
            plan_res = 3;
        }
        else if(s == 1){
            ROS_WARN("FindCorridors main");
            // cout<<"path_main size:"<<path_main.size()<<endl;
            if(LRM_.FindCorridors(path_main, toi.corridors_main, toi.corridorVs_main, pp, true, INFINITY) != 1) {
                plan_res = 3;
                cout<<"s:"<<s<<endl;
                ROS_ERROR("path_main plan_res == 2");
                return planning_failure(true, "main corridor");
            }

            if(pp.size() == 0){
                ROS_ERROR("path_main size == 0");
                return planning_failure(true, "main corridor empty");
            }
            toi.endState_main.head(3) = LRM_.GetStdPos(pp.back()) - Eigen::Vector3d::Ones() * 0.05;;
            toi.endState_main(3) = y_main;

            ROS_WARN("FindCorridors sub");
            // cout<<"path_sub size:"<<path_sub.size()<<endl;
            // for(auto ps : path_sub){
            //     cout<<"ps:"<<ps.transpose()<<endl;
            // }
            if(LRM_.FindCorridors(path_sub, toi.corridors_sub, toi.corridorVs_sub, pp, false, traj_sub_length_) == 0) {
                plan_res = 3;
                cout<<"s:"<<s<<endl;
                ROS_ERROR("path_sub plan_res == 2");
                return planning_failure(true, "sub corridor");
            }
            if(pp.size() == 0){
                ROS_ERROR("path_sub size == 0");
                return planning_failure(true, "sub corridor empty");
            }
            toi.endState_sub.head(3) = LRM_.GetStdPos(pp.back()) + Eigen::Vector3d::Ones() * 0.05;
            toi.endState_sub(3) = y_sub;
        }
        else{
            ROS_ERROR("plan_res == 2, s == 0");
            return planning_failure(true, "stem corridor");
        }
    }

    if(plan_res == 3 || plan_res == 4){
        ROS_WARN("FindCorridors norm");
        int s = LRM_.FindCorridors(path_norm, toi.corridors_norm, toi.corridorVs_norm, pp, false, traj_length_);

        if(pp.size() == 0){
            ROS_ERROR("path_norm size == 0");
            return planning_failure(true, "normal corridor empty");
        }
        toi.endState_norm.head(3) = LRM_.GetStdPos(pp.back());
        toi.endState_norm(3) = y_norm;

        if(s == 0){
            cout<<"s:"<<s<<endl;
            ROS_ERROR("path_norm plan_res == 2");
            return planning_failure(true, "normal corridor");
        }
        else if(s == 2){
            if((pp.back().head(2)- path_norm.back().head(2)).norm() > 1e-3){
                y_norm = atan2(path_norm.back()(1) - pp.back()(1), path_norm.back()(0) - pp.back()(0));
            }
        }
    }
    corridor_duration = ros::WallTime::now().toSec() - plan_ts;

    toi.camFov = camFov_;
    toi.camV = camV_;
    toi.traj_bbxs_main.resize(toi.corridors_main.size());
    toi.traj_bbxs_sub.resize(toi.corridors_sub.size());
    toi.traj_bbxs_stem.resize(toi.corridors_stem.size());
    toi.traj_bbxs_norm.resize(toi.corridors_norm.size());


    if(plan_res == 2){
        double dp = (ps - EROI_.EROI_[tar_stem.first].center_ - EROI_.vps_[tar_stem.second].head(3)).norm();
        double dyaw = abs(EROI_.YawDiff(EROI_.vps_[tar_stem.second](3), ys));
        if((dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25) || !EROI_.StrongCheckViewpoint(tar_stem.first, tar_stem.second, true)){
            DTG_.RemoveVp(tar_stem.first, tar_stem.second);
            cout<<"too close branch!"<<endl;
            return planning_failure(true, "branch viewpoint invalid");
        }
        plan_ts = ros::WallTime::now().toSec();

        if(TrajOpt_.AggressiveBranchTrajOptimize(toi)){
            traj_duration = ros::WallTime::now().toSec() - plan_ts;
            std::string validation_error;
            if(!EdenSafety::ValidateTrajectory(
                    TrajOpt_.stem_traj_, &validation_error)){
                ROS_ERROR_STREAM("reject branch trajectory: "
                                 << validation_error);
                return planning_failure(
                    true, "branch trajectory validation");
            }
            if(stat_){
                CS_.SetVolume(target_duration, 3);
                CS_.SetVolume(corridor_duration, 4);
                CS_.SetVolume(traj_duration, 5);
            }

            exc_traj_ = TrajOpt_.stem_traj_;
            run_branch_ = true;
            traj_start_t_ = hand_t;
            replan_t_ = min(replan_duration_, max(exc_traj_.getTotalDuration() - 0.15, 0.0)) + traj_start_t_;
            traj_end_t_ = exc_traj_.getTotalDuration() + traj_start_t_;
            target_f_id_ = tar_stem.first;
            target_v_id_ = tar_stem.second;
            // cout<<"duration:"<<exc_traj_.getTotalDuration()<<endl;
            // cout<<"replan_t_1:"<<replan_t_ - cur_t<<endl;
            // cout<<"eroi state:"<<int(EROI_.EROI_[target_f_id_].f_state_)<<"  vp state:"<<int(EROI_.EROI_[target_f_id_].local_vps_[target_v_id_])<<endl;

            PublishTraj();
            ShowTraj();
            ShowPathCorridor(path_stem, toi.corridors_stem);

            // V4: Commit target after successful trajectory publish
            {
                DTGPlus::CommittedTarget ct;
                ct.valid = true;
                ct.frontier_id = tar_stem.first;
                ct.viewpoint_id = tar_stem.second;
                DTG_.CommitTargetAfterPublish(ct);
            }

            return true;
        }
        else{
            return planning_failure(false, "branch optimizer");
        }
    }
    else if(plan_res == 3 || plan_res == 4){
        if(plan_res == 3){
            double dp = (ps - EROI_.EROI_[tar_norm.first].center_ - EROI_.vps_[tar_norm.second].head(3)).norm();
            double dyaw = abs(EROI_.YawDiff(EROI_.vps_[tar_norm.second](3), ys));
            if(!EROI_.StrongCheckViewpoint(tar_norm.first, tar_norm.second, true) || (dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25)){
                DTG_.RemoveVp(tar_norm.first, tar_norm.second);
                cout<<"too close norm!"<<endl;
                return planning_failure(true, "normal viewpoint invalid");
            }
        }

        plan_ts = ros::WallTime::now().toSec();
        if(TrajOpt_.NormTrajOptimize(toi)){
            traj_duration = ros::WallTime::now().toSec() - plan_ts;
            std::string validation_error;
            if(!EdenSafety::ValidateTrajectory(
                    TrajOpt_.norm_traj_, &validation_error)){
                ROS_ERROR_STREAM("reject normal trajectory: "
                                 << validation_error);
                return planning_failure(
                    true, "normal trajectory validation");
            }
            if(stat_){
                CS_.SetVolume(target_duration, 3);
                CS_.SetVolume(corridor_duration, 4);
                CS_.SetVolume(traj_duration, 5);
            }
            exc_traj_ = TrajOpt_.norm_traj_;
            run_branch_ = false;
            traj_start_t_ = hand_t;
            replan_t_ = min(replan_duration_ * 2.5, max(exc_traj_.getTotalDuration() - 0.15, 0.0)) + traj_start_t_;
            traj_end_t_ = exc_traj_.getTotalDuration() + traj_start_t_;
            if(plan_res == 3){
                target_f_id_ = tar_norm.first;
                target_v_id_ = tar_norm.second;
            }
            else{
                target_f_id_ = -1;
                target_v_id_ = -1;
            }
            cout<<"duration:"<<exc_traj_.getTotalDuration()<<endl;
            cout<<"replan_t_2:"<<replan_t_ - cur_t<<endl;
            
            PublishTraj();
            ShowTraj();
            ShowPathCorridor(path_norm, toi.corridors_norm);

            // V4: Commit target after successful trajectory publish
            {
                DTGPlus::CommittedTarget ct;
                ct.valid = true;
                if(plan_res == 3){
                    ct.frontier_id = tar_norm.first;
                    ct.viewpoint_id = tar_norm.second;
                }
                DTG_.CommitTargetAfterPublish(ct);
            }

            cout<<"plan cost:"<<ros::WallTime::now().toSec() - cur_t<<endl;

            return true;
        }
        else{
            return planning_failure(false, "normal optimizer");
        }
    }
    else{
        return planning_failure(false, "target planning");
    }
    
}

int SingleExp::ViewPointsCheck(const double &t){
    if(target_f_id_ == -1 || target_v_id_ == -1) return 0;

    // uint32_t tfid = target_f_id_;

    // EROI_.SampleTargetEroi(tfid);

    Eigen::Vector4d tar_pose;
    if(!EROI_.GetVp(target_f_id_, target_v_id_, tar_pose)) {
        cout<<"cant get vp:"<<target_f_id_<<"  "<<target_v_id_<<endl;
        target_f_id_ = -1;
        target_v_id_ = -1;
        return 1;
    }
    double dp, dyaw;
    dyaw = abs(EROI_.YawDiff(yaw_, tar_pose(3)));
    dp = (tar_pose.head(3) - p_).norm();
    if((dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25) || !EROI_.StrongCheckViewpoint(target_f_id_, target_v_id_, true)){
        cout<<"too close:"<<(dp < LRM_.node_scale_.norm() * 1.0 && dyaw < 0.25)<<endl;
        if(target_f_id_ >= 0 &&
           static_cast<size_t>(target_f_id_) < EROI_.EROI_.size() &&
           target_v_id_ >= 0 &&
           static_cast<size_t>(target_v_id_) <
               EROI_.EROI_[target_f_id_].local_vps_.size()){
            DTG_.RemoveVp(target_f_id_, target_v_id_);
        }
        cout<<"erase vp:"<<target_f_id_<<"  "<<target_v_id_<<endl;
        target_f_id_ = -1;
        target_v_id_ = -1;
        dtg_flag_ = false;
        return 1;
    }

    return 0;
}

void SingleExp::TargetPlanning(const Eigen::Vector4d &p, const Eigen::Vector4d &v,
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
    if(!DTG_.TspApproxiPlan(ps, route_h, path1, d1)){
        plan_res = 1; // route failed
        return;
    }

    if(DTG_.CurrentMacroTargetIsRegionEntry()){
        if(path1.empty()){
            plan_res = 1;
            return;
        }
        plan_res = 4;
        path_norm = path1;
        if(path_norm.size() >= 2){
            const Eigen::Vector3d direction =
                path_norm.back() - path_norm[path_norm.size() - 2];
            y_norm = direction.head(2).norm() > 1.0e-6
                ? std::atan2(direction.y(), direction.x()) : p(3);
        }
        else{
            y_norm = p(3);
        }
        return;
    }
   
    // local exp sequence
    DTG_.FindFastExpTarget(route_h, p, v, path1, d1, 
        plan_res, path_stem, path_main, path_sub, path_norm, 
        tar_stem, tar_main, tar_sub, tar_norm,
        y_stem, y_main, y_sub, y_norm);
    return;

}

void SingleExp::PublishTraj(){
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
            traj.coef_p[j + i * 6].x = cM(0, j);
            traj.coef_p[j + i * 6].y = cM(1, j);
            traj.coef_p[j + i * 6].z = cM(2, j);
            traj.coef_yaw[(i+1) * 6 - j - 1] = cM(3, j);
        }
    }

    traj_pub_.publish(traj);
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
    ROS_WARN("UpdateDTG!");
}

void SingleExp::DataStatistic(const ros::TimerEvent &e){
    if(!have_odom_) return;
    const double elapsed =
        ros::WallTime::now().toSec() - ts_;
    if(stat_){
        CS_.SetVolume(
            elapsed > 1.0e-6 ? len_ / elapsed : 0.0, 2);
        CS_.SetVolume(len_, 1);
        struct rusage usage;
        getrusage(RUSAGE_SELF, &usage);
        CS_.SetVolume(usage.ru_maxrss / 1024.0, 6);
    }
    if(!volume_coverage_enabled_ || exploration_finished_ ||
       total_explorable_volume_ <= 0.0)
        return;
    const double explored_volume = BM_.ExploredVolume();
    const double coverage =
        explored_volume / total_explorable_volume_;
    const double now = ros::WallTime::now().toSec();
    if(std::isfinite(coverage) &&
       coverage >= volume_coverage_threshold_){
        if(finish_stable_since_ < 0.0) finish_stable_since_ = now;
        if(now - finish_stable_since_ >= finish_stable_duration_){
            exploration_finished_ = true;
            ROS_WARN(
                "exploration finished: volume=%.3f coverage=%.6f",
                explored_volume, coverage);
        }
    }
    else{
        finish_stable_since_ = -1.0;
    }
}

void SingleExp::ForceUpdateEroiDtg(){
    if(!have_odom_) return;
    if(target_f_id_ != -1){
        uint32_t tfid = target_f_id_;
        EROI_.SampleTargetEroi(tfid);
        for(auto &f : EROI_.dead_fnodes_){
            cout<<"3erase dtg fn:"<<f.first<<"  v:"<<int(f.second)<<endl;
            DTG_.EraseFnodeFromGraph(f.first, f.second);
        }
        EROI_.dead_fnodes_.clear();
    }
    EROI_.SampleVps();
    EROI_.SampleSingleVpsCallback();
    DTG_.BfUpdate(p_);

    ROS_WARN("ForceUpdateEroiDtg!");
    dtg_flag_ = true;
    EROI_.vp_update_ = false;
    EROI_.single_sample_ = false;
    EROI_.rough_sample_ = false;
}

void SingleExp::ImgOdomCallback(const sensor_msgs::ImageConstPtr& img,
    const nav_msgs::OdometryConstPtr& odom){
    double tc = ros::WallTime::now().toSec();

    if(tc - last_map_update_t_ < update_interval_ || !have_odom_) return;
    last_map_update_t_ = tc; 

    BM_.OdomCallback(odom);
    BM_.CastInsertImg(img);
    if(!BM_.changed_pts_.empty()){
        vector<Eigen::Vector3d> new_free_nodes;
        LRM_.SetNodeStatesBf(new_free_nodes);
        EROI_.UpdateFrontier(BM_.newly_register_idx_);
        DTG_.NotifyFrontierGainChanges(
            EROI_.updated_frontier_ids_);
        EROI_.LoadNewFeasibleLrmVox(new_free_nodes);
        EROI_.Robot_pos_ = p_;
        for(auto &f : EROI_.dead_fnodes_){
            DTG_.EraseFnodeFromGraph(f.first, f.second);
        }
        EROI_.dead_fnodes_.clear();
        BM_.changed_pts_.clear();
    }
    Eigen::Vector4d cur_pose(p_(0), p_(1), p_(2), yaw_);
    list<pair<uint32_t, uint8_t>> clear_list;
    EROI_.GetReachVps(cur_pose, LRM_.node_scale_.norm(), 0.5, clear_list);
    for(auto i : clear_list){
        EROI_.RemoveVp(i.first, i.second, false);
        DTG_.EraseFnodeFromGraph(i.first, i.second);
    }
}

void SingleExp::BodyOdomCallback(const nav_msgs::OdometryConstPtr& odom){
    Quaterniond qua;
    const Eigen::Vector3d new_position(
        odom->pose.pose.position.x,
        odom->pose.pose.position.y,
        odom->pose.pose.position.z);
    if(!new_position.allFinite()){
        ROS_WARN_THROTTLE(
            1.0, "discarding non-finite odometry position");
        return;
    }
    qua.x() = odom->pose.pose.orientation.x;
    qua.y() = odom->pose.pose.orientation.y;
    qua.z() = odom->pose.pose.orientation.z;
    qua.w() = odom->pose.pose.orientation.w;
    if(!qua.coeffs().allFinite() || qua.norm() < 1.0e-6){
        ROS_WARN_THROTTLE(
            1.0, "discarding invalid odometry orientation");
        return;
    }
    qua.normalize();
    robot_pose_.setZero();
    if(have_odom_){
        len_ += (p_ - new_position).norm();
    }
    else{
        ts_ = ros::WallTime::now().toSec();
    }
    have_odom_ = true;
    p_ = new_position;
    
    robot_pose_.block(0, 0, 3, 3) = qua.toRotationMatrix();
    robot_pose_.block(0, 3, 3, 1) = p_;

    v_(0) = odom->twist.twist.linear.x;
    v_(1) = odom->twist.twist.linear.y;
    v_(2) = odom->twist.twist.linear.z;
    if(!v_.allFinite()){
        ROS_WARN_THROTTLE(
            1.0, "discarding non-finite odometry velocity");
        v_.setZero();
    }
    // if(v_.norm() > 5.0) ROS_ERROR("large v!");//debug
    v_ = qua.toRotationMatrix() * v_;
    yaw_ = atan2(qua.matrix()(1, 0), qua.matrix()(0, 0));
    yaw_v_ = odom->twist.twist.angular.z * robot_pose_(2, 2) + odom->twist.twist.angular.y * robot_pose_(2, 1)
                + odom->twist.twist.angular.x * robot_pose_(2, 0);

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
