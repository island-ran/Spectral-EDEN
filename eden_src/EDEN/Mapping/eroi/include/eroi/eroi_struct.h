#ifndef EROI_STRUCT_H_
#define EROI_STRUCT_H_
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Dense>
#include <vector>
#include <deque>
#include <fstream>
#include <iostream>
#include <ros/ros.h>
#include <list>
#include <bitset>
#include <memory>
#include <math.h>
#include <cstddef>
#include <cstdint>
#include <std_msgs/ColorRGBA.h>
using namespace std;
using namespace Eigen;

namespace EROIStruct{
enum SensorType{LIVOX, CAMERA};

struct EroiNode{
    uint8_t f_state_;                    //0: unexplored; 1:exploring; 2: explored; 3: invalid; 
    uint8_t owner_;
    float owner_dist_;

    Eigen::Vector3d up_, down_, center_;

    vector<uint8_t> local_vps_;         //0: unsampled; 1:alive; 2: dead; 
    vector<uint8_t> valid_vps_;       // the idx that are valid in dtg
    double last_sample_;
    double last_strong_check_;
    int unknown_num_, thresh_num_;
    uint8_t flags_;                     //(local new)(show)(sample)(temp)
};

/**
 * @brief Read-only diagnostic summary for one EROI.
 *
 * valid_vp_num_ counts unique, in-range entries in EroiNode::valid_vps_.
 * raw_valid_vp_num_ additionally includes duplicate and out-of-range entries.
 * Gain is deliberately opt-in: gain_evaluated_ is false unless the caller
 * explicitly requests ray-casting gain evaluation.
 */
struct FrontierSummary{
    uint32_t eroi_id_;
    uint8_t f_state_;
    std::size_t raw_valid_vp_num_;
    std::size_t valid_vp_num_;
    std::size_t alive_local_vp_num_;
    std::size_t alive_valid_vp_num_;
    std::size_t duplicate_valid_vp_num_;
    std::size_t out_of_range_valid_vp_num_;
    int unknown_num_;
    bool gain_evaluated_;
    std::size_t gain_evaluation_num_;
    double max_gain_;

    FrontierSummary()
        : eroi_id_(0), f_state_(3), raw_valid_vp_num_(0),
          valid_vp_num_(0), alive_local_vp_num_(0),
          alive_valid_vp_num_(0), duplicate_valid_vp_num_(0),
          out_of_range_valid_vp_num_(0), unknown_num_(0),
          gain_evaluated_(false), gain_evaluation_num_(0), max_gain_(0.0) {}
};

/**
 * @brief Aggregate summary of all active (f_state_ == 1) EROIs.
 *
 * Counts below intentionally exclude unexplored, explored, and invalid EROIs.
 * This makes active_eroi_num_ suitable for exploration-completion checks while
 * retaining malformed valid_vps_ counters for diagnostics.
 */
struct ExplorationSummary{
    std::size_t active_eroi_num_;
    std::size_t eroi_with_alive_local_vp_num_;
    std::size_t raw_valid_vp_num_;
    std::size_t valid_vp_num_;
    std::size_t alive_local_vp_num_;
    std::size_t alive_valid_vp_num_;
    std::size_t duplicate_valid_vp_num_;
    std::size_t out_of_range_valid_vp_num_;
    int max_unknown_num_;
    bool gain_evaluated_;
    std::size_t gain_evaluation_num_;
    double max_gain_;

    ExplorationSummary()
        : active_eroi_num_(0), eroi_with_alive_local_vp_num_(0),
          raw_valid_vp_num_(0), valid_vp_num_(0),
          alive_local_vp_num_(0), alive_valid_vp_num_(0),
          duplicate_valid_vp_num_(0), out_of_range_valid_vp_num_(0),
          max_unknown_num_(0), gain_evaluated_(false),
          gain_evaluation_num_(0), max_gain_(0.0) {}
};
}

#endif

//                 @@@@@
//                 ##**#%@@
//           @@@@%*-:::--+%
//         @@%#*%#-::::=:.+@%
//        @#=:..-#-.:::+++#@%
//       @#:...:.:#-...:*@@%
//     @%=........-#:++=+%@%
//    @%- . =:.....+*-...:#@%
//    @%: . =#......#: ... +@%
//    @%:   -:.....:#..... :%@
//      @*: ...... -* ..... =@%
//       @%=:... . -= ..... +@%
//        @@*==--::--:...  .%@%
//        @+=++++=======--=*@%
//       @#++++==========+@@
//      @#++++====-=======*%
//     %*+++===--===----===+%
//    #*++===--=*%%%#=-=====*@
//  *++==--+%@@%%%@%*--====#@
// #*====-=#@        @#=--===%@
// #*+=--+##          @@*=--=+#@
// #*+++=*#              @%+=+++%
// #*++=+#*                #++++*#
// #*++=+#                  #=+++#
// #*+++#                   %*=++*#
// #++=*%                   #%+=++#
