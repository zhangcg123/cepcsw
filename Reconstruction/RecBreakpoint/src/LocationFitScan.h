#ifndef RECBREAKPOINT_LOCATION_FIT_SCAN_H
#define RECBREAKPOINT_LOCATION_FIT_SCAN_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

class TTree;
namespace edm4hep { class TrackerHit; class TrackState; }
namespace breakpoint {
class BreakpointFitter;
struct FitSettings;
struct FreeLossSettings;

inline std::vector<int> locationScanSetups(bool neutralEnabled) {
  return neutralEnabled ? std::vector<int>{0,100,110,111} : std::vector<int>{0,100,110};
}

inline std::vector<int> checkedLocationScanSetups(std::vector<int> requested) {
  std::sort(requested.begin(), requested.end());
  if (std::adjacent_find(requested.begin(), requested.end()) != requested.end())
    throw std::invalid_argument("LocationScanSetups must not contain duplicates");
  for (int setup : requested)
    if (setup!=0 && setup!=100 && setup!=110 && setup!=111)
      throw std::invalid_argument("LocationScanSetups accepts only 0,100,110,111");
  return requested;
}

// Null, reconstructed silicon sites plus the first exact TPC row0, then an
// optional truth-only site. Truth never changes the reconstructed candidates.
inline std::vector<int> locationScanPoints(const std::vector<int>& systems,
    const std::vector<int>& layers, int truthHit=-1) {
  if (systems.size()!=layers.size())
    throw std::invalid_argument("Misaligned location-scan detector and layer IDs");
  std::vector<int> points{-1};
  bool addedTpcRow0=false;
  for (std::size_t i=0;i+1<systems.size();++i) {
    const bool silicon=systems[i]>=1 && systems[i]<=3;
    const bool tpcRow0=systems[i]==4 && layers[i]==0 && !addedTpcRow0;
    if (silicon || tpcRow0) points.push_back(static_cast<int>(i));
    if (tpcRow0) addedTpcRow0=true;
  }
  if (truthHit>=0 && static_cast<std::size_t>(truthHit+1)<systems.size()) {
    bool present=false;
    for (int i : points) if (i==truthHit) present=true;
    if (!present) points.push_back(truthHit);
  }
  return points;
}

/// Diagnostic only: one aligned vector entry per (point, setup). No ranking,
/// NLL-gain cut, selected location or published refitted-track endpoint.
struct LocationFitScan {
  int status=0, pointCount=0, truthStatus=0, truthHit=-1, truthPoint=-1;
  std::string truthError;
  // kind: 0 null, 1 silicon candidate, 2 truth-only reference, 3 TPC row0 candidate.
  std::vector<int> point, kind, hit, system, layer, setup, actualMask;
  std::vector<int> resolvedEcalSum;
  std::vector<int> fitStatus, attemptStatus, minuitStatus, attemptMinuitStatus;
  std::vector<int> sourceSetup, measurementDimensions;
  std::vector<double> nll2, chi2, logDeterminant, pt, ptError;
  // Fixed strides per (point,setup): 5 helix parameters, 21 EDM covariances, 3 reference coordinates.
  std::vector<double> ipParameters, ipCovariance, ipReferencePoint, ipTime;
  std::vector<int> ipLocation;
  std::vector<double> priorCenter, fittedLoss, fittedVariance;
  std::vector<std::string> error;

  void reset() { *this=LocationFitScan{}; }
  void book(TTree& tree, const std::string& prefix = "location_scan_");
  void appendIP(const edm4hep::TrackState* state);
  void run(const BreakpointFitter& fitter, const std::vector<edm4hep::TrackerHit>& hits,
           const FitSettings& observations, const FreeLossSettings& controls,
           double bz, std::optional<int> truthOwner, bool neutralEnabled,
           const std::vector<int>& requestedSetups = {}, bool referenceOnly = false);
};
} // namespace breakpoint
#endif
