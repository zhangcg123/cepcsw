#include "NeutralEnergyMeasurement.h"
#include "TrackLikelihood.h"
#include <cmath>
#include <stdexcept>
#include <sstream>

using namespace breakpoint;
namespace {
void close(double a, double b, double tolerance=1.e-8) {
  if (!std::isfinite(a) || !std::isfinite(b) || std::abs(a-b)>tolerance*(1.+std::abs(b)))
  {
    std::ostringstream error;
    error.precision(17); error << "Neutral-energy test mismatch: " << a << " != " << b;
    throw std::runtime_error(error.str());
  }
}
}
int main() {
  LossTrackState state;
  state.mean.Zero(); state.covariance.UnitMatrix(); state.covariance *= .0001;
  state.mean(2,0)=-.1; state.mean(4,0)=.4; state.mean(5,0)=.15;
  state.covariance(2,5)=state.covariance(5,2)=.00003;
  const auto projection=neutralEnergyProjection(state);
  for (int column=0;column<6;++column) {
    auto plus=state, minus=state;
    plus.mean(column,0)+=1.e-7; minus.mean(column,0)-=1.e-7;
    close(projection.derivative(0,column),
          (neutralEnergyProjection(plus).energy-neutralEnergyProjection(minus).energy)/2.e-7);
  }
  // Mapping is invertible for fixed b; the post-loss observation recovers p_pre-p_post.
  TMatrixD mapping;
  const auto mapped=applyBreakpointLoss(state,mapping);
  const double before=std::hypot(1.,state.mean(4,0))/std::abs(state.mean(2,0));
  close(neutralEnergyProjection(mapped).energy,before*(1.-std::exp(-state.mean(5,0))));

  const double energy=projection.energy+.3, sigma=.04;
  auto observation=updateNeutralEnergy(state,energy,sigma);
  const auto& h=projection.derivative;
  const TMatrixD priorPrecision=inverseCovariance(state.covariance);
  const TMatrixD batchCov=inverseCovariance(priorPrecision+transpose(h)*h*(1./(sigma*sigma)));
  const TMatrixD batchDelta=batchCov*transpose(h)*(observation.residual/(sigma*sigma));
  for (int i=0;i<6;++i) {
    close(observation.filtered.mean(i,0)-state.mean(i,0),batchDelta(i,0));
    for (int j=0;j<6;++j) close(observation.filtered.covariance(i,j),batchCov(i,j));
  }
  if (observation.filtered.mean(2,0)==state.mean(2,0) ||
      observation.filtered.mean(5,0)==state.mean(5,0))
    throw std::runtime_error("Neutral observation did not update both track and loss");

  // A later tracker observation refines the photon estimate; the measured E stays fixed.
  TMatrixD trackerH(1,6); trackerH.Zero(); trackerH(0,2)=1;
  const double trackerR=1.e-6, trackerResidual=.002;
  const double trackerS=(trackerH*batchCov*transpose(trackerH))(0,0)+trackerR;
  const TMatrixD gain=batchCov*transpose(trackerH)*(1./trackerS);
  auto smoothed=observation.filtered;
  smoothed.mean+=gain*trackerResidual;
  smoothed.covariance-=gain*trackerH*batchCov;
  assignSmoothedNeutralEnergy(observation,smoothed);
  close(observation.energy,energy); close(observation.sigma,sigma);
  if (observation.smoothedEnergy==observation.filteredEnergy ||
      observation.smoothedError>observation.filteredError)
    throw std::runtime_error("Downstream information failed to refine photon estimate");

  // The virtual photon site contributes once to the shared Gaussian likelihood.
  GaussianTrackModel model;
  model.seedCovariance.ResizeTo(5,5);
  TMatrixD cross(1,5), birth(6,5), birthNoise(6,6), seedShift(5,1);
  birth.Zero(); birthNoise.Zero(); seedShift.Zero();
  for (int i=0;i<5;++i) {
    birth(i,i)=1.; cross(0,i)=state.covariance(5,i);
    for (int j=0;j<5;++j) model.seedCovariance(i,j)=state.covariance(i,j);
  }
  const TMatrixD conditional=cross*inverseCovariance(model.seedCovariance);
  for (int i=0;i<5;++i) birth(5,i)=conditional(0,i);
  birthNoise(5,5)=state.covariance(5,5)-(conditional*transpose(cross))(0,0);
  // The track likelihood starts in 5D, then introduces the loss coordinate.
  GaussianHitModel seedHit{TMatrixD(1,5),TMatrixD(1,1),TMatrixD(1,1)};
  seedHit.derivative.Zero(); seedHit.noise(0,0)=1.; seedHit.residual.Zero();
  model.hits.push_back(seedHit);
  model.transitions.push_back({birth,birthNoise,seedShift});
  model.hits.push_back(observation.observation);
  GaussianHitModel tracker;
  tracker.derivative.ResizeTo(trackerH); tracker.derivative=trackerH;
  tracker.noise.ResizeTo(1,1); tracker.noise(0,0)=trackerR;
  tracker.residual.ResizeTo(1,1); tracker.residual(0,0)=trackerResidual;
  model.hits.push_back(tracker);
  TMatrixD identity(6,6), zero(6,6); identity.UnitMatrix(); zero.Zero();
  model.transitions.push_back({identity,zero,state.mean-observation.filtered.mean});
  const auto score=evaluateTrackLikelihood(model);
  const double expected=observation.chi2+trackerResidual*trackerResidual/trackerS;
  close(score.quadratic,expected,1.e-7);
  close(score.logDeterminant,std::log(observation.innovationVariance*trackerS),1.e-7);
  const TMatrixD delta=smoothed.mean-state.mean;
  const double trackerPostResidual=trackerResidual-(trackerH*(smoothed.mean-observation.filtered.mean))(0,0);
  const double complete=(transpose(delta)*priorPrecision*delta)(0,0)+observation.smoothedChi2
      +trackerPostResidual*trackerPostResidual/trackerR;
  close(complete,score.quadratic,1.e-7);
  close(evaluateSmoothedTrackLikelihood(model,complete).nll2,score.nll2,1.e-7);

  const auto weak=updateNeutralEnergy(state,energy,1.e10);
  for (int i=0;i<6;++i) close(weak.filtered.mean(i,0),state.mean(i,0));
  bool rejected=false;
  try { updateNeutralEnergy(state,energy,0); } catch (const std::invalid_argument&) { rejected=true; }
  if (!rejected) throw std::runtime_error("Invalid neutral measurement accepted");
  rejected=false;
  try { updateNeutralEnergy(state,1.e6,sigma); } catch (const std::runtime_error&) { rejected=true; }
  if (!rejected) throw std::runtime_error("Energy update crossed curvature pole");
  rejected=false;
  auto wrongSign=smoothed; wrongSign.mean(2,0)=-state.mean(2,0);
  try { assignSmoothedNeutralEnergy(observation,wrongSign); }
  catch (const std::runtime_error&) { rejected=true; }
  if (!rejected) throw std::runtime_error("Smoothed energy crossed curvature pole");
}
