#include "CollectedNeutralTuple.h"
#include "NeutralLossCandidate.h"
#include "ChargedEnergyCandidate.h"
#include "HcalEnergyCandidate.h"
#include "TTree.h"
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>

namespace gsf {
void NeutralCollectionSettings::validate() const {
  if (thetaMrad >= 3141 || phiMrad >= 3141)
    throw std::invalid_argument("Neutral window must be below 3141 mrad, matching BP steering");
  const edm4hep::ReconstructedParticleCollection pfos;
  const edm4hep::ClusterCollection clusters;
  edm4hep::TrackCollection dummy;
  const auto track = dummy.create();
  breakpoint::collectNeutralLoss(track, pfos, clusters, thetaMrad * 1.e-3,
                                phiMrad * 1.e-3, stochastic, constant);
  breakpoint::collectHcalEnergy(pfos, {}, clusters, hcalScale, hcalStochastic, hcalConstant);
}

void CollectedNeutralTuple::book(TTree& tree) {
  tree.Branch("neutral_loss_theta_window_mrad", &settings.thetaMrad);
  tree.Branch("neutral_loss_phi_window_mrad", &settings.phiMrad);
  tree.Branch("neutral_loss_stochastic_error", &settings.stochastic);
  tree.Branch("neutral_loss_constant_error", &settings.constant);
  tree.Branch("hcal_energy_scale", &settings.hcalScale);
  tree.Branch("hcal_stochastic_error", &settings.hcalStochastic);
  tree.Branch("hcal_constant_error", &settings.hcalConstant);
  tree.Branch("neutral_collection_available", &snapshotAvailable);
  tree.Branch("neutral_collection_hcal_available", &hcalAvailable);
  tree.Branch("neutral_collection_track_input_index", &inputIndices);
  tree.Branch("neutral_collection_kf_calo_status", &referenceStatus);
  tree.Branch("neutral_collection_kf_calo_reference_point_mm", &references);
  tree.Branch("collected_neutral_ecal_status", &status);
  tree.Branch("collected_neutral_ecal_error", &errors);
  tree.Branch("neutral_collection_charged_ecal_status", &chargedStatus);
  tree.Branch("neutral_collection_charged_ecal_error", &chargedErrors);
  tree.Branch("collected_neutral_ecal_cluster_count", &counts);
  tree.Branch("collected_neutral_ecal_pfo_indices", &pfoIndices);
  tree.Branch("collected_neutral_ecal_cluster_indices", &clusterIndices);
  tree.Branch("collected_neutral_ecal_cluster_energy", &clusterEnergy);
  tree.Branch("collected_neutral_ecal_cluster_energy_error", &clusterError);
  tree.Branch("collected_neutral_ecal_energy", &ecalEnergy);
  tree.Branch("collected_neutral_ecal_energy_error", &ecalError);
  tree.Branch("collected_neutral_hcal_energy", &hcalEnergy);
  tree.Branch("collected_neutral_hcal_energy_error", &hcalError);
  tree.Branch("collected_neutral_hcal_cluster_indices", &hcalIndices);
  tree.Branch("collected_neutral_hcal_cluster_raw_energy", &hcalRawEnergy);
  tree.Branch("collected_neutral_hcal_cluster_energy", &hcalClusterEnergy);
  tree.Branch("collected_neutral_hcal_cluster_energy_error", &hcalClusterError);
  tree.Branch("collected_neutral_hcal_cluster_x", &hcalX);
  tree.Branch("collected_neutral_hcal_cluster_y", &hcalY);
  tree.Branch("collected_neutral_hcal_cluster_z", &hcalZ);
  tree.Branch("collected_neutral_hcal_link_pfo_index", &hcalPfoLink);
  tree.Branch("collected_neutral_hcal_link_cluster_index", &hcalClusterLink);
  tree.Branch("collected_neutral_calo_energy", &caloEnergy);
  tree.Branch("collected_neutral_calo_energy_error", &caloError);
}

void CollectedNeutralTuple::assign(const edm4hep::TrackCollection& tracks,
    const std::vector<int>& inputs, const edm4hep::ReconstructedParticleCollection* pfos,
    const edm4hep::ClusterCollection* ecal, const edm4hep::ClusterCollection* hcal) {
  const auto steering = settings;
  *this = CollectedNeutralTuple{};
  settings = steering;
  if (bool(pfos) != bool(ecal)) throw std::runtime_error("Incomplete pre-ECAL/PFO snapshot");
  snapshotAvailable = pfos && ecal;
  hcalAvailable = snapshotAvailable && hcal;
  const double nan = std::numeric_limits<double>::quiet_NaN();
  std::set<int> seen;
  std::map<int, edm4hep::Cluster> hcalByIndex;
  if (hcalAvailable) for (const auto& cluster : *hcal)
    hcalByIndex.emplace(cluster.id().index, cluster);
  for (int input : inputs) {
    if (input < 0 || std::size_t(input) >= tracks.size() || !seen.insert(input).second)
      throw std::runtime_error("Invalid neutral-collection input track index");
    const auto track = tracks.at(input);
    inputIndices.push_back(input);
    std::vector<double> reference(3, nan);
    int hasReference = 0;
    for (const auto& state : track.getTrackStates()) {
      if (state.location != edm4hep::TrackState::AtCalorimeter) continue;
      const auto& p = state.referencePoint;
      reference = {p.x, p.y, p.z};
      hasReference = std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z) &&
                     std::hypot(std::hypot(p.x, p.y), p.z) > 0 ? 1 : -1;
      break;
    }
    references.push_back(reference);
    referenceStatus.push_back(hasReference);
    breakpoint::NeutralLossCandidate neutral;
    std::string error;
    int state = 0;
    if (snapshotAvailable) {
      try {
        neutral = breakpoint::collectNeutralLoss(track, *pfos, *ecal,
            settings.thetaMrad * 1.e-3, settings.phiMrad * 1.e-3,
            settings.stochastic, settings.constant);
        state = !neutral.hasReference ? 0 : neutral.clusterIndices.empty() ? 1 : 2;
      } catch (const std::exception& failure) {
        state = -1; error = failure.what(); neutral.energy = neutral.sigmaEnergy = nan;
      }
    }
    status.push_back(state); errors.push_back(error);
    counts.push_back(neutral.clusterIndices.size());
    pfoIndices.push_back(neutral.pfoIndices);
    clusterIndices.push_back(neutral.clusterIndices);
    clusterEnergy.push_back(neutral.clusterEnergies);
    clusterError.push_back(neutral.clusterEnergyErrors);
    ecalEnergy.push_back(snapshotAvailable ? neutral.energy : nan);
    ecalError.push_back(snapshotAvailable ? neutral.sigmaEnergy : nan);
    breakpoint::HcalEnergyCandidate selected;
    int chargedState = 0;
    std::string chargedError;
    if (hcalAvailable) {
      // Match BP's charged-HCAL exclusion, including unavailable/ambiguous charged ECAL.
      std::vector<int> chargedOwners;
      try {
        const auto charged = breakpoint::collectChargedEnergy(track, *pfos, *ecal, *ecal,
            settings.stochastic, settings.constant);
        chargedState = charged.status;
        if (charged.status == 2) chargedOwners = charged.pfoIndices;
      } catch (const std::exception& failure) {
        chargedState = -1; chargedError = failure.what();
      }
      const auto chargedHcal = breakpoint::collectHcalEnergy(*pfos,
          chargedOwners, *hcal,
          settings.hcalScale, settings.hcalStochastic, settings.hcalConstant);
      selected = breakpoint::collectHcalEnergy(*pfos, neutral.pfoIndices, *hcal,
          settings.hcalScale, settings.hcalStochastic, settings.hcalConstant,
          chargedHcal.clusterIndices);
    }
    chargedStatus.push_back(chargedState); chargedErrors.push_back(chargedError);
    hcalEnergy.push_back(hcalAvailable ? selected.energy : nan);
    hcalError.push_back(hcalAvailable ? selected.sigma : nan);
    hcalIndices.push_back(selected.clusterIndices);
    hcalRawEnergy.push_back(selected.rawEnergies);
    hcalClusterEnergy.push_back(selected.energies);
    hcalClusterError.push_back(selected.errors);
    std::vector<double> x, y, z;
    for (int index : selected.clusterIndices) {
      const auto& p = hcalByIndex.at(index).getPosition();
      x.push_back(p.x); y.push_back(p.y); z.push_back(p.z);
    }
    hcalX.push_back(x); hcalY.push_back(y); hcalZ.push_back(z);
    std::vector<int> owners, links;
    if (hcalAvailable) {
      const std::set<int> selectedPfos(neutral.pfoIndices.begin(), neutral.pfoIndices.end());
      const std::set<int> selectedClusters(selected.clusterIndices.begin(), selected.clusterIndices.end());
      std::set<std::pair<int, int>> edges;
      for (const auto& pfo : *pfos) if (selectedPfos.count(pfo.id().index))
        for (const auto& cluster : pfo.getClusters())
          if (cluster.id().collectionID == hcal->getID() && selectedClusters.count(cluster.id().index))
            edges.emplace(pfo.id().index, cluster.id().index);
      for (const auto& edge : edges) { owners.push_back(edge.first); links.push_back(edge.second); }
    }
    hcalPfoLink.push_back(owners); hcalClusterLink.push_back(links);
    caloEnergy.push_back(hcalAvailable ? neutral.energy + selected.energy : nan);
    caloError.push_back(hcalAvailable ? (selected.energy > 0 ?
        std::hypot(neutral.sigmaEnergy, selected.sigma) : neutral.sigmaEnergy) : nan);
  }
}
} // namespace gsf
