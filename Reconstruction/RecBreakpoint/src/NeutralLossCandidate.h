#ifndef RECBREAKPOINT_NEUTRALLOSSCANDIDATE_H
#define RECBREAKPOINT_NEUTRALLOSSCANDIDATE_H

#include "edm4hep/Track.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/ClusterCollection.h"

#include <vector>

namespace breakpoint {

struct NeutralLossCandidate {
  bool hasReference = false; // Valid input-track AtCalorimeter reference point
  double energy = 0;       // GeV, sum of selected hit-supported ECAL clusters
  double sigmaEnergy = 0;  // GeV, independent-cluster uncertainty approximation
  std::vector<int> clusterIndices;
  std::vector<double> clusterEnergies, clusterEnergyErrors;
};

/// Reconstructed-only association; no photon or Geant4 truth enters the fit.
/// Angles refer to the input KF AtCalorimeter reference point from the IP.
NeutralLossCandidate collectNeutralLoss(const edm4hep::Track& track,
    const edm4hep::ReconstructedParticleCollection& pfos,
    const edm4hep::ClusterCollection& ecalClusters,
    double thetaWindow, double phiWindow,
    double stochasticError, double constantError);

} // namespace breakpoint
#endif
