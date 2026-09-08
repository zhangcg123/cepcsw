#include "BreakpointFitter.h"
#include "RecBreakpoint/AugmentedTransport.h"

#include <algorithm>
#include <cmath>
#include <numeric>
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

// TruthOverride replaces the selected loss response, not the hit update or
// material transport. A fixed truth loss has zero added loss variance.
FitSettings intervalSettings(const FitSettings& settings, int interval) {
  auto result = settings;
  if (settings.lossStateMode == "TruthOverride") {
    const auto found = settings.truthLogLoss.find(interval);
    if (found == settings.truthLogLoss.end() || !std::isfinite(found->second) || found->second < 0)
      throw std::runtime_error("Missing/invalid truth loss for configured interval");
    result.meanLogLoss = found->second;
    result.sigmaLogLoss = 0;
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

FitResult BreakpointFitter::fit(const std::vector<edm4hep::TrackerHit>& hits,
                               const FitSettings& settings) const {
  const bool iteratedBackward = settings.lossStateMode == "LocalMarginal"
      && settings.backwardMode == "BackwardFilter";
  if (settings.maxFitIterations < 1 || settings.maxFitIterations > 20 ||
      !std::isfinite(settings.relinearizationTolerance) || settings.relinearizationTolerance <= 0)
    throw std::invalid_argument("Invalid breakpoint iteration limit/tolerance");
  if (settings.maxFitIterations > 1) {
    if (settings.intervals.size() != 1 || (!iteratedBackward &&
        !(settings.lossStateMode == "Persistent6D" && settings.backwardMode == "RTS")))
      throw std::invalid_argument("Iterations require one interval and Persistent6D/RTS or LocalMarginal/BackwardFilter");
    if (iteratedBackward) return fitIterated(hits, settings);
  }
  if (settings.lossStateMode == "Persistent6D") {
    if (settings.backwardMode != "RTS" || settings.intervals.size() > 1)
      throw std::invalid_argument("Persistent6D requires RTS and at most one breakpoint");
    if (!settings.intervals.empty()) return fitIterated(hits, settings);
    // No loss coordinate is introduced in the empty-list 5D reference.
  } else if (settings.lossStateMode != "LocalMarginal" && settings.lossStateMode != "TruthOverride") {
    throw std::invalid_argument("Unknown LossStateMode");
  }
  if (settings.backwardMode != "RTS" && settings.backwardMode != "BackwardFilter")
    throw std::invalid_argument("BackwardMode must be RTS or BackwardFilter");
  if (hits.size() < 3) throw std::runtime_error("Insufficient hits");
  for (int interval : settings.intervals)
    if (interval < 0 || static_cast<std::size_t>(interval + 1) >= hits.size())
      throw std::runtime_error("Configured breakpoint interval outside this track");

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

  if (settings.backwardMode == "BackwardFilter")
    return finishBackward(hits, settings, std::move(result));

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
  result.ip = m_adapter.atIP(result.smoothed.front(), hits.front());
  result.endpoint = result.smoothed;
  return result;
}

FitResult BreakpointFitter::finishBackward(const std::vector<edm4hep::TrackerHit>& hits,
    const FitSettings& settings, FitResult result, const FitResult* reference) const {
    result.breakpoints.clear();
    // Copy the full outward posterior without inflation. This deliberately
    // reuses hit information; it is not an independent two-filter smoother.
    result.backwardFiltered.resize(hits.size());
    result.backwardPredicted.resize(hits.size());
    result.backwardChi2.assign(hits.size(), 0);
    result.backwardFiltered.back() = result.filtered.back();
    result.backwardPredicted.back() = result.filtered.back(); // seed, no hit update

    struct PendingLoss {
      IntervalResult result;
      TMatrixD stateCross{1, 5};
    };
    std::vector<PendingLoss> pending;
    for (int i = static_cast<int>(hits.size()) - 2; i >= 0; --i) {
      const bool selected = std::find(settings.intervals.begin(), settings.intervals.end(), i)
          != settings.intervals.end();
      const auto configuredLoss = selected ? intervalSettings(settings, i) : settings;
      const auto step = reference ? m_adapter.advanceBackwardRelinearized(
          result.backwardFiltered[i + 1], hits[i + 1], hits[i], selected,
          settings.meanLogLoss, settings.sigmaLogLoss, reference->breakpoints.front().fittedLogLoss,
          reference->backwardFiltered[i + 1], reference->backwardFiltered[i])
          : m_adapter.advanceBackward(result.backwardFiltered[i + 1],
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
    const FitSettings& settings, const FitResult* reference, const TrackState* originalPrior) const {
  const int interval = settings.intervals.front();
  if (hits.size() < 3 || interval < 0 || interval + 1 >= static_cast<int>(hits.size()))
    throw std::invalid_argument("Persistent6D breakpoint outside track");
  FitResult result;
  const auto seed = reference ? m_adapter.seedRelinearized(*originalPrior,hits.front(),reference->smoothed.front())
                             : m_adapter.seed(hits, settings.seedScale);
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
      const auto step = reference ? m_adapter.advanceRelinearized(result.filtered.back(),hits[i],hits[i+1],
          reference->smoothed[i],reference->smoothed[i+1])
          : m_adapter.advance(result.filtered.back(), hits[i], hits[i + 1]);
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
        settings.meanLogLoss, settings.sigmaLogLoss * settings.sigmaLogLoss);
    LossTrackState expansion;
    if (reference) expansion = birth ? LossTrackState::introduce(reference->smoothed[i],
        reference->breakpoints.front().fittedLogLoss,settings.sigmaLogLoss*settings.sigmaLogLoss)
        : reference->persistentSmoothed[i-interval-1];
    const auto step = reference ? m_adapter.advanceRelinearized(live,hits[i],hits[i+1],birth,
        expansion,reference->smoothed[i+1]) : m_adapter.advancePersistent(live, hits[i], hits[i + 1], birth);
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
      noises.push_back(step.noise + settings.sigmaLogLoss * settings.sigmaLogLoss
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
  result.breakpoints.push_back({interval, settings.meanLogLoss, live.mean(5, 0),
      live.covariance(5, 5), local.mean(5, 0), local.covariance(5, 5), closure});
  result.endpoint = result.smoothed;
  result.ip = m_adapter.atIP(result.endpoint.front(), hits.front());
  return result;
}

FitResult BreakpointFitter::fitIterated(const std::vector<edm4hep::TrackerHit>& hits,
                                       const FitSettings& settings) const {
  const bool backward = settings.backwardMode == "BackwardFilter";
  auto onePassSettings = settings;
  onePassSettings.maxFitIterations = 1;
  auto current = backward ? fit(hits, onePassSettings) : fitPersistent(hits, settings);
  // The backward experiment iterates only the inward fit. Its complete
  // original forward posterior is frozen, not regenerated from inward output.
  const FitResult fixedForward = backward ? current : FitResult{};
  const TrackState originalPrior=current.predicted.front();
  const auto onePass=current.ip;
  // History is stored separately: replacing the live fit cannot accumulate
  // old posterior covariances, loss priors or measurement contributions.
  std::vector<double> pts,losses,variances,norms,chi2;
  auto record=[&](const FitResult& fit,double step) {
    // The algorithm converts inverse curvature to GeV using its field value.
    pts.push_back(1.0/std::abs(fit.ip.omega)); // converted to GeV by the algorithm
    losses.push_back(fit.breakpoints.front().fittedLogLoss);
    variances.push_back(fit.breakpoints.front().fittedVariance);
    norms.push_back(step);
    chi2.push_back(backward ? std::accumulate(fit.backwardChi2.begin(), fit.backwardChi2.end(), 0.)
                           : fit.chi2);
  };
  record(current,0);
  int status=settings.maxFitIterations>1 ? 2 : 0;
  std::string error;
  for(int iteration=1;iteration<settings.maxFitIterations;++iteration) {
    try {
      auto next = backward ? finishBackward(hits, settings, fixedForward, &current)
          : fitPersistent(hits,settings,&current,&originalPrior);
      double change=0;
      for(std::size_t i=0;i<hits.size();++i) {
        const auto delta=stateDifference(next.endpoint[i].mean,current.endpoint[i].mean);
        for(int j=0;j<5;++j) change=std::max(change,std::abs(delta(j,0))
            /std::sqrt(current.endpoint[i].covariance(j,j)));
      }
      change=std::max(change,std::abs(next.breakpoints.front().fittedLogLoss-
          current.breakpoints.front().fittedLogLoss)/std::sqrt(current.breakpoints.front().fittedVariance));
      if (!std::isfinite(change)) throw std::runtime_error("Nonfinite iteration convergence metric");
      record(next,change);
      current=std::move(next);
      if(change<settings.relinearizationTolerance) { status=1; break; }
    } catch(const std::exception& failure) {
      status=-1; error=failure.what(); break; // retain last completed fit, explicitly tagged
    }
  }
  current.onePassIP=onePass;
  current.iterationStatus=status;
  current.iterationError=error;
  current.iterationInverseAbsOmega=std::move(pts);
  current.iterationLoss=std::move(losses);
  current.iterationLossVariance=std::move(variances);
  current.iterationStepNorm=std::move(norms);
  current.iterationLinearizedChi2=std::move(chi2);
  return current;
}
} // namespace breakpoint
