#ifndef RecGsfTracking_GsfComponent_h
#define RecGsfTracking_GsfComponent_h

#include "kaltest/TKalTrack.h"
#include "kaltest/THelicalTrack.h"
#include "TMatrixD.h"
#include "TVector3.h"
#include "edm4hep/TrackState.h"

#include <string>
#include <map>
#include <utility>
#include <vector>

/// One forward transition recorded by the Gaussian-sum smoother.  All
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
  /// Fraction of aggregate component weight carried by its strongest real
  /// (pre-KL-merge) lineage. Likelihood updates and global normalization leave
  /// this fraction unchanged; moment merging updates it without summing
  /// mutually exclusive lineage evidence.
  double    dominantLineageFraction = 1.0;
  int       charge = 1;
  TKalTrack* kaltrack = nullptr;
  double    fitChi2 = 0.0;
  int       debugId = 0;
  int       debugParentId = -1;
  int       generation = 0;
  /// Current immutable node in the passive component-lineage DAG.  The node
  /// record outlives this component when the component is rejected, removed,
  /// or consumed by a KL merge.  This identifier never steers the fit.
  int       lineageNodeId = -1;
  /// True only for the lineage that selected the exact no-radiation atom at
  /// every BH convolution. It is protected from cutoff and radiative merges.
  bool      noRadiationLineage = true;
  /// Most recent reverse-process child represented by this component. These
  /// fields are diagnostics only; KL merging retains the representative's
  /// values and does not use them for selection.
  int       lastReverseProcessHit = -1;
  int       lastReverseProcessComponent = -1;
  double    lastReverseProcessFraction = 1.0;
  std::string forwardProcessSignature;
  std::string reverseProcessSignature;
  /// Diagnostic marginal fractions of the aggregate component weight carried
  /// by each (measurement hit, BH process mode).  Empty in normal running;
  /// populated only by the opt-in surface-lineage-mass diagnostic.  KL moment
  /// merges combine these fractions by incoming component weight without
  /// changing the merge distance, representative history, or selection.
  std::map<std::pair<int, int>, double> forwardProcessModeFractions;
  std::map<std::pair<int, int>, double> reverseProcessModeFractions;
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
  /// Current node in the reduction-aware forward graph used by the optional
  /// Gaussian-sum smoother.
  int smoothingNodeId = -1;
  /// Pre-reduction graph nodes represented by the current KL-reduced
  /// component, expressed as conditional mixture fractions.
  std::map<int, double> smoothingSourceFractions;
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
