#ifndef EDEN_EDEN_SAFETY_H_
#define EDEN_EDEN_SAFETY_H_

#include <gcopter/trajectory4.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <string>

namespace EdenSafety {

constexpr double kMinTrajectoryDuration = 1.0e-6;
constexpr double kTrajectoryValidationStep = 0.05;
constexpr std::size_t kMaxTrajectoryValidationSamples = 20000U;

inline double RelativeTrajectoryTime(double absolute_time,
                                     double trajectory_start_time,
                                     double total_duration) {
  if (!std::isfinite(absolute_time) ||
      !std::isfinite(trajectory_start_time) ||
      !std::isfinite(total_duration) || total_duration <= 0.0) {
    return 0.0;
  }
  const double latest =
      std::max(0.0, total_duration - kMinTrajectoryDuration);
  return std::max(
      0.0, std::min(absolute_time - trajectory_start_time, latest));
}

inline bool ValidateTrajectory(const Trajectory4<5>& trajectory,
                               std::string* reason) {
  const auto fail = [reason](const std::string& message) {
    if (reason != nullptr) {
      *reason = message;
    }
    return false;
  };

  const int piece_num = trajectory.getPieceNum();
  if (piece_num <= 0) {
    return fail("trajectory contains no piece");
  }

  const double total_duration = trajectory.getTotalDuration();
  if (!std::isfinite(total_duration) ||
      total_duration <= kMinTrajectoryDuration) {
    return fail("invalid total duration");
  }

  for (int index = 0; index < piece_num; ++index) {
    const auto& piece = trajectory[index];
    const double duration = piece.getDuration();
    const auto& coefficients = piece.getCoeffMat();
    if (!std::isfinite(duration) ||
        duration <= kMinTrajectoryDuration) {
      return fail("invalid piece duration at index " +
                  std::to_string(index));
    }
    if (!coefficients.allFinite()) {
      return fail("invalid trajectory coefficient at index " +
                  std::to_string(index));
    }
    if (!piece.getPos(0.0).allFinite() ||
        !piece.getVel(0.0).allFinite() ||
        !piece.getAcc(0.0).allFinite() ||
        !piece.getPos(duration).allFinite() ||
        !piece.getVel(duration).allFinite() ||
        !piece.getAcc(duration).allFinite()) {
      return fail("non-finite piece boundary state at index " +
                  std::to_string(index));
    }
  }

  const double requested_samples =
      std::ceil(total_duration / kTrajectoryValidationStep);
  if (!std::isfinite(requested_samples) || requested_samples < 0.0) {
    return fail("invalid trajectory sampling count");
  }
  const std::size_t sample_count = std::max<std::size_t>(
      1U, std::min<std::size_t>(
              kMaxTrajectoryValidationSamples,
              static_cast<std::size_t>(std::min(
                  requested_samples,
                  static_cast<double>(kMaxTrajectoryValidationSamples)))));
  for (std::size_t index = 0U; index <= sample_count; ++index) {
    const double time = total_duration * static_cast<double>(index) /
                        static_cast<double>(sample_count);
    if (!trajectory.getPos(time).allFinite() ||
        !trajectory.getVel(time).allFinite() ||
        !trajectory.getAcc(time).allFinite()) {
      return fail("sampled trajectory state contains NaN or Inf");
    }
  }

  if (reason != nullptr) {
    reason->clear();
  }
  return true;
}

inline bool UpdateKnownVolumeCompletion(
    bool enabled, bool statistics_available, double explored_volume,
    double total_explorable_volume, double coverage_threshold,
    double stable_duration, double now, double* stable_since,
    double* coverage_ratio = nullptr) {
  if (stable_since == nullptr) {
    return false;
  }
  if (!enabled || !statistics_available ||
      !std::isfinite(explored_volume) || explored_volume < 0.0 ||
      !std::isfinite(total_explorable_volume) ||
      total_explorable_volume <= 0.0 ||
      !std::isfinite(coverage_threshold) ||
      !std::isfinite(stable_duration) || stable_duration < 0.0 ||
      !std::isfinite(now)) {
    *stable_since = -1.0;
    if (coverage_ratio != nullptr) {
      *coverage_ratio = std::numeric_limits<double>::quiet_NaN();
    }
    return false;
  }

  const double ratio = explored_volume / total_explorable_volume;
  if (coverage_ratio != nullptr) {
    *coverage_ratio = ratio;
  }
  if (!std::isfinite(ratio) || ratio < coverage_threshold) {
    *stable_since = -1.0;
    return false;
  }

  if (!std::isfinite(*stable_since) || *stable_since < 0.0) {
    *stable_since = now;
  }
  return now - *stable_since >= stable_duration;
}

}  // namespace EdenSafety

#endif  // EDEN_EDEN_SAFETY_H_
