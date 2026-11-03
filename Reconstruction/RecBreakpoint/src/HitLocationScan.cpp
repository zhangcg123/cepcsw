#include "HitLocationScan.h"
#include "FreeLossFitter.h"
#include "UTIL/BitField64.h"
#include "UTIL/ILDConf.h"
#include "TTree.h"
#include <algorithm>

namespace breakpoint {
void HitLocationScan::book(TTree& tree) {
  tree.Branch("hit_scan_status", &status);
  tree.Branch("hit_scan_constraint_mask", &constraintMask);
  tree.Branch("hit_scan_charged_energy_mode", &chargedEnergyMode);
  tree.Branch("hit_scan_min_nll_gain", &minNllGain);
  tree.Branch("hit_scan_null_nll2", &nullNll2);
  tree.Branch("hit_scan_null_ip_omega", &nullIpOmega);
  tree.Branch("hit_scan_measurement_dimensions", &measurementDimensions);
  tree.Branch("hit_scan_best_hit", &bestHit);
  tree.Branch("hit_scan_selected_hit", &selectedHit);
  tree.Branch("hit_scan_best_nll2", &bestNll2);
  tree.Branch("hit_scan_second_nll2", &secondNll2);
  tree.Branch("hit_scan_nll_gain", &gain);
  tree.Branch("hit_scan_failed_candidates", &failedCandidates);
  tree.Branch("hit_scan_error", &error);
  tree.Branch("hit_scan_candidate_hit", &hit);
  tree.Branch("hit_scan_candidate_system", &system);
  tree.Branch("hit_scan_candidate_layer", &layer);
  tree.Branch("hit_scan_candidate_fit_status", &fitStatus);
  tree.Branch("hit_scan_candidate_minuit_status", &minuitStatus);
  tree.Branch("hit_scan_candidate_nll2", &nll2);
  tree.Branch("hit_scan_candidate_prior_center", &priorCenter);
  tree.Branch("hit_scan_candidate_fitted_log_loss", &fittedLoss);
  tree.Branch("hit_scan_candidate_fitted_variance", &fittedVariance);
  tree.Branch("hit_scan_candidate_ip_omega", &ipOmega);
  tree.Branch("hit_scan_candidate_error", &candidateError);
  tree.Branch("hit_scan_truth_status", &truthStatus);
  tree.Branch("hit_scan_truth_hit", &truthHit);
  tree.Branch("hit_scan_truth_minuit_status", &truthMinuitStatus);
  tree.Branch("hit_scan_truth_reused_candidate", &truthReusedCandidate);
  tree.Branch("hit_scan_truth_nll2", &truthNll2);
  tree.Branch("hit_scan_truth_ip_omega", &truthIpOmega);
  tree.Branch("hit_scan_truth_pt", &truthPt);
  tree.Branch("hit_scan_truth_prior_center", &truthPriorCenter);
  tree.Branch("hit_scan_truth_fitted_log_loss", &truthFittedLoss);
  tree.Branch("hit_scan_truth_fitted_variance", &truthFittedVariance);
  tree.Branch("hit_scan_truth_error", &truthError);
}

void HitLocationScan::run(const BreakpointFitter& fitter,
    const std::vector<edm4hep::TrackerHit>& hits, const FitSettings& settings,
    const FreeLossSettings& controls) {
  const double minimumGain=minNllGain;
  reset(minimumGain);
  auto hypothesis=settings;
  hypothesis.intervals.clear(); hypothesis.intervalMeanLogLoss.clear();
  hypothesis.neutralEcalEnergy.reset(); hypothesis.delayedNeutral=false;
  hypothesis.captureGaussianModel=true;
  constraintMask=(hypothesis.beamSpot ? 1 : 0) | (hypothesis.chargedEcalEnergy ? 2 : 0);
  const auto candidateMode=hypothesis.chargedEcalEnergyMode;
  if (hypothesis.chargedEcalEnergy)
    chargedEnergyMode=candidateMode==ChargedEnergyMode::ElectronPhoton ? "ElectronPhoton" : "ElectronOnly";
  auto candidateControls=controls;
  candidateControls.beamSpot=hypothesis.beamSpot;
  candidateControls.chargedEcalEnergy=hypothesis.chargedEcalEnergy;
  candidateControls.chargedEcalEnergyMode=candidateMode;
  candidateControls.neutralEcalEnergy.reset();
  // The null is a separate 5D model, not b0=0 with finite SigmaLogLoss.
  try {
    // Same measured cluster and beam; an unbroken track has no photon coordinate.
    hypothesis.chargedEcalEnergyMode=ChargedEnergyMode::ElectronOnly;
    const auto null=fitter.fit(hits,hypothesis,false);
    const auto score=evaluateFitLikelihood(null.rts);
    nullNll2=score.nll2; measurementDimensions=score.measurementDimensions;
    nullIpOmega=null.rts.ip.omega;
    if (!std::isfinite(nullNll2)) throw std::runtime_error("Nonfinite no-breakpoint likelihood");
  } catch (const std::exception& failure) {
    status=-1; error=failure.what(); return;
  }
  hypothesis.chargedEcalEnergyMode=candidateMode;
  std::vector<int> systems, layers;
  UTIL::BitField64 cell(lcio::ILDCellID0::encoder_string);
  for (const auto& measurement : hits) {
    cell.setValue(measurement.getCellID());
    systems.push_back(cell["subdet"]); layers.push_back(cell["layer"]);
  }
  for (int index : siliconScanHits(systems)) {
    hypothesis.intervals={index};
    const auto candidate=FreeLossFitter(fitter).fit(hits,hypothesis,candidateControls,false);
    const auto& diagnostic=candidate.diagnostics;
    const bool sameMeasurements=diagnostic.likelihood.measurementDimensions==measurementDimensions;
    const bool valid=diagnostic.status==FreeLossStatus::Applied && candidate.fitted.has_value() && sameMeasurements;
    const double nan=std::numeric_limits<double>::quiet_NaN();
    hit.push_back(index); system.push_back(systems[index]); layer.push_back(layers[index]);
    fitStatus.push_back(sameMeasurements ? static_cast<int>(diagnostic.status) : -2);
    minuitStatus.push_back(diagnostic.minuitStatus);
    candidateError.push_back(diagnostic.status==FreeLossStatus::Applied && !sameMeasurements
        ? "Candidate and null have different measurement dimensions" : diagnostic.error);
    nll2.push_back(valid ? diagnostic.objectiveNll2 : nan);
    priorCenter.push_back(valid ? diagnostic.b : nan);
    fittedLoss.push_back(valid ? candidate.fitted->rts.breakpoints.front().fittedLogLoss : nan);
    fittedVariance.push_back(valid ? candidate.fitted->rts.breakpoints.front().fittedVariance : nan);
    ipOmega.push_back(valid ? candidate.fitted->rts.ip.omega : nan);
  }
  select();
  if (status==-2) error="All silicon candidate optimizations failed; keep no-breakpoint refit";
}

void HitLocationScan::reference(const BreakpointFitter& fitter,
    const std::vector<edm4hep::TrackerHit>& hits, const FitSettings& settings,
    const FreeLossSettings& controls, int ownerHit) {
  truthHit=ownerHit;
  if (ownerHit<0) {
    truthStatus=std::isfinite(nullNll2) ? 1 : -2;
    truthNll2=nullNll2; truthIpOmega=nullIpOmega;
    if (truthStatus<0) truthError="No valid no-breakpoint reference";
    return;
  }
  const auto found=std::find(hit.begin(),hit.end(),ownerHit);
  if (found!=hit.end()) {
    const auto i=std::distance(hit.begin(),found);
    truthReusedCandidate=true;
    truthStatus=fitStatus[i]==2 ? 2 : -2;
    truthNll2=nll2[i]; truthIpOmega=ipOmega[i]; truthMinuitStatus=minuitStatus[i];
    truthPriorCenter=priorCenter[i]; truthFittedLoss=fittedLoss[i];
    truthFittedVariance=fittedVariance[i]; truthError=candidateError[i];
    return;
  }
  // A non-silicon truth owner is a reference only, never a scan candidate.
  auto hypothesis=settings;
  hypothesis.intervals={ownerHit}; hypothesis.intervalMeanLogLoss.clear();
  hypothesis.neutralEcalEnergy.reset(); hypothesis.delayedNeutral=false;
  auto referenceControls=controls;
  referenceControls.beamSpot=hypothesis.beamSpot;
  referenceControls.chargedEcalEnergy=hypothesis.chargedEcalEnergy;
  referenceControls.chargedEcalEnergyMode=hypothesis.chargedEcalEnergyMode;
  referenceControls.neutralEcalEnergy.reset();
  const auto result=FreeLossFitter(fitter).fit(hits,hypothesis,referenceControls,false);
  truthStatus=-2; truthMinuitStatus=result.diagnostics.minuitStatus;
  truthError=result.diagnostics.error;
  if (result.diagnostics.status==FreeLossStatus::Applied && result.fitted &&
      result.diagnostics.likelihood.measurementDimensions==measurementDimensions) {
    truthStatus=2; truthNll2=result.diagnostics.objectiveNll2;
    truthIpOmega=result.fitted->rts.ip.omega;
    truthPriorCenter=result.diagnostics.b;
    truthFittedLoss=result.fitted->rts.breakpoints.front().fittedLogLoss;
    truthFittedVariance=result.fitted->rts.breakpoints.front().fittedVariance;
  } else if (truthError.empty()) truthError="Invalid truth-location fit or measurement dimensions";
}
} // namespace breakpoint
