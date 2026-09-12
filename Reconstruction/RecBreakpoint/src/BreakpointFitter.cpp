#include "BreakpointFitter.h"
#include "RecBreakpoint/AugmentedTransport.h"
#include "TrackLikelihood.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <limits>
#include <stdexcept>

namespace breakpoint {
namespace {
struct Transition {
  TrackState predicted;
  TMatrixD sourceTargetCross{5, 5};
  TMatrixD transport{5, 5};
  TMatrixD noise{5, 5};
  TMatrixD lossTargetCross{1, 5};
  bool breakpoint = false;
  double meanLoss = 0, varianceLoss = 0, closure = 0;
};

Matrix5 array5(const TMatrixD& matrix) {
  Matrix5 result{};
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j) result[i * 5 + j] = matrix(i, j);
  return result;
}

// Resolve a per-interval prior center through the same code for every fit.
// The uncertainty and fit method are never changed by the center's origin.
FitSettings intervalSettings(const FitSettings& settings, int interval) {
  auto result = settings;
  const auto found = settings.intervalMeanLogLoss.find(interval);
  if (found != settings.intervalMeanLogLoss.end()) {
    result.meanLogLoss = found->second;
  }
  return result;
}

/// Expand only a selected source into (helix, b), b=log(p_before/p_after).
/// The loss is applied at the upstream surface before ordinary propagation.
TrackState applyBreakpoint(const TrackState& source, const FitSettings& settings,
                            TMatrixD& helixJacobian, TMatrixD& lossDerivative,
                            Matrix6& localPrior, Matrix6& localJacobian) {
  const double scale = std::exp(settings.meanLogLoss);
  TrackState result = source;
  result.mean(2, 0) *= scale;
  helixJacobian.UnitMatrix();
  helixJacobian(2, 2) = scale;
  lossDerivative(2, 0) = result.mean(2, 0);
  Vector5 derivative{};
  derivative[2] = result.mean(2, 0);
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j) localPrior[i * 6 + j] = source.covariance(i, j);
  localPrior[35] = settings.sigmaLogLoss * settings.sigmaLogLoss;
  localJacobian = AugmentedTransport::jacobian(array5(helixJacobian), derivative);
  const auto mapped = AugmentedTransport::covariance(localPrior, localJacobian, {});
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j) result.covariance(i, j) = mapped[i * 6 + j];
  validateCovariance(result.covariance);
  return result;
}

std::pair<double, double> inferLoss(const Transition& transition,
                                   const TrackState& target) {
  const TMatrixD gain = transition.lossTargetCross * inverseCovariance(transition.predicted.covariance);
  const TMatrixD correction = gain * stateDifference(target.mean, transition.predicted.mean);
  const TMatrixD varianceCorrection = gain *
      (target.covariance - transition.predicted.covariance) * transpose(gain);
  const double variance = transition.varianceLoss + varianceCorrection(0, 0);
  if (!std::isfinite(variance) || variance < -1.e-10 * transition.varianceLoss)
    throw std::runtime_error("Invalid conditional breakpoint variance");
  return {transition.meanLoss + correction(0, 0), std::max(0.0, variance)};
}
} // namespace

PairedFitResult BreakpointFitter::fit(const std::vector<edm4hep::TrackerHit>& hits,
                                     const FitSettings& settings) const {
  if (!std::isfinite(settings.backwardSeedScale) || settings.backwardSeedScale <= 0)
    throw std::invalid_argument("BackwardSeedScale must be finite and positive");
  if (settings.captureGaussianModel &&
      (settings.lossStateMode != "LocalMarginal" || settings.sigmaLogLoss != 0))
    throw std::invalid_argument("Gaussian likelihood capture requires fixed-loss LocalMarginal");
  if (settings.lossStateMode == "Persistent6D") {
    if (settings.intervals.size() > 1)
      throw std::invalid_argument("Persistent6D requires at most one breakpoint");
    // No loss coordinate is introduced in the empty-list 5D reference.
  } else if (settings.lossStateMode != "LocalMarginal") {
    throw std::invalid_argument("Unknown LossStateMode");
  }
  if (hits.size() < 3) throw std::runtime_error("Insufficient hits");
  for (int interval : settings.intervals)
    if (interval < 0 || static_cast<std::size_t>(interval + 1) >= hits.size())
      throw std::runtime_error("Configured breakpoint interval outside this track");
  for (const auto& center : settings.intervalMeanLogLoss)
    if (!std::isfinite(center.second) || center.second < 0 ||
        std::find(settings.intervals.begin(), settings.intervals.end(), center.first) == settings.intervals.end())
      throw std::invalid_argument("Invalid per-interval loss prior center");

  // Share the first forward population. Neither continuation feeds the other.
  const auto first = settings.lossStateMode == "Persistent6D" && !settings.intervals.empty()
      ? fitPersistent(hits,settings) : fitLocalRTS(hits,settings);
  auto inward = finishBackward(hits,settings,first);
  return {first,std::move(inward)};
}

FitResult BreakpointFitter::fitLocalRTS(const std::vector<edm4hep::TrackerHit>& hits,
    const FitSettings& settings, const std::vector<TrackState>* reference) const {
  FitResult result;
  auto seed = m_adapter.seed(hits, settings.seedScale);
  if (reference) {
    if (reference->size() != hits.size() || settings.sigmaLogLoss != 0)
      throw std::runtime_error("Reference fitting requires fixed loss and all hit references");
    // Discard the native seed's updated result. Start again from its ORIGINAL
    // predicted prior; no previous iteration posterior is used as a prior.
    seed = m_adapter.updateAtReference(seed.predicted, hits.front(), reference->front());
    result.reference = *reference;
  }
  result.predicted.push_back(seed.predicted);
  result.filtered.push_back(seed.filtered);
  result.localChi2.push_back(seed.chi2);
  result.chi2 = seed.chi2;
  result.measurementDimensions = seed.dimension;
  std::vector<Transition> transitions;

  // Forward pass. Every real hit is updated exactly once by native KalTest.
  for (std::size_t i = 0; i + 1 < hits.size(); ++i) {
    const auto& source = result.filtered.back();
    Transition transition;
    transition.breakpoint = std::find(settings.intervals.begin(), settings.intervals.end(), i)
        != settings.intervals.end();
    TMatrixD lossMap(5, 5), derivative(5, 1);
    lossMap.UnitMatrix();
    derivative.Zero();
    Matrix6 jointPrior{}, jointMap{};
    TrackState propagationSource = source;
    if (transition.breakpoint) {
      const auto loss = intervalSettings(settings, i);
      propagationSource = applyBreakpoint(source, loss, lossMap, derivative, jointPrior, jointMap);
      transition.meanLoss = loss.meanLogLoss;
      transition.varianceLoss = loss.sigmaLogLoss * loss.sigmaLogLoss;
    }
    MeasurementStep step;
    if (reference) {
      auto referenceSource = (*reference)[i];
      if (transition.breakpoint) {
        TMatrixD map(5, 5), d(5, 1); Matrix6 joint{}, jacobian{};
        referenceSource = applyBreakpoint(referenceSource, intervalSettings(settings, i),
                                          map, d, joint, jacobian);
      }
      step = m_adapter.advanceAtReference(propagationSource, referenceSource,
          hits[i], hits[i+1], (*reference)[i+1]);
    } else {
      step = m_adapter.advance(propagationSource, hits[i], hits[i + 1]);
    }
    transition.predicted = step.predicted;
    const TMatrixD fullHelixTransport = step.transport * lossMap;
    transition.transport = fullHelixTransport;
    transition.noise = step.noise;
    transition.sourceTargetCross = source.covariance * transpose(fullHelixTransport);

    if (transition.breakpoint) {
      const TMatrixD propagatedDerivative = step.transport * derivative;
      transition.noise += transition.varianceLoss * propagatedDerivative * transpose(propagatedDerivative);
      Vector5 column{};
      for (int j = 0; j < 5; ++j) {
        column[j] = propagatedDerivative(j, 0);
        transition.lossTargetCross(0, j) = transition.varianceLoss * column[j];
      }
      const auto fullJacobian = AugmentedTransport::jacobian(array5(fullHelixTransport), column);
      const auto predictedJoint = AugmentedTransport::covariance(
          jointPrior, fullJacobian, AugmentedTransport::processNoise(array5(step.noise)));
      // Check the independently assembled 6D prediction against the actual KF
      // marginal, including baseline process noise. All live matrices remain
      // double precision; record the largest normalized discrepancy.
      for (int j = 0; j < 5; ++j)
        for (int k = 0; k < 5; ++k) {
          const double denominator = std::sqrt(step.predicted.covariance(j, j) * step.predicted.covariance(k, k));
          transition.closure = std::max(transition.closure,
              std::abs(predictedJoint[j * 6 + k] - step.predicted.covariance(j, k)) / denominator);
        }
      if (transition.closure > 1.e-3)
        throw std::runtime_error("6D/KF covariance transport closure failed");
    }
    transitions.push_back(transition);
    result.predicted.push_back(step.predicted);
    result.filtered.push_back(step.filtered);
    result.localChi2.push_back(step.chi2);
    result.chi2 += step.chi2;
    result.measurementDimensions += step.dimension;
  }

  // Rauch-Tung-Striebel backward pass using each retained transition joint.
  // Ordinary edges stay 5D; only selected edges additionally infer their loss.
  result.smoothed = result.filtered;
  for (int i = static_cast<int>(transitions.size()) - 1; i >= 0; --i) {
    const auto& edge = transitions[i];
    const auto& downstream = result.smoothed[i + 1];
    const TMatrixD gain = edge.sourceTargetCross * inverseCovariance(edge.predicted.covariance);
    auto& smoothed = result.smoothed[i];
    smoothed.mean = result.filtered[i].mean + gain * stateDifference(downstream.mean, edge.predicted.mean);
    // Conditional-covariance (Joseph) form avoids cancellation of loose seed errors.
    TMatrixD remaining(5, 5);
    remaining.UnitMatrix();
    remaining -= gain * edge.transport;
    smoothed.covariance = remaining * result.filtered[i].covariance * transpose(remaining)
        + gain * (edge.noise + downstream.covariance) * transpose(gain);
    try { validateCovariance(smoothed.covariance); }
    catch (const std::exception& error) {
      throw std::runtime_error("Smoothed covariance at hit " + std::to_string(i) + ": " + error.what());
    }
    if (edge.breakpoint) {
      const auto local = inferLoss(edge, result.filtered[i + 1]);
      const auto allHits = inferLoss(edge, downstream);
      result.breakpoints.push_back({i, edge.meanLoss, allHits.first, allHits.second,
                                    local.first, local.second, edge.closure});
    }
  }
  std::reverse(result.breakpoints.begin(), result.breakpoints.end());
  result.ip = m_adapter.propagateToIP(result.smoothed.front(), hits.front());
  result.endpoint = result.smoothed;
  std::vector<TMatrixD> predicted, covariances, smoothed, noises;
  for (std::size_t i=0;i<hits.size();++i) {
    predicted.push_back(result.predicted[i].mean);
    covariances.push_back(result.predicted[i].covariance);
    smoothed.push_back(result.smoothed[i].mean);
    if (i) noises.push_back(transitions[i-1].noise);
  }
  scoreSmoothed(hits,result,predicted,covariances,smoothed,noises,
                reference ? *reference : result.predicted);
  if (settings.captureGaussianModel) {
    auto model = std::make_shared<GaussianTrackModel>();
    model->seedCovariance.ResizeTo(result.predicted.front().covariance);
    model->seedCovariance = result.predicted.front().covariance;
    for (std::size_t i = 0; i < hits.size(); ++i) {
      auto measurement = m_adapter.gaussianHitModel(hits[i],
          reference ? (*reference)[i] : result.predicted[i]);
      if (reference) {
        // GaussianTrackModel uses predicted-state origins, whereas H/h were
        // evaluated at a. Retain the affine intercept when changing origins.
        measurement.residual -= measurement.derivative *
            stateDifference(result.predicted[i].mean, (*reference)[i].mean);
      }
      model->hits.push_back(measurement);
      if (i) {
        const auto& edge = transitions[i - 1];
        model->transitions.push_back({edge.transport, edge.noise,
            stateDifference(result.predicted[i - 1].mean, result.filtered[i - 1].mean)});
      }
    }
    result.gaussianModel = std::move(model);
  }
  return result;
}

PairedFitResult BreakpointFitter::fitWithReference(
    const std::vector<edm4hep::TrackerHit>& hits, const FitSettings& settings,
    const ReferenceFitSettings& controls) const {
  if (!controls.maxIterations) return fit(hits, settings);
  if (settings.lossStateMode != "LocalMarginal" || settings.intervals.size() != 1 ||
      settings.sigmaLogLoss != 0 || !settings.intervalMeanLogLoss.empty())
    throw std::runtime_error("Reference iterations require one fixed-b LocalMarginal interval");
  if (!std::isfinite(controls.stateTolerance) || controls.stateTolerance <= 0 ||
      !std::isfinite(controls.objectiveTolerance) || controls.objectiveTolerance <= 0)
    throw std::runtime_error("Invalid reference convergence tolerances");
  auto capture = settings;
  capture.captureGaussianModel = true;
  // The ordinary entry point validates settings and generates the initial
  // trajectory. Its backward result is not used to update the reference.
  auto current = fit(hits, capture).rts;
  double previousObjective = evaluateTrackLikelihood(*current.gaussianModel).nll2;
  for (unsigned iteration = 1; iteration <= controls.maxIterations; ++iteration) {
    auto next = fitLocalRTS(hits, capture, &current.smoothed);
    const double objective = evaluateTrackLikelihood(*next.gaussianModel).nll2;
    double change = 0;
    for (std::size_t i = 0; i < hits.size(); ++i) {
      const auto delta = stateDifference(next.smoothed[i].mean, current.smoothed[i].mean);
      const auto& covariance = next.smoothed[i].covariance;
      for (int row = 0; row < 5; ++row) {
        change = std::max(change, std::abs(delta(row, 0)) / std::sqrt(covariance(row, row)));
        for (int col = 0; col < 5; ++col)
          change = std::max(change,
              std::abs(covariance(row, col)-current.smoothed[i].covariance(row, col)) /
              std::sqrt(covariance(row, row)*covariance(col, col)));
      }
    }
    next.referenceIterations = iteration;
    next.referenceStateChange = change;
    next.referenceObjectiveChange = std::abs(objective-previousObjective);
    if (change <= controls.stateTolerance &&
        next.referenceObjectiveChange <= controls.objectiveTolerance) {
      auto backward = finishBackward(hits, settings, next);
      if (!settings.captureGaussianModel) next.gaussianModel.reset();
      return {std::move(next), std::move(backward)};
    }
    previousObjective = objective;
    current = std::move(next);
  }
  throw std::runtime_error("Fixed-b reference trajectory did not converge after " +
      std::to_string(controls.maxIterations) + " passes; state change=" +
      std::to_string(current.referenceStateChange) + ", objective change=" +
      std::to_string(current.referenceObjectiveChange));
}

FitResult BreakpointFitter::finishBackward(const std::vector<edm4hep::TrackerHit>& hits,
    const FitSettings& settings, FitResult result) const {
    result.breakpoints.clear();
    result.gaussianModel.reset(); // the marginal likelihood uses the forward model
    result.smoothed.clear();
    result.smoothedChi2.clear(); result.smoothedMeasurementChi2.clear();
    result.smoothedProcessChi2.clear(); result.smoothedNativeMeasurementChi2.clear();
    result.smoothedChi2Status=0;
    result.persistentHits.clear();result.persistentPredicted.clear();result.persistentFiltered.clear();
    result.persistentSmoothed.clear();result.persistentTransport.clear();result.persistentNoise.clear();
    // Scale a COPY of the original first-pass forward endpoint covariance.
    // Preserve its mean and every correlation; apply the scale once.
    // This still reuses hit information; it is not an independent smoother.
    result.backwardFiltered.resize(hits.size());
    result.backwardPredicted.resize(hits.size());
    result.backwardChi2.assign(hits.size(), 0);
    auto backwardSeed = result.filtered.back();
    if (settings.backwardSeedScale != 1.0) {
      backwardSeed.covariance *= settings.backwardSeedScale;
      validateCovariance(backwardSeed.covariance);
    }
    result.backwardFiltered.back() = backwardSeed;
    result.backwardPredicted.back() = backwardSeed; // seed, no hit update

    struct PendingLoss {
      IntervalResult result;
      TMatrixD stateCross{1, 5};
    };
    std::vector<PendingLoss> pending;
    for (int i = static_cast<int>(hits.size()) - 2; i >= 0; --i) {
      const bool selected = std::find(settings.intervals.begin(), settings.intervals.end(), i)
          != settings.intervals.end();
      const auto configuredLoss = selected ? intervalSettings(settings, i) : settings;
      const auto step = m_adapter.advanceBackward(result.backwardFiltered[i + 1],
              hits[i + 1], hits[i], selected, configuredLoss.meanLogLoss, configuredLoss.sigmaLogLoss);
      if (step.covarianceClosure > 1.e-3)
        throw std::runtime_error("Inverse-breakpoint covariance closure failed");
      result.backwardPredicted[i] = step.predicted;
      result.backwardFiltered[i] = step.filtered;
      result.backwardChi2[i] = step.chi2;
      // Retain each already-crossed loss's covariance with the live state.
      // Subsequent inner hits can refine that scalar without an RTS pass.
      for (auto& loss : pending)
        loss.stateCross = loss.stateCross * transpose(step.transport);
      if (selected) {
        PendingLoss loss;
        loss.result.index = i;
        loss.result.priorLogLoss = configuredLoss.meanLogLoss;
        loss.result.fittedLogLoss = configuredLoss.meanLogLoss;
        loss.result.fittedVariance = configuredLoss.sigmaLogLoss * configuredLoss.sigmaLogLoss;
        loss.result.covarianceClosure = step.covarianceClosure;
        loss.stateCross = step.lossTargetCross;
        pending.push_back(loss);
      }
      const auto inversePrediction = inverseCovariance(step.predicted.covariance);
      for (auto& loss : pending) {
        const TMatrixD gain = loss.stateCross * inversePrediction;
        const TMatrixD delta = gain * stateDifference(step.filtered.mean, step.predicted.mean);
        const TMatrixD varianceDelta = gain *
            (step.filtered.covariance - step.predicted.covariance) * transpose(gain);
        loss.result.fittedLogLoss += delta(0, 0);
        loss.result.fittedVariance += varianceDelta(0, 0);
        if (!std::isfinite(loss.result.fittedLogLoss) ||
            !std::isfinite(loss.result.fittedVariance) || loss.result.fittedVariance < -1.e-12)
          throw std::runtime_error("Invalid backward loss posterior");
        loss.result.fittedVariance = std::max(0.0, loss.result.fittedVariance);
        loss.stateCross = gain * step.filtered.covariance;
        if (loss.result.index == i) {
          loss.result.localLogLoss = loss.result.fittedLogLoss;
          loss.result.localVariance = loss.result.fittedVariance;
        }
      }
    }
    for (auto it = pending.rbegin(); it != pending.rend(); ++it)
      result.breakpoints.push_back(it->result);
    result.endpoint = result.backwardFiltered;
    result.ip = m_adapter.propagateToIP(result.endpoint.front(), hits.front());
    return result;
}

FitResult BreakpointFitter::fitPersistent(const std::vector<edm4hep::TrackerHit>& hits,
    const FitSettings& settings) const {
  const int interval = settings.intervals.front();
  const auto loss = intervalSettings(settings, interval);
  if (hits.size() < 3 || interval < 0 || interval + 1 >= static_cast<int>(hits.size()))
    throw std::invalid_argument("Persistent6D breakpoint outside track");
  FitResult result;
  const auto seed = m_adapter.seed(hits, settings.seedScale);
  result.predicted.push_back(seed.predicted);
  result.filtered.push_back(seed.filtered);
  result.localChi2.push_back(seed.chi2);
  result.chi2 = seed.chi2;
  result.measurementDimensions = seed.dimension;

  // State dimension changes ONCE: 5 before birth, then 6 at every later hit.
  // Rectangular birth transport permits a genuine joint RTS pass back to 5D.
  std::vector<TMatrixD> predictedMean{seed.predicted.mean}, predictedCov{seed.predicted.covariance};
  std::vector<TMatrixD> filteredMean{seed.filtered.mean}, filteredCov{seed.filtered.covariance};
  std::vector<TMatrixD> transports, noises;
  LossTrackState live;
  double closure = 0;
  for (int i = 0; i + 1 < static_cast<int>(hits.size()); ++i) {
    if (i < interval) {
      const auto step = m_adapter.advance(result.filtered.back(), hits[i], hits[i + 1]);
      result.predicted.push_back(step.predicted);
      result.filtered.push_back(step.filtered);
      result.localChi2.push_back(step.chi2);
      result.chi2 += step.chi2;
      result.measurementDimensions += step.dimension;
      predictedMean.push_back(step.predicted.mean); predictedCov.push_back(step.predicted.covariance);
      filteredMean.push_back(step.filtered.mean); filteredCov.push_back(step.filtered.covariance);
      transports.push_back(step.transport); noises.push_back(step.noise);
      continue;
    }
    const bool birth = i == interval;
    if (birth) live = LossTrackState::introduce(result.filtered.back(),
        loss.meanLogLoss, loss.sigmaLogLoss * loss.sigmaLogLoss);
    const auto step = m_adapter.advancePersistent(live, hits[i], hits[i + 1], birth);
    live = step.filtered; // Entire six-dimensional posterior is the next input.
    result.predicted.push_back(step.predicted.track());
    result.filtered.push_back(step.filtered.track());
    result.localChi2.push_back(step.chi2);
    result.chi2 += step.chi2;
    result.measurementDimensions += step.dimension;
    predictedMean.push_back(step.predicted.mean); predictedCov.push_back(step.predicted.covariance);
    filteredMean.push_back(step.filtered.mean); filteredCov.push_back(step.filtered.covariance);
    result.persistentHits.push_back(i + 1);
    result.persistentPredicted.push_back(step.predicted);
    result.persistentFiltered.push_back(step.filtered);
    result.persistentTransport.push_back(step.transport);
    result.persistentNoise.push_back(step.noise);
    if (birth) {
      TMatrixD transport(6, 5), lossColumn(6, 1);
      for (int row = 0; row < 6; ++row) {
        lossColumn(row, 0) = step.transport(row, 5);
        for (int col = 0; col < 5; ++col) transport(row, col) = step.transport(row, col);
      }
      transports.push_back(transport);
      noises.push_back(step.noise + loss.sigmaLogLoss * loss.sigmaLogLoss
          * lossColumn * transpose(lossColumn));
    } else {
      transports.push_back(step.transport); noises.push_back(step.noise);
    }
    const TMatrixD reconstructed = transports.back() * filteredCov[i]
        * transpose(transports.back()) + noises.back();
    for (int row = 0; row < 6; ++row)
      for (int col = 0; col < 6; ++col)
        closure = std::max(closure, std::abs(reconstructed(row, col) - step.predicted.covariance(row, col))
            / std::sqrt(step.predicted.covariance(row, row) * step.predicted.covariance(col, col)));
    if (closure > 1.e-3) throw std::runtime_error("Persistent6D transition covariance closure failed");
  }

  auto smoothedMean = filteredMean;
  auto smoothedCov = filteredCov;
  for (int i = static_cast<int>(hits.size()) - 2; i >= 0; --i) {
    const auto& f = transports[i];
    const TMatrixD gain = filteredCov[i] * transpose(f) * inverseCovariance(predictedCov[i + 1]);
    smoothedMean[i] = filteredMean[i] + gain * stateDifference(smoothedMean[i + 1], predictedMean[i + 1]);
    TMatrixD remaining(filteredCov[i].GetNrows(), filteredCov[i].GetNrows());
    remaining.UnitMatrix();
    remaining -= gain * f;
    smoothedCov[i] = remaining * filteredCov[i] * transpose(remaining)
        + gain * (noises[i] + smoothedCov[i + 1]) * transpose(gain);
    validateCovariance(smoothedCov[i]);
  }
  for (std::size_t i = 0; i < hits.size(); ++i) {
    TrackState track = result.filtered[i];
    for (int row = 0; row < 5; ++row) {
      track.mean(row, 0) = smoothedMean[i](row, 0);
      for (int col = 0; col < 5; ++col) track.covariance(row, col) = smoothedCov[i](row, col);
    }
    result.smoothed.push_back(track);
    if (static_cast<int>(i) > interval) {
      LossTrackState joint;
      joint.pivot = track.pivot;
      joint.mean = smoothedMean[i]; joint.covariance = smoothedCov[i];
      result.persistentSmoothed.push_back(joint);
    }
  }
  const auto& local = result.persistentFiltered.front();
  // The final outward posterior has already used ALL downstream measurements.
  result.breakpoints.push_back({interval, loss.meanLogLoss, live.mean(5, 0),
      live.covariance(5, 5), local.mean(5, 0), local.covariance(5, 5), closure});
  result.endpoint = result.smoothed;
  result.ip = m_adapter.propagateToIP(result.endpoint.front(), hits.front());
  scoreSmoothed(hits,result,predictedMean,predictedCov,smoothedMean,noises,
                result.predicted);
  return result;
}

void BreakpointFitter::scoreSmoothed(const std::vector<edm4hep::TrackerHit>& hits,
    FitResult& result, const std::vector<TMatrixD>& predictedMeans,
    const std::vector<TMatrixD>& predictedCovs, const std::vector<TMatrixD>& smoothedMeans,
    const std::vector<TMatrixD>& noises, const std::vector<TrackState>& references) const {
  const double nan=std::numeric_limits<double>::quiet_NaN();
  result.smoothedChi2.assign(hits.size(),nan);
  result.smoothedMeasurementChi2.assign(hits.size(),nan);
  result.smoothedNativeMeasurementChi2.assign(hits.size(),nan);
  result.smoothedProcessChi2.assign(hits.size(),nan);
  result.smoothedTotalChi2=result.smoothedSeedChi2=nan;
  try {
    const TMatrixD seedDelta=stateDifference(smoothedMeans.front(),predictedMeans.front());
    result.smoothedSeedChi2=(transpose(seedDelta)*inverseCovariance(predictedCovs.front())*seedDelta)(0,0);
    double total=0;
    for (std::size_t i=0;i<hits.size();++i) {
      const auto measurement=m_adapter.measurementScore(hits[i],result.smoothed[i],references[i]);
      double process=0;
      if (i) {
        const TMatrixD delta=stateDifference(smoothedMeans[i],predictedMeans[i]);
        const TMatrixD u=inverseCovariance(predictedCovs[i])*delta;
        // At the RTS mean, w=Q*u, hence w^T Q^+ w = u^T Q u.
        // This exact conditional identity avoids inverting singular MS noise
        // or adding artificial noise to the static loss coordinate. Birth Q
        // contains its independent prior ONCE, also for rectangular 5->6 edges.
        process=(transpose(u)*noises[i-1]*u)(0,0);
        if (!std::isfinite(process) || process < -1.e-8)
          throw std::runtime_error("Invalid smoothed process penalty");
        process=std::max(0.,process);
      }
      result.smoothedMeasurementChi2[i]=measurement.affine;
      result.smoothedNativeMeasurementChi2[i]=measurement.native;
      result.smoothedProcessChi2[i]=process;
      result.smoothedChi2[i]=measurement.affine+process+(i==0 ? result.smoothedSeedChi2 : 0.);
      total+=result.smoothedChi2[i];
    }
    if (!std::isfinite(total) || total<0) throw std::runtime_error("Invalid complete smoothed chi2");
    result.smoothedTotalChi2=total;
    result.smoothedChi2Status=1;
  } catch (const std::exception& error) {
    // Passive diagnostics must not change endpoint publication or filtering.
    result.smoothedChi2Status=-1;
    result.smoothedChi2Error=error.what();
    result.smoothedTotalChi2=nan;
  }
}

} // namespace breakpoint
