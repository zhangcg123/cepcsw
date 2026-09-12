#ifndef RECBREAKPOINT_FITTER_H
#define RECBREAKPOINT_FITTER_H

#include "KalmanAdapter.h"
#include <vector>
#include <map>

namespace breakpoint {
struct FitSettings {
  std::vector<int> intervals; // i selects the outward transition hit[i] -> hit[i+1]
  double meanLogLoss = 0;
  double sigmaLogLoss = 0.001;
  double seedScale = 1;
  double backwardSeedScale = 100; // scales the full first-forward endpoint covariance
  std::string lossStateMode = "LocalMarginal";
  std::string lossPriorMode = "Gaussian"; // Unconstrained: zero b prior precision
  // Optional per-interval prior centers. All intervals retain sigmaLogLoss;
  // the fitter does not distinguish truth-supplied from manually set centers.
  std::map<int, double> intervalMeanLogLoss;
  // Internal request from FreeLossFitter only; ordinary/oracle passes leave off.
  bool captureGaussianModel = false;
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
  // Complete final-pass quadratic objective, indexed by receiving hit.
  // Process includes the loss birth prior ONCE; no second b penalty is added.
  std::vector<double> smoothedChi2, smoothedMeasurementChi2, smoothedProcessChi2;
  std::vector<double> smoothedNativeMeasurementChi2;
  double smoothedSeedChi2 = 0, smoothedTotalChi2 = 0;
  int smoothedChi2Status = 0;
  std::string smoothedChi2Error;
  std::vector<IntervalResult> breakpoints;
  std::vector<int> persistentHits;
  std::vector<LossTrackState> persistentPredicted, persistentFiltered, persistentSmoothed;
  std::vector<TMatrixD> persistentTransport, persistentNoise;
  double chi2 = 0;
  int measurementDimensions = 0;
  std::shared_ptr<const GaussianTrackModel> gaussianModel;
  // Unconstrained mode: flat arrays are hit-major, five Cov(state,b) entries
  // per hit. Predictions with a still-diffuse loss have NaN covariance and
  // predictionValid=0; no finite uncertainty is invented for those states.
  std::vector<double> stateLossCovariance;
  std::vector<int> predictionValid, filteredValid;
  double lossInformation = 0, lossReference = 0, lossChi2Closure = 0;
};

struct PairedFitResult {
  FitResult rts;
  FitResult backward;
};

/// Persistent6D: one loss coordinate stays live through every downstream hit.
/// LocalMarginal retains the earlier 5D/local-joint implementation for comparisons.
class BreakpointFitter {
public:
  explicit BreakpointFitter(const KalmanAdapter& adapter) : m_adapter(adapter) {}
  PairedFitResult fit(const std::vector<edm4hep::TrackerHit>& hits,
                const FitSettings& settings) const;
private:
  FitResult fitLocalRTS(const std::vector<edm4hep::TrackerHit>& hits,
                       const FitSettings& settings) const;
  FitResult fitPersistent(const std::vector<edm4hep::TrackerHit>& hits,
                          const FitSettings& settings) const;
  void scoreSmoothed(const std::vector<edm4hep::TrackerHit>& hits, FitResult& result,
      const std::vector<TMatrixD>& predictedMeans, const std::vector<TMatrixD>& predictedCovs,
      const std::vector<TMatrixD>& smoothedMeans, const std::vector<TMatrixD>& noises,
      const std::vector<TrackState>& measurementReferences) const;
  FitResult finishBackward(const std::vector<edm4hep::TrackerHit>& hits,
      const FitSettings& settings, FitResult result) const;
  const KalmanAdapter& m_adapter;
};
} // namespace breakpoint
#endif
