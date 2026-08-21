#ifndef RecGsfTracking_TruthBHLossScopeStatus_h
#define RecGsfTracking_TruthBHLossScopeStatus_h

#include <cstdint>

/// Per-input-track status for the default-off truth BH-loss oracle.
///
/// Only Valid means that the complete truth interval map was accepted before
/// the refit and was therefore eligible to replace executed BH responses.
/// Every other status uses the configured BH model.
enum class TruthBHLossScopeStatus : std::int32_t {
  TrackNotProcessed = -4,
  InvalidIntervalMapping = -3,
  EndpointDistanceExceeded = -2,
  InvalidTruthEvent = -1,
  Disabled = 0,
  Valid = 1,
  NotSelected = 2,
};

constexpr std::int32_t truthBHLossStatusValue(
    TruthBHLossScopeStatus status) {
  return static_cast<std::int32_t>(status);
}

constexpr bool truthBHLossStatusIsValid(TruthBHLossScopeStatus status) {
  return status == TruthBHLossScopeStatus::Valid;
}

#endif
