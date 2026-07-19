#ifndef EDEN_EDEN_SAFETY_H_
#define EDEN_EDEN_SAFETY_H_

#include <gcopter/trajectory4.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <string>

namespace EdenSafety {

constexpr double kMinTrajectoryDuration = 1.0e-6;
constexpr double kTrajectoryValidationStep = 0.05;
constexpr std::size_t kMaxTrajectoryValidationSamples = 20000U;
constexpr double kDefaultPlanInterval = 0.10;
constexpr double kMaxPlanInterval = 2.0;

inline double SanitizePlanInterval(
    double interval,
    double fallback = kDefaultPlanInterval,
    double maximum = kMaxPlanInterval) {
  const double safe_maximum =
      std::isfinite(maximum) && maximum > 0.0
          ? maximum
          : kMaxPlanInterval;
  const double safe_fallback =
      std::isfinite(fallback) && fallback >= 0.0
          ? std::min(fallback, safe_maximum)
          : std::min(kDefaultPlanInterval, safe_maximum);
  if (!std::isfinite(interval)) return safe_fallback;
  return std::max(0.0, std::min(interval, safe_maximum));
}

inline double RelativeTrajectoryTime(double absolute_time,
                                     double trajectory_start_time,
                                     double total_duration) {
  if (!std::isfinite(absolute_time) ||
      !std::isfinite(trajectory_start_time) ||
      !std::isfinite(total_duration) || total_duration <= 0.0) {
    return 0.0;
  }
  return std::max(
      0.0, std::min(absolute_time - trajectory_start_time,
                    std::max(0.0, total_duration -
                                      kMinTrajectoryDuration)));
}

inline bool ValidateTrajectory(const Trajectory4<5>& trajectory,
                               std::string* reason) {
  const auto fail = [reason](const std::string& message) {
    if (reason != nullptr) *reason = message;
    return false;
  };
  const int piece_num = trajectory.getPieceNum();
  if (piece_num <= 0) return fail("trajectory contains no piece");
  const double total_duration = trajectory.getTotalDuration();
  if (!std::isfinite(total_duration) ||
      total_duration <= kMinTrajectoryDuration)
    return fail("invalid total duration");

  for (int index = 0; index < piece_num; ++index) {
    const auto& piece = trajectory[index];
    const double duration = piece.getDuration();
    if (!std::isfinite(duration) ||
        duration <= kMinTrajectoryDuration ||
        !piece.getCoeffMat().allFinite())
      return fail("invalid trajectory piece at index " +
                  std::to_string(index));
  }

  const std::size_t sample_count = std::max<std::size_t>(
      1U, std::min<std::size_t>(
              kMaxTrajectoryValidationSamples,
              static_cast<std::size_t>(
                  std::ceil(total_duration /
                            kTrajectoryValidationStep))));
  for (std::size_t index = 0; index <= sample_count; ++index) {
    const double time =
        total_duration * static_cast<double>(index) /
        static_cast<double>(sample_count);
    if (!trajectory.getPos(time).allFinite() ||
        !trajectory.getVel(time).allFinite() ||
        !trajectory.getAcc(time).allFinite())
      return fail("sampled trajectory state contains NaN or Inf");
  }
  if (reason != nullptr) reason->clear();
  return true;
}

}  // namespace EdenSafety

#endif  // EDEN_EDEN_SAFETY_H_
