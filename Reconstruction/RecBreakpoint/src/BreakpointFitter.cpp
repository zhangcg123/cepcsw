#include "BreakpointFitter.h"
#include "RecBreakpoint/AugmentedTransport.h"

#include <algorithm>
#include <cmath>
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
      propagationSource = applyBreakpoint(source, settings, lossMap, derivative, jointPrior, jointMap);
      transition.meanLoss = settings.meanLogLoss;
      transition.varianceLoss = settings.sigmaLogLoss * settings.sigmaLogLoss;
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

  if (settings.backwardMode == "BackwardFilter") {
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
      const bool selected = transitions[i].breakpoint;
      const auto step = m_adapter.advanceBackward(result.backwardFiltered[i + 1],
          hits[i + 1], hits[i], selected, settings.meanLogLoss, settings.sigmaLogLoss);
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
        loss.result.priorLogLoss = settings.meanLogLoss;
        loss.result.fittedLogLoss = settings.meanLogLoss;
        loss.result.fittedVariance = settings.sigmaLogLoss * settings.sigmaLogLoss;
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

  // Rauch-Tung-Striebel backward pass using each retained transition joint.
  // Ordinary edges stay 5D; only selected edges additionally infer their loss.
  result.smoothed = result.filtered;
  for (int i = static_cast<int>(transitions.size()) - 1; i >= 0; --i) {
    const auto& edge = transitions[i];
    const auto& downstream = result.smoothed[i + 1];
    const TMatrixD gain = edge.sourceTargetCross * inverseCovariance(edge.predicted.covariance);
    auto& smoothed = result.smoothed[i];
    smoothed.mean = result.filtered[i].mean + gain * stateDifference(downstream.mean, edge.predicted.mean);
    // Conditional-covariance (Joseph) form avoids subtracting almost equal,
    // very large seed covariances at the first few measurement surfaces.
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
} // namespace breakpoint
