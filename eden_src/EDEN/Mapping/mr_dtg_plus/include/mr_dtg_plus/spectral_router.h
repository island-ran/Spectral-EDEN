#ifndef MR_DTG_PLUS_SPECTRAL_ROUTER_H_
#define MR_DTG_PLUS_SPECTRAL_ROUTER_H_

#include <mr_dtg_plus/spectral_types.h>

namespace DTGPlus {

// Pure C++14/Eigen spectral kernel.  It owns no ROS state and no DTG node
// pointers, so a graph snapshot can be unit-tested independently and safely
// passed across planner layers.
class SpectralRouter {
 public:
  SpectralRouter() = default;
  explicit SpectralRouter(const SpectralConfig& config) : config_(config) {}

  const SpectralConfig& config() const { return config_; }
  void set_config(const SpectralConfig& config) { config_ = config; }

  SpectralResult Solve(
      const SpectralGraphSnapshot& snapshot,
      const FiedlerHistory& previous_fiedler = FiedlerHistory()) const;

 private:
  bool ValidateConfig(std::string* reason) const;

  SpectralConfig config_;
};

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_SPECTRAL_ROUTER_H_
