#include "RecoAssociationTuple.h"

#include "edm4hep/EDM4hepVersion.h"
#include "TTree.h"

#include <cmath>
#include <limits>
#include <map>
#include <set>

namespace breakpoint {
namespace {
int particleCode(const edm4hep::ReconstructedParticle& pfo) {
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
  return pfo.getPDG();
#else
  return pfo.getType();
#endif
}

std::set<edm4hep::Cluster> ecalObjects(const edm4hep::ClusterCollection& clusters) {
  std::set<edm4hep::Cluster> result;
  for (const auto& cluster : clusters) result.insert(cluster);
  return result;
}
} // namespace

void TrackRecoTuple::book(TTree& tree) {
  tree.Branch("truth_pt", &m_truthPt);
  tree.Branch("truth_match_status", &m_truthStatus); // 0 no match, 1 unique best, 2 tied best
  tree.Branch("truth_mc_index", &m_truthMCIndex);
  tree.Branch("truth_pdg", &m_truthPDG);
  tree.Branch("truth_generator_status", &m_truthGeneratorStatus);
  tree.Branch("truth_simulator_status", &m_truthSimulatorStatus);
  tree.Branch("truth_energy", &m_truthEnergy);
  tree.Branch("truth_px", &m_truthPx);
  tree.Branch("truth_py", &m_truthPy);
  tree.Branch("truth_pz", &m_truthPz);
  tree.Branch("truth_charge", &m_truthCharge);
  tree.Branch("truth_vertex_x", &m_truthVertexX);
  tree.Branch("truth_vertex_y", &m_truthVertexY);
  tree.Branch("truth_vertex_z", &m_truthVertexZ);
  tree.Branch("truth_endpoint_x", &m_truthEndpointX);
  tree.Branch("truth_endpoint_y", &m_truthEndpointY);
  tree.Branch("truth_endpoint_z", &m_truthEndpointZ);
  tree.Branch("truth_parent_mc_index", &m_truthParentMCIndex);
  tree.Branch("truth_track_hit_count", &m_truthTrackHitCount);
  tree.Branch("truth_best_mc_hit_count", &m_truthBestHitCount);
  tree.Branch("truth_linked_hit_count", &m_truthLinkedHitCount);
  tree.Branch("truth_match_purity", &m_truthPurity);
  tree.Branch("charged_pfo_count", &m_chargedPfoCount);
  tree.Branch("charged_pfo_index", &m_chargedPfoIndex);
  tree.Branch("charged_pfo_pdg", &m_chargedPfoPDG);
  tree.Branch("charged_pfo_energy", &m_chargedPfoEnergy);
  tree.Branch("charged_pid_pfo_index", &m_chargedPidPfoIndex);
  tree.Branch("charged_pid_pdg", &m_chargedPidPDG);
  tree.Branch("charged_pid_likelihood", &m_chargedPidLikelihood);
  tree.Branch("charged_ecal_cluster_count", &m_chargedEcalClusterCount);
  tree.Branch("charged_ecal_cluster_pfo_index", &m_chargedEcalClusterPfoIndex);
  tree.Branch("charged_ecal_cluster_index", &m_chargedEcalClusterIndex);
  tree.Branch("charged_ecal_cluster_energy", &m_chargedEcalClusterEnergy);
  tree.Branch("charged_ecal_cluster_x", &m_chargedEcalClusterX);
  tree.Branch("charged_ecal_cluster_y", &m_chargedEcalClusterY);
  tree.Branch("charged_ecal_cluster_z", &m_chargedEcalClusterZ);
}

void TrackRecoTuple::assign(const edm4hep::Track& track,
                            const edm4hep::MCRecoTrackParticleAssociationCollection& associations,
                            const edm4hep::ReconstructedParticleCollection& pfos,
                            const edm4hep::ClusterCollection& ecalClusters) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  m_truthPt = m_truthEnergy = m_truthPurity = nan;
  m_truthPx = m_truthPy = m_truthPz = m_truthCharge = nan;
  m_truthVertexX = m_truthVertexY = m_truthVertexZ = nan;
  m_truthEndpointX = m_truthEndpointY = m_truthEndpointZ = nan;
  m_truthStatus = m_truthPDG = m_truthGeneratorStatus = m_truthSimulatorStatus = 0;
  m_truthMCIndex = -1;
  m_truthParentMCIndex.clear();
  m_truthBestHitCount = m_truthLinkedHitCount = 0;
  m_truthTrackHitCount = track.trackerHits_size();
  m_chargedPfoCount = m_chargedEcalClusterCount = 0;
  m_chargedPfoIndex.clear(); m_chargedPfoPDG.clear(); m_chargedPfoEnergy.clear();
  m_chargedPidPfoIndex.clear(); m_chargedPidPDG.clear(); m_chargedPidLikelihood.clear();
  m_chargedEcalClusterPfoIndex.clear(); m_chargedEcalClusterIndex.clear();
  m_chargedEcalClusterEnergy.clear(); m_chargedEcalClusterX.clear();
  m_chargedEcalClusterY.clear(); m_chargedEcalClusterZ.clear();

  // The association producer counts one truth assignment per reconstructed hit.
  // Aggregate by MC object in case a collection contains repeated links.
  std::map<edm4hep::MCParticle, int> hitCounts;
  for (const auto& association : associations) {
    if (!(association.getRec() == track) || !association.getSim().isAvailable()) continue;
    const int count = static_cast<int>(std::lround(association.getWeight()));
    if (count > 0) hitCounts[association.getSim()] += count;
  }
  edm4hep::MCParticle best;
  int bestCount = 0, bestMultiplicity = 0;
  for (const auto& [particle, count] : hitCounts) {
    m_truthLinkedHitCount += count;
    if (count > bestCount) { best = particle; bestCount = count; bestMultiplicity = 1; }
    else if (count == bestCount) ++bestMultiplicity;
  }
  m_truthBestHitCount = bestCount;
  if (m_truthTrackHitCount > 0)
    m_truthPurity = static_cast<double>(bestCount) / m_truthTrackHitCount;
  if (bestCount > 0) {
    m_truthStatus = bestMultiplicity == 1 ? 1 : 2;
    // Do not publish an arbitrary MC particle's momentum for a tied match.
    if (m_truthStatus == 1) {
      const auto momentum = best.getMomentum();
      m_truthPt = std::hypot(momentum.x, momentum.y);
      m_truthEnergy = best.getEnergy();
      m_truthPx = momentum.x; m_truthPy = momentum.y; m_truthPz = momentum.z;
      m_truthCharge = best.getCharge();
      const auto vertex = best.getVertex();
      const auto endpoint = best.getEndpoint();
      m_truthVertexX = vertex.x; m_truthVertexY = vertex.y; m_truthVertexZ = vertex.z;
      m_truthEndpointX = endpoint.x; m_truthEndpointY = endpoint.y; m_truthEndpointZ = endpoint.z;
      m_truthMCIndex = best.id().index;
      m_truthPDG = best.getPDG();
      m_truthGeneratorStatus = best.getGeneratorStatus();
      m_truthSimulatorStatus = best.getSimulatorStatus();
      for (const auto& parent : best.getParents())
        m_truthParentMCIndex.push_back(parent.id().index);
    }
  }

  const auto ecal = ecalObjects(ecalClusters);
  for (const auto& pfo : pfos) {
    bool ownsTrack = false;
    for (const auto& linkedTrack : pfo.getTracks())
      if (linkedTrack == track) { ownsTrack = true; break; }
    if (!ownsTrack || pfo.getCharge() == 0) continue;
    const int pfoIndex = pfo.id().index;
    ++m_chargedPfoCount;
    m_chargedPfoIndex.push_back(pfoIndex);
    m_chargedPfoPDG.push_back(particleCode(pfo));
    m_chargedPfoEnergy.push_back(pfo.getEnergy());
    for (const auto& pid : pfo.getParticleIDs()) {
      m_chargedPidPfoIndex.push_back(pfoIndex);
      m_chargedPidPDG.push_back(pid.getPDG());
      m_chargedPidLikelihood.push_back(pid.getLikelihood());
    }
    for (const auto& cluster : pfo.getClusters()) {
      if (!ecal.count(cluster)) continue;
      const auto position = cluster.getPosition();
      ++m_chargedEcalClusterCount;
      m_chargedEcalClusterPfoIndex.push_back(pfoIndex);
      m_chargedEcalClusterIndex.push_back(cluster.id().index);
      m_chargedEcalClusterEnergy.push_back(cluster.getEnergy());
      m_chargedEcalClusterX.push_back(position.x);
      m_chargedEcalClusterY.push_back(position.y);
      m_chargedEcalClusterZ.push_back(position.z);
    }
  }
}

void NeutralPfoTuple::book(TTree& tree) {
  tree.Branch("event_index", &m_eventIndex);
  tree.Branch("neutral_pfo_index", &m_pfoIndex);
  tree.Branch("neutral_pfo_pdg", &m_pfoPDG);
  tree.Branch("neutral_pfo_energy", &m_pfoEnergy);
  tree.Branch("neutral_ecal_cluster_pfo_index", &m_ecalClusterPfoIndex);
  tree.Branch("neutral_ecal_cluster_index", &m_ecalClusterIndex);
  tree.Branch("neutral_ecal_cluster_energy", &m_ecalClusterEnergy);
  tree.Branch("neutral_ecal_cluster_x", &m_ecalClusterX);
  tree.Branch("neutral_ecal_cluster_y", &m_ecalClusterY);
  tree.Branch("neutral_ecal_cluster_z", &m_ecalClusterZ);
  tree.Branch("ebrem_photon_index", &m_photonIndex);
  tree.Branch("ebrem_photon_g4_track_id", &m_photonTrackID);
  tree.Branch("ebrem_photon_parent_g4_track_id", &m_photonParentTrackID);
  tree.Branch("ebrem_photon_parent_step_number", &m_photonParentStepNumber);
  tree.Branch("ebrem_photon_birth_x", &m_photonBirthX);
  tree.Branch("ebrem_photon_birth_y", &m_photonBirthY);
  tree.Branch("ebrem_photon_birth_z", &m_photonBirthZ);
  tree.Branch("ebrem_photon_birth_energy", &m_photonBirthEnergy);
  tree.Branch("ebrem_photon_birth_px", &m_photonBirthPx);
  tree.Branch("ebrem_photon_birth_py", &m_photonBirthPy);
  tree.Branch("ebrem_photon_birth_pz", &m_photonBirthPz);
  tree.Branch("ebrem_photon_ecal_entry_status", &m_photonEcalEntryStatus);
  tree.Branch("ebrem_photon_ecal_entry_step_number", &m_photonEcalEntryStepNumber);
  tree.Branch("ebrem_photon_ecal_entry_x", &m_photonEcalEntryX);
  tree.Branch("ebrem_photon_ecal_entry_y", &m_photonEcalEntryY);
  tree.Branch("ebrem_photon_ecal_entry_z", &m_photonEcalEntryZ);
  tree.Branch("ebrem_photon_ecal_entry_energy", &m_photonEcalEntryEnergy);
  tree.Branch("ebrem_photon_ecal_entry_px", &m_photonEcalEntryPx);
  tree.Branch("ebrem_photon_ecal_entry_py", &m_photonEcalEntryPy);
  tree.Branch("ebrem_photon_ecal_entry_pz", &m_photonEcalEntryPz);
  tree.Branch("ebrem_photon_last_step_status", &m_photonLastStepStatus);
  tree.Branch("ebrem_photon_last_step_number", &m_photonLastStepNumber);
  tree.Branch("ebrem_photon_last_step_process_subtype", &m_photonLastStepProcessSubtype);
  tree.Branch("ebrem_photon_last_step_pre_step_status", &m_photonLastStepPreStepStatus);
  tree.Branch("ebrem_photon_last_step_post_step_status", &m_photonLastStepPostStepStatus);
  tree.Branch("ebrem_photon_last_step_track_status", &m_photonLastStepTrackStatus);
  tree.Branch("ebrem_photon_last_step_pre_volume_copy_no", &m_photonLastStepPreVolumeCopyNo);
  tree.Branch("ebrem_photon_last_step_post_volume_copy_no", &m_photonLastStepPostVolumeCopyNo);
  tree.Branch("ebrem_photon_last_step_post_in_ecal", &m_photonLastStepPostInEcal);
  tree.Branch("ebrem_photon_last_step_pre_x", &m_photonLastStepPreX);
  tree.Branch("ebrem_photon_last_step_pre_y", &m_photonLastStepPreY);
  tree.Branch("ebrem_photon_last_step_pre_z", &m_photonLastStepPreZ);
  tree.Branch("ebrem_photon_last_step_post_x", &m_photonLastStepPostX);
  tree.Branch("ebrem_photon_last_step_post_y", &m_photonLastStepPostY);
  tree.Branch("ebrem_photon_last_step_post_z", &m_photonLastStepPostZ);
  tree.Branch("ebrem_photon_last_step_pre_energy", &m_photonLastStepPreEnergy);
  tree.Branch("ebrem_photon_last_step_post_energy", &m_photonLastStepPostEnergy);
  tree.Branch("ebrem_photon_last_step_pre_px", &m_photonLastStepPrePx);
  tree.Branch("ebrem_photon_last_step_pre_py", &m_photonLastStepPrePy);
  tree.Branch("ebrem_photon_last_step_pre_pz", &m_photonLastStepPrePz);
  tree.Branch("ebrem_photon_last_step_post_px", &m_photonLastStepPostPx);
  tree.Branch("ebrem_photon_last_step_post_py", &m_photonLastStepPostPy);
  tree.Branch("ebrem_photon_last_step_post_pz", &m_photonLastStepPostPz);
  tree.Branch("ebrem_photon_last_step_pre_time", &m_photonLastStepPreTime);
  tree.Branch("ebrem_photon_last_step_post_time", &m_photonLastStepPostTime);
  tree.Branch("ebrem_photon_last_step_length", &m_photonLastStepLength);
  tree.Branch("ebrem_photon_last_step_energy_deposit", &m_photonLastStepEnergyDeposit);
}

void NeutralPfoTuple::assign(int eventIndex,
                             const edm4hep::ReconstructedParticleCollection& pfos,
                             const edm4hep::ClusterCollection& ecalClusters,
                             const gsftruth::G4BremsPhotonCollection& photons,
                             const gsftruth::G4BremsPhotonStepCollection& photonSteps) {
  m_eventIndex = eventIndex;
  m_pfoIndex.clear(); m_pfoPDG.clear(); m_pfoEnergy.clear();
  m_ecalClusterPfoIndex.clear(); m_ecalClusterIndex.clear();
  m_ecalClusterEnergy.clear(); m_ecalClusterX.clear();
  m_ecalClusterY.clear(); m_ecalClusterZ.clear();
  m_photonIndex.clear(); m_photonTrackID.clear(); m_photonParentTrackID.clear();
  m_photonParentStepNumber.clear(); m_photonEcalEntryStatus.clear();
  m_photonEcalEntryStepNumber.clear();
  m_photonBirthX.clear(); m_photonBirthY.clear(); m_photonBirthZ.clear();
  m_photonBirthEnergy.clear(); m_photonBirthPx.clear(); m_photonBirthPy.clear(); m_photonBirthPz.clear();
  m_photonEcalEntryX.clear(); m_photonEcalEntryY.clear(); m_photonEcalEntryZ.clear();
  m_photonEcalEntryEnergy.clear(); m_photonEcalEntryPx.clear();
  m_photonEcalEntryPy.clear(); m_photonEcalEntryPz.clear();
  m_photonLastStepStatus.clear(); m_photonLastStepNumber.clear();
  m_photonLastStepProcessSubtype.clear(); m_photonLastStepPreStepStatus.clear();
  m_photonLastStepPostStepStatus.clear(); m_photonLastStepTrackStatus.clear();
  m_photonLastStepPreVolumeCopyNo.clear(); m_photonLastStepPostVolumeCopyNo.clear();
  m_photonLastStepPostInEcal.clear();
  m_photonLastStepPreX.clear(); m_photonLastStepPreY.clear(); m_photonLastStepPreZ.clear();
  m_photonLastStepPostX.clear(); m_photonLastStepPostY.clear(); m_photonLastStepPostZ.clear();
  m_photonLastStepPreEnergy.clear(); m_photonLastStepPostEnergy.clear();
  m_photonLastStepPrePx.clear(); m_photonLastStepPrePy.clear(); m_photonLastStepPrePz.clear();
  m_photonLastStepPostPx.clear(); m_photonLastStepPostPy.clear(); m_photonLastStepPostPz.clear();
  m_photonLastStepPreTime.clear(); m_photonLastStepPostTime.clear();
  m_photonLastStepLength.clear(); m_photonLastStepEnergyDeposit.clear();
  const auto ecal = ecalObjects(ecalClusters);
  for (const auto& pfo : pfos) {
    if (pfo.getCharge() != 0 || !pfo.getTracks().empty()) continue;
    const int pfoIndex = pfo.id().index;
    m_pfoIndex.push_back(pfoIndex);
    m_pfoPDG.push_back(particleCode(pfo));
    m_pfoEnergy.push_back(pfo.getEnergy());
    for (const auto& cluster : pfo.getClusters()) {
      if (!ecal.count(cluster)) continue;
      const auto position = cluster.getPosition();
      m_ecalClusterPfoIndex.push_back(pfoIndex);
      m_ecalClusterIndex.push_back(cluster.id().index);
      m_ecalClusterEnergy.push_back(cluster.getEnergy());
      m_ecalClusterX.push_back(position.x);
      m_ecalClusterY.push_back(position.y);
      m_ecalClusterZ.push_back(position.z);
    }
  }

  // A step is linked to its exact photon object. Keep the highest-numbered
  // recorded step, including photons that never enter ECAL. The recorder's
  // first postInEcal step supplies the separate ECAL entrance.
  std::map<int, gsftruth::G4BremsPhotonStep> firstEcalStep;
  std::map<int, gsftruth::G4BremsPhotonStep> lastStep;
  for (const auto& step : photonSteps) {
    if (!step.getPhoton().isAvailable()) continue;
    const int photonIndex = step.getPhoton().id().index;
    const auto last = lastStep.find(photonIndex);
    if (last == lastStep.end() || step.getStepNumber() > last->second.getStepNumber())
      lastStep[photonIndex] = step;
    if (!step.getPostInEcal()) continue;
    const auto previous = firstEcalStep.find(photonIndex);
    if (previous == firstEcalStep.end() || step.getStepNumber() < previous->second.getStepNumber())
      firstEcalStep[photonIndex] = step;
  }
  const double nan = std::numeric_limits<double>::quiet_NaN();
  for (const auto& photon : photons) {
    const int photonIndex = photon.id().index;
    const auto birth = photon.getPosition();
    const auto momentum = photon.getMomentum();
    m_photonIndex.push_back(photonIndex);
    m_photonTrackID.push_back(photon.getTrackID());
    m_photonParentTrackID.push_back(photon.getParentTrackID());
    m_photonParentStepNumber.push_back(photon.getParentStepNumber());
    m_photonBirthX.push_back(birth.x); m_photonBirthY.push_back(birth.y);
    m_photonBirthZ.push_back(birth.z);
    m_photonBirthEnergy.push_back(photon.getEnergy());
    m_photonBirthPx.push_back(momentum.x); m_photonBirthPy.push_back(momentum.y);
    m_photonBirthPz.push_back(momentum.z);
    const auto last = lastStep.find(photonIndex);
    const bool hasLast = last != lastStep.end();
    m_photonLastStepStatus.push_back(hasLast ? 1 : 0);
    m_photonLastStepNumber.push_back(hasLast ? last->second.getStepNumber() : -1);
    m_photonLastStepProcessSubtype.push_back(hasLast ? last->second.getProcessSubtype() : -1);
    m_photonLastStepPreStepStatus.push_back(hasLast ? last->second.getPreStepStatus() : -1);
    m_photonLastStepPostStepStatus.push_back(hasLast ? last->second.getPostStepStatus() : -1);
    m_photonLastStepTrackStatus.push_back(hasLast ? last->second.getTrackStatus() : -1);
    m_photonLastStepPreVolumeCopyNo.push_back(hasLast ? last->second.getPreVolumeCopyNo() : -1);
    m_photonLastStepPostVolumeCopyNo.push_back(hasLast ? last->second.getPostVolumeCopyNo() : -1);
    m_photonLastStepPostInEcal.push_back(hasLast ? last->second.getPostInEcal() : -1);
    if (hasLast) {
      const auto pre = last->second.getPrePosition();
      const auto post = last->second.getPostPosition();
      const auto preMomentum = last->second.getPreMomentum();
      const auto postMomentum = last->second.getPostMomentum();
      m_photonLastStepPreX.push_back(pre.x); m_photonLastStepPreY.push_back(pre.y);
      m_photonLastStepPreZ.push_back(pre.z);
      m_photonLastStepPostX.push_back(post.x); m_photonLastStepPostY.push_back(post.y);
      m_photonLastStepPostZ.push_back(post.z);
      m_photonLastStepPreEnergy.push_back(last->second.getPreEnergy());
      m_photonLastStepPostEnergy.push_back(last->second.getPostEnergy());
      m_photonLastStepPrePx.push_back(preMomentum.x); m_photonLastStepPrePy.push_back(preMomentum.y);
      m_photonLastStepPrePz.push_back(preMomentum.z);
      m_photonLastStepPostPx.push_back(postMomentum.x); m_photonLastStepPostPy.push_back(postMomentum.y);
      m_photonLastStepPostPz.push_back(postMomentum.z);
      m_photonLastStepPreTime.push_back(last->second.getPreGlobalTime());
      m_photonLastStepPostTime.push_back(last->second.getPostGlobalTime());
      m_photonLastStepLength.push_back(last->second.getStepLength());
      m_photonLastStepEnergyDeposit.push_back(last->second.getEnergyDeposit());
    } else {
      m_photonLastStepPreX.push_back(nan); m_photonLastStepPreY.push_back(nan);
      m_photonLastStepPreZ.push_back(nan);
      m_photonLastStepPostX.push_back(nan); m_photonLastStepPostY.push_back(nan);
      m_photonLastStepPostZ.push_back(nan);
      m_photonLastStepPreEnergy.push_back(nan); m_photonLastStepPostEnergy.push_back(nan);
      m_photonLastStepPrePx.push_back(nan); m_photonLastStepPrePy.push_back(nan);
      m_photonLastStepPrePz.push_back(nan);
      m_photonLastStepPostPx.push_back(nan); m_photonLastStepPostPy.push_back(nan);
      m_photonLastStepPostPz.push_back(nan);
      m_photonLastStepPreTime.push_back(nan); m_photonLastStepPostTime.push_back(nan);
      m_photonLastStepLength.push_back(nan); m_photonLastStepEnergyDeposit.push_back(nan);
    }
    const auto entry = firstEcalStep.find(photonIndex);
    m_photonEcalEntryStatus.push_back(entry == firstEcalStep.end() ? 0 : 1);
    m_photonEcalEntryStepNumber.push_back(entry == firstEcalStep.end() ? -1 : entry->second.getStepNumber());
    if (entry == firstEcalStep.end()) {
      m_photonEcalEntryX.push_back(nan); m_photonEcalEntryY.push_back(nan);
      m_photonEcalEntryZ.push_back(nan); m_photonEcalEntryEnergy.push_back(nan);
      m_photonEcalEntryPx.push_back(nan); m_photonEcalEntryPy.push_back(nan);
      m_photonEcalEntryPz.push_back(nan);
    } else {
      const auto position = entry->second.getPostPosition();
      const auto p = entry->second.getPostMomentum();
      m_photonEcalEntryX.push_back(position.x); m_photonEcalEntryY.push_back(position.y);
      m_photonEcalEntryZ.push_back(position.z);
      m_photonEcalEntryEnergy.push_back(entry->second.getPostEnergy());
      m_photonEcalEntryPx.push_back(p.x); m_photonEcalEntryPy.push_back(p.y);
      m_photonEcalEntryPz.push_back(p.z);
    }
  }
}
} // namespace breakpoint
