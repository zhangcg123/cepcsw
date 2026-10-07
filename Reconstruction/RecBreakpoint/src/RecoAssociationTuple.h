#ifndef RECBREAKPOINT_RECOASSOCIATIONTUPLE_H
#define RECBREAKPOINT_RECOASSOCIATIONTUPLE_H

#include "edm4hep/ClusterCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/Track.h"
#include "GsfTruthEventData/G4BremsPhotonCollection.h"
#include "GsfTruthEventData/G4BremsPhotonStepCollection.h"

#include <vector>

class TTree;

namespace breakpoint {

// One record per CompleteTracks row. Association weights are matched-hit
// counts, not probabilities; truth_pt is never filled from the event gun.
class TrackRecoTuple {
public:
  void book(TTree& tree);
  void assign(const edm4hep::Track& track,
              const edm4hep::MCRecoTrackParticleAssociationCollection& associations,
              const edm4hep::ReconstructedParticleCollection& pfos,
              const edm4hep::ClusterCollection& ecalClusters);

private:
  double m_truthPt = 0, m_truthEnergy = 0, m_truthPurity = 0;
  double m_truthPx = 0, m_truthPy = 0, m_truthPz = 0, m_truthCharge = 0;
  double m_truthVertexX = 0, m_truthVertexY = 0, m_truthVertexZ = 0;
  double m_truthEndpointX = 0, m_truthEndpointY = 0, m_truthEndpointZ = 0;
  int m_truthStatus = 0, m_truthMCIndex = -1, m_truthPDG = 0;
  int m_truthGeneratorStatus = 0, m_truthSimulatorStatus = 0;
  int m_truthBestHitCount = 0, m_truthLinkedHitCount = 0;
  int m_truthTrackHitCount = 0, m_chargedPfoCount = 0, m_chargedEcalClusterCount = 0;
  std::vector<int> m_truthParentMCIndex;
  std::vector<int> m_chargedPfoIndex, m_chargedPfoPDG, m_chargedPidPfoIndex, m_chargedPidPDG;
  std::vector<int> m_chargedEcalClusterPfoIndex, m_chargedEcalClusterIndex;
  std::vector<double> m_chargedPfoEnergy, m_chargedPidLikelihood;
  std::vector<double> m_chargedEcalClusterEnergy, m_chargedEcalClusterX;
  std::vector<double> m_chargedEcalClusterY, m_chargedEcalClusterZ;
};

// One record per processed event, including events without neutral PFOs.
// Its event_index joins to the per-track breakpoint tree within the same file.
class NeutralPfoTuple {
public:
  void book(TTree& tree);
  void assign(int eventIndex, const edm4hep::ReconstructedParticleCollection& pfos,
              const edm4hep::ClusterCollection& ecalClusters,
              const gsftruth::G4BremsPhotonCollection& photons,
              const gsftruth::G4BremsPhotonStepCollection& photonSteps);

private:
  int m_eventIndex = -1;
  std::vector<int> m_pfoIndex, m_pfoPDG, m_ecalClusterPfoIndex, m_ecalClusterIndex;
  std::vector<double> m_pfoEnergy, m_ecalClusterEnergy;
  std::vector<double> m_ecalClusterX, m_ecalClusterY, m_ecalClusterZ;
  std::vector<int> m_photonIndex, m_photonTrackID, m_photonParentTrackID;
  std::vector<int> m_photonParentStepNumber, m_photonEcalEntryStatus, m_photonEcalEntryStepNumber;
  std::vector<double> m_photonBirthX, m_photonBirthY, m_photonBirthZ;
  std::vector<double> m_photonBirthEnergy, m_photonBirthPx, m_photonBirthPy, m_photonBirthPz;
  std::vector<double> m_photonEcalEntryX, m_photonEcalEntryY, m_photonEcalEntryZ;
  std::vector<double> m_photonEcalEntryEnergy, m_photonEcalEntryPx, m_photonEcalEntryPy;
  std::vector<double> m_photonEcalEntryPz;
  std::vector<int> m_photonLastStepStatus, m_photonLastStepNumber;
  std::vector<int> m_photonLastStepProcessSubtype, m_photonLastStepPreStepStatus;
  std::vector<int> m_photonLastStepPostStepStatus, m_photonLastStepTrackStatus;
  std::vector<int> m_photonLastStepPreVolumeCopyNo, m_photonLastStepPostVolumeCopyNo;
  std::vector<int> m_photonLastStepPostInEcal;
  std::vector<double> m_photonLastStepPreX, m_photonLastStepPreY, m_photonLastStepPreZ;
  std::vector<double> m_photonLastStepPostX, m_photonLastStepPostY, m_photonLastStepPostZ;
  std::vector<double> m_photonLastStepPreEnergy, m_photonLastStepPostEnergy;
  std::vector<double> m_photonLastStepPrePx, m_photonLastStepPrePy, m_photonLastStepPrePz;
  std::vector<double> m_photonLastStepPostPx, m_photonLastStepPostPy, m_photonLastStepPostPz;
  std::vector<double> m_photonLastStepPreTime, m_photonLastStepPostTime;
  std::vector<double> m_photonLastStepLength, m_photonLastStepEnergyDeposit;
};

} // namespace breakpoint
#endif
