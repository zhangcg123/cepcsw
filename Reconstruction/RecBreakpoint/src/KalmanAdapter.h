#ifndef RECBREAKPOINT_KALMANADAPTER_H
#define RECBREAKPOINT_KALMANADAPTER_H

#include "TrackState.h"
#include "RecBreakpoint/SeedHitSelection.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"

#include <memory>
#include <vector>

namespace breakpoint {
class BreakpointTrackSystem;
struct MeasurementStep {
  TrackState predicted, filtered;
  TMatrixD transport{5, 5};
  TMatrixD noise{5, 5};
  double chi2 = 0;
  int dimension = 0;
};

/// Owns temporary tracks only. The algorithm owns the supplied tracking system.
/// All physical propagation and hit updates remain baseline KalTest calls.
class KalmanAdapter {
public:
  KalmanAdapter(BreakpointTrackSystem* system, double bz, double maxChi2,
                SeedHitSelection seedSelection)
      : m_system(system), m_bz(bz), m_maxChi2(maxChi2), m_seedSelection(seedSelection) {}
  std::array<int, 3> seedHitIndices(const std::vector<edm4hep::TrackerHit>& hits) const;
  MeasurementStep seed(const std::vector<edm4hep::TrackerHit>& hits, double scale) const;
  MeasurementStep advance(const TrackState& source, edm4hep::TrackerHit sourceHit,
                          edm4hep::TrackerHit targetHit) const;
  edm4hep::TrackState atIP(const TrackState& state, edm4hep::TrackerHit hit) const;
  /// Independent, unbroken MarlinTrk fit and its native smoother. Diagnostic
  /// reference for the empty-breakpoint limit, with exactly the same seed/hits.
  edm4hep::TrackState referenceKF(const std::vector<edm4hep::TrackerHit>& hits, double scale) const;
private:
  edm4hep::TrackState prefit(const std::vector<edm4hep::TrackerHit>& hits, double scale) const;
  std::unique_ptr<MarlinTrk::IMarlinTrack> initialized(
      const edm4hep::TrackState& state, edm4hep::TrackerHit hit, bool inward) const;
  BreakpointTrackSystem* m_system;
  double m_bz, m_maxChi2;
  SeedHitSelection m_seedSelection;
};
} // namespace breakpoint
#endif
