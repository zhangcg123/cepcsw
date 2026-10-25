#include "ChargedEnergyMeasurement.h"
#include "ScalarMeasurementSite.h"
#include <stdexcept>

using namespace breakpoint;
void close(double a, double b, double tolerance=1.e-7) {
  if (!std::isfinite(a) || !std::isfinite(b) || std::abs(a-b) > tolerance*(1.+std::abs(b)))
    throw std::runtime_error("Charged energy projection regression failed");
}
int main() {
  for (double kappa : {-.1, .1}) for (double b : {0., .15}) {
    LossTrackState state;
    state.mean.Zero(); state.covariance.UnitMatrix(); state.covariance *= 1.e-5;
    state.mean(2,0)=kappa; state.mean(4,0)=.4; state.mean(5,0)=b;
    state.covariance(2,5)=state.covariance(5,2)=2.e-6;
    for (auto mode : {ChargedEnergyMode::ElectronOnly, ChargedEnergyMode::ElectronPhoton}) {
      const auto projection = chargedEnergyProjection(state,mode);
      const double electron = std::hypot(1.,.4)/std::abs(kappa);
      close(projection.energy, mode == ChargedEnergyMode::ElectronOnly ? electron : electron*std::exp(b));
      for (int column=0; column<6; ++column) {
        auto plus=state, minus=state;
        plus.mean(column,0)+=1.e-7; minus.mean(column,0)-=1.e-7;
        close(projection.derivative(0,column),
              (chargedEnergyProjection(plus,mode).energy-chargedEnergyProjection(minus,mode).energy)/2.e-7);
      }
      GaussianHitModel observation{projection.derivative,TMatrixD(1,1),TMatrixD(1,1)};
      observation.noise(0,0)=.04; observation.residual(0,0)=.1;
      const auto filtered=updateScalarState(state,observation);
      const auto& h=projection.derivative;
      const TMatrixD gain=state.covariance*transpose(h)*(1./((h*state.covariance*transpose(h))(0,0)+.04));
      for (int i=0; i<6; ++i)
        close(filtered.mean(i,0), state.mean(i,0)+gain(i,0)*.1);
    }
  }
  if (parseChargedEnergyMode("ElectronPhoton") != ChargedEnergyMode::ElectronPhoton)
    throw std::runtime_error("Wrong charged energy mode");
  bool rejected=false;
  try { parseChargedEnergyMode("Sum"); } catch (const std::invalid_argument&) { rejected=true; }
  if (!rejected) throw std::runtime_error("Invalid charged energy mode accepted");
}
