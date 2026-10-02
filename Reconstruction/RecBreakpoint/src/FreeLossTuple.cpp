#include "FreeLossTuple.h"
#include "TTree.h"
#include <cmath>
#include <limits>

namespace breakpoint {
void FreeLossTuple::book(TTree& tree, const std::string& prefix) {
  auto branch = [&](const char* suffix, auto* address) {
    tree.Branch((prefix + suffix).c_str(), address);
  };
  branch("enabled", &m_enabled);
  branch("max_log_loss", &m_maxLogLoss);
  branch("max_calls_per_start", &m_maxCalls);
  branch("tolerance", &m_tolerance);
  branch("applied", &m_applied);
  branch("status", &m_status);
  branch("interval", &m_interval);
  branch("b", &m_b);
  branch("treatment", &m_lossTreatment);
  branch("prior_mean_log_loss", &m_b);
  branch("prior_sigma_log_loss", &m_priorSigmaLogLoss);
  branch("b_error", &m_errorB);
  branch("minuit_status", &m_minuitStatus);
  branch("edm", &m_edm);
  branch("nll2", &m_nll2); // hit-only normalized likelihood
  branch("quadratic", &m_quadratic);
  branch("logdet", &m_logdet);
  branch("objective_nll2", &m_objectiveNll2);
  branch("beam_nll2", &m_beamNll2);
  branch("beam_quadratic", &m_beamQuadratic);
  branch("beam_logdet", &m_beamLogdet);
  branch("beam_residual_mm", &m_beamResidual);
  branch("beam_innovation_variance_mm2", &m_beamVariance);
  branch("beam_track_variance_mm2", &m_beamTrackVariance);
  branch("beam_spot_variance_mm2", &m_beamSpotVariance);
  branch("lower_bound", &m_lower);
  branch("upper_bound", &m_upper);
  branch("covariance_conditional", &m_covarianceConditional);
  branch("error", &m_error);
  branch("trial_b", &m_trialB);
  branch("trial_nll2", &m_trialNll2); // hit-only, retained name
  branch("trial_objective_nll2", &m_trialObjectiveNll2);
  branch("trial_beam_nll2", &m_trialBeamNll2);
  branch("trial_valid", &m_trialValid);
  branch("trial_phase", &m_trialPhase);
  branch("trial_error", &m_trialError);
}

void FreeLossTuple::reset(bool enabled, const FreeLossSettings& controls, double sigmaLogLoss) {
  m_enabled = enabled;
  m_priorSigmaLogLoss = sigmaLogLoss;
  m_maxLogLoss = controls.maxLogLoss;
  m_maxCalls = controls.maxCallsPerStart;
  m_tolerance = controls.tolerance;
  m_applied = m_lower = m_upper = m_covarianceConditional = false;
  m_status = 0; m_interval = -1; m_minuitStatus = -99;
  const double nan = std::numeric_limits<double>::quiet_NaN();
  m_b = m_errorB = m_edm = m_nll2 = m_quadratic = m_logdet = nan;
  m_objectiveNll2 = m_beamNll2 = m_beamQuadratic = m_beamLogdet = nan;
  m_beamResidual = m_beamVariance = m_beamTrackVariance = m_beamSpotVariance = nan;
  m_error.clear();
  m_trialB.clear(); m_trialNll2.clear(); m_trialObjectiveNll2.clear(); m_trialBeamNll2.clear();
  m_trialValid.clear(); m_trialPhase.clear(); m_trialError.clear();
}

void FreeLossTuple::assign(const FreeLossDiagnostics& diagnostic) {
  m_status = int(diagnostic.status);
  m_interval = diagnostic.interval;
  m_applied = applied();
  // Legacy flag meant conditional on an EXACT fixed b. The new output includes
  // posterior loss variance, but still conditions on the optimized prior center.
  m_covarianceConditional = false;
  if (std::isfinite(diagnostic.priorSigmaLogLoss))
    m_priorSigmaLogLoss = diagnostic.priorSigmaLogLoss;
  m_minuitStatus = diagnostic.minuitStatus;
  m_b = diagnostic.b; m_errorB = diagnostic.bError; m_edm = diagnostic.edm;
  m_lower = diagnostic.lowerBound; m_upper = diagnostic.upperBound;
  m_error = diagnostic.error;
  if (m_applied) {
    m_nll2 = diagnostic.likelihood.nll2;
    m_quadratic = diagnostic.likelihood.quadratic;
    m_logdet = diagnostic.likelihood.logDeterminant;
    m_objectiveNll2 = diagnostic.objectiveNll2;
    m_beamNll2 = diagnostic.beam.nll2;
    m_beamQuadratic = diagnostic.beam.quadratic;
    m_beamLogdet = diagnostic.beam.logDeterminant;
    m_beamResidual = diagnostic.beam.residual;
    m_beamVariance = diagnostic.beam.innovationVariance;
    m_beamTrackVariance = diagnostic.beam.trackVariance;
    m_beamSpotVariance = diagnostic.beam.beamVariance;
  }
  for (const auto& trial : diagnostic.trials) {
    m_trialB.push_back(trial.b); m_trialNll2.push_back(trial.likelihood.nll2);
    m_trialObjectiveNll2.push_back(trial.objectiveNll2);
    m_trialBeamNll2.push_back(trial.valid ? trial.beam.nll2
                                            : std::numeric_limits<double>::quiet_NaN());
    m_trialValid.push_back(trial.valid); m_trialPhase.push_back(int(trial.phase));
    m_trialError.push_back(trial.error);
  }
}
} // namespace breakpoint
