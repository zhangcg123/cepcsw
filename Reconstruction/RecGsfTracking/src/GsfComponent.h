#ifndef RecGsfTracking_GsfComponent_h
#define RecGsfTracking_GsfComponent_h

#include "kaltest/TKalTrack.h"
#include "kaltest/THelicalTrack.h"
#include "TMatrixD.h"
#include "TVector3.h"
#include "edm4hep/TrackState.h"

#include <string>

/// One Gaussian component in the GSF mixture.
/// Owns a TKalTrack holding the KF state across all measurement sites.
struct GsfComponent {
  double    weight = 1.0;
  int       charge = 1;
  TKalTrack* kaltrack = nullptr;
  double    fitChi2 = 0.0;
  int       debugId = 0;
  int       debugParentId = -1;
  int       generation = 0;
  int       hitsSinceSplit = 0;
  std::string debugHistory;
  /// State used to continue propagation after a surface-local process
  /// transition.  The Kalman history remains the pre-process measurement
  /// history; this snapshot may contain the post-process component state.
  bool continuationValid = false;
  edm4hep::TrackState continuationState;

  ~GsfComponent();

  /// Extract helix from this component's current (last) site
  THelicalTrack helixAtLastSite(double bzTesla) const;

  /// Extract the unchanged filtered state at the last measurement site.
  THelicalTrack helixAtMeasurementSite(double bzTesla) const;

  /// 5×5 covariance at the last site
  TMatrixD covAtLastSite(double bzTesla) const;

  /// Seed the continuation snapshot from the current measurement state.
  bool snapshotContinuation(double bzTesla, int location = 0);

  /// Replace the continuation snapshot with parameters/covariance expressed
  /// at its current common surface.  Measurement history is not modified.
  bool setContinuationSurfaceState(const TMatrixD& mean,
                                   const TMatrixD& covariance,
                                   double bzTesla);

  /// Deep-clone: copies the full TKalTrack (all sites + states)
  GsfComponent* clone() const;
};

#endif
