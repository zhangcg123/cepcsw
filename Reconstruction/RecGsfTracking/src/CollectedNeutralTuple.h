#ifndef RECGSFTRACKING_COLLECTEDNEUTRALTUPLE_H
#define RECGSFTRACKING_COLLECTEDNEUTRALTUPLE_H

#include "edm4hep/TrackCollection.h"
#include "edm4hep/ClusterCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include <string>
#include <vector>

class TTree;

namespace gsf {
struct NeutralCollectionSettings {
  double thetaMrad = 25, phiMrad = 200;
  double stochastic = .011, constant = .004;
  double hcalScale = 4, hcalStochastic = .30, hcalConstant = .07;
  void validate() const;
};

// Passive observations keyed by original input tracks, independent of fit success.
struct CollectedNeutralTuple {
  NeutralCollectionSettings settings;
  int snapshotAvailable = 0, hcalAvailable = 0;
  std::vector<int> inputIndices, status, referenceStatus, counts;
  std::vector<int> chargedStatus;
  std::vector<std::vector<double>> references;
  std::vector<std::string> errors;
  std::vector<std::string> chargedErrors;
  std::vector<double> ecalEnergy, ecalError, hcalEnergy, hcalError, caloEnergy, caloError;
  std::vector<std::vector<int>> pfoIndices, clusterIndices, hcalIndices;
  std::vector<std::vector<double>> clusterEnergy, clusterError;
  std::vector<std::vector<double>> hcalRawEnergy, hcalClusterEnergy, hcalClusterError;
  std::vector<std::vector<double>> hcalX, hcalY, hcalZ;
  std::vector<std::vector<int>> hcalPfoLink, hcalClusterLink;

  void book(TTree& tree);
  void assign(const edm4hep::TrackCollection& tracks, const std::vector<int>& inputs,
              const edm4hep::ReconstructedParticleCollection* pfos,
              const edm4hep::ClusterCollection* ecal,
              const edm4hep::ClusterCollection* hcal);
};
} // namespace gsf
#endif
