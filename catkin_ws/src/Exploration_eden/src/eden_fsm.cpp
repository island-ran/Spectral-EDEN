#include <eden/eden_fsm.h>

void SingleExpFSM::init(ros::NodeHandle &nh, ros::NodeHandle &nh_private){
    ros::NodeHandle nh_ = nh;
    trigger_sub_ = nh_.subscribe("/start_trigger", 1, &SingleExpFSM::TriggerCallback, this);
    fsm_timer_ = nh.createTimer(ros::Duration(0.01), &SingleExpFSM::FSMCallback, this);
    S_planner_.init(nh, nh_private);
    exploring_ = false;
    start_trigger_ = false;
    state_ = M_State::SLEEP;
    t0_ = ros::WallTime::now().toSec();
    force_sample_ = false;
    const std::string ns = ros::this_node::getName();
    nh_private.param(
        ns + "/Exp/RetryBackoffInitial",
        retry_backoff_initial_, 0.10);
    nh_private.param(
        ns + "/Exp/RetryBackoffMax",
        retry_backoff_max_, 1.00);
    retry_backoff_initial_ =
        std::max(0.01, retry_backoff_initial_);
    retry_backoff_max_ =
        std::max(retry_backoff_initial_, retry_backoff_max_);
    retry_backoff_current_ = retry_backoff_initial_;
    consecutive_planning_failures_ = 0;
    nh_private.param(
        ns + "/Exp/FinishHoldInterval",
        finish_hold_interval_, 1.00);
    finish_hold_interval_ = std::max(0.05, finish_hold_interval_);
}

void SingleExpFSM::TriggerCallback(const std_msgs::EmptyConstPtr &msg){
    start_trigger_ = true;
    exploring_ = true;
}

void SingleExpFSM::FSMCallback(const ros::TimerEvent &e){
    if(S_planner_.ExplorationFinished() &&
       state_ != M_State::FINISH){
        ChangeState(M_State::FINISH);
    }
    const int ap =
        S_planner_.AllowPlan(ros::WallTime::now().toSec());
    bool exc_plan = ap != 1;

    if(force_sample_){
        force_sample_ = false;
        exc_plan = false;
        S_planner_.ForceUpdateEroiDtg();
    }
    // cout<<"ap:"<<ap<<endl;
    // cout<<"state_:"<<int(state_)<<endl;

    if(!exc_plan) {
        if(state_ == M_State::LOCALPLAN){
            ROS_DEBUG_THROTTLE(
                1.0, "local planner waiting for retry deadline");
        }
        return;
    }
    switch (state_)
    {
        case M_State::EXCUTE :{
            /* trajectory check */
            if(!S_planner_.TrajCheck()){     
                ROS_WARN("traj occ or relpan");
                ChangeState(M_State::LOCALPLAN);
                S_planner_.SetPlanInterval(0.009);
                break;
            }

            /* viewpoints check */
            int vp_st = S_planner_.ViewPointsCheck(0.005);
            if(vp_st == 1){        //local plan
                if(ap == 5) ROS_WARN("traj time out");
                if(vp_st == 1) ROS_WARN("target explored");
                ChangeState(M_State::LOCALPLAN);
                S_planner_.SetPlanInterval(0.009);
                break;
            }
            
            /* still EXCUTE */
            S_planner_.SetPlanInterval(0.05);
            break;
        }
        case M_State::FINISH :{
            S_planner_.SetPlanInterval(finish_hold_interval_);
            break;
        }
        case M_State::LOCALPLAN :{
            if(S_planner_.EdenPlan()){                 //plan success
                retry_backoff_current_ = retry_backoff_initial_;
                consecutive_planning_failures_ = 0;
                S_planner_.SetPlanInterval(0.009);
                ChangeState(M_State::EXCUTE);
                break;
            }
            else{
                ++consecutive_planning_failures_;
                if(consecutive_planning_failures_ >= 3){
                    ROS_WARN(
                        "planning failed %d consecutive times; "
                        "invalidating the committed target and "
                        "forcing a fresh map/graph sample",
                        consecutive_planning_failures_);
                    S_planner_.EscalatePlanningFailure();
                    force_sample_ = true;
                    consecutive_planning_failures_ = 0;
                }
                S_planner_.SetPlanInterval(retry_backoff_current_);
                ROS_WARN_THROTTLE(
                    1.0,
                    "local planning failed; retrying in %.2f s",
                    retry_backoff_current_);
                retry_backoff_current_ = std::min(
                    retry_backoff_max_,
                    retry_backoff_current_ * 2.0);
                break;
            }
            break;
        }
        case M_State::SLEEP :{
            if(!exploring_){
                if(start_trigger_)
                    // S_planner_.Stay(S_planner_.init_pose_);
                S_planner_.SetPlanInterval(0.009);
            }
            else{
                ChangeState(M_State::LOCALPLAN);
                S_planner_.SetPlanInterval(0.009);
            }
            break;
        }
    }
}
