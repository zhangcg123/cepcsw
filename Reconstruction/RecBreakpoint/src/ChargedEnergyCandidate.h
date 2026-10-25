#ifndef RECBREAKPOINT_CHARGEDENERGYCANDIDATE_H
#define RECBREAKPOINT_CHARGEDENERGYCANDIDATE_H

#include "edm4hep/Track.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include <vector>

namespace breakpoint {
struct ChargedEnergyCandidate {
  int status = 1; // 1 unavailable, 2 selected, 3 ambiguous
  double energy = 0, sigma = 0;
  std::vector<int> clusterIndices;
};

/// Keep final PFO-track ownership; alternate snapshots must uniquely contain
/// the final charged cluster's constituent hit identities. No geometry guess.
ChargedEnergyCandidate collectChargedEnergy(const edm4hep::Track& track,
    const edm4hep::ReconstructedParticleCollection& pfos,
    const edm4hep::ClusterCollection& finalClusters,
    const edm4hep::ClusterCollection& inputClusters,
    double stochasticError, double constantError);
} // namespace breakpoint
#endif
