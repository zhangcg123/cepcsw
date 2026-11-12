#include "BeamSpotFsrFitter.h"
#include "BreakpointFitter.h"
#include "ChargedEnergyMeasurement.h"
#include "NeutralEnergyMeasurement.h"
#include "DelayedNeutralConditioning.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace breakpoint {
FsrProfile profileFsr(const TMatrixD& r,const TMatrixD& s) {
  const int n=r.GetNrows();
  if (n<1 || n>2 || r.GetNcols()!=1 || s.GetNrows()!=n || s.GetNcols()!=n)
    throw std::invalid_argument("FSR profile dimensions");
  TMatrixD checked(s);validateCovariance(checked);
  const TMatrixD inverse=inverseCovariance(s);
  const double determinant=n==1 ? s(0,0) : s(0,0)*s(1,1)-s(0,1)*s(1,0);
  FsrProfile best; best.quadratic=std::numeric_limits<double>::infinity();
  for (int mask=0;mask<(1<<n);++mask) {
    TMatrixD f(r); // unconstrained amplitudes equal innovations
    if (mask==((1<<n)-1)) f.Zero();
    else if (mask) {
      const int a=(mask&1) ? 0 : 1, b=1-a;
      f(a,0)=0.; f(b,0)=r(b,0)-s(b,a)*r(a,0)/s(a,a);
    }
    bool feasible=true;
    for (int i=0;i<n;++i) if (f(i,0)<-1.e-9) feasible=false;
    if (!feasible) continue;
    for (int i=0;i<n;++i) f(i,0)=std::max(0.,f(i,0));
    const TMatrixD delta=r-f;
    const double score=(transpose(delta)*inverse*delta)(0,0);
    if (score<best.quadratic) {
      best.quadratic=score;best.activeMask=mask;best.energy.ResizeTo(f);best.energy=f;
    }
  }
  if (!std::isfinite(best.quadratic) || !(determinant>0))
    throw std::runtime_error("FSR profile failed");
  best.innovation.ResizeTo(s); best.innovation=s;best.logDet=std::log(determinant);
  return best;
}

TMatrixD conditionFsrTrajectory(std::vector<TMatrixD>& means,std::vector<TMatrixD>& covs,
    const std::vector<TMatrixD>& gains,const TMatrixD& h,const TMatrixD& noise,
    const TMatrixD& residual,int activeMask) {
  const int d=means.back().GetNrows(),m=h.GetNrows();
  if(means.size()!=covs.size() || gains.size()+1!=means.size())
    throw std::invalid_argument("Incomplete FSR conditioning chain");
  std::vector<TMatrixD> cross;
  for(const auto& mean:means)cross.emplace_back(mean.GetNrows(),d);
  cross.back()=covs.back();
  for(int i=static_cast<int>(means.size())-2;i>=0;--i)cross[i]=gains[i]*cross[i+1];
  const TMatrixD terminalCov=covs.back();
  std::vector<int> active;
  for(int i=0;i<m;++i)if(activeMask&(1<<i))active.push_back(i);
  if(active.empty())return cross.front();
  const int n=active.size();
  TMatrixD ha(n,d),va(n,n),r(n,1);
  for(int i=0;i<n;++i) {
    r(i,0)=residual(active[i],0);
    for(int j=0;j<d;++j)ha(i,j)=h(active[i],j);
    for(int j=0;j<n;++j)va(i,j)=noise(active[i],active[j]);
  }
  const TMatrixD innovation=ha*terminalCov*transpose(ha)+va;
  const TMatrixD conditionedCross=cross.front()-cross.front()*transpose(ha)*inverseCovariance(innovation)*ha*terminalCov;
  // Each node uses the SAME pre-update joint posterior. Correlated effective
  // measurement errors are whitened before calling native scalar KalTest.
  for(std::size_t k=0;k<means.size();++k) {
    LossTrackState local;local.mean.ResizeTo(means[k]);local.mean=means[k];
    local.covariance.ResizeTo(covs[k]);local.covariance=covs[k];
    const TMatrixD hi=ha*transpose(cross[k])*inverseCovariance(covs[k]);
    TMatrixD effective=innovation-hi*covs[k]*transpose(hi);validateCovariance(effective);
    const double a=n==2 ? effective(1,0)/effective(0,0) : 0.;
    for(int i=0;i<n;++i) {
      GaussianHitModel obs{TMatrixD(1,local.mean.GetNrows()),TMatrixD(1,1),TMatrixD(1,1)};
      for(int j=0;j<local.mean.GetNrows();++j)obs.derivative(0,j)=hi(i,j)-(i==1 ? a*hi(0,j) : 0.);
      obs.noise(0,0)=i==0 ? effective(0,0) : effective(1,1)-a*effective(0,1);
      obs.residual(0,0)=r(i,0)-(i==1 ? a*r(0,0) : 0.)-(obs.derivative*stateDifference(local.mean,means[k]))(0,0);
      local=updateScalarState(local,obs);
    }
    means[k]=local.mean;covs[k]=local.covariance;
  }
  return conditionedCross;
}

ElectronFsrP4 electronAndFsrP4(const TrackState& electron,double energy,const TMatrixD& joint) {
  if(!std::isfinite(energy) || energy<0 || joint.GetNrows()!=6 || joint.GetNcols()!=6 ||
      electron.mean.GetNrows()!=5 || electron.mean.GetNcols()!=1)
    throw std::invalid_argument("Invalid electron/FSR energy or covariance dimensions");
  const double k=electron.mean(2,0),t=electron.mean(4,0);
  const double phi=electron.mean(1,0)+std::acos(-1.)/2.;
  if(!std::isfinite(k) || k==0 || !std::isfinite(t) || !std::isfinite(phi))
    throw std::invalid_argument("Invalid electron direction/curvature");
  for(int i=0;i<6;++i)for(int j=0;j<6;++j)
    if(!std::isfinite(joint(i,j)))throw std::invalid_argument("Nonfinite electron/FSR covariance");
  const double pt=1./std::abs(k),s=std::hypot(1.,t),p=pt*s,c=std::cos(phi),v=std::sin(phi);
  constexpr double mass=.00051099895;
  ElectronFsrP4 result;
  result.electron={std::hypot(p,mass),pt*c,pt*v,pt*t};
  result.photon={energy,energy*c/s,energy*v/s,energy*t/s};
  TMatrixD jac(8,6);jac.Zero();
  jac(0,2)=-p*p/(k*result.electron[0]);jac(0,4)=pt*pt*t/result.electron[0];
  jac(1,1)=-result.electron[2];jac(1,2)=-result.electron[1]/k;
  jac(2,1)=result.electron[1];jac(2,2)=-result.electron[2]/k;
  jac(3,2)=-result.electron[3]/k;jac(3,4)=pt;
  jac(4,5)=1.;
  jac(5,1)=-energy*v/s;jac(5,4)=-energy*c*t/(s*s*s);jac(5,5)=c/s;
  jac(6,1)=energy*c/s;jac(6,4)=-energy*v*t/(s*s*s);jac(6,5)=v/s;
  jac(7,4)=energy/(s*s*s);jac(7,5)=t/s;
  const TMatrixD covariance=jac*joint*transpose(jac);
  for(int i=0;i<8;++i)for(int j=0;j<8;++j)result.covariance[8*i+j]=covariance(i,j);
  return result;
}

FitResult BreakpointFitter::fitBeamSpotFsr(const std::vector<edm4hep::TrackerHit>& hits,
                                         const FitSettings& settings) const {
  if (!settings.chargedEcalEnergy || !settings.beamSpot)
    throw std::invalid_argument("FSR requires ECAL and beam boundary; 000/100 are undetermined");
  if (settings.intervals.size()>1)
    throw std::invalid_argument("FSR prototype supports zero/one material breakpoint");
  if (settings.beamSpot->x!=0 || settings.beamSpot->y!=0)
    throw std::invalid_argument("FSR prototype requires the nominal zero beam pivot");
  auto core=settings;core.beamSpotFsrLoss=false;core.captureFsrTrajectory=true;
  core.captureGaussianModel=true;core.chargedEcalEnergy.reset();core.neutralEcalEnergy.reset();
  core.resolvedEcalSum=false;core.delayedNeutral=false;
  auto result=fit(hits,core,false).rts;
  auto diagnostic=std::make_shared<BeamSpotFsrResult>();
  auto baseScore=evaluateFitLikelihood(result);
  auto means=result.fsrMeans,covs=result.fsrCovariances,gains=result.fsrGains;
  auto nodes=result.fsrNodeHits;
  if (means.size()!=hits.size() || gains.size()+1!=means.size())
    throw std::runtime_error("Missing FSR smoother chain");
  auto& beam=*result.beamSpot;
  const TMatrixD beamGain=beam.filtered.covariance*transpose(beam.transport)*inverseCovariance(beam.firstHitPredicted.covariance);
  means.insert(means.begin(),beam.smoothed.mean);covs.insert(covs.begin(),beam.smoothed.covariance);
  gains.insert(gains.begin(),beamGain);nodes.insert(nodes.begin(),-3);
  LossTrackState terminal;
  TMatrixD transport,noise;
  if (core.intervals.empty()) {
    const auto step=m_adapter.predictCalorimeter(result.smoothed.back(),hits.back());
    terminal.mean.ResizeTo(step.predicted.mean);terminal.mean=step.predicted.mean;
    terminal.covariance.ResizeTo(step.predicted.covariance);terminal.covariance=step.predicted.covariance;
    terminal.pivot=step.predicted.pivot;transport.ResizeTo(step.transport);transport=step.transport;
    noise.ResizeTo(step.noise);noise=step.noise;
  } else {
    const auto step=m_adapter.predictCalorimeter(result.persistentSmoothed.back(),hits.back());
    terminal=step.predicted;transport.ResizeTo(step.transport);transport=step.transport;
    noise.ResizeTo(step.noise);noise=step.noise;
  }
  const TMatrixD terminalGain=covs.back()*transpose(transport)*inverseCovariance(terminal.covariance);
  gains.push_back(terminalGain);means.push_back(terminal.mean);covs.push_back(terminal.covariance);nodes.push_back(-2);
  const int d=terminal.mean.GetNrows(),m=settings.neutralEcalEnergy ? 2 : 1;
  const bool separate=m==2;
  auto projection=chargedEnergyProjection(terminal,separate ? ChargedEnergyMode::ElectronOnly : settings.chargedEcalEnergyMode);
  TMatrixD h(m,d),r(m,1),v(m,m);h.Zero();v.Zero();
  for(int j=0;j<d;++j)h(0,j)=projection.derivative(0,j);
  r(0,0)=settings.chargedEcalEnergy->first-projection.energy;
  v(0,0)=std::pow(settings.chargedEcalEnergy->second,2);
  std::vector<double> prediction{projection.energy};
  if(separate) {
    double energy=0;
    if(d==6) {
      const auto neutral=neutralEnergyProjection(terminal);energy=neutral.energy;
      for(int j=0;j<d;++j)h(1,j)=neutral.derivative(0,j);
    }
    prediction.push_back(energy);r(1,0)=settings.neutralEcalEnergy->first-energy;
    v(1,1)=std::pow(settings.neutralEcalEnergy->second,2);
  }
  const TMatrixD s=h*terminal.covariance*transpose(h)+v;
  auto profiled=profileFsr(r,s);diagnostic->activeMask=profiled.activeMask;diagnostic->dimensions=m;
  const TMatrixD ipCross=conditionFsrTrajectory(means,covs,gains,h,v,r,profiled.activeMask);
  TMatrixD totalH(1,d);totalH.Zero();double variance=0,energy=0;
  const TMatrixD displacement=stateDifference(means.back(),terminal.mean);
  for(int i=0;i<m;++i) {
    const double f=(profiled.activeMask&(1<<i)) ? 0. : r(i,0)-(h*displacement)(i,0);
    if(f < -1.e-7 || !std::isfinite(f))throw std::runtime_error("Invalid fitted FSR energy");
    if(i==0)diagnostic->firstEcalPart=std::max(0.,f);else diagnostic->neutralPart=std::max(0.,f);
    energy+=std::max(0.,f);
    if(!(profiled.activeMask&(1<<i))) {variance+=v(i,i);for(int j=0;j<d;++j)totalH(0,j)+=h(i,j);}
  }
  variance+=(totalH*covs.back()*transpose(totalH))(0,0);
  diagnostic->energy=energy;diagnostic->variance=variance;
  const TMatrixD ipPhoton=(-1.)*(ipCross*transpose(totalH));
  TrackState post;post.mean=means.front();post.covariance=covs.front();post.pivot={0,0,0};
  TMatrixD joint(6,6);joint.Zero();
  for(int i=0;i<5;++i)for(int j=0;j<5;++j)joint(i,j)=post.covariance(i,j);
  for(int i=0;i<5;++i)joint(i,5)=joint(5,i)=ipPhoton(i,0);
  joint(5,5)=variance;
  diagnostic->p4=electronAndFsrP4(post,energy,joint);
  diagnostic->post=toEDM(post,m_adapter.bz(),edm4hep::TrackState::AtIP);
  for(int i=0;i<6;++i)for(int j=0;j<6;++j)diagnostic->jointIpEnergyCovariance.push_back(joint(i,j));
  terminal.mean=means.back();terminal.covariance=covs.back();
  const auto finalCharged=chargedEnergyProjection(terminal,separate ? ChargedEnergyMode::ElectronOnly : settings.chargedEcalEnergyMode);
  diagnostic->nonlinearClosure=std::abs(finalCharged.energy-prediction[0]-(h*displacement)(0,0));
  if(separate && d==6)diagnostic->nonlinearClosure=std::max(diagnostic->nonlinearClosure,std::abs(neutralEnergyProjection(terminal).energy-prediction[1]-(h*displacement)(1,0)));
  if(diagnostic->nonlinearClosure>std::max(.005,1.e-4*settings.chargedEcalEnergy->first))
    throw std::runtime_error("FSR affine/full-energy closure failed");
  for(std::size_t i=0;i<nodes.size();++i)if(nodes[i]>=0) {
    auto& state=result.smoothed.at(nodes[i]);for(int j=0;j<5;++j)state.mean(j,0)=means[i](j,0);
    for(int a=0;a<5;++a)for(int b=0;b<5;++b)state.covariance(a,b)=covs[i](a,b);
    if(means[i].GetNrows()==6) {
      const int position=nodes[i]-core.intervals.front()-1;
      auto& persistent=result.persistentSmoothed.at(position);persistent.mean=means[i];persistent.covariance=covs[i];
    }
  }
  if(!result.breakpoints.empty()) {
    result.breakpoints.front().fittedLogLoss=means.back()(5,0);
    result.breakpoints.front().fittedVariance=covs.back()(5,5);
  }
  if(!core.intervals.empty()) {
    const auto momentum=[](const TMatrixD& state) {return std::hypot(1.,state(4,0))/std::abs(state(2,0));};
    const double before=momentum(result.smoothed.at(core.intervals.front()).mean);
    const double after=momentum(result.persistentSmoothed.front().mean);
    diagnostic->materialClosure=std::abs(before-after*std::exp(result.breakpoints.front().fittedLogLoss));
    if(diagnostic->materialClosure>std::max(.01,1.e-4*before))
      throw std::runtime_error("FSR material-loss nonlinear closure failed");
  }
  result.endpoint=result.smoothed;result.ip=diagnostic->post;
  beam.smoothed=post;beam.smoothedDrho=post.mean(0,0);
  beam.smoothedAffineDrho=beam.filteredDrho+(beam.observation.derivative*stateDifference(post.mean,beam.filtered.mean))(0,0);
  beam.smoothedChi2=beam.smoothedAffineDrho*beam.smoothedAffineDrho/beam.beamVariance;
  diagnostic->likelihood=baseScore;diagnostic->likelihood.quadratic+=profiled.quadratic;
  diagnostic->likelihood.logDeterminant+=profiled.logDet;
  diagnostic->likelihood.nll2+=profiled.quadratic+profiled.logDet+m*std::log(2.*std::acos(-1.));
  diagnostic->likelihood.measurementDimensions+=m;
  result.smoothedTotalChi2=diagnostic->likelihood.quadratic;
  // Only the profiled total is evaluated after the joint ECAL conditioning.
  result.smoothedChi2.clear();result.smoothedMeasurementChi2.clear();
  result.smoothedProcessChi2.clear();result.smoothedNativeMeasurementChi2.clear();
  result.smoothedSeedChi2=std::numeric_limits<double>::quiet_NaN();
  result.smoothedChi2Status=2;result.smoothedChi2Error="FSR profiled total; no per-hit decomposition";
  result.beamFsr=diagnostic;return result;
}
}
