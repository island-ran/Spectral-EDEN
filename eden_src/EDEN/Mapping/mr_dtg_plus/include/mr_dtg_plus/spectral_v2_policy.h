#ifndef MR_DTG_PLUS_SPECTRAL_V2_POLICY_H_
#define MR_DTG_PLUS_SPECTRAL_V2_POLICY_H_

#include <cstddef>
#include <limits>

namespace DTGPlus {

// ── Lightweight spectral utility functions ──
// The V2/V3 state machine, lock-debt, dual-route comparison, and partition-
// confidence scoring have been removed in the V4 unified pipeline.  Only the
// stateless numerical helpers remain.

// Invalid/non-finite values are mapped to zero.  Otherwise the result is
// clipped to [0, 1].
double Clamp01(double value);

// g_t = (lambda3 - lambda2) / (lambda3 + epsilon), clipped to [0, 1].
// Invalid inputs return NaN so callers can distinguish missing evidence from
// a genuinely weak eigengap.
double RelativeEigengap(double lambda2, double lambda3,
                        double epsilon = 1.0e-9);

// lambda2_ema(t) = alpha * lambda2_ema(t-1) + (1-alpha) * lambda2(t).
// A missing previous value initializes the EMA from the current sample; a
// missing current sample preserves the previous finite value.
double UpdateLambda2Ema(double lambda2, double previous_ema, double alpha);

// r_t = lambda2 / (lambda2_ema + epsilon).  The ratio is intentionally not
// clipped because values above one are meaningful.
double RelativeLambda2(double lambda2, double lambda2_ema,
                       double epsilon = 1.0e-9);

}  // namespace DTGPlus

#endif  // MR_DTG_PLUS_SPECTRAL_V2_POLICY_H_
