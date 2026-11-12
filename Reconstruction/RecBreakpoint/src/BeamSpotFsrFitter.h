#ifndef BP_BEAM_SPOT_FSR_FITTER_H
#define BP_BEAM_SPOT_FSR_FITTER_H
#include "TrackLikelihood.h"
#include "TrackState.h"
#include "edm4hep/TrackState.h"
#include <array>
#include <vector>
namespace breakpoint {
struct FsrProfile {
  int activeMask=0;
  double quadratic=0, logDet=0;
  TMatrixD energy, innovation;
};
// Profile nonnegative additive photon energies, with a flat (no BH) FSR prior.
FsrProfile profileFsr(const TMatrixD& residual,const TMatrixD& covariance);
TMatrixD conditionFsrTrajectory(std::vector<TMatrixD>& means, std::vector<TMatrixD>& covariance,
    const std::vector<TMatrixD>& gains, const TMatrixD& h, const TMatrixD& noise,
    const TMatrixD& residual, int activeMask);
struct ElectronFsrP4 {
  std::array<double,4> electron{},photon{}; // E,px,py,pz [GeV]
  std::array<double,64> covariance{}; // joint electron/photon p4, row-major
};
ElectronFsrP4 electronAndFsrP4(const TrackState& electron,double energy,const TMatrixD& joint);
struct BeamSpotFsrResult {
  TrackLikelihoodResult likelihood;
  int activeMask=0, dimensions=0;
  double energy=0, variance=0, nonlinearClosure=0, materialClosure=0;
  double firstEcalPart=0, neutralPart=0;
  edm4hep::TrackState post{};
  ElectronFsrP4 p4;
  std::vector<double> jointIpEnergyCovariance;
};
}
#endif
