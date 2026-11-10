#include "RecoAssociationTuple.h"
#include "TrackTruthMatch.h"

#include "edm4hep/EDM4hepVersion.h"
#include "TTree.h"

#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>

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

void PreEcalTuple::book(TTree& tree) {
  tree.Branch("pre_ecal_available", &m_available);
  tree.Branch("pre_ecal_cluster_index", &m_clusterIndex);
  tree.Branch("pre_ecal_cluster_hit_count", &m_clusterHitCount);
  tree.Branch("pre_ecal_cluster_energy", &m_energy);
  tree.Branch("pre_ecal_cluster_x", &m_x);
  tree.Branch("pre_ecal_cluster_y", &m_y);
  tree.Branch("pre_ecal_cluster_z", &m_z);
  tree.Branch("pre_ecal_pfo_charge", &m_pfoCharge);
  tree.Branch("pre_ecal_pfo_track_pfo_index", &m_trackPfoIndex);
  tree.Branch("pre_ecal_pfo_track_input_index", &m_trackInputIndex);
  tree.Branch("pre_ecal_pfo_cluster_pfo_index", &m_clusterPfoIndex);
  tree.Branch("pre_ecal_pfo_cluster_index", &m_pfoClusterIndex);
}

void PreEcalTuple::assign(const edm4hep::TrackCollection& tracks,
                          const edm4hep::ReconstructedParticleCollection* pfos,
                          const edm4hep::ClusterCollection* clusters) {
  m_available = 0;
  m_clusterIndex.clear(); m_clusterHitCount.clear();
  m_energy.clear(); m_x.clear(); m_y.clear(); m_z.clear(); m_pfoCharge.clear();
  m_trackPfoIndex.clear(); m_trackInputIndex.clear();
  m_clusterPfoIndex.clear(); m_pfoClusterIndex.clear();
  if (bool(pfos) != bool(clusters))
    throw std::runtime_error("Incomplete pre-reclustering ECAL/PFO snapshot");
  if (!clusters) return;
  m_available = 1;
  for (const auto& cluster : *clusters) {
    const auto position = cluster.getPosition();
    m_clusterIndex.push_back(cluster.id().index);
    m_clusterHitCount.push_back(cluster.hits_size());
    m_energy.push_back(cluster.getEnergy());
    m_x.push_back(position.x); m_y.push_back(position.y); m_z.push_back(position.z);
  }
  for (const auto& pfo : *pfos) {
    m_pfoCharge.push_back(pfo.getCharge());
    for (const auto& track : pfo.getTracks()) {
      if (!track.isAvailable() || track.id().collectionID != tracks.getID() ||
          track.id().index < 0 || size_t(track.id().index) >= tracks.size())
        throw std::runtime_error("Pre-reclustering PFO has an invalid CompleteTracks link");
      m_trackPfoIndex.push_back(pfo.id().index);
      m_trackInputIndex.push_back(track.id().index);
    }
    for (const auto& cluster : pfo.getClusters()) {
      if (!cluster.isAvailable())
        throw std::runtime_error("Pre-reclustering PFO has an unavailable cluster link");
      if (cluster.id().collectionID != clusters->getID()) continue;
      if (cluster.id().index < 0 || size_t(cluster.id().index) >= clusters->size())
        throw std::runtime_error("Pre-reclustering PFO has an invalid ECAL cluster link");
      m_clusterPfoIndex.push_back(pfo.id().index);
      m_pfoClusterIndex.push_back(cluster.id().index);
    }
  }
}

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
  tree.Branch("truth_ecal_entry_status", &m_truthEcalStatus);
  tree.Branch("truth_ecal_entry_g4_track_id", &m_truthEcalTrackID);
  tree.Branch("truth_ecal_entry_step_number", &m_truthEcalStep);
  tree.Branch("truth_ecal_entry_x", &m_truthEcalX);
  tree.Branch("truth_ecal_entry_y", &m_truthEcalY);
  tree.Branch("truth_ecal_entry_z", &m_truthEcalZ);
  tree.Branch("truth_ecal_entry_px", &m_truthEcalPx);
  tree.Branch("truth_ecal_entry_py", &m_truthEcalPy);
  tree.Branch("truth_ecal_entry_pz", &m_truthEcalPz);
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
                            const edm4hep::ClusterCollection& ecalClusters,
                            const gsftruth::G4MaterialStepCollection* steps,
                            const gsftruth::SimTrackerHitG4StepLinkCollection* links) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  m_truthPt = m_truthEnergy = m_truthPurity = nan;
  m_truthPx = m_truthPy = m_truthPz = m_truthCharge = nan;
  m_truthVertexX = m_truthVertexY = m_truthVertexZ = nan;
  m_truthEndpointX = m_truthEndpointY = m_truthEndpointZ = nan;
  m_truthEcalStatus = m_truthEcalTrackID = m_truthEcalStep = -1;
  m_truthEcalX = m_truthEcalY = m_truthEcalZ = nan;
  m_truthEcalPx = m_truthEcalPy = m_truthEcalPz = nan;
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

  const auto match = matchTrackTruth(track, associations);
  const auto best = match.particle;
  const int bestCount = match.count, bestMultiplicity = match.multiplicity;
  m_truthLinkedHitCount = match.linkedCount;
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
      assignEcalEntry(best, steps, links);
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

void TrackRecoTuple::assignEcalEntry(const edm4hep::MCParticle& particle,
                                    const gsftruth::G4MaterialStepCollection* steps,
                                    const gsftruth::SimTrackerHitG4StepLinkCollection* links) {
  if (!steps || !links) { m_truthEcalStatus = -5; return; }
  std::set<int> trackIDs;
  for (const auto& link : *links) {
    const auto hit = link.getSimHit();
    if (hit.isAvailable() && hit.getMCParticle().isAvailable() &&
        hit.getMCParticle() == particle && link.getTrackID() > 0)
      trackIDs.insert(link.getTrackID());
  }
  if (trackIDs.empty()) { m_truthEcalStatus = -2; return; }
  if (trackIDs.size() != 1) { m_truthEcalStatus = -3; return; }
  m_truthEcalTrackID = *trackIDs.begin();
  gsftruth::G4MaterialStep entry{nullptr};
  for (const auto& step : *steps) {
    if (step.getTrackID() != m_truthEcalTrackID || !step.getFirstEcalEntry()) continue;
    if (entry.isAvailable() || step.getPdg() != particle.getPDG()) {
      m_truthEcalStatus = -3;
      return;
    }
    entry = step;
  }
  // Legacy files and particles without a recorded arrival are indistinguishable.
  if (!entry.isAvailable()) { m_truthEcalStatus = 0; return; }
  const auto position = entry.getPostPosition();
  const auto momentum = entry.getPostMomentum();
  if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
      !std::isfinite(momentum.x) || !std::isfinite(momentum.y) || !std::isfinite(momentum.z)) {
    m_truthEcalStatus = -4;
    return;
  }
  m_truthEcalStatus = 1;
  m_truthEcalStep = entry.getStepNumber();
  m_truthEcalX = position.x; m_truthEcalY = position.y; m_truthEcalZ = position.z;
  m_truthEcalPx = momentum.x; m_truthEcalPy = momentum.y; m_truthEcalPz = momentum.z;
}

void NeutralPfoTuple::book(TTree& tree, const std::string& photonPrefix, bool includePfos) {
  if (includePfos) {
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
  }
  tree.Branch((photonPrefix+"index").c_str(), &m_photonIndex);
  tree.Branch((photonPrefix+"g4_track_id").c_str(), &m_photonTrackID);
  tree.Branch((photonPrefix+"parent_g4_track_id").c_str(), &m_photonParentTrackID);
  tree.Branch((photonPrefix+"parent_step_number").c_str(), &m_photonParentStepNumber);
  tree.Branch((photonPrefix+"birth_x").c_str(), &m_photonBirthX);
  tree.Branch((photonPrefix+"birth_y").c_str(), &m_photonBirthY);
  tree.Branch((photonPrefix+"birth_z").c_str(), &m_photonBirthZ);
  tree.Branch((photonPrefix+"birth_energy").c_str(), &m_photonBirthEnergy);
  tree.Branch((photonPrefix+"birth_px").c_str(), &m_photonBirthPx);
  tree.Branch((photonPrefix+"birth_py").c_str(), &m_photonBirthPy);
  tree.Branch((photonPrefix+"birth_pz").c_str(), &m_photonBirthPz);
  tree.Branch((photonPrefix+"ecal_entry_status").c_str(), &m_photonEcalEntryStatus);
  tree.Branch((photonPrefix+"ecal_entry_step_number").c_str(), &m_photonEcalEntryStepNumber);
  tree.Branch((photonPrefix+"ecal_entry_x").c_str(), &m_photonEcalEntryX);
  tree.Branch((photonPrefix+"ecal_entry_y").c_str(), &m_photonEcalEntryY);
  tree.Branch((photonPrefix+"ecal_entry_z").c_str(), &m_photonEcalEntryZ);
  tree.Branch((photonPrefix+"ecal_entry_energy").c_str(), &m_photonEcalEntryEnergy);
  tree.Branch((photonPrefix+"ecal_entry_px").c_str(), &m_photonEcalEntryPx);
  tree.Branch((photonPrefix+"ecal_entry_py").c_str(), &m_photonEcalEntryPy);
  tree.Branch((photonPrefix+"ecal_entry_pz").c_str(), &m_photonEcalEntryPz);
  tree.Branch((photonPrefix+"last_step_status").c_str(), &m_photonLastStepStatus);
  tree.Branch((photonPrefix+"last_step_number").c_str(), &m_photonLastStepNumber);
  tree.Branch((photonPrefix+"last_step_process_subtype").c_str(), &m_photonLastStepProcessSubtype);
  tree.Branch((photonPrefix+"last_step_pre_step_status").c_str(), &m_photonLastStepPreStepStatus);
  tree.Branch((photonPrefix+"last_step_post_step_status").c_str(), &m_photonLastStepPostStepStatus);
  tree.Branch((photonPrefix+"last_step_track_status").c_str(), &m_photonLastStepTrackStatus);
  tree.Branch((photonPrefix+"last_step_pre_volume_copy_no").c_str(), &m_photonLastStepPreVolumeCopyNo);
  tree.Branch((photonPrefix+"last_step_post_volume_copy_no").c_str(), &m_photonLastStepPostVolumeCopyNo);
  tree.Branch((photonPrefix+"last_step_post_in_ecal").c_str(), &m_photonLastStepPostInEcal);
  tree.Branch((photonPrefix+"last_step_pre_x").c_str(), &m_photonLastStepPreX);
  tree.Branch((photonPrefix+"last_step_pre_y").c_str(), &m_photonLastStepPreY);
  tree.Branch((photonPrefix+"last_step_pre_z").c_str(), &m_photonLastStepPreZ);
  tree.Branch((photonPrefix+"last_step_post_x").c_str(), &m_photonLastStepPostX);
  tree.Branch((photonPrefix+"last_step_post_y").c_str(), &m_photonLastStepPostY);
  tree.Branch((photonPrefix+"last_step_post_z").c_str(), &m_photonLastStepPostZ);
  tree.Branch((photonPrefix+"last_step_pre_energy").c_str(), &m_photonLastStepPreEnergy);
  tree.Branch((photonPrefix+"last_step_post_energy").c_str(), &m_photonLastStepPostEnergy);
  tree.Branch((photonPrefix+"last_step_pre_px").c_str(), &m_photonLastStepPrePx);
  tree.Branch((photonPrefix+"last_step_pre_py").c_str(), &m_photonLastStepPrePy);
  tree.Branch((photonPrefix+"last_step_pre_pz").c_str(), &m_photonLastStepPrePz);
  tree.Branch((photonPrefix+"last_step_post_px").c_str(), &m_photonLastStepPostPx);
  tree.Branch((photonPrefix+"last_step_post_py").c_str(), &m_photonLastStepPostPy);
  tree.Branch((photonPrefix+"last_step_post_pz").c_str(), &m_photonLastStepPostPz);
  tree.Branch((photonPrefix+"last_step_pre_time").c_str(), &m_photonLastStepPreTime);
  tree.Branch((photonPrefix+"last_step_post_time").c_str(), &m_photonLastStepPostTime);
  tree.Branch((photonPrefix+"last_step_length").c_str(), &m_photonLastStepLength);
  tree.Branch((photonPrefix+"last_step_energy_deposit").c_str(), &m_photonLastStepEnergyDeposit);
  if (!includePfos) {
    tree.Branch((photonPrefix+"generator_mc_index").c_str(),&m_generatorIndex);
    tree.Branch((photonPrefix+"emitter_mc_index").c_str(),&m_emitterIndex);
    tree.Branch((photonPrefix+"final_electron_mc_index").c_str(),&m_finalElectronIndex);
    tree.Branch((photonPrefix+"origin_status").c_str(),&m_originStatus);
  }
}

void NeutralPfoTuple::assign(int eventIndex,
                             const edm4hep::ReconstructedParticleCollection& pfos,
                             const edm4hep::ClusterCollection& ecalClusters,
                             const gsftruth::G4BremsPhotonCollection& photons,
                             const gsftruth::G4BremsPhotonStepCollection& photonSteps) {
  m_eventIndex = eventIndex;
  m_photonCollectionID = photons.getID();
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
void NeutralPfoTuple::assignOrigins(const gsftruth::GeneratorPhotonLinkCollection* origins) {
  m_generatorIndex.assign(m_photonIndex.size(),-1); m_emitterIndex=m_finalElectronIndex=m_generatorIndex;
  m_originStatus.assign(m_photonIndex.size(),0);
  if (!origins) return;
  std::map<int,gsftruth::GeneratorPhotonLink> byIndex;
  for (const auto& link : *origins) {
    if (!link.getPhoton().isAvailable()) continue;
    if (link.getPhoton().id().collectionID!=m_photonCollectionID)
      throw std::runtime_error("Generator origin refers to a different photon collection");
    if (!byIndex.emplace(link.getPhoton().id().index,link).second)
      throw std::runtime_error("Duplicate generator photon provenance");
  }
  for (std::size_t i=0;i<m_photonIndex.size();++i) {
    const auto found=byIndex.find(m_photonIndex[i]);
    if (found==byIndex.end()) continue;
    m_generatorIndex[i]=found->second.getPhotonMCIndex();
    m_emitterIndex[i]=found->second.getEmitterMCIndex();
    m_finalElectronIndex[i]=found->second.getFinalElectronMCIndex();
    m_originStatus[i]=found->second.getStatus();
  }
}
} // namespace breakpoint
