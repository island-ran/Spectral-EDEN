#include <eden/eden_safety.h>

#include <gtest/gtest.h>

#include <limits>
#include <string>

namespace {

Trajectory4<5> MakeTrajectory(
    double duration,
    const Piece4<5>::CoefficientMat4& coefficients) {
  Trajectory4<5> trajectory;
  trajectory.emplace_back(duration, coefficients);
  return trajectory;
}

TEST(EdenSafetyTest, ConvertsAbsoluteTimeToRelativeTime) {
  EXPECT_DOUBLE_EQ(
      1.0,
      EdenSafety::RelativeTrajectoryTime(1001.0, 1000.0, 5.0));
  EXPECT_DOUBLE_EQ(
      0.0,
      EdenSafety::RelativeTrajectoryTime(999.0, 1000.0, 5.0));
  EXPECT_NEAR(
      5.0 - EdenSafety::kMinTrajectoryDuration,
      EdenSafety::RelativeTrajectoryTime(2000.0, 1000.0, 5.0),
      1.0e-12);
}

TEST(EdenSafetyTest, RejectsNonFiniteTrajectoryData) {
  Piece4<5>::CoefficientMat4 coefficients =
      Piece4<5>::CoefficientMat4::Zero();
  std::string reason;
  EXPECT_TRUE(EdenSafety::ValidateTrajectory(
      MakeTrajectory(1.0, coefficients), &reason));

  coefficients(0, 0) =
      std::numeric_limits<double>::infinity();
  EXPECT_FALSE(EdenSafety::ValidateTrajectory(
      MakeTrajectory(1.0, coefficients), &reason));

  coefficients.setZero();
  EXPECT_FALSE(EdenSafety::ValidateTrajectory(
      MakeTrajectory(
          std::numeric_limits<double>::quiet_NaN(),
          coefficients),
      &reason));
}

TEST(EdenSafetyTest, BoundsPlannerRetryIntervals) {
  EXPECT_DOUBLE_EQ(
      0.0, EdenSafety::SanitizePlanInterval(-1.0));
  EXPECT_DOUBLE_EQ(
      EdenSafety::kMaxPlanInterval,
      EdenSafety::SanitizePlanInterval(1000.0));
  EXPECT_DOUBLE_EQ(
      EdenSafety::kDefaultPlanInterval,
      EdenSafety::SanitizePlanInterval(
          std::numeric_limits<double>::infinity()));
  EXPECT_DOUBLE_EQ(
      EdenSafety::kDefaultPlanInterval,
      EdenSafety::SanitizePlanInterval(
          std::numeric_limits<double>::quiet_NaN()));
}

}  // namespace
