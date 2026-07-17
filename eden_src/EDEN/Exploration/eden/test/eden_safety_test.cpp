#include <eden/eden_safety.h>

#include <gtest/gtest.h>

#include <limits>
#include <string>

namespace {

Trajectory4<5> MakeTrajectory(double duration,
                              const Piece4<5>::CoefficientMat4& coefficients) {
  Trajectory4<5> trajectory;
  trajectory.emplace_back(duration, coefficients);
  return trajectory;
}

TEST(EdenSafetyTest, ConvertsAbsoluteWallTimeToRelativeTrajectoryTime) {
  EXPECT_DOUBLE_EQ(1.0, EdenSafety::RelativeTrajectoryTime(1001.0, 1000.0, 5.0));
  EXPECT_DOUBLE_EQ(0.0, EdenSafety::RelativeTrajectoryTime(999.0, 1000.0, 5.0));
  EXPECT_NEAR(5.0 - EdenSafety::kMinTrajectoryDuration,
              EdenSafety::RelativeTrajectoryTime(2000.0, 1000.0, 5.0),
              1.0e-12);
}

TEST(EdenSafetyTest, RejectsNonFiniteTrajectoryData) {
  Piece4<5>::CoefficientMat4 coefficients =
      Piece4<5>::CoefficientMat4::Zero();
  std::string reason;

  EXPECT_TRUE(EdenSafety::ValidateTrajectory(
      MakeTrajectory(1.0, coefficients), &reason));

  coefficients(0, 0) = std::numeric_limits<double>::infinity();
  EXPECT_FALSE(EdenSafety::ValidateTrajectory(
      MakeTrajectory(1.0, coefficients), &reason));

  coefficients.setZero();
  EXPECT_FALSE(EdenSafety::ValidateTrajectory(
      MakeTrajectory(std::numeric_limits<double>::quiet_NaN(), coefficients),
      &reason));

  coefficients.setZero();
  coefficients(0, 0) = 1.0e308;
  EXPECT_FALSE(EdenSafety::ValidateTrajectory(
      MakeTrajectory(2.0, coefficients), &reason));
}

TEST(EdenSafetyTest, KnownVolumeCompletionUsesTimeNotPlannerStatus) {
  double stable_since = -1.0;
  double ratio = 0.0;

  EXPECT_FALSE(EdenSafety::UpdateKnownVolumeCompletion(
      true, true, 997.0, 1000.0, 0.99, 2.0, 100.0,
      &stable_since, &ratio));
  EXPECT_NEAR(0.997, ratio, 1.0e-12);
  EXPECT_FALSE(EdenSafety::UpdateKnownVolumeCompletion(
      true, true, 997.0, 1000.0, 0.99, 2.0, 101.0,
      &stable_since, &ratio));
  EXPECT_TRUE(EdenSafety::UpdateKnownVolumeCompletion(
      true, true, 997.0, 1000.0, 0.99, 2.0, 102.0,
      &stable_since, &ratio));

  EXPECT_FALSE(EdenSafety::UpdateKnownVolumeCompletion(
      true, true, 980.0, 1000.0, 0.99, 2.0, 103.0,
      &stable_since, &ratio));
  EXPECT_LT(stable_since, 0.0);
}

}  // namespace
