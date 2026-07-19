#ifndef MR_DTG_PLUS_COMMITTED_TARGET_MANAGER_H_
#define MR_DTG_PLUS_COMMITTED_TARGET_MANAGER_H_

#include <cstdint>
#include <mr_dtg_plus/mr_dtg_plus_structures.h>

namespace DTGPlus {

// ── Deterministic committed-target manager ──
// Replaces soft-lock, lock-debt, and random cooldown with a simple
// hysteresis rule: keep the current target until either a hard event
// invalidates it or a new candidate beats it by a fixed margin.

class CommittedTargetManager {
 public:
  CommittedTargetManager() = default;
  explicit CommittedTargetManager(const SpectralV4Config& config);

  // Check whether we should keep the current target.
  // Returns true if the current target is still valid and the minimum
  // commitment time since acceptance has not elapsed.
  bool CanKeepCurrent(double now,
                      const CommittedTarget& target,
                      bool frontier_valid,
                      bool path_valid,
                      bool trajectory_safe) const;

  // Select a new target from candidates with deterministic hysteresis.
  // Returns the index of the selected candidate, or -1 to keep current.
  int SelectWithHysteresis(double now,
                           const CommittedTarget& current,
                           const std::vector<double>& candidate_costs,
                           const std::vector<double>& candidate_gains) const;

  // Record that a target was accepted after successful trajectory publish.
  CommittedTarget CommitAfterPublish(const CommittedTarget& candidate,
                                     uint32_t frontier_id,
                                     uint8_t viewpoint_id,
                                     double now,
                                     double initial_unknown_count) const;

  // Hard events that can immediately invalidate the current target:
  bool IsHardInvalidation(bool frontier_invalid,
                          bool path_blocked,
                          bool trajectory_unsafe,
                          int consecutive_low_gain) const;

 private:
  SpectralV4Config config_;
};

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_COMMITTED_TARGET_MANAGER_H_
