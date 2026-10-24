#include "BeamSpotMeasurement.h"
#include "TrackLikelihood.h"
#include "ScalarMeasurementSite.h"
#include <cmath>
#include <stdexcept>
#include <iostream>

using namespace breakpoint;
namespace {
void close(double a, double b, double tolerance=1.e-7) {
  if (!std::isfinite(a) || !std::isfinite(b) || std::abs(a-b)>tolerance*(1.+std::abs(b)))
    throw std::runtime_error("Beam measurement mismatch: " + std::to_string(a) + " vs " + std::to_string(b));
}
}
int main() {
  TrackState state;
  state.pivot = {0,0,0};
  state.mean.Zero(); state.covariance.UnitMatrix(); state.covariance *= .001;
  state.mean(0,0)=.02; state.mean(1,0)=1.57; state.mean(2,0)=-.1; state.mean(4,0)=.3;
  const BeamSpotSettings beam;
  auto measurement = updateBeamSpot(state, beam);
  const auto& h = measurement.observation.derivative;
  for (int j=0;j<5;++j) {
    auto plus=state, minus=state;
    const double step=1.e-6;
    plus.mean(j,0)+=step; minus.mean(j,0)-=step;
    const double derivative = (-beamSpotObservation(plus,beam).residual(0,0)
                               +beamSpotObservation(minus,beam).residual(0,0))/(2*step);
    close(h(0,j),derivative,1.e-5);
  }
  const double variance = measurement.beamVariance;
  const TMatrixD information = inverseCovariance(state.covariance) + transpose(h)*h*(1./variance);
  const TMatrixD expectedCov = inverseCovariance(information);
  const TMatrixD expectedMean = state.mean + expectedCov*transpose(h)*measurement.observation.residual*(1./variance);
  for(int i=0;i<5;++i) {
    close(measurement.filtered.mean(i,0),expectedMean(i,0));
    for(int j=0;j<5;++j) close(measurement.filtered.covariance(i,j),expectedCov(i,j));
  }
  // Check that conditioning the seed and multiplying by p(beam|seed) counts
  // the observation exactly once, against the joint Gaussian model.
  GaussianHitModel hit{TMatrixD(1,5),TMatrixD(1,1),TMatrixD(1,1)};
  hit.derivative.Zero(); hit.derivative(0,0)=1.; hit.noise(0,0)=.00001; hit.residual(0,0)=.002;
  measurement.transport.UnitMatrix(); measurement.transport(0,1)=20.; measurement.transport(1,2)=.1;
  measurement.noise.Zero(); measurement.noise(0,0)=.000003; measurement.noise(1,1)=1.e-7;
  measurement.firstHitPredicted.mean = measurement.transport * measurement.filtered.mean;
  measurement.firstHitPredicted.covariance = measurement.transport * measurement.filtered.covariance *
      transpose(measurement.transport) + measurement.noise;
  GaussianTrackModel joint;
  joint.seedCovariance.ResizeTo(state.covariance);
  joint.seedCovariance=state.covariance;
  joint.hits={measurement.observation,hit};
  joint.transitions.push_back({measurement.transport,measurement.noise,stateDifference(state.mean,measurement.filtered.mean)});
  GaussianTrackModel conditional;
  conditional.seedCovariance.ResizeTo(measurement.firstHitPredicted.covariance);
  conditional.seedCovariance=measurement.firstHitPredicted.covariance;
  conditional.hits={hit};
  const auto a=evaluateTrackLikelihood(joint),b=evaluateTrackLikelihood(conditional);
  close(a.quadratic,b.quadratic+measurement.chi2);
  close(a.nll2,b.nll2+measurement.chi2+std::log(2*std::acos(-1.)*measurement.innovationVariance));
  const auto finalHit = updateScalarState(measurement.firstHitPredicted,hit);
  smoothBeamSpot(measurement,finalHit);
  const TMatrixD seedDelta=stateDifference(measurement.smoothed.mean,state.mean);
  const double seedChi2=(transpose(seedDelta)*inverseCovariance(state.covariance)*seedDelta)(0,0);
  const TMatrixD hitResidual=hit.residual-hit.derivative*stateDifference(finalHit.mean,measurement.firstHitPredicted.mean);
  const double complete=seedChi2+measurement.smoothedChi2+measurement.processChi2+
      hitResidual(0,0)*hitResidual(0,0)/hit.noise(0,0);
  close(complete,a.quadratic);
  // Independent batch posterior after marginalizing the boundary process noise.
  const TMatrixD hf=hit.derivative*measurement.transport;
  const double effectiveNoise=hit.noise(0,0)+(hit.derivative*measurement.noise*transpose(hit.derivative))(0,0);
  const TMatrixD batchCov=inverseCovariance(information+transpose(hf)*hf*(1./effectiveNoise));
  const TMatrixD initialHitResidual=hit.residual+hf*stateDifference(measurement.filtered.mean,state.mean);
  const TMatrixD batchMean=state.mean+batchCov*(transpose(h)*measurement.observation.residual*(1./variance)
      +transpose(hf)*initialHitResidual*(1./effectiveNoise));
  for(int i=0;i<5;++i) {
    close(measurement.smoothed.mean(i,0),batchMean(i,0));
    for(int j=0;j<5;++j) close(measurement.smoothed.covariance(i,j),batchCov(i,j));
  }
  bool rejected=false;
  try { auto invalid=beam; invalid.sigmaX=0; updateBeamSpot(state,invalid); }
  catch(const std::invalid_argument&) { rejected=true; }
  if(!rejected) throw std::runtime_error("Invalid beam width accepted");
  std::cout << "Beam measurement/native update/material-edge RTS/likelihood checks passed\n";
}
