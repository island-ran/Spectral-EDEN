#include <mr_dtg_plus/mr_dtg_plus.h>

using namespace DTGPlus;

namespace {
constexpr double kUnreachableDistance = 999998.0;
constexpr double kBlockedEdgeDistance = 200000.0;
}

GlobalPlanStatus MultiDtgPlus::PlanGlobalRouteEden(
    const Eigen::Vector3d &ps,
    vector<h_ptr> &route_h,
    vector<Eigen::Vector3d> &path2fh,
    double &d1)
{
    // ── 1. Initialize diagnostics ──
    global_plan_diagnostics_ = GlobalPlanDiagnostics();
    global_plan_diagnostics_.dtg_version = dtg_version_;
    global_plan_diagnostics_.frontier_version = frontier_version_;
    global_plan_diagnostics_.planner_mode = static_cast<int>(GlobalPlannerMode::EDEN_BASELINE);
    route_h.clear();
    path2fh.clear();
    d1 = 0.0;

    const double t_start = ros::WallTime::now().toSec();

    // ── 2. Collect all active anchor H-nodes (boundary regions) ──
    GlobalRouteContext context;
    GlobalPlanStatus status = CollectActiveBoundaryRegions(ps, context, true);

    // ── 3. Handle collection errors ──
    if (status != GlobalPlanStatus::SUCCESS) {
        global_plan_diagnostics_.status = status;
        global_plan_diagnostics_.method = GlobalPlanMethod::NONE;
        global_plan_diagnostics_.active_anchor_count = context.active_hnodes.size();

        switch (status) {
        case GlobalPlanStatus::NO_ACTIVE_FRONTIER:
            global_plan_diagnostics_.detail = "EDEN baseline: no active frontier regions";
            break;
        case GlobalPlanStatus::NO_FEASIBLE_SEED:
            global_plan_diagnostics_.detail = "EDEN baseline: no feasible seed from robot position";
            break;
        case GlobalPlanStatus::GRAPH_DISCONNECTED:
            global_plan_diagnostics_.detail = "EDEN baseline: graph disconnected, cannot reach active regions";
            break;
        default:
            global_plan_diagnostics_.detail = "EDEN baseline: region collection failed";
            break;
        }

        PublishGlobalPlanDiagnostics();
        return status;
    }

    global_plan_diagnostics_.active_anchor_count = context.active_hnodes.size();

    // ── 4. Run the isolated EDEN EOHDT baseline ──
    //     BuildActiveDistanceMatrix was already called inside
    //     CollectActiveBoundaryRegions, so the distance matrix is populated.
    status = BuildOriginalEohdtRoute(
        ps, context, route_h, path2fh, d1);

    // ── 5. Handle EOHDT errors ──
    if (status != GlobalPlanStatus::SUCCESS) {
        global_plan_diagnostics_.status = status;
        global_plan_diagnostics_.method = GlobalPlanMethod::NONE;
        global_plan_diagnostics_.route_size = route_h.size();

        switch (status) {
        case GlobalPlanStatus::NO_ACTIVE_FRONTIER:
            global_plan_diagnostics_.detail = "EDEN baseline: no active frontier in EOHDT";
            break;
        case GlobalPlanStatus::GRAPH_DISCONNECTED:
            global_plan_diagnostics_.detail = "EDEN baseline: graph disconnected during MST construction";
            break;
        case GlobalPlanStatus::PATH_FAILED:
            global_plan_diagnostics_.detail = "EDEN baseline: failed to build path to first frontier node";
            break;
        case GlobalPlanStatus::INTERNAL_ERROR:
            global_plan_diagnostics_.detail = "EDEN baseline: internal error in EOHDT tour construction";
            break;
        default:
            global_plan_diagnostics_.detail = "EDEN baseline: EOHDT routing failed";
            break;
        }

        PublishGlobalPlanDiagnostics();
        return status;
    }

    // ── 6. Success ──
    const double plan_time_ms = (ros::WallTime::now().toSec() - t_start) * 1000.0;

    global_plan_diagnostics_.status = GlobalPlanStatus::SUCCESS;
    global_plan_diagnostics_.method = GlobalPlanMethod::EOHDT_BASELINE;
    global_plan_diagnostics_.route_size = route_h.size();
    global_plan_diagnostics_.selected_path_cost = d1;
    global_plan_diagnostics_.detail = std::string("EDEN baseline: route with ")
        + std::to_string(route_h.size()) + " nodes planned in "
        + std::to_string(static_cast<int>(plan_time_ms)) + " ms";

    PublishGlobalPlanDiagnostics();
    return GlobalPlanStatus::SUCCESS;
}
