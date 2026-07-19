#ifndef MR_DTG_PLUS_H_
#define MR_DTG_PLUS_H_

#include <ros/ros.h>
#include <thread>
#include <mutex>
#include <Eigen/Eigen>
#include <vector>
#include <list>
#include <visualization_msgs/MarkerArray.h>
#include <std_msgs/Float64MultiArray.h>
#include <std_msgs/String.h>
#include <swarm_exp_msgs/DtgFFEdge.h>
#include <swarm_exp_msgs/DtgHFEdge.h>
#include <swarm_exp_msgs/DtgHHEdge.h>
#include <swarm_exp_msgs/DtgHNode.h>
#include <swarm_exp_msgs/DtgFNode.h>

#include <fstream>
#include <tr1/unordered_map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <cstdint>
#include <eroi/eroi.h>

#include <mr_dtg_plus/mr_dtg_plus_structures.h>
#include <mr_dtg_plus/spectral_worker.h>
#include <mr_dtg_plus/committed_target_manager.h>
#include <block_map_lite/color_manager.h>
#include <block_map_lite/block_map_lite.h>
#include <lowres_map_lite/lowres_map_lite.h>
#include <swarm_data/swarm_data.h>

using namespace std;
namespace DTGPlus{
typedef shared_ptr<H_node> h_ptr;
typedef shared_ptr<FC_node> f_ptr;
typedef shared_ptr<DTG_edge<H_node, FC_node>> hfe_ptr;
typedef shared_ptr<DTG_edge<H_node, H_node>> hhe_ptr;
typedef shared_ptr<DTG_edge<FC_node, FC_node>> ffe_ptr;
typedef DTG_edge<H_node, FC_node> hfe;
typedef DTG_edge<H_node, H_node> hhe;
typedef DTG_edge<FC_node, FC_node> ffe;
class GraphVoronoiPartition;
class MultiDtgPlus{

public:
    MultiDtgPlus(){};
    ~MultiDtgPlus() = default;

    void AlignInit(ros::NodeHandle &nh, ros::NodeHandle &nh_private);
    void GetHnodesBBX(const Eigen::Vector3d &upbd, const Eigen::Vector3d &lowbd, list<h_ptr> &H_list);
    void GetFnodesBBX(const Eigen::Vector3d &upbd, const Eigen::Vector3d &lowbd, list<f_ptr> &F_list);
    void GetHnodesAndFnodesBBX(const Eigen::Vector3d &upbd, const Eigen::Vector3d &lowbd,
         list<h_ptr> &H_list, list<f_ptr> &F_list);


    void BfUpdate(const Eigen::Vector3d &robot_pos);
    void SetLowresMap(lowres_lite::LowResMap *LRM){LRM_ = LRM;};
    void SetBlockMap(BlockMapLite *BM){BM_ = BM;};
    void SetFrontierMap(EroiGrid *FG){EROI_ = FG;};
    void SetColorManager(ColorManager *CM){CM_ = CM;};

    void RemoveVp(const Eigen::Vector3d &center, int const &f_id, int const &v_id, bool broad_cast = false);
    void RemoveVp(uint32_t const &f_id, uint8_t const &v_id, bool broad_cast = false);

    void FindFastExpTarget( const vector<h_ptr> &route,
        const Eigen::Vector4d &ps, const Eigen::Vector4d &vs,
        const vector<Eigen::Vector3d> &path1, const double &d1, uint8_t &plan_res,
        vector<Eigen::Vector3d> &path_stem, vector<Eigen::Vector3d> &path_main,
        vector<Eigen::Vector3d> &path_sub, vector<Eigen::Vector3d> &path_norm,
        pair<uint32_t, uint8_t> &tar_stem, pair<uint32_t, uint8_t> &tar_main,
        pair<uint32_t, uint8_t> &tar_sub, pair<uint32_t, uint8_t> &tar_norm,
        double &y_stem, double &y_main, double &y_sub, double &y_norm);

    bool GetClosestGlobalTarget(list<Eigen::Vector3d> &path, list<h_ptr> &H_path, int &f_id, int &v_id, double &length);
    void GetGlobalTarget(h_ptr &s_hn, vector<pair<double, list<Eigen::Vector3d>>> &paths, vector<h_ptr> &t_hn);

    void ClearSearched(list<h_ptr> &h_l);
    void ClearSearched(list<f_ptr> &f_l);

    void RetrieveHFPath(f_ptr &tar_f, list<h_ptr> &h_path, list<Eigen::Vector3d> &path, double &length);
    void RetrieveHPath(h_ptr &start_h, h_ptr &tar_h, list<Eigen::Vector3d> &path, double &length);
    void RetrieveHPath(h_ptr &start_h, h_ptr &tar_h, vector<Eigen::Vector3d> &path, double &length);
    void RetrieveHPathDebug(h_ptr &start_h, h_ptr &tar_h, list<Eigen::Vector3d> &path, double &length);
    bool RetrieveHnPath(h_ptr &tar_h, list<h_ptr> &path);
    void RetrieveHDebug(h_ptr &start_h, h_ptr &tar_h, list<h_ptr> &h_path);

    bool TspApproxiPlan(const Eigen::Vector3d &ps, vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1);

    // ── Dual-mode entry points ──
    GlobalPlanStatus PlanGlobalRoute(const Eigen::Vector3d &ps,
        vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1);

    // EDEN baseline: frozen original EOHDT planning
    GlobalPlanStatus PlanGlobalRouteEden(const Eigen::Vector3d &ps,
        vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1);

    // Spectral-EDEN V4: committed single-target planning
    GlobalPlanStatus PlanGlobalRouteV4(const Eigen::Vector3d &ps,
        vector<h_ptr> &route_h, vector<Eigen::Vector3d> &path2fh, double &d1);

    inline const GlobalPlanDiagnostics &diagnostics() const { return global_plan_diagnostics_; }
    inline uint64_t dtg_version() const { return dtg_version_; }
    inline uint64_t frontier_version() const { return frontier_version_; }
    inline GlobalPlannerMode planner_mode() const { return global_planner_mode_; }

    inline void EraseFnodeFromGraph(const uint32_t &fid, const uint8_t &vid, bool erase_edge = true);
    inline void EraseEROIFromGraph(const uint32_t &fid);

    inline bool FindFnode(const Eigen::Vector3d &vp_pos, const uint32_t &id, const uint8_t &vid, f_ptr &fp);
    inline bool FindHnode(const Eigen::Vector3d &pos, const uint32_t &id, h_ptr &hp);
    inline bool FindHnode(const uint32_t &idx, const uint32_t &id, h_ptr &hp);

    bool Astar(const Eigen::Vector3d &ps, const vector<h_ptr> &p_hs, const vector<double> &g0_hs, h_ptr &tars,
        const vector<vector<Eigen::Vector3d>> &paths, vector<Eigen::Vector3d> &path, double &dist);

    // ── V4: GraphDelta submission and region state consumption ──
    GraphDelta BuildGraphDelta(const Eigen::Vector3d &robot_pos);
    bool ValidateRegionSnapshot(const RegionStateSnapshot &snapshot) const;
    void CommitTargetAfterPublish(const CommittedTarget &target);
    void RejectPendingTargetAfterPlanningFailure(bool hard_invalidation);
    bool CurrentMacroTargetIsRegionEntry() const;
    void NotifyFrontierGainChanges(
        const vector<uint32_t> &frontier_ids);

private:
    friend class GraphVoronoiPartition;

    struct HPathCacheEntry{
        double distance = 999999.0;
        list<h_ptr> h_path;
        uint64_t dtg_version = 0;
    };

    struct GlobalRouteContext{
        vector<h_ptr> seed_hnodes;
        vector<double> seed_distances;
        vector<vector<Eigen::Vector3d>> seed_paths;
        vector<h_ptr> active_hnodes;
        vector<pair<double, list<h_ptr>>> root_paths;
        Eigen::MatrixXd distance_matrix;
        std::unordered_map<uint32_t, size_t> active_index;
        bool pairwise_connected = true;
    };

    // ── EDEN baseline helpers (kept for PlanGlobalRouteEden) ──
    GlobalPlanStatus CollectActiveBoundaryRegions(const Eigen::Vector3d &ps,
        GlobalRouteContext &context, bool populate_missing_pair_distances);
    GlobalPlanStatus BuildActiveDistanceMatrix(GlobalRouteContext &context,
        bool populate_missing_pair_distances);
    GlobalPlanStatus BuildOriginalEohdtRoute(const Eigen::Vector3d &ps,
        GlobalRouteContext &context, vector<h_ptr> &route_h,
        vector<Eigen::Vector3d> &path2fh, double &d1);
    GlobalPlanStatus BuildPathToFirst(const Eigen::Vector3d &ps,
        const GlobalRouteContext &context, const h_ptr &first_h,
        vector<Eigen::Vector3d> &path2fh, double &d1);
    bool HasAnyActiveBoundary() const;
    bool IsActiveBoundary(const h_ptr &h) const;
    static uint64_t HPairKey(const uint32_t h1, const uint32_t h2);
    void MarkTopologyChanged();
    void MarkFrontierChanged();
    void RecordV4NodeUpsert(const h_ptr &node);
    void RecordV4EdgeUpsert(const hhe_ptr &edge);
    void RecordV4EdgeErase(uint32_t from, uint32_t to);
    void RecordV4FrontierAnchor(const h_ptr &node);
    FrontierAnchorUpdate BuildV4FrontierUpdate(const h_ptr &node) const;
    bool IsV4FrontierValid(uint32_t frontier_id, uint8_t viewpoint_id) const;
    h_ptr FindHnodeById(uint32_t h_id) const;
    double EstimatePathClearance(
        const list<Eigen::Vector3d> &path) const;
    double EdgeClearance(const hhe_ptr &edge);
    bool HasEffectiveFrontier(
        const h_ptr &h, int region_id = -1) const;
    void PublishGlobalPlanDiagnostics();

    inline bool InsideMap(const Eigen::Vector3i &idx3);
    inline bool InsideMap(const Eigen::Vector3d &pos);
    inline bool GetVox(const Eigen::Vector3i &idx3, list<h_ptr> &h_l, list<f_ptr> &f_l);
    inline bool GetVox(const Eigen::Vector3d &pos, list<h_ptr> &h_l, list<f_ptr> &f_l);
    inline bool GetVox(const Eigen::Vector3d &pos, const uint32_t &idx, h_ptr &h_l);

    inline Eigen::Vector3i GetVoxId3(const Eigen::Vector3d &pos);
    inline int GetVoxId(const Eigen::Vector3i &idx3);
    inline int GetVoxId(const Eigen::Vector3d &pos);

    inline void BreakHFEdge(h_ptr &h, f_ptr &f);

    inline bool GetEdge(h_ptr &h, f_ptr &f, hfe_ptr &e, bool search_f = true);
    inline bool GetEdge(h_ptr &h1, h_ptr &h2, hhe_ptr &e);

    inline h_ptr CreateSwarmHnode(const Eigen::Vector3d &pos, const uint32_t &id);
    inline h_ptr CreateHnode(const Eigen::Vector3d &pos);
    inline bool CreateConnectFnode(const Eigen::Vector3d &vp_pos, const uint32_t &f_id, const uint8_t &vid, const double &el, h_ptr &hn);

    inline void BlockEdge(hhe_ptr &e);
    inline void BlockEdge(hfe_ptr &e);

    inline void EraseEdge(hhe_ptr &e, const bool &broadcast = true);
    inline void EraseFnodeOnly(const uint32_t &fid, const uint8_t &vid);
    inline void EraseEdge(hfe_ptr &e, const bool &broadcast = true);
    inline void ConnectHF(h_ptr &h, f_ptr &f, const int &v_id,  list<Eigen::Vector3d> &path, list<Eigen::Vector3d> &path_old, double length);
    inline void ConnectHH(h_ptr &head, h_ptr &tail, list<Eigen::Vector3d> &path, list<Eigen::Vector3d> &path_old, double length);
    inline bool ConnectSwarmHH(h_ptr &head, h_ptr &tail, list<Eigen::Vector3d> &path);
    inline bool ConnectSwarmHF(h_ptr &head, f_ptr &tail, list<Eigen::Vector3d> &path, uint8_t &vp_id);
    inline bool CheckNode(const Eigen::Vector3d &pos, const h_ptr &h, const f_ptr &f);

    void NodesDebug();
    void ShowAll(const ros::TimerEvent &e);
    void DistMaintTimerCallback(const ros::TimerEvent &e);
    void Show();
    void Debug();
    void Debug(list<Eigen::Vector3d>  &fl, int idx = 0);
    void Debug(vector<Eigen::Vector3d>  &fl, int idx = 0);
    void DebugTree(Eigen::Vector3d ps, vector<h_ptr> &hs, vector<vector<int>> &branches);
    void DebugLineStrip(Eigen::Vector3d ps, vector<h_ptr> &route_h);
    void MainTainDistMap(h_ptr &hs, vector<h_ptr> &tars);
    void ParallelDijkstra(vector<h_ptr> &p_hs, vector<double> &g0_hs, vector<h_ptr> &tars, vector<pair<double, list<h_ptr>>> &paths);
    void Prim(Eigen::MatrixXd &dist_mat, vector<int> &parent, vector<vector<int>> &branches);

    void ReFineTargetVp(const Eigen::Vector4d &ps, const Eigen::Vector4d &vs,
        const Eigen::Vector4d &vp2, const uint32_t &eroi_id, const uint8_t &vp_id,
        const vector<Eigen::Vector3d> &path2v2, const double &motion_gain, const double &vm, const double &arc,
        Eigen::Vector4d &refined_vp1, vector<Eigen::Vector3d> &path2v1_r, vector<Eigen::Vector3d> &path2v2_r);
    void ReFineTargetVpSample(const Eigen::Vector4d &ps, const Eigen::Vector4d &vs,
        const Eigen::Vector3d &r_vp_pos, const Eigen::Vector4d &vp2, const uint32_t &eroi_id,
        const double &vm, const double &arc, const double &gain_0, const double &motion_gain,
                                Eigen::Vector4d &r_vp, double &better_gain);

    inline double GetGainExp1(const Eigen::Vector4d &ps, const Eigen::Vector4d &vp, const Eigen::Vector3d &vs, const double &dist);
    inline double GetGainExp2(const Eigen::Vector4d &ps, const Eigen::Vector4d &vp1, const Eigen::Vector3d &vs, const double &dist,
                            const Eigen::Vector3d &exp_center, const double &arc, const double &vm, const double &y2);

    inline void StopageDebug(string c);

    ros::NodeHandle nh_, nh_private_;
    ros::Publisher topo_pub_, debug_pub_, spectral_diag_pub_, spectral_event_pub_;
    uint32_t cur_hid_;
    double H_thresh_, sensor_range_, eurange_;
    double lambda_e_, lambda_a_;
    double v_max_, a_max_, yv_max_;
    int uav_id_;
    bool show_e_details_;

    vector<list<h_ptr>> H_depot_;
    vector<list<f_ptr>> F_depot_;
    list<h_ptr> H_list_;
    list<Eigen::Vector3d> debug_pts_;
    h_ptr root_;
    Eigen::Vector3d origin_, vox_scl_, map_upbd_, map_lowbd_;
    Eigen::Vector3i vox_num_;

    prio_A open_A_;
    prio_D open_D_;

    ros::Timer show_timer_, maintain_timer_;

    EroiGrid *EROI_;
    lowres_lite::LowResMap *LRM_;
    BlockMapLite *BM_;
    ColorManager *CM_;

    random_device rd_;
    default_random_engine eng_;
    bool maintain_h_dist_;
    std::unordered_map<uint64_t, HPathCacheEntry> h_dist_map_;
    list<h_ptr> update_list_;

    // Versioning
    uint64_t dtg_version_;
    uint64_t frontier_version_;

    // ── Dual-mode state ──
    GlobalPlannerMode global_planner_mode_ = GlobalPlannerMode::EDEN_BASELINE;
    GlobalPlanDiagnostics global_plan_diagnostics_;

    // ── V4 state ──
    SpectralV4Config spectral_v4_config_;
    std::unique_ptr<SpectralWorker> spectral_worker_;
    std::shared_ptr<const RegionStateSnapshot> region_snapshot_;
    CommittedTargetManager target_manager_;
    CommittedTarget committed_target_;
    CommittedTarget pending_target_;
    bool v4_bootstrap_pending_ = true;
    uint32_t last_robot_h_id_ = 0;
    std::mutex v4_delta_mutex_;
    std::unordered_map<uint32_t, NodeInsert> pending_node_upserts_;
    std::unordered_set<uint32_t> pending_node_erases_;
    std::unordered_map<uint64_t, EdgeUpsert> pending_edge_upserts_;
    std::unordered_set<uint64_t> pending_edge_erases_;
    std::unordered_map<uint32_t, FrontierAnchorUpdate>
        pending_frontier_updates_;
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>>
        frontier_anchor_index_;

    double acc_gain_, yaw_gain_, yaw_slice_ang_, g_thr_fac_;
    int refine_num_;

    ros::Timer swarm_timer_;
    bool use_swarm_;
    int drone_num_;

    bool debug_plan_;
    ofstream debug_f_;

};

// ── Inline implementations (unchanged from original) ──

inline void MultiDtgPlus::EraseFnodeFromGraph(const uint32_t &fid, const uint8_t &vid, bool erase_edge){
    if(fid < EROI_->EROI_.size()){
        Eigen::Vector3d vp_pos = EROI_->vps_[vid].head(3);
        vp_pos += EROI_->EROI_[fid].center_;
        int idx = GetVoxId(vp_pos);
        if(idx == -1) return;
        for(auto fit = F_depot_[idx].begin(); fit != F_depot_[idx].end(); fit++){
            if((*fit)->vid_ == vid && fid == (*fit)->fid_){
                if(erase_edge) {
                    EraseEdge((*fit)->hf_edge_);
                }
                F_depot_[idx].erase(fit);
                break;
            }
        }
        EROI_->DeleteValidFnode(fid, vid);
    }
}

inline void MultiDtgPlus::EraseEROIFromGraph(const uint32_t &fid){
    if(fid < EROI_->EROI_.size()){
        int c = 0;
        for(auto &i : EROI_->EROI_[fid].valid_vps_){
            if(fid < EROI_->EROI_.size()){
                Eigen::Vector3d vp_pos = EROI_->vps_[i].head(3);
                vp_pos += EROI_->EROI_[fid].center_;
                int idx = GetVoxId(vp_pos);
                if(idx == -1) return;
                for(auto fit = F_depot_[idx].begin(); fit != F_depot_[idx].end(); fit++){
                    if((*fit)->vid_ == i && fid == (*fit)->fid_){
                        EraseEdge((*fit)->hf_edge_);
                        F_depot_[idx].erase(fit);
                        break;
                    }
                }
            }
        }
        EROI_->EROI_[fid].valid_vps_.clear();
    }
}

inline bool MultiDtgPlus::InsideMap(const Eigen::Vector3i &idx3){
    if(idx3(0) < 0 || idx3(1) < 0 || idx3(2) < 0 ||
        idx3(0) >  vox_num_(0) - 1 || idx3(1) > vox_num_(1) - 1 || idx3(2) > vox_num_(2) - 1 )
        return false;
    return true;
}

inline bool MultiDtgPlus::InsideMap(const Eigen::Vector3d &pos){
    if(pos(0) < map_lowbd_(0)|| pos(1) < map_lowbd_(1)|| pos(2) < map_lowbd_(2)||
        pos(0) >  map_upbd_(0) || pos(1) > map_upbd_(1) || pos(2) > map_upbd_(2) )
        return false;
    return true;
}

inline bool MultiDtgPlus::GetVox(const Eigen::Vector3i &idx3, list<h_ptr> &h_l, list<f_ptr> &f_l){
    int idx = GetVoxId(idx3);
    if(idx != -1){
        h_l = H_depot_[idx];
        f_l = F_depot_[idx];
        return true;
    }
    return false;
}

inline bool MultiDtgPlus::GetVox(const Eigen::Vector3d &pos, list<h_ptr> &h_l, list<f_ptr> &f_l){
    int idx = GetVoxId(pos);
    if(idx != -1){
        h_l = H_depot_[idx];
        f_l = F_depot_[idx];
        return true;
    }
    return false;
}

inline bool MultiDtgPlus::GetVox(const Eigen::Vector3d &pos, const uint32_t &idx, h_ptr &h_l){
    int vox_idx = GetVoxId(pos);
    if(vox_idx != -1){
        for(auto &h : H_depot_[vox_idx]){
            if(h->id_ == idx){
                h_l = h;
                return true;
            }
        }
    }
    return false;
}

inline Eigen::Vector3i MultiDtgPlus::GetVoxId3(const Eigen::Vector3d &pos){
    return Eigen::Vector3i((int)floor((pos(0) - origin_(0))/vox_scl_(0)), (int)floor((pos(1) - origin_(1))/vox_scl_(1)),
         (int)floor((pos(2) - origin_(2))/vox_scl_(2)));
}

inline int MultiDtgPlus::GetVoxId(const Eigen::Vector3i &idx3){
    if(InsideMap(idx3)){
        return idx3(0) + idx3(1) * vox_num_(0) + idx3(2) * vox_num_(0) * vox_num_(1);
    }
    else return -1;
}

inline int MultiDtgPlus::GetVoxId(const Eigen::Vector3d &pos){
    if(InsideMap(pos)){
        int x = floor((pos(0) - origin_(0)) / vox_scl_(0));
        int y = floor((pos(1) - origin_(1)) / vox_scl_(1));
        int z = floor((pos(2) - origin_(2)) / vox_scl_(2));
        return x + y * vox_num_(0) + z * vox_num_(0) * vox_num_(1);
    }
    else return -1;
}

inline void MultiDtgPlus::BreakHFEdge(h_ptr &h, f_ptr &f){
    for(list<shared_ptr<DTG_edge<H_node, FC_node>>>::iterator e = h->hf_edges_.begin(); e != h->hf_edges_.end(); e++){
        if((*e)->tail_n_ == f){
            h->hf_edges_.erase(e);
            return;
        }
    }
}

inline bool MultiDtgPlus::GetEdge(h_ptr &h, f_ptr &f, hfe_ptr &e, bool search_f){
    if(search_f){
        if(f->hf_edge_ != NULL && h == f->hf_edge_->head_n_){
            e = f->hf_edge_;
            return true;
        }
    }
    else{
        if(h != NULL){
            for(auto &hfe : h->hf_edges_){
                if(hfe->tail_n_->fid_ == f->fid_){
                    e = hfe;
                    return true;
                }
            }
        }
    }
    return false;
}

inline bool MultiDtgPlus::GetEdge(h_ptr &h1, h_ptr &h2, hhe_ptr &e){
    for(auto &ei : h1->hh_edges_){
        if(ei->tail_n_ == h2 || ei->head_n_ == h2){
            e = ei;
            return true;
        }
    }
    return false;
}

inline bool MultiDtgPlus::FindFnode(const Eigen::Vector3d &vp_pos, const uint32_t &fid, const uint8_t &vid, f_ptr &fp){
    int idx = GetVoxId(vp_pos);
    if(idx >= F_depot_.size() || idx < 0) return false;
    for(auto &c : F_depot_[idx]){
        if(fid == c->fid_ && vid == c->vid_){
            fp = c;
            return true;
        }
    }
    return false;
}

inline bool MultiDtgPlus::FindHnode(const Eigen::Vector3d &pos, const uint32_t &id, h_ptr &hp){
    int idx = GetVoxId(pos);
    if(idx == -1) return false;
    for(auto &h_l : H_depot_){
        for(auto &h : h_l){
            if(h->id_ == id){
                hp = h;
                return true;
            }
        }
    }
    return false;
}

inline bool MultiDtgPlus::FindHnode(const uint32_t &idx, const uint32_t &id, h_ptr &hp){
    if(idx < 0 || idx >= H_depot_.size()) return false;
    for(auto &h : H_depot_[idx]){
        if(h->id_ == id){
            hp = h;
            return true;
        }
    }
    return false;
}

inline h_ptr MultiDtgPlus::CreateHnode(const Eigen::Vector3d &pos){
    h_ptr hnode = make_shared<H_node>();
    hnode->pos_ = LRM_->GetStdPos(pos);
    if(!LRM_->IsFeasible(pos)) return NULL;
    std::cout << "\033[0;42m try create:"<<hnode->pos_.transpose()<<"  ;"<<pos.transpose()<<" \033[0m"<< std::endl;
    int idx = GetVoxId(hnode->pos_ );
    if(idx == -1) return NULL;
    hnode->id_ = cur_hid_;
    cur_hid_ += drone_num_;
    std::cout << "\033[0;42m new id:"<<int(hnode->id_)<<"  "<<idx<<" \033[0m"<< std::endl;
    H_depot_[idx].emplace_back(hnode);
    H_list_.emplace_back(hnode);
    hnode->h_flags_ |= 6;
    if(maintain_h_dist_) hnode->last_maintain_t_ = ros::WallTime::now().toSec() - 999999;
    MarkTopologyChanged();
    RecordV4NodeUpsert(hnode);
    return hnode;
}

inline bool MultiDtgPlus::CreateConnectFnode(const Eigen::Vector3d &vp_pos, const uint32_t &f_id, const uint8_t &vid, const double &el, h_ptr &hn){
    double l;
    list<Eigen::Vector3d> path;
    hfe_ptr edge;
    bool change_vp = false;
    int idx_old, idx;
    for(auto &hfe : hn->hf_edges_){
        if(hfe->tail_n_->fid_ == f_id){
            if(el >= hfe->length_) return false;
            edge = hfe;
            change_vp = true;
            Eigen::Vector3d vp_old = EROI_->EROI_[hfe->tail_n_->fid_].center_ + EROI_->vps_[hfe->tail_n_->vid_].head(3);
            idx_old = GetVoxId(vp_old);
            break;
        }
    }
    if(LRM_->RetrieveHPath(vp_pos, path, l, true)){
        idx = GetVoxId(vp_pos);
        if(change_vp){
            for(auto fi = F_depot_[idx_old].begin(); fi != F_depot_[idx_old].end(); fi++){
                if((*fi)->fid_ == f_id && (*fi)->hf_edge_->head_n_ == hn){
                    if(vid != (*fi)->vid_){
                        EROI_->DeleteValidFnode(f_id, (*fi)->vid_);
                        EROI_->AddValidFnode(f_id, vid);
                        (*fi)->vid_ = vid;
                        F_depot_[idx].emplace_back((*fi));
                        F_depot_[idx_old].erase(fi);
                    }
                    edge->path_ = path;
                    edge->length_ = el;
                    MarkFrontierChanged();
                    RecordV4FrontierAnchor(hn);
                    return true;
                }
            }
            for(auto &hfe : hn->hf_edges_){
                if(hfe->tail_n_->fid_ == f_id){
                    edge = hfe;
                    change_vp = true;
                    Eigen::Vector3d vp_old = EROI_->EROI_[hfe->tail_n_->fid_].center_ + EROI_->vps_[hfe->tail_n_->vid_].head(3);
                    idx_old = GetVoxId(vp_old);
                    break;
                }
            }
            for(auto fi = F_depot_[idx_old].begin(); fi != F_depot_[idx_old].end(); fi++){
                cout<<"fi:"<<(*fi)->fid_<<"  "<<int((*fi)->vid_)<<endl;
            }
            StopageDebug("CreateConnectFnode not find fi");
            return false;
        }
        else{
            EROI_->AddValidFnode(f_id, vid);
            f_ptr f = make_shared<FC_node>();
            f->fid_ = f_id;
            f->vid_ = vid;
            F_depot_[idx].emplace_back(f);
            edge = make_shared<hfe>();
            edge->head_n_ = hn;
            edge->tail_n_ = f;
            edge->path_ = path;
            edge->length_ = el;
            edge->e_flag_ = 4;
            f->hf_edge_ = edge;
            if(hn->hf_edges_.empty()) hn->last_maintain_t_ = ros::WallTime::now().toSec() - 999999;
            hn->hf_edges_.emplace_back(edge);
        }
        MarkFrontierChanged();
        RecordV4FrontierAnchor(hn);
        return true;
    }
    else{
        ROS_WARN("CreateConnectFnode RetrieveHPath failed");
        return false;
    }
}

inline void MultiDtgPlus::BlockEdge(hhe_ptr &e){
    if(e == NULL) return;
    if(e->length_ >= 2e5) return;
    e->length_ = 2e5;
    e->e_flag_ &= 239;
    MarkTopologyChanged();
    if(e->head_n_ != nullptr && e->tail_n_ != nullptr)
        RecordV4EdgeErase(e->head_n_->id_, e->tail_n_->id_);
}

inline void MultiDtgPlus::BlockEdge(hfe_ptr &e){
    if(e == NULL) return;
    if(e->length_ >= 2e5) return;
    e->length_ = 2e5;
    e->e_flag_ &= 239;
    f_ptr f;
    f = e->tail_n_;
    MarkFrontierChanged();
    RecordV4FrontierAnchor(e->head_n_);
}

inline void MultiDtgPlus::EraseEdge(hhe_ptr &e, const bool &broadcast){
    if(e == NULL) return;
    const uint32_t from = e->head_n_ == nullptr ? 0U : e->head_n_->id_;
    const uint32_t to = e->tail_n_ == nullptr ? 0U : e->tail_n_->id_;
    list<hhe_ptr>::iterator e_it;
    h_ptr h;
    bool removed = false;
    h = e->head_n_;
    for(e_it = h->hh_edges_.begin(); e_it != h->hh_edges_.end(); e_it++){
        if((*e_it) == e) {
            h->hh_edges_.erase(e_it);
            removed = true;
            break;
        }
    }
    h = e->tail_n_;
    for(e_it = h->hh_edges_.begin(); e_it != h->hh_edges_.end(); e_it++){
        if((*e_it) == e) {
            h->hh_edges_.erase(e_it);
            removed = true;
            break;
        }
    }
    if(removed) {
        MarkTopologyChanged();
        RecordV4EdgeErase(from, to);
    }
}

inline void MultiDtgPlus::EraseEdge(hfe_ptr &e, const bool &broadcast){
    if(e == NULL) return;
    list<hfe_ptr>::iterator e_it;
    h_ptr h;
    f_ptr f;
    h = e->head_n_;
    f = e->tail_n_;
    bool removed = false;
    for(e_it = h->hf_edges_.begin(); e_it != h->hf_edges_.end(); e_it++){
        if((*e_it) == e) {
            h->hf_edges_.erase(e_it);
            removed = true;
            break;
        }
    }
    f->hf_edge_ = NULL;
    if(removed) {
        MarkFrontierChanged();
        RecordV4FrontierAnchor(h);
    }
}

inline void MultiDtgPlus::ConnectHF(h_ptr &h, f_ptr &f, const int &v_id, list<Eigen::Vector3d> &path, list<Eigen::Vector3d> &path_old, double length){
    hfe_ptr e_ptr;
    h_ptr changed_old_head;
    path_old.clear();
    if(!GetEdge(h, f, e_ptr, true)) {
        h_ptr h_old = f->hf_edge_->head_n_;
        changed_old_head = h_old;
        if(GetEdge(h, f, e_ptr, false)){
            if(e_ptr->length_ < length + 1e-3) return;
            EraseFnodeFromGraph(e_ptr->tail_n_->fid_, e_ptr->tail_n_->vid_, false);
            e_ptr->tail_n_ = f;
            f->hf_edge_ = e_ptr;
        }
        else{
            if(length + 1e-3 > f->hf_edge_->length_) return;
            if(h->hf_edges_.empty()) h->last_maintain_t_ = ros::WallTime::now().toSec() - 999999;
            h->hf_edges_.emplace_back(f->hf_edge_);
        }
        for(list<hfe_ptr>::iterator e_it = h_old->hf_edges_.begin(); e_it != h_old->hf_edges_.end(); e_it++){
            if((*e_it)->tail_n_ == f){
                h_old->hf_edges_.erase(e_it);
                break;
            }
        }
        f->hf_edge_->head_n_ = h;
    }
    else{
        if(length + 1e-3 > e_ptr->length_) return;
        if(e_ptr->e_flag_ & 4){
            path_old = e_ptr->path_;
        }
    }
    f->hf_edge_->length_ = length;
    f->hf_edge_->path_ = path;
    MarkFrontierChanged();
    RecordV4FrontierAnchor(h);
    if(changed_old_head != nullptr && changed_old_head != h)
        RecordV4FrontierAnchor(changed_old_head);
}

inline void MultiDtgPlus::ConnectHH(h_ptr &head, h_ptr &tail, list<Eigen::Vector3d> &path, list<Eigen::Vector3d> &path_old, double length){
    hhe_ptr e_ptr;
    bool push_edge = false;
    path_old.clear();
    if(!GetEdge(head, tail, e_ptr)){
        e_ptr = make_shared<hhe>();
        push_edge = true;
    }
    else{
        if(e_ptr->e_flag_ & 4){
            path_old = e_ptr->path_;
        }
    }
    if(length >= e_ptr->length_) return;
    e_ptr->head_n_ = head;
    e_ptr->tail_n_ = tail;
    e_ptr->length_ = length;
    e_ptr->path_ = path;
    e_ptr->clearance_ = std::numeric_limits<double>::quiet_NaN();
    e_ptr->e_flag_ |= 16;
    if(push_edge){
        head->hh_edges_.emplace_back(e_ptr);
        tail->hh_edges_.emplace_back(e_ptr);
    }
    if(e_ptr->length_ < e_ptr->length_s_){
        e_ptr->e_flag_ |= 32;
        e_ptr->length_s_ = e_ptr->length_;
    }
    MarkTopologyChanged();
    RecordV4EdgeUpsert(e_ptr);
}

inline bool MultiDtgPlus::ConnectSwarmHH(h_ptr &head, h_ptr &tail, list<Eigen::Vector3d> &path){
    return false;
}

inline bool MultiDtgPlus::ConnectSwarmHF(h_ptr &head, f_ptr &tail, list<Eigen::Vector3d> &path, uint8_t &vp_id){
    return false;
}

inline bool MultiDtgPlus::CheckNode(const Eigen::Vector3d &pos, const h_ptr &h, const f_ptr &f){
    bool have_h = false;
    bool have_f = false;
    list<h_ptr> h_list;
    list<f_ptr> f_list;
    GetVox(pos, h_list, f_list);
    if(h != NULL){
        for(auto &h_it: h_list){
            if(h == h_it){
                have_h = true;
                break;
            }
        }
    }
    else have_h = true;
    if(f != NULL){
        for(auto &f_it: f_list){
            if(f == f_it){
                have_f = true;
                break;
            }
        }
    }
    else have_f = true;
    return (have_f && have_h);
}

inline double MultiDtgPlus::GetGainExp1(const Eigen::Vector4d &ps, const Eigen::Vector4d &vp, const Eigen::Vector3d &vs, const double &dist){
    Eigen::Vector3d dp = (vp.head(3) - ps.head(3)).normalized();
    Eigen::Vector3d dv = dp * v_max_ - vs.head(3);
    double a_cost = 0.0;
    double det_theta;
    double v1_max;
    if(vs.norm() < 0.01){
        a_cost = 0;
        v1_max = v_max_;
    }
    else{
        det_theta = acos(dp.dot(vs.normalized()))*2;
        double r = (vp.head(3) - ps.head(3)).norm() / sin(det_theta*0.5);
        double omiga = sqrt(a_max_ * lambda_a_ / r);
        v1_max = min(r * omiga, v_max_);
        a_cost = det_theta / omiga * acc_gain_;
        if(vs.head(3).norm() > v1_max){
        }
        else{
        }
    }
    double t = max(dist / v1_max, abs(EROI_->YawDiff(vp(3), ps(3))) / yv_max_ * yaw_gain_) + a_cost;
    return exp(-lambda_e_ * t);
}

inline double MultiDtgPlus::GetGainExp2(const Eigen::Vector4d &ps, const Eigen::Vector4d &vp1,
    const Eigen::Vector3d &vs, const double &dist,
    const Eigen::Vector3d &vp2, const double &arc, const double &vm, const double &y2){
    Eigen::Vector3d dp = (vp1.head(3) - ps.head(3)).normalized();
    Eigen::Vector3d dv = dp * v_max_ - vs.head(3);
    double a_cost = 0.0;
    double det_theta, det_theta2;
    double v1_max;
    if(vs.norm() < 0.01){
        a_cost = 0;
        v1_max = v_max_;
    }
    else{
        det_theta = acos(dp.dot(vs.normalized()))*2;
        double r = (vp1.head(3) - ps.head(3)).norm() / sin(det_theta*0.5);
        double omiga = sqrt(a_max_ * lambda_a_ / r);
        v1_max = min(r * omiga, v_max_);
        a_cost = det_theta / omiga * acc_gain_;
        if(vs.head(3).norm() > v1_max){
        }
        else{
        }
    }
    double t = max(dist / v1_max, abs(EROI_->YawDiff(vp1(3), ps(3))) / yv_max_ * yaw_gain_) + a_cost;
    double gain = exp(-lambda_e_ * t);
    dv = (vp2 - vp1.head(3)).normalized()*v_max_ - (vp1.head(3) - ps.head(3)).normalized()*v1_max;
    t += max(arc / v1_max , abs(EROI_->YawDiff(y2, vp1(3))) / yv_max_ * yaw_gain_);
    gain += exp(-lambda_e_ * t);
    return gain;
}

inline void MultiDtgPlus::StopageDebug(string c){
    ROS_ERROR_STREAM("[MR_DTG invariant] " << c);
}

}
#endif
