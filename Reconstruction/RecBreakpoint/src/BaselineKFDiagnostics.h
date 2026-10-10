#pragma once

#include "TrackState.h"
#include "TrackSystemSvc/IMarlinTrkSystem.h"
#include "edm4hep/TrackerHit.h"
#include <string>
#include <vector>

namespace breakpoint {
class BreakpointTrackSystem;
// All vectors stay indexed by the original outward-ordered hit list. Failed
// updates have an explicit status and empty matrix slots, never fake zeros.
struct BaselineDirection {
  int status = 0, smoothingStatus = 0;
  int productionContinuationStatus = 0;
  std::string error;
  double chi2 = 0;
  std::vector<double> seedParameters, seedCovariance, seedPivot, ipParameters,
      ipCovariance;
  std::vector<double> productionIPParameters, productionIPCovariance;
  std::vector<int> updateStatus, diagnosticValid, dimension, previousHit,
      smoothStatus;
  std::vector<int> nativeDimension;
  std::vector<std::vector<double>> nativePredicted, nativePredictedCovariance,
      nativeProjector, nativeTransport, nativeNoise, backendResidual;
  std::vector<double> localChi2, residualChi2, smoothedResidualChi2;
  std::vector<std::vector<double>> predicted, predictedCovariance, updated,
      updatedCovariance, pivot, innovation, innovationCovariance, projector,
      measurement, measurementCovariance, transport, processNoise, smoothed,
      smoothedCovariance, smoothedResidual, smoothedResidualCovariance;
  void resize(std::size_t n);
};

// Uses only MarlinTrk public operations. No augmented state, truth, ECAL,
// private-member access, or replacement measurement-update implementation.
BaselineDirection
baselineDiagnostics(BreakpointTrackSystem &system,
                    const std::vector<edm4hep::TrackerHit> &orderedHits,
                    double bz, const std::vector<double> &seedVariances,
                    double maxChi2, bool backward, bool smooth);
} // namespace breakpoint
