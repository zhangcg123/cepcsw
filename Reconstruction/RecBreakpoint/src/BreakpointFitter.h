#ifndef RECBREAKPOINT_FITTER_H
#define RECBREAKPOINT_FITTER_H

#include "KalmanAdapter.h"
#include <vector>

namespace breakpoint {
struct FitSettings {
  std::vector<int> intervals; // i selects the outward transition hit[i] -> hit[i+1]
  double meanLogLoss = 0;
  double sigmaLogLoss = 0.05;
  double seedScale = 1;
  std::string backwardMode = "RTS";
  std::string lossStateMode = "Persistent6D";
};

struct IntervalResult {
  int index = -1;
  double priorLogLoss = 0;
  double fittedLogLoss = 0;
  double fittedVariance = 0;
  double localLogLoss = 0;
  double localVariance = 0;
  double covarianceClosure = 0;
};

struct FitResult {
  edm4hep::TrackState ip{};
  std::vector<TrackState> predicted, filtered, smoothed;
  std::vector<TrackState> backwardPredicted, backwardFiltered, endpoint;
  std::vector<double> backwardChi2;
  std::vector<double> localChi2;
  std::vector<IntervalResult> breakpoints;
  std::vector<int> persistentHits;
  std::vector<LossTrackState> persistentPredicted, persistentFiltered, persistentSmoothed;
  std::vector<TMatrixD> persistentTransport, persistentNoise;
  double chi2 = 0;
  int measurementDimensions = 0;
};

/// Persistent6D: one loss coordinate stays live through every downstream hit.
/// LocalMarginal retains the earlier 5D/local-joint implementation for comparisons.
class BreakpointFitter {
public:
  explicit BreakpointFitter(const KalmanAdapter& adapter) : m_adapter(adapter) {}
  FitResult fit(const std::vector<edm4hep::TrackerHit>& hits,
                const FitSettings& settings) const;
private:
  FitResult fitPersistent(const std::vector<edm4hep::TrackerHit>& hits,
                          const FitSettings& settings) const;
  const KalmanAdapter& m_adapter;
};
} // namespace breakpoint
#endif
