#pragma once

// Isolated scalar profile experiment. No production configurable or endpoint changes.
#include "BreakpointFitter.h"
#include "TrackLikelihood.h"
#include "Math/Factory.h"
#include "Math/Functor.h"
#include "Math/Minimizer.h"
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <numeric>
#include <iostream>

namespace breakpoint_probe {
struct Evaluation {
  double b=0, objective=1.e20, forward=1.e20, backward=1.e20;
  double nativeHit=1.e20, pt=0, backwardPt=0, smoothed=1.e20;
  LikelihoodScores likelihood;
  bool valid=false;
};

inline void run(const breakpoint::BreakpointFitter& fitter,
    const std::vector<edm4hep::TrackerHit>& hits,
    const breakpoint::FitSettings& ordinary, double bz, int event, int track,
    double truthB) {
  const char* prefix=std::getenv("BP_PROBE_PREFIX");
  if (!prefix) throw std::runtime_error("BP_PROBE_PREFIX missing");
  const char* objectiveEnv=std::getenv("BP_PROBE_OBJECTIVE");
  const std::string objective=objectiveEnv ? objectiveEnv : "Smoothed";
  if (objective!="Forward" && objective!="Backward" && objective!="Smoothed")
    throw std::runtime_error("BP_PROBE_OBJECTIVE must be Forward, Backward or Smoothed");
  std::ofstream log(std::string(prefix)+"_trials.csv",std::ios::app);
  std::ofstream summary(std::string(prefix)+"_minimum.csv",std::ios::app);
  std::ofstream evidence(std::string(prefix)+"_likelihood.csv",std::ios::app);
  evidence << std::setprecision(17);
  if(evidence.tellp()==0) evidence << "event,track,phase,b,forward_nll2,backward_nll2,smoothed_nll2,forward_quad,backward_quad,smoothed_quad,forward_logdet,backward_logdet,smoothed_logdet,hit_penalty,seed_penalty,process_penalty,n_measurements,n_latent\n";
  log << std::setprecision(17); summary << std::setprecision(17);
  if (log.tellp()==0) log << "event,track,interval,phase,b,chi2,forward_chi2,backward_chi2,native_hit_chi2,rts_pt,backward_pt,valid,smoothed_chi2,objective\n";
  if (summary.tellp()==0) summary << "event,track,interval,b,chi2,rts_pt,backward_pt,minuit_status,edm,truth_b,truth_chi2,truth_rts_pt,evaluations,objective,forward_chi2,backward_chi2,smoothed_chi2\n";
  auto settings=ordinary;
  settings.lossStateMode="LocalMarginal";
  settings.sigmaLogLoss=0; // CONDITIONAL trial: b is fixed, not a zero-width fitted prior.
  settings.intervalMeanLogLoss.clear();
  std::map<double,Evaluation> cache;
  std::string phase="scan";
  auto evaluate = [&](double b, bool force=false) {
    if (!force && cache.count(b)) return cache.at(b);
    Evaluation e; e.b=b; settings.meanLogLoss=b;
    try {
      const auto pair=fitter.fit(hits,settings);
      const auto& r=pair.rts;
      if (r.smoothedChi2Status!=1 || r.breakpoints.size()!=1 ||
          r.breakpoints.front().fittedLogLoss!=b || r.breakpoints.front().fittedVariance!=0)
        throw std::runtime_error("Fixed-b/score invariant failed");
      e.forward=r.chi2;
      e.backward=std::accumulate(pair.backward.backwardChi2.begin(),pair.backward.backwardChi2.end(),0.);
      e.smoothed=r.smoothedTotalChi2;
      e.likelihood=likelihoodScores(r);
      e.objective=e.likelihood.smoothed;
      if (objective=="Forward") e.objective=e.likelihood.forward;
      if (objective=="Backward") e.objective=e.likelihood.backward;
      const auto& s=e.likelihood;
      evidence << event << ',' << track << ',' << phase << ',' << b << ','
               << s.forward << ',' << s.backward << ',' << s.smoothed << ','
               << s.forwardQuadratic << ',' << s.backwardQuadratic << ',' << s.smoothedQuadratic << ','
               << s.forwardLogdet << ',' << s.backwardLogdet << ',' << s.smoothedLogdet << ','
               << s.hitPenalty << ',' << s.seedPenalty << ',' << s.processPenalty << ','
               << s.measurements << ',' << s.latentDimensions << '\n';
      evidence.flush();
      e.nativeHit=std::accumulate(r.smoothedNativeMeasurementChi2.begin(),r.smoothedNativeMeasurementChi2.end(),0.);
      e.pt=std::abs(bz*2.99792458e-4/r.ip.omega);
      e.backwardPt=std::abs(bz*2.99792458e-4/pair.backward.ip.omega);
      // A normalized continuous-density -2logL can legitimately be negative.
      e.valid=std::isfinite(e.objective)&&std::isfinite(e.pt);
      if (!e.valid) e.objective=1.e20;
    } catch (const std::exception& ex) {
      std::cerr << "PROBE event=" << event << " b=" << b << " failed: " << ex.what() << '\n';
    }
    log << event << ',' << track << ',' << settings.intervals.front() << ',' << phase << ','
        << e.b << ',' << e.objective << ',' << e.forward << ',' << e.backward << ','
        << e.nativeHit << ',' << e.pt << ',' << e.backwardPt << ',' << e.valid << ',' << e.smoothed << ',' << objective << '\n';
    log.flush(); cache[b]=e; return e;
  };
  // Identical search grid and physical bounds for every event; never truth-seeded.
  Evaluation best;
  for (double b : {0.,.0005,.001,.002,.005,.01,.02,.04,.08,.15,.3,.5,.75,1.}) {
    auto e=evaluate(b); if(e.valid && e.objective<best.objective) best=e;
  }
  if (!best.valid) throw std::runtime_error("No valid profile trials");
  int status=-99; double edm=1.e20;
  const double scanBest=best.b;
  for (double start : {scanBest, .005, .05}) {
    std::unique_ptr<ROOT::Math::Minimizer> minimizer(ROOT::Math::Factory::CreateMinimizer("Minuit2","Migrad"));
    if (!minimizer) throw std::runtime_error("Minuit2 unavailable");
    ROOT::Math::Functor f([&](const double* x){return evaluate(x[0]).objective;},1);
    phase="migrad";
    minimizer->SetFunction(f); minimizer->SetMaxFunctionCalls(180);
    minimizer->SetTolerance(.001); minimizer->SetPrecision(1.e-8);
    minimizer->SetErrorDef(1.); minimizer->SetPrintLevel(0);
    minimizer->SetLimitedVariable(0,"b",start,.002,0.,1.);
    minimizer->Minimize();
    const auto e=evaluate(minimizer->X()[0]);
    if(e.valid && e.objective<=best.objective) {
      best=e; status=minimizer->Status(); edm=minimizer->Edm();
    }
  }
  // A repeated evaluation tests deterministic inner fits; no cached value used.
  phase="repeat_minimum"; const auto repeated=evaluate(best.b,true);
  if (std::abs(repeated.objective-best.objective)>1.e-7)
    throw std::runtime_error("Non-reproducible profile evaluation");
  // Fine local scan is fixed around the blind minimum, not around truth.
  phase="local_scan";
  for(int j=-10;j<=10;++j) {
    const double b=best.b+j*.001;
    if(b>=0 && b<=1) evaluate(b);
  }
  // Truth amount enters ONLY after the search has finished.
  phase="truth_check";
  Evaluation truth;
  if(truthB>=0 && std::isfinite(truthB)) truth=evaluate(truthB,true);
  summary << event << ',' << track << ',' << settings.intervals.front() << ',' << best.b << ','
          << best.objective << ',' << best.pt << ',' << best.backwardPt << ',' << status << ',' << edm << ','
          << truthB << ',' << truth.objective << ',' << truth.pt << ',' << cache.size() << ',' << objective << ','
          << best.forward << ',' << best.backward << ',' << best.smoothed << '\n';
  summary.flush();
  // Full state/covariance audit at the fitted minimum, separate from ordinary tuple.
  settings.meanLogLoss=best.b;
  const auto final=fitter.fit(hits,settings);
  std::ofstream states(std::string(prefix)+"_states.txt",std::ios::app);
  states << std::setprecision(17);
  for(std::size_t i=0;i<hits.size();++i) {
    for(const auto& named:std::vector<std::pair<const char*,const breakpoint::TrackState*>>{
        {"predicted",&final.rts.predicted[i]}, {"filtered",&final.rts.filtered[i]},
        {"smoothed",&final.rts.smoothed[i]}, {"backward",&final.backward.backwardFiltered[i]}}) {
      states << "event=" << event << " track=" << track << " hit=" << i << " " << named.first << " mean=";
      for(int k=0;k<5;++k) states << named.second->mean(k,0) << ' ';
      states << " covariance=";
      for(int k=0;k<5;++k) for(int l=0;l<5;++l) states << named.second->covariance(k,l) << ' ';
      states << '\n';
    }
  }
  std::cout << "PROBE_DONE event=" << event << " track=" << track << " b=" << best.b
            << " chi2=" << best.objective << " status=" << status << std::endl;
}
} // namespace breakpoint_probe
