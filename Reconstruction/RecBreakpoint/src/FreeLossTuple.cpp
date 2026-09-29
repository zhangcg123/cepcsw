#include "FreeLossTuple.h"
#include "TTree.h"
#include <cmath>

namespace breakpoint {
void FreeLossTuple::book(TTree& tree) {
  tree.Branch("free_loss_enabled", &m_enabled);
  tree.Branch("free_loss_max_log_loss", &m_maxLogLoss);
  tree.Branch("free_loss_max_calls_per_start", &m_maxCalls);
  tree.Branch("free_loss_tolerance", &m_tolerance);
  tree.Branch("free_loss_applied", &m_applied);
  tree.Branch("free_loss_status", &m_status);
  tree.Branch("free_loss_interval", &m_interval);
  tree.Branch("free_loss_b", &m_b);
  tree.Branch("free_loss_treatment", &m_lossTreatment);
  tree.Branch("free_loss_prior_mean_log_loss", &m_b);
  tree.Branch("free_loss_prior_sigma_log_loss", &m_priorSigmaLogLoss);
  tree.Branch("free_loss_b_error", &m_errorB);
  tree.Branch("free_loss_minuit_status", &m_minuitStatus);
  tree.Branch("free_loss_edm", &m_edm);
  tree.Branch("free_loss_nll2", &m_nll2);
  tree.Branch("free_loss_quadratic", &m_quadratic);
  tree.Branch("free_loss_logdet", &m_logdet);
  tree.Branch("free_loss_lower_bound", &m_lower);
  tree.Branch("free_loss_upper_bound", &m_upper);
  tree.Branch("free_loss_covariance_conditional", &m_covarianceConditional);
  tree.Branch("free_loss_error", &m_error);
  tree.Branch("free_loss_trial_b", &m_trialB);
  tree.Branch("free_loss_trial_nll2", &m_trialNll2);
  tree.Branch("free_loss_trial_valid", &m_trialValid);
  tree.Branch("free_loss_trial_phase", &m_trialPhase);
  tree.Branch("free_loss_trial_error", &m_trialError);
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
  m_error.clear();
  m_trialB.clear(); m_trialNll2.clear();
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
  }
  for (const auto& trial : diagnostic.trials) {
    m_trialB.push_back(trial.b); m_trialNll2.push_back(trial.likelihood.nll2);
    m_trialValid.push_back(trial.valid); m_trialPhase.push_back(int(trial.phase));
    m_trialError.push_back(trial.error);
  }
}
} // namespace breakpoint
