#include "FreeLossFitter.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/Minimizer.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <stdexcept>

namespace breakpoint {

FreeLossFitResult FreeLossFitter::fit(const std::vector<edm4hep::TrackerHit>& hits,
    const FitSettings& ordinary, const FreeLossSettings& controls) const {
  FreeLossFitResult result;
  auto& diagnostic = result.diagnostics;
  if (ordinary.intervals.empty()) {
    diagnostic.status = FreeLossStatus::NoInterval;
    return result;
  }
  if (ordinary.intervals.size() != 1 || ordinary.lossStateMode != "LocalMarginal") {
    diagnostic.status = FreeLossStatus::Unsupported;
    diagnostic.error = "FreeLossFit supports one selected LocalMarginal interval";
    return result;
  }
  diagnostic.interval = ordinary.intervals.front();
  diagnostic.priorSigmaLogLoss = ordinary.sigmaLogLoss;
  try {
    if (!std::isfinite(controls.maxLogLoss) || controls.maxLogLoss <= 0 || controls.maxLogLoss > 5 ||
        !controls.maxCallsPerStart || !std::isfinite(controls.tolerance) || controls.tolerance <= 0)
      throw std::invalid_argument("Invalid free-loss optimization settings");
    if (!std::isfinite(ordinary.sigmaLogLoss) || ordinary.sigmaLogLoss <= 0)
      throw std::invalid_argument("FreeLossFit requires a finite positive SigmaLogLoss");

    // Minuit varies the loss PRIOR CENTER, not a fixed loss. Every trial and
    // the final refit retain the SAME sigma as the ordinary/truth-prior fits.
    // Clear per-interval centers so no external/truth amount overrides Minuit.
    auto settings = ordinary;
    settings.intervalMeanLogLoss.clear();
    settings.captureGaussianModel = true;
    std::map<double, FreeLossTrial> cache; // scalar records only, never cached trajectories
    FreeLossTrialPhase phase = FreeLossTrialPhase::Scan;
    auto evaluate = [&](double priorMean, bool force = false) {
      if (!force && cache.count(priorMean)) return cache.at(priorMean);
      FreeLossTrial trial;
      trial.b = priorMean;
      trial.phase = phase;
      trial.likelihood.nll2 = 1.e20;
      trial.objectiveNll2 = 1.e20;
      try {
        settings.meanLogLoss = priorMean;
        const auto pair = m_fitter.fit(hits, settings);
        const auto& rts = pair.rts;
        if (rts.smoothedChi2Status != 1 || rts.breakpoints.size() != 1 ||
            rts.breakpoints.front().priorLogLoss != priorMean ||
            !std::isfinite(rts.breakpoints.front().fittedLogLoss) ||
            !std::isfinite(rts.breakpoints.front().fittedVariance) ||
            rts.breakpoints.front().fittedVariance < 0 || !rts.gaussianModel)
          throw std::runtime_error("Gaussian-prior free-loss fit invariant failed");
        // Score this trial's existing RTS trajectory: measurement + process
        // + seed chi2, then the SAME joint-measurement normalization. No new
        // filter pass, reference iteration, or backward chi2 is added.
        trial.likelihood = evaluateSmoothedTrackLikelihood(
            *rts.gaussianModel, rts.smoothedTotalChi2);
        if (controls.beamSpot)
          trial.beam = evaluateBeamSpotLikelihood(rts.ip, m_bz, *controls.beamSpot);
        trial.objectiveNll2 = trial.likelihood.nll2 + trial.beam.nll2;
        trial.valid = std::isfinite(trial.objectiveNll2) &&
                      std::isfinite(rts.ip.omega) && rts.ip.omega != 0;
        if (!trial.valid) throw std::runtime_error("Nonfinite free-loss candidate");
      } catch (const std::exception& error) {
        trial.error = error.what();
        trial.likelihood.nll2 = 1.e20;
        trial.objectiveNll2 = 1.e20;
      }
      cache[priorMean] = trial;
      diagnostic.trials.push_back(trial);
      return trial;
    };

    // Preserve the tested blind scan and multistart MIGRAD strategy. The grid
    // is scaled only when the user changes the upper b bound from one.
    FreeLossTrial best;
    best.objectiveNll2 = 1.e20;
    for (double fraction : {0., .0005, .001, .002, .005, .01, .02, .04, .08, .15, .3, .5, .75, 1.}) {
      const auto trial = evaluate(fraction * controls.maxLogLoss);
      if (trial.valid && trial.objectiveNll2 < best.objectiveNll2) best = trial;
    }
    if (!best.valid) throw std::runtime_error("No valid free-loss trial");
    const double scanBest = best.b;
    // The coarse scan seeds Minuit; it is not a successful optimization.
    best.valid = false;
    best.objectiveNll2 = 1.e20;
    for (double start : {scanBest, std::min(.005, controls.maxLogLoss), std::min(.05, controls.maxLogLoss)}) {
      std::unique_ptr<ROOT::Math::Minimizer> minimizer(ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad"));
      if (!minimizer) throw std::runtime_error("Minuit2 is unavailable");
      ROOT::Math::Functor objective([&](const double* b) { return evaluate(b[0]).objectiveNll2; }, 1);
      phase = FreeLossTrialPhase::Minimize;
      minimizer->SetFunction(objective);
      minimizer->SetMaxFunctionCalls(controls.maxCallsPerStart);
      minimizer->SetTolerance(controls.tolerance);
      minimizer->SetPrecision(1.e-8);
      minimizer->SetErrorDef(1.);
      minimizer->SetPrintLevel(0);
      minimizer->SetLimitedVariable(0, "mean_log_loss", start, std::min(.002, .02 * controls.maxLogLoss), 0., controls.maxLogLoss);
      const bool converged = minimizer->Minimize();
      const auto trial = evaluate(minimizer->X()[0]);
      if (converged && minimizer->Status() == 0 && std::isfinite(minimizer->Edm()) &&
          trial.valid && trial.objectiveNll2 <= best.objectiveNll2) {
        best = trial;
        diagnostic.minuitStatus = minimizer->Status();
        diagnostic.edm = minimizer->Edm();
        diagnostic.bError = minimizer->Errors() ? minimizer->Errors()[0]
                                               : std::numeric_limits<double>::quiet_NaN();
      }
    }
    if (!best.valid) throw std::runtime_error("No converged valid Minuit minimum");
    phase = FreeLossTrialPhase::Repeat;
    const auto repeated = evaluate(best.b, true);
    if (!repeated.valid ||
        std::abs(repeated.objectiveNll2 - best.objectiveNll2) > 1.e-7 ||
        std::abs(repeated.likelihood.nll2 - best.likelihood.nll2) > 1.e-7)
      throw std::runtime_error("Free-loss minimum is not reproducible");

    // Diagnostic scan only: it must not silently replace the Minuit result.
    phase = FreeLossTrialPhase::LocalScan;
    for (int step = -10; step <= 10; ++step) {
      const double b = best.b + step * .001;
      if (b >= 0 && b <= controls.maxLogLoss) evaluate(b);
    }
    diagnostic.b = best.b;
    diagnostic.likelihood = best.likelihood;
    diagnostic.beam = best.beam;
    diagnostic.objectiveNll2 = best.objectiveNll2;
    diagnostic.lowerBound = best.b <= 1.e-6;
    diagnostic.upperBound = best.b >= controls.maxLogLoss - 1.e-6;

    // Refit with the optimized center AND configured sigma. The shared fitter
    // supplies posterior loss variance and track/loss correlations. The Minuit
    // error on the center is a different quantity and is not added again.
    settings.meanLogLoss = best.b;
    settings.captureGaussianModel = false;
    result.fitted.emplace(m_fitter.fit(hits, settings));
    for (const auto* endpoint : {&result.fitted->rts.ip, &result.fitted->backward.ip})
      if (!std::isfinite(endpoint->omega) || endpoint->omega == 0)
        throw std::runtime_error("Invalid free-loss endpoint curvature");
    diagnostic.status = FreeLossStatus::Applied;
  } catch (const std::exception& error) {
    diagnostic.status = FreeLossStatus::Failed;
    diagnostic.error = error.what();
    result.fitted.reset();
  }
  return result;
}
} // namespace breakpoint
