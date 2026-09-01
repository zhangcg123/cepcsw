#ifndef RecGsfTracking_GsfTrackInitializer_h
#define RecGsfTracking_GsfTrackInitializer_h

#include "edm4hep/TrackState.h"
#include "edm4hep/TrackerHit.h"

#include <array>
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
  edm4hep::TrackState seedFilteredState;
  double seedHitDeltaChi2 = 0.0;
  double prefitOmegaVariance = 0.0;
  double prefitKappaVariance = 0.0;
  int seedHitNdf = 0;
  int seedHitMeasurementDimension = 0;
  int twoDimensionalHitCount = 0;
  std::array<int, 3> prefitHitIndices{{-1, -1, -1}};
  std::string error;

  bool valid() const { return site != nullptr; }
};

enum class GsfTrackInitializationDirection {
  Outward,
  Inward,
};

/// Build a fresh GSF seed from the three two-dimensional hits nearest the
/// requested starting boundary: the three innermost hits for outward fitting
/// or the three outermost hits for inward fitting.  The seed retains the full
/// loose LCIO covariance and explicitly updates the first measurement consumed
/// in the requested fit direction through the baseline MarlinTrk interface.
class GsfTrackInitializer {
public:
  explicit GsfTrackInitializer(MarlinTrk::IMarlinTrkSystem* trackSystem)
      : m_trackSystem(trackSystem) {}

  GsfTrackInitializationResult initialize(
      const std::vector<edm4hep::TrackerHit>& orderedHits,
      const DDVMeasLayer& seedLayer,
      const DDVTrackHit& seedKalHit,
      double bz,
      GsfTrackInitializationDirection direction,
      double kappaCovarianceOverride = -1.0) const;

private:
  MarlinTrk::IMarlinTrkSystem* m_trackSystem = nullptr;
};

#endif
