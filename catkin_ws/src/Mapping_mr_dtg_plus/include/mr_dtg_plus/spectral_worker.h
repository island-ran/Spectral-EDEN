#ifndef MR_DTG_PLUS_SPECTRAL_WORKER_H_
#define MR_DTG_PLUS_SPECTRAL_WORKER_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <mr_dtg_plus/mr_dtg_plus_structures.h>

namespace DTGPlus {

// ── Permanent worker with latest-only mailbox ──
// The worker maintains its own sparse graph copy.  All relative deltas are
// applied in order, but only the newest resulting graph state is solved.

class SpectralWorker {
 public:
  explicit SpectralWorker(const SpectralV4Config& config);
  ~SpectralWorker();

  SpectralWorker(const SpectralWorker&) = delete;
  SpectralWorker& operator=(const SpectralWorker&) = delete;

  void Start();
  void Stop();

  // Submit a lightweight relative delta. Pending deltas are accumulated so
  // topology updates can never be lost while an older state is being solved.
  void Submit(std::shared_ptr<const GraphDelta> delta);

  // Non-blocking read of the latest region state snapshot.
  // Returns nullptr if no result is available.
  std::shared_ptr<const RegionStateSnapshot> TryConsume();

 private:
  void Run();

  SpectralV4Config config_;
  std::thread thread_;
  std::mutex mailbox_mutex_;
  std::condition_variable cv_;
  std::vector<std::shared_ptr<const GraphDelta>> pending_deltas_;
  std::shared_ptr<const RegionStateSnapshot> latest_result_;
  bool running_ = false;
};

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_SPECTRAL_WORKER_H_
