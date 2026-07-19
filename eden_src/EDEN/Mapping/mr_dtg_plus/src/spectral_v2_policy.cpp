#include <mr_dtg_plus/spectral_v2_policy.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace DTGPlus {
namespace {

double QuietNaN() {
  return std::numeric_limits<double>::quiet_NaN();
}

double PositiveEpsilon(double epsilon) {
  return std::isfinite(epsilon) && epsilon > 0.0 ? epsilon : 1.0e-9;
}

}  // namespace

double Clamp01(double value) {
  if (!std::isfinite(value)) {
    return 0.0;
  }
  return std::max(0.0, std::min(1.0, value));
}

double RelativeEigengap(double lambda2, double lambda3, double epsilon) {
  if (!std::isfinite(lambda2) || !std::isfinite(lambda3) || lambda2 < 0.0 ||
      lambda3 < 0.0) {
    return QuietNaN();
  }
  const double denominator = lambda3 + PositiveEpsilon(epsilon);
  if (!std::isfinite(denominator) || denominator <= 0.0) {
    return QuietNaN();
  }
  return Clamp01((lambda3 - lambda2) / denominator);
}

double UpdateLambda2Ema(double lambda2, double previous_ema, double alpha) {
  const bool current_valid = std::isfinite(lambda2) && lambda2 >= 0.0;
  const bool previous_valid =
      std::isfinite(previous_ema) && previous_ema >= 0.0;
  if (!current_valid) {
    return previous_valid ? previous_ema : QuietNaN();
  }
  if (!previous_valid) {
    return lambda2;
  }
  const double clipped_alpha = Clamp01(alpha);
  return clipped_alpha * previous_ema + (1.0 - clipped_alpha) * lambda2;
}

double RelativeLambda2(double lambda2, double lambda2_ema, double epsilon) {
  if (!std::isfinite(lambda2) || !std::isfinite(lambda2_ema) ||
      lambda2 < 0.0 || lambda2_ema < 0.0) {
    return QuietNaN();
  }
  const double denominator = lambda2_ema + PositiveEpsilon(epsilon);
  if (!std::isfinite(denominator) || denominator <= 0.0) {
    return QuietNaN();
  }
  return lambda2 / denominator;
}

}  // namespace DTGPlus
