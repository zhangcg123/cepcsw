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
  double chi2 = 0;
  int measurementDimensions = 0;
};

/// Five-dimensional filter with a local six-dimensional joint at each
/// configured breakpoint. Independent local losses are marginalized for live
/// propagation and estimated using the retained joint in the backward smoother.
class BreakpointFitter {
public:
  explicit BreakpointFitter(const KalmanAdapter& adapter) : m_adapter(adapter) {}
  FitResult fit(const std::vector<edm4hep::TrackerHit>& hits,
                const FitSettings& settings) const;
private:
  const KalmanAdapter& m_adapter;
};
} // namespace breakpoint
#endif
