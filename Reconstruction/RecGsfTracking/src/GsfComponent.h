#ifndef RecGsfTracking_GsfComponent_h
#define RecGsfTracking_GsfComponent_h

#include "kaltest/TKalTrack.h"
#include "kaltest/THelicalTrack.h"
#include "TMatrixD.h"
#include "TVector3.h"
#include "edm4hep/TrackState.h"

#include <string>
#include <vector>

/// One retained-lineage transition used by the branch RTS smoother.  All
/// quantities use KalTest's local five-parameter helix convention.
struct GsfSmoothingStep {
  bool transitionValid = false;
  double transportCovarianceClosure = 0.0;
  double bhAddedKappaVariance = 0.0;
  double rtsKappaGainNorm = 0.0;
  double rtsKappaCorrection = 0.0;
  TMatrixD filteredMean{5, 1};
  TMatrixD filteredCovariance{5, 5};
  TMatrixD processInputMean{5, 1};
  TMatrixD processInputCovariance{5, 5};
  TMatrixD predictedMean{5, 1};
  TMatrixD predictedCovariance{5, 5};
  TMatrixD processJacobian{5, 5};
  TMatrixD transportJacobian{5, 5};
  TMatrixD transportProcessNoise{5, 5};
  TMatrixD transitionJacobian{5, 5};
};

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
  /// BH/process Jacobian at the current surface, composed with the exact
  /// MarlinTrk transport Jacobian when the next measurement is accepted.
  TMatrixD pendingProcessJacobian{5, 5};
  std::vector<GsfSmoothingStep> smoothingSteps;
  bool smoothedInnerValid = false;
  TMatrixD smoothedInnerMean{5, 1};
  TMatrixD smoothedInnerCovariance{5, 5};

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
