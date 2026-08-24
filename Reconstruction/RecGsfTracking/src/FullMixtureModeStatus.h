#ifndef RecGsfTracking_FullMixtureModeStatus_h
#define RecGsfTracking_FullMixtureModeStatus_h

#include <cstdint>

/// Persisted status for the default-on full five-dimensional posterior-mode
/// endpoint produced by the smoother and ordinary reverse workflows.
enum class FullMixtureModeStatus : std::int32_t {
  NotApplicable = 0,
  Success = 1,
  IncompleteComponentSet = -1,
  OptimizationFailed = -2,
  InvalidLocalCovariance = -3,
  MethodEndpointUnavailable = -4,
};

inline std::int32_t fullMixtureModeStatusValue(
    FullMixtureModeStatus status) {
  return static_cast<std::int32_t>(status);
}

#endif
