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
  branch("nll2", &m_nll2); // includes ECAL only in the ECAL-guided family
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
  branch("charged_ecal_predicted_energy", &m_chargedEcalPredicted);
  branch("charged_ecal_residual", &m_chargedEcalResidual);
  branch("charged_ecal_innovation_variance", &m_chargedEcalVariance);
  branch("charged_ecal_chi2", &m_chargedEcalChi2);
  branch("neutral_ecal_energy", &m_neutral.energy);
  branch("neutral_ecal_energy_error", &m_neutral.sigma);
  branch("neutral_ecal_predicted_energy", &m_neutral.predictedEnergy);
  branch("neutral_ecal_predicted_error", &m_neutral.predictedError);
  branch("neutral_ecal_filtered_energy", &m_neutral.filteredEnergy);
  branch("neutral_ecal_filtered_error", &m_neutral.filteredError);
  branch("neutral_ecal_smoothed_energy", &m_neutral.smoothedEnergy);
  branch("neutral_ecal_smoothed_error", &m_neutral.smoothedError);
  branch("neutral_ecal_smoothed_affine_energy", &m_neutral.smoothedAffineEnergy);
  branch("neutral_ecal_residual", &m_neutral.residual);
  branch("neutral_ecal_innovation_variance", &m_neutral.innovationVariance);
  branch("neutral_ecal_chi2", &m_neutral.chi2);
  branch("neutral_ecal_smoothed_chi2", &m_neutral.smoothedChi2);
  branch("neutral_ecal_predicted_state", &m_neutralPredictedState);
  branch("neutral_ecal_filtered_state", &m_neutralFilteredState);
  branch("neutral_ecal_smoothed_state", &m_neutralSmoothedState);
  branch("neutral_ecal_predicted_covariance", &m_neutralPredictedCov);
  branch("neutral_ecal_filtered_covariance", &m_neutralFilteredCov);
  branch("neutral_ecal_smoothed_covariance", &m_neutralSmoothedCov);
  branch("lower_bound", &m_lower);
  branch("upper_bound", &m_upper);
  branch("covariance_conditional", &m_covarianceConditional);
  branch("error", &m_error);
  branch("trial_b", &m_trialB);
  branch("trial_nll2", &m_trialNll2); // retained name; ECAL family includes ECAL
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
  m_chargedEcalPredicted = m_chargedEcalResidual = m_chargedEcalVariance = m_chargedEcalChi2 = nan;
  m_neutral = NeutralEnergyMeasurement{};
  m_neutralPredictedState.clear(); m_neutralFilteredState.clear(); m_neutralSmoothedState.clear();
  m_neutralPredictedCov.clear(); m_neutralFilteredCov.clear(); m_neutralSmoothedCov.clear();
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
    if (diagnostic.neutralEcal) {
      m_neutral = *diagnostic.neutralEcal;
      auto flatten = [](const TMatrixD& matrix, std::vector<double>& values) {
        values.assign(matrix.GetMatrixArray(), matrix.GetMatrixArray() + matrix.GetNoElements());
      };
      flatten(m_neutral.predicted.mean, m_neutralPredictedState);
      flatten(m_neutral.filtered.mean, m_neutralFilteredState);
      flatten(m_neutral.smoothed.mean, m_neutralSmoothedState);
      flatten(m_neutral.predicted.covariance, m_neutralPredictedCov);
      flatten(m_neutral.filtered.covariance, m_neutralFilteredCov);
      flatten(m_neutral.smoothed.covariance, m_neutralSmoothedCov);
    }
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
    m_chargedEcalPredicted = diagnostic.chargedEcalPredictedEnergy;
    m_chargedEcalResidual = diagnostic.chargedEcalResidual;
    m_chargedEcalVariance = diagnostic.chargedEcalInnovationVariance;
    m_chargedEcalChi2 = diagnostic.chargedEcalChi2;
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
