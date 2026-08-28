#ifndef RecGsfTracking_GsfTrackInitializer_h
#define RecGsfTracking_GsfTrackInitializer_h

#include "edm4hep/TrackState.h"
#include "edm4hep/TrackerHit.h"

#include <string>
#include <vector>

class DDVMeasLayer;
class DDVTrackHit;
class TKalTrackSite;

namespace MarlinTrk {
class IMarlinTrkSystem;
}

/// Result of the standard-KF-style initialization used by every GSF method.
/// Ownership of `site` transfers to the caller on success.
struct GsfTrackInitializationResult {
  TKalTrackSite* site = nullptr;
  edm4hep::TrackState prefitState;
  edm4hep::TrackState firstFilteredState;
  double firstHitDeltaChi2 = 0.0;
  double prefitOmegaVariance = 0.0;
  double prefitKappaVariance = 0.0;
  int firstHitNdf = 0;
  int firstHitMeasurementDimension = 0;
  int twoDimensionalHitCount = 0;
  std::string error;

  bool valid() const { return site != nullptr; }
};

/// Build a fresh GSF seed using the same initialization convention as the
/// standard KalTest refit: a first/middle/last two-dimensional-hit helix,
/// the full loose LCIO covariance, and an explicit first-hit update through
/// the baseline MarlinTrk interface.
class GsfTrackInitializer {
public:
  explicit GsfTrackInitializer(MarlinTrk::IMarlinTrkSystem* trackSystem)
      : m_trackSystem(trackSystem) {}

  GsfTrackInitializationResult initialize(
      const std::vector<edm4hep::TrackerHit>& orderedHits,
      const DDVMeasLayer& firstLayer,
      const DDVTrackHit& firstKalHit,
      double bz,
      double kappaCovarianceOverride = -1.0) const;

private:
  MarlinTrk::IMarlinTrkSystem* m_trackSystem = nullptr;
};

#endif
