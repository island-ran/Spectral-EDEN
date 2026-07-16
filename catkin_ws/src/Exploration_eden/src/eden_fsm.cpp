#include <eden/eden_fsm.h>

void SingleExpFSM::init(ros::NodeHandle &nh, ros::NodeHandle &nh_private){
    ros::NodeHandle nh_ = nh;
    trigger_sub_ = nh_.subscribe("/start_trigger", 1, &SingleExpFSM::TriggerCallback, this);
    debug_sub_ = nh_.subscribe("/debug_topic", 1, &SingleExpFSM::DebugSub, this);
    fsm_timer_ = nh.createTimer(ros::Duration(0.01), &SingleExpFSM::FSMCallback, this);
    S_planner_.init(nh, nh_private);
    const std::string ns = ros::this_node::getName();
    nh_private.param(ns + "/Exp/RetryBackoffInitial", retry_backoff_initial_, 0.10);
    nh_private.param(ns + "/Exp/RetryBackoffMax", retry_backoff_max_, 1.00);
    nh_private.param(ns + "/Exp/FinishHoldInterval", finish_hold_interval_, 1.00);
    retry_backoff_initial_ = std::max(retry_backoff_initial_, 0.01);
    retry_backoff_max_ = std::max(retry_backoff_max_, retry_backoff_initial_);
    finish_hold_interval_ = std::max(finish_hold_interval_, 0.10);
    retry_backoff_ = retry_backoff_initial_;
    exploring_ = false;
    start_trigger_ = false;
    state_ = M_State::SLEEP;
    t0_ = ros::WallTime::now().toSec();
    last_finish_hold_t_ = -std::numeric_limits<double>::infinity();
    force_sample_ = false;
}

void SingleExpFSM::TriggerCallback(const std_msgs::EmptyConstPtr &msg){
    start_trigger_ = true;
    exploring_ = true;
}

void SingleExpFSM::DebugSub(const std_msgs::Empty &msg){
    S_planner_.StopDebugFunc();
    getchar();
}

void SingleExpFSM::FSMCallback(const ros::TimerEvent &e){
    const double now = ros::WallTime::now().toSec();

    // FINISH is a hold state, not a terminal process state.  A new trigger
    // starts a fresh confirmation window and resumes exploration.
    if(state_ == M_State::FINISH){
        if(exploring_ && start_trigger_){
            start_trigger_ = false;
            retry_backoff_ = retry_backoff_initial_;
            S_planner_.ResetExplorationState();
            ChangeState(M_State::LOCALPLAN);
            S_planner_.SetPlanInterval(0.0);
            return;
        }
        if(now - last_finish_hold_t_ >= finish_hold_interval_){
            S_planner_.PublishHoldPosition();
            last_finish_hold_t_ = now;
        }
        return;
    }

    if(state_ == M_State::SLEEP){
        if(!exploring_){
            S_planner_.SetPlanInterval(0.1);
            return;
        }
        start_trigger_ = false;
        retry_backoff_ = retry_backoff_initial_;
        S_planner_.ResetExplorationState();
        ChangeState(M_State::LOCALPLAN);
        S_planner_.SetPlanInterval(0.0);
        return;
    }

    // A transition to LOCALPLAN, or a failed plan, requests exactly one
    // forced EROI/DTG refresh.  Planning then waits for the current backoff;
    // this prevents a 100 Hz retry/update loop.
    if(force_sample_){
        force_sample_ = false;
        S_planner_.ForceUpdateEroiDtg();
        S_planner_.SetPlanInterval(retry_backoff_);
        return;
    }

    const int ap = S_planner_.AllowPlan(now);
    if(ap == 1) return;  // plan interval has not elapsed
    if(ap == 2 || ap == 3){
        ROS_WARN_THROTTLE(1.0, "Planning deferred while pose/map is not ready (reason=%d)", ap);
        if(state_ != M_State::LOCALPLAN) ChangeState(M_State::LOCALPLAN);
        else force_sample_ = true;
        retry_backoff_ = std::min(retry_backoff_max_,
            std::max(retry_backoff_initial_, retry_backoff_ * 2.0));
        S_planner_.SetPlanInterval(retry_backoff_);
        return;
    }

    switch (state_)
    {
        case M_State::EXCUTE :{
            /* trajectory check */
            if(!S_planner_.TrajCheck()){     
                if(ap == 2){                 //recovery
                    S_planner_.SetPlanInterval(0.009);
                    break;
                }
                else{   //try to replan
                    ROS_WARN("traj occ or relpan");
                    ChangeState(M_State::LOCALPLAN);
                    S_planner_.SetPlanInterval(0.009);
                    break;
                }
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
            
            // if(S_planner_.FindShorterPath()){
            //     ROS_WARN("shorter path");
            //     ChangeState(M_State::LOCALPLAN);
            // }
            // if(S_planner_.AllowReplan()){
            //     ChangeState(M_State::TRAJREPLAN);
            //     S_planner_.SetPlanInterval(0.009);
            //     break;
            // }

            /* still EXCUTE */
            S_planner_.SetPlanInterval(0.05);
            break;
        }
        case M_State::FINISH :{
            // Handled before AllowPlan().
            break;
        }
        case M_State::LOCALPLAN :{
            const EdenPlanStatus plan_status = S_planner_.EdenPlan();
            if(plan_status == EdenPlanStatus::SUCCESS){
                retry_backoff_ = retry_backoff_initial_;
                S_planner_.SetPlanInterval(0.009);
                ChangeState(M_State::EXCUTE);
                break;
            }
            if(plan_status == EdenPlanStatus::FINISHED){
                exploring_ = false;
                start_trigger_ = false;
                S_planner_.PublishHoldPosition();
                last_finish_hold_t_ = now;
                S_planner_.SetPlanInterval(0.5);
                ChangeState(M_State::FINISH);
                break;
            }

            // All non-completion outcomes are recoverable.  This includes
            // spectral timeout/instability/route rejection (which normally
            // already fall back inside MR-DTG), disconnected paths, invalid
            // targets, and trajectory optimization failures.
            force_sample_ = true;
            retry_backoff_ = std::min(retry_backoff_max_,
                std::max(retry_backoff_initial_, retry_backoff_ * 2.0));
            S_planner_.SetPlanInterval(retry_backoff_);
            ROS_WARN_THROTTLE(1.0, "Exploration plan deferred (status=%d), retry in %.2fs",
                static_cast<int>(plan_status), retry_backoff_);
            break;
        }
        // case M_State::TRAJREPLAN :{
        //     S_planner_.TrajReplan();
        //     S_planner_.SetPlanInterval(0.009);
        //     ChangeState(M_State::EXCUTE);
        //     break;
        // }
        case M_State::SLEEP :{
            // Handled before AllowPlan().
            break;
        }
        case M_State::RECOVER :{
            ChangeState(M_State::LOCALPLAN);
            S_planner_.SetPlanInterval(retry_backoff_);
            break;
        }
        default:
            break;
    }
}
