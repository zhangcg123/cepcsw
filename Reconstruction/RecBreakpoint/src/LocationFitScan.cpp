#include "LocationFitScan.h"
#include "FreeLossFitter.h"
#include "UTIL/BitField64.h"
#include "UTIL/ILDConf.h"
#include "TTree.h"
#include <algorithm>

namespace breakpoint {
namespace {
struct LocationScanResult {
  int status=-2, mask=0, minuit=-99, source=-1, dimensions=0;
  bool resolvedEcalSum=false;
  double nll2=std::numeric_limits<double>::quiet_NaN(), chi2=nll2, logDet=nll2;
  double pt=nll2, ptError=nll2, prior=nll2, loss=nll2, variance=nll2;
  std::optional<edm4hep::TrackState> ip;
  std::string error;
};

LocationScanResult fitLocation(const BreakpointFitter& fitter,
    const std::vector<edm4hep::TrackerHit>& hits, const FitSettings& settings,
    FreeLossSettings controls, double bz) {
  LocationScanResult result;
  try {
    std::optional<PairedFitResult> pair;
    TrackLikelihoodResult score;
    if (settings.intervals.empty()) {
      auto null=settings;
      null.beamSpotFsrLoss=false;
      // Compare the same observed C+N, but predict electron energy with a 5D state.
      const bool sum=combineResolvedEcalMeasurements(null,true);
      null.resolvedEcalSum=false;
      null.chargedEcalEnergyMode=ChargedEnergyMode::ElectronOnly;
      pair.emplace(fitter.fit(hits,null,false));
      pair->rts.resolvedEcalSum=sum;
      score=evaluateFitLikelihood(pair->rts);
    } else {
      controls.beamSpot=settings.beamSpot;
      controls.chargedEcalEnergy=settings.chargedEcalEnergy;
      controls.chargedEcalEnergyMode=settings.chargedEcalEnergyMode;
      controls.neutralEcalEnergy=settings.neutralEcalEnergy;
      auto optimized=FreeLossFitter(fitter).fit(hits,settings,controls,false);
      result.minuit=optimized.diagnostics.minuitStatus;
      if (optimized.diagnostics.status!=FreeLossStatus::Applied || !optimized.fitted)
        throw std::runtime_error(optimized.diagnostics.error);
      score=optimized.diagnostics.likelihood;
      result.prior=optimized.diagnostics.b;
      pair=std::move(optimized.fitted);
      result.loss=pair->rts.breakpoints.front().fittedLogLoss;
      result.variance=pair->rts.breakpoints.front().fittedVariance;
    }
    const auto& fit=pair->rts;
    if (!std::isfinite(score.nll2) || !std::isfinite(fit.ip.omega) || fit.ip.omega==0)
      throw std::runtime_error("Nonfinite location fit");
    result.nll2=score.nll2; result.chi2=score.quadratic;
    result.logDet=score.logDeterminant; result.dimensions=score.measurementDimensions;
    result.pt=std::abs(bz*2.99792458e-4/fit.ip.omega);
    result.ptError=transverseMomentumError(fit.ip,bz);
    result.ip=fit.ip;
    result.resolvedEcalSum=fit.resolvedEcalSum;
    result.mask=(fit.beamSpot ? 1 : 0) | (fit.chargedEcalStep ? 2 : 0) | (fit.neutralEcal ? 4 : 0);
    result.status=2;
  } catch (const std::exception& failure) { result.error=failure.what(); }
  return result;
}
} // namespace

void LocationFitScan::run(const BreakpointFitter& fitter,
    const std::vector<edm4hep::TrackerHit>& hits, const FitSettings& observations,
    const FreeLossSettings& controls, double bz, std::optional<int> truthOwner, bool neutralEnabled,
    const std::vector<int>& requestedSetups, bool referenceOnly) {
  reset();
  const auto configurations = referenceOnly ? std::vector<int>{110} : requestedSetups.empty() ? locationScanSetups(neutralEnabled)
      : checkedLocationScanSetups(requestedSetups);
  const auto requested = [&](int setup) {
    return std::find(configurations.begin(), configurations.end(), setup) != configurations.end();
  };
  std::vector<int> systems,layers;
  UTIL::BitField64 cell(lcio::ILDCellID0::encoder_string);
  if (!referenceOnly) for (const auto& measurement : hits) {
    cell.setValue(measurement.getCellID());
    systems.push_back(cell["subdet"]); layers.push_back(cell["layer"]);
  }
  if (truthOwner && (*truthOwner < -1 || (*truthOwner>=0 &&
      static_cast<std::size_t>(*truthOwner+1)>=hits.size()))) {
    truthError="Invalid truth owner index"; truthOwner.reset();
  }
  if (referenceOnly) truthOwner.reset();
  truthStatus=referenceOnly ? 0 : truthOwner ? (*truthOwner<0 ? 1 : 2) : -1;
  truthHit=truthOwner.value_or(-1);
  const auto candidatePointCount=locationScanPoints(systems,layers).size();
  const auto points=referenceOnly ? std::vector<int>{-1} : locationScanPoints(systems,layers,truthHit);
  pointCount=points.size();
  for (std::size_t p=0;p<points.size();++p) {
    const int site=points[p];
    if (truthOwner && site==truthHit) truthPoint=p;
    const int type=site<0 ? 0 : p>=candidatePointCount ? 2 :
        (systems[site]>=1 && systems[site]<=3 ? 1 : 3);
    auto accepted=observations;
    accepted.intervals=site<0 ? std::vector<int>{} : std::vector<int>{site};
    accepted.intervalMeanLogLoss.clear();
    accepted.beamSpot.reset(); accepted.chargedEcalEnergy.reset();
    accepted.neutralEcalEnergy.reset(); accepted.delayedNeutral=false;
    accepted.resolvedEcalSum=false;
    accepted.captureGaussianModel=true;
    accepted.chargedEcalEnergyMode=site<0 ? ChargedEnergyMode::ElectronOnly : observations.chargedEcalEnergyMode;
    LocationScanResult previous;
    for (int configuration : configurations) {
      auto proposed=accepted;
      // An omitted predecessor supplies raw observations, never an extra fit.
      if (configuration>=100 && !requested(100)) proposed.beamSpot=observations.beamSpot;
      if (configuration==111 && !requested(110))
        proposed.chargedEcalEnergy=observations.chargedEcalEnergy;
      bool available=configuration==0;
      std::string reason;
      if (configuration==100 && observations.beamSpot) {
        proposed.beamSpot=observations.beamSpot; available=true;
      } else if (configuration==110 && observations.chargedEcalEnergy &&
                 (!referenceOnly || observations.beamSpot)) {
        proposed.chargedEcalEnergy=observations.chargedEcalEnergy; available=true;
        if (observations.chargedEcalEnergyMode==ChargedEnergyMode::ElectronPhoton &&
            observations.neutralEcalEnergy) {
          proposed.resolvedEcalSum=true;
          proposed.chargedEcalEnergyMode=ChargedEnergyMode::ElectronPhoton;
          proposed.neutralEcalEnergy=observations.neutralEcalEnergy;
        }
      } else if (configuration==111 && site>=0 && observations.neutralEcalEnergy) {
        proposed.resolvedEcalSum=false;
        proposed.chargedEcalEnergyMode=ChargedEnergyMode::ElectronOnly;
        proposed.neutralEcalEnergy=observations.neutralEcalEnergy;
        proposed.delayedNeutral=observations.delayedNeutral; available=true;
      }
      auto result=previous;
      int attempt=0,attemptMinuit=-99;
      if (available) {
        auto trial=fitLocation(fitter,hits,proposed,controls,bz);
        attempt=trial.status; attemptMinuit=trial.minuit;
        if (trial.status==2) {
          accepted=proposed; trial.source=configuration; previous=trial; result=trial;
        } else {
          reason=trial.error;
          if (previous.status!=2) result=trial;
        }
      } else reason=referenceOnly ? "110 reference requires beam and charged ECAL observations" :
          configuration==111 && site<0 ? "No breakpoint: neutral observation is inapplicable" : "Observation unavailable";
      if (attempt!=2 && previous.status==2) {
        result=previous; result.status=1;
        result.error=reason+"; retain preceding accepted setup";
      }
      if (referenceOnly && !available) result.error=reason;
      point.push_back(p); kind.push_back(type); hit.push_back(site);
      system.push_back(site<0 ? -1 : systems[site]); layer.push_back(site<0 ? -1 : layers[site]);
      setup.push_back(configuration); actualMask.push_back(result.mask);
      resolvedEcalSum.push_back(result.resolvedEcalSum);
      fitStatus.push_back(result.status); attemptStatus.push_back(attempt);
      minuitStatus.push_back(result.minuit); attemptMinuitStatus.push_back(attemptMinuit);
      sourceSetup.push_back(result.source); measurementDimensions.push_back(result.dimensions);
      nll2.push_back(result.nll2); chi2.push_back(result.chi2); logDeterminant.push_back(result.logDet);
      pt.push_back(result.pt); ptError.push_back(result.ptError); priorCenter.push_back(result.prior);
      appendIP(result.ip ? &*result.ip : nullptr);
      fittedLoss.push_back(result.loss); fittedVariance.push_back(result.variance);
      error.push_back(result.error);
    }
  }
  status=1; // Scan recorded, not a claim that any hypothesis is the correct one.
}
} // namespace breakpoint
