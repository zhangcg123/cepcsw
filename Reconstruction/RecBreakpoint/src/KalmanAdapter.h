#ifndef RECBREAKPOINT_KALMANADAPTER_H
#define RECBREAKPOINT_KALMANADAPTER_H

#include "TrackState.h"
#include "LossTrackState.h"
#include "GaussianTrackModel.h"
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
  TMatrixD lossTargetCross{1, 5};
  double covarianceClosure = 0;
};
struct MeasurementScore {
  double affine = 0;
  double native = 0;
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
  /// Passive evaluation only: no hit update. The affine score uses the SAME
  /// measurement expansion point as the corresponding filter pass.
  MeasurementScore measurementScore(edm4hep::TrackerHit hit, const TrackState& state,
                                     const TrackState& reference) const;
  /// Passive native H/V/residual capture; does not perform a measurement update.
  GaussianHitModel gaussianHitModel(edm4hep::TrackerHit hit,
                                   const TrackState& reference) const;
  MeasurementStep advance(const TrackState& source, edm4hep::TrackerHit sourceHit,
                          edm4hep::TrackerHit targetHit) const;
  /// Full 6D prediction and native 6D hit update. Apply the selected loss
  /// mapping only at birth; coordinate 5 persists through downstream hits.
  LossMeasurementStep advancePersistent(const LossTrackState& source,
      edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit, bool applyLoss,
      LossCoordinate coordinate = LossCoordinate::LogRatio,
      const TrackState* lossReference = nullptr) const;
  /// Native geometry/material prediction without a hit update. Diffuse b uses
  /// this same transition, then its own exact-rank-one update until identified.
  LossMeasurementStep predictPersistent(const LossTrackState& source,
      edm4hep::TrackerHit sourceHit, edm4hep::TrackerHit targetHit, bool applyLoss,
      LossCoordinate coordinate = LossCoordinate::LogRatio,
      const TrackState* lossReference = nullptr) const;
  /// Propagate inward first, undo the selected upstream loss before its hit.
  MeasurementStep advanceBackward(const TrackState& source, edm4hep::TrackerHit sourceHit,
      edm4hep::TrackerHit targetHit, bool breakpoint, double meanLoss, double sigmaLoss) const;
  // One native, material-aware IP operation for every endpoint family.
  edm4hep::TrackState propagateToIP(const TrackState& state, edm4hep::TrackerHit hit) const;
  // The same native ECAL-face extrapolation used by CompleteTracks. It starts
  // from this endpoint's own outermost fitted state, not the input KF state.
  edm4hep::TrackState propagateToCalorimeter(const TrackState& state,
                                             edm4hep::TrackerHit lastHit) const;
  /// Independent, unbroken MarlinTrk fit and its native smoother. Diagnostic
  /// reference for the empty-breakpoint limit, with exactly the same seed/hits.
  edm4hep::TrackState referenceKF(const std::vector<edm4hep::TrackerHit>& hits, double scale,
                                bool backwardFilter = false) const;
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
