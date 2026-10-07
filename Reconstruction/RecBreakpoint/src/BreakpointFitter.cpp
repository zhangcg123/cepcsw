#include "BreakpointFitter.h"
#include "DiffuseLossState.h"
#include "RecBreakpoint/AugmentedTransport.h"

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
  if (settings.absoluteLossPrior)
    throw std::invalid_argument("Ordinary paired fitter cannot consume an absolute-loss prior");
  if (!std::isfinite(settings.backwardSeedScale) || settings.backwardSeedScale <= 0)
    throw std::invalid_argument("BackwardSeedScale must be finite and positive");
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

FitResult BreakpointFitter::fitAbsoluteLossRTS(const std::vector<edm4hep::TrackerHit>& hits,
    const FitSettings& settings, double energy, double sigmaEnergy,
    const TrackState* lossReference) const {
  if (settings.intervals.size() != 1 ||
      !std::isfinite(energy) || energy <= 0 ||
      !std::isfinite(sigmaEnergy) || sigmaEnergy <= 0)
    throw std::invalid_argument("Absolute-loss RTS requires one interval and positive energy/error");
  auto absolute = settings;
  absolute.absoluteLossPrior = std::make_pair(energy, sigmaEnergy);
  return fitPersistent(hits, absolute, lossReference);
}

FitResult BreakpointFitter::fitLocalRTS(const std::vector<edm4hep::TrackerHit>& hits,
                                      const FitSettings& settings) const {
  FitResult result;
  const auto seed = m_adapter.seed(hits, settings.seedScale);
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
    const auto step = m_adapter.advance(propagationSource, hits[i], hits[i + 1]);
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
  scoreSmoothed(hits,result,predicted,covariances,smoothed,noises,result.predicted);
  if (settings.captureGaussianModel) {
    try {
      auto model = std::make_shared<GaussianTrackModel>();
      model->seedCovariance.ResizeTo(result.predicted.front().covariance);
      model->seedCovariance = result.predicted.front().covariance;
      for (std::size_t i = 0; i < hits.size(); ++i) {
        model->hits.push_back(m_adapter.gaussianHitModel(hits[i], result.predicted[i]));
        if (i) {
          const auto& edge = transitions[i - 1];
          model->transitions.push_back({edge.transport, edge.noise,
              stateDifference(result.predicted[i - 1].mean, result.filtered[i - 1].mean)});
        }
      }
      result.gaussianModel = std::move(model);
    } catch (const std::exception& error) {
      result.gaussianModelError = error.what();
    }
  }
  return result;
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
    const FitSettings& settings, const TrackState* lossReference) const {
  const int interval = settings.intervals.front();
  const auto loss = intervalSettings(settings, interval);
  const bool absolute = settings.absoluteLossPrior.has_value();
  const double priorMean = absolute ? settings.absoluteLossPrior->first : loss.meanLogLoss;
  const double priorSigma = absolute ? settings.absoluteLossPrior->second : loss.sigmaLogLoss;
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
        priorMean, priorSigma * priorSigma);
    const auto step = m_adapter.advancePersistent(live, hits[i], hits[i + 1], birth,
        absolute ? LossCoordinate::AbsoluteMomentum : LossCoordinate::LogRatio,
        birth ? lossReference : nullptr);
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
      noises.push_back(step.noise + priorSigma * priorSigma
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
  if (absolute) {
    const auto& smoothedLoss = result.persistentSmoothed.front();
    result.absoluteLoss = AbsoluteLossResult{interval, priorMean, priorSigma,
        smoothedLoss.mean(5, 0), smoothedLoss.covariance(5, 5),
        live.mean(5, 0), live.covariance(5, 5)};
  } else
    result.breakpoints.push_back({interval, loss.meanLogLoss, live.mean(5, 0),
        live.covariance(5, 5), local.mean(5, 0), local.covariance(5, 5), closure});
  result.endpoint = result.smoothed;
  result.ip = m_adapter.propagateToIP(result.endpoint.front(), hits.front());
  scoreSmoothed(hits,result,predictedMean,predictedCov,smoothedMean,noises,
                result.predicted);
  if (settings.captureGaussianModel) {
    try {
      auto model = std::make_shared<GaussianTrackModel>();
      model->seedCovariance.ResizeTo(predictedCov.front());
      model->seedCovariance = predictedCov.front();
      for (std::size_t i = 0; i < hits.size(); ++i) {
        auto hitModel = m_adapter.gaussianHitModel(hits[i], result.predicted[i]);
        const int dimensions = predictedMean[i].GetNrows();
        if (dimensions == 6) {
          TMatrixD derivative(hitModel.derivative.GetNrows(), dimensions);
          derivative.Zero();
          for (int row = 0; row < derivative.GetNrows(); ++row)
            for (int column = 0; column < 5; ++column)
              derivative(row, column) = hitModel.derivative(row, column);
          hitModel.derivative.ResizeTo(derivative);
          hitModel.derivative = derivative; // hit observes helix, not latent b directly
        }
        model->hits.push_back(std::move(hitModel));
        if (i) model->transitions.push_back({transports[i - 1], noises[i - 1],
            stateDifference(predictedMean[i - 1], filteredMean[i - 1])});
      }
      result.gaussianModel = std::move(model);
    } catch (const std::exception& error) {
      result.gaussianModelError = error.what();
    }
  }
  return result;
}

FitResult
BreakpointFitter::fitDiffuseRTS(const std::vector<edm4hep::TrackerHit> &hits,
                                const FitSettings &settings) const {
  if (settings.intervals.size() != 1 || hits.size() < 3)
    throw std::invalid_argument(
        "DiffuseAugmentedRTS requires exactly one selected interval");
  const int interval = settings.intervals.front();
  if (interval < 0 || interval + 1 >= static_cast<int>(hits.size()))
    throw std::invalid_argument("DiffuseAugmentedRTS interval outside track");
  FitResult result;
  result.freeLossParameterCount = 1;
  const auto seed = m_adapter.seed(hits, settings.seedScale);
  result.predicted.push_back(seed.predicted);
  result.filtered.push_back(seed.filtered);
  result.localChi2.push_back(seed.chi2);
  result.chi2 = seed.chi2;
  result.measurementDimensions = seed.dimension;
  std::vector<TMatrixD> predictedMean{seed.predicted.mean},
      predictedCov{seed.predicted.covariance};
  std::vector<TMatrixD> filteredMean{seed.filtered.mean},
      filteredCov{seed.filtered.covariance};
  std::vector<TMatrixD> predictedDiffuse, filteredDiffuse, transports, noises;
  TMatrixD zero5(5, 1);
  zero5.Zero();
  predictedDiffuse.push_back(zero5);
  filteredDiffuse.push_back(zero5);
  LossTrackState live;
  TMatrixD liveDirection(6, 1);
  liveDirection.Zero();
  double firstHitB = 0, firstHitVariance = 0;

  for (int i = 0; i + 1 < static_cast<int>(hits.size()); ++i) {
    const char *stage = "prefix";
    try {
      if (i < interval) {
        const auto step =
            m_adapter.advance(result.filtered.back(), hits[i], hits[i + 1]);
        result.predicted.push_back(step.predicted);
        result.filtered.push_back(step.filtered);
        result.localChi2.push_back(step.chi2);
        result.chi2 += step.chi2;
        result.measurementDimensions += step.dimension;
        predictedMean.push_back(step.predicted.mean);
        predictedCov.push_back(step.predicted.covariance);
        filteredMean.push_back(step.filtered.mean);
        filteredCov.push_back(step.filtered.covariance);
        predictedDiffuse.push_back(zero5);
        filteredDiffuse.push_back(zero5);
        transports.push_back(step.transport);
        noises.push_back(step.noise);
        continue;
      }
      const bool birth = i == interval;
      if (birth) {
        // The finite variance 1 is ONLY a decomposition reference for P*.
        // e_b carries the genuinely unbounded direction and is never replaced
        // with a huge finite SigmaLogLoss.
        live = LossTrackState::introduce(result.filtered.back(), 0., 1.);
        liveDirection.Zero();
        liveDirection(5, 0) = 1.;
      }
      LossMeasurementStep step;
      TMatrixD targetDirection(6, 1);
      targetDirection.Zero();
      if (birth || liveDirection(5, 0) != 0.) {
        // Native propagation owns geometry and process noise. Before the first
        // informative hit, only the scalar Kalman measurement update is
        // diffuse.
        stage = "diffuse prediction";
        step = m_adapter.predictPersistent(live, hits[i], hits[i + 1], birth);
        targetDirection = step.transport * liveDirection;
        auto model =
            m_adapter.gaussianHitModel(hits[i + 1], step.predicted.track());
        TMatrixD derivative(model.derivative.GetNrows(), 6);
        derivative.Zero();
        for (int row = 0; row < derivative.GetNrows(); ++row)
          for (int col = 0; col < 5; ++col)
            derivative(row, col) = model.derivative(row, col);
        DiffuseLossState diffuse;
        diffuse.mean = step.predicted.mean;
        diffuse.finiteCovariance = step.predicted.covariance;
        diffuse.diffuseDirection = targetDirection;
        stage = "diffuse measurement";
        const auto score =
            updateDiffuseHit(diffuse, derivative, model.noise, model.residual);
        step.filtered = step.predicted;
        step.filtered.mean = diffuse.mean;
        step.filtered.covariance = diffuse.finiteCovariance;
        liveDirection = diffuse.diffuseDirection;
        step.chi2 = score.finiteChi2;
        step.dimension = derivative.GetNrows();
      } else {
        stage = "finite 6D measurement";
        step = m_adapter.advancePersistent(live, hits[i], hits[i + 1], false);
        liveDirection.Zero();
      }
      live = step.filtered;
      result.predicted.push_back(step.predicted.track());
      result.filtered.push_back(step.filtered.track());
      result.localChi2.push_back(step.chi2);
      result.chi2 += step.chi2;
      result.measurementDimensions += step.dimension;
      predictedMean.push_back(step.predicted.mean);
      predictedCov.push_back(step.predicted.covariance);
      filteredMean.push_back(step.filtered.mean);
      filteredCov.push_back(step.filtered.covariance);
      predictedDiffuse.push_back(targetDirection);
      filteredDiffuse.push_back(liveDirection);
      result.persistentHits.push_back(i + 1);
      result.persistentPredicted.push_back(step.predicted);
      result.persistentFiltered.push_back(step.filtered);
      result.persistentPredictedDiffuse.push_back(targetDirection(5, 0) != 0.);
      result.persistentFilteredDiffuse.push_back(liveDirection(5, 0) != 0.);
      result.persistentTransport.push_back(step.transport);
      result.persistentNoise.push_back(step.noise);
      if (birth) {
        TMatrixD rectangular(6, 5);
        for (int row = 0; row < 6; ++row)
          for (int col = 0; col < 5; ++col)
            rectangular(row, col) = step.transport(row, col);
        transports.push_back(rectangular);
        firstHitB = step.filtered.mean(5, 0);
        firstHitVariance = step.filtered.covariance(5, 5);
      } else
        transports.push_back(step.transport);
      // P* at birth includes the arbitrary unit reference variance of b;
      // retain it here so the Joseph RTS form closes the finite covariance.
      noises.push_back(
          birth ? TMatrixD(step.noise +
                           targetDirection * transpose(targetDirection))
                : step.noise);
    } catch (const std::exception &error) {
      throw std::runtime_error("Diffuse interval " + std::to_string(interval) +
                               " transition " + std::to_string(i) + " " +
                               stage + ": " + error.what());
    }
  }
  if ((liveDirection * transpose(liveDirection))(5, 5) > 0.)
    throw std::runtime_error(
        "Diffuse loss is unidentifiable from downstream hits");

  auto smoothedMean = filteredMean;
  auto smoothedCov = filteredCov;
  for (int i = static_cast<int>(hits.size()) - 2; i >= 0; --i) {
    try {
      const auto &map = transports[i];
      const TMatrixD gain =
          diffuseRtsGain(filteredCov[i], filteredDiffuse[i], map,
                         predictedCov[i + 1], predictedDiffuse[i + 1]);
      smoothedMean[i] =
          filteredMean[i] +
          gain * stateDifference(smoothedMean[i + 1], predictedMean[i + 1]);
      smoothedCov[i] = diffuseRtsCovariance(filteredCov[i], gain, map,
                                            noises[i], smoothedCov[i + 1]);
    } catch (const std::exception &error) {
      throw std::runtime_error("Diffuse RTS hit " + std::to_string(i) + ": " +
                               error.what());
    }
  }
  for (std::size_t i = 0; i < hits.size(); ++i) {
    TrackState track = result.filtered[i];
    for (int row = 0; row < 5; ++row) {
      track.mean(row, 0) = smoothedMean[i](row, 0);
      for (int col = 0; col < 5; ++col)
        track.covariance(row, col) = smoothedCov[i](row, col);
    }
    result.smoothed.push_back(track);
    if (static_cast<int>(i) > interval) {
      LossTrackState joint;
      joint.pivot = track.pivot;
      joint.mean = smoothedMean[i];
      joint.covariance = smoothedCov[i];
      result.persistentSmoothed.push_back(joint);
    }
  }
  const int first = interval + 1;
  result.breakpoints.push_back({interval, 0., smoothedMean[first](5, 0),
                                smoothedCov[first](5, 5), firstHitB,
                                firstHitVariance, 0.});
  result.endpoint = result.smoothed;
  result.ip = m_adapter.propagateToIP(result.endpoint.front(), hits.front());
  // A flat-prior rank-one filter has no normalized absolute NLL on b.
  // Its finite innovation chi2 omits the diffuse-consuming scalar; do not
  // compare that number with the Gaussian-prior ordinary chi2/NLL.
  result.smoothedChi2Status = 0;
  result.smoothedChi2Error =
      "Absolute score undefined for flat diffuse b prior";
  result.smoothedTotalChi2 = std::numeric_limits<double>::quiet_NaN();
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
