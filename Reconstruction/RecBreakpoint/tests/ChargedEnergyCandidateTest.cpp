#include "ChargedEnergyCandidate.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include <cmath>
#include <stdexcept>

void require(bool ok) {
  if (!ok) throw std::runtime_error("Charged ECAL candidate regression failed");
}
int main() {
  using namespace breakpoint;
  edm4hep::TrackCollection tracks;
  edm4hep::ClusterCollection final, merged;
  edm4hep::CalorimeterHitCollection finalHits, mergedHits;
  edm4hep::ReconstructedParticleCollection pfos;
  const auto track = tracks.create(), other = tracks.create();
  auto charged = final.create(); charged.setEnergy(10);
  auto hit = finalHits.create(); hit.setCellID(32); hit.setPosition({1000, 10, 20});
  charged.addToHits(hit);
  auto pfo = pfos.create(); pfo.setCharge(-1); pfo.addToTracks(track); pfo.addToClusters(charged);
  pfo.addToClusters(charged); // duplicate relation must not duplicate energy
  auto source = merged.create(); source.setEnergy(13);
  auto copied = mergedHits.create(); copied.setCellID(32); copied.setPosition(hit.getPosition());
  copied.setEnergy(99); source.addToHits(copied); // ignore calibrated hit energies and IDs
  auto extra = mergedHits.create(); extra.setCellID(33); extra.setPosition({1000, 11, 20});
  source.addToHits(extra); // source can contain hits absent from the final cached list
  auto neutral = merged.create(); neutral.setEnergy(5); neutral.addToHits(extra);
  auto synthetic = merged.create(); synthetic.setEnergy(8); // no hits: never a match
  const auto old = collectChargedEnergy(track, pfos, final, final, .011, .004);
  require(old.status == 2 && old.energy == 10 && old.clusterIndices.size() == 1);
  require(std::abs(old.sigma - (.011*std::sqrt(10.)+.004*10.)) < 1.e-12);
  const auto selected = collectChargedEnergy(track, pfos, final, merged, .011, .004);
  require(selected.status == 2 && selected.energy == 13 && selected.clusterIndices == std::vector<int>{0});
  require(collectChargedEnergy(other, pfos, final, merged, .011, .004).status == 1);
  auto duplicate = merged.create(); duplicate.setEnergy(12); duplicate.addToHits(copied);
  require(collectChargedEnergy(track, pfos, final, merged, .011, .004).status == 3);
  // Pre-PFOs link directly to merged-energy copies: no hit-rematching ambiguity.
  edm4hep::ReconstructedParticleCollection prePfos;
  auto pre = prePfos.create(); pre.setCharge(-1); pre.addToTracks(track);
  pre.addToClusters(source);
  edm4hep::ClusterCollection hcal;
  auto hc = hcal.create(); hc.setEnergy(50); hc.addToHits(extra);
  pre.addToClusters(hc); // HCAL is not part of the ECAL energy measurement.
  const auto direct = collectChargedEnergy(track, prePfos, merged, merged, .011, .004);
  require(direct.status == 2 && direct.energy == 13 && direct.clusterIndices == std::vector<int>{0});
  edm4hep::ClusterCollection missing;
  require(collectChargedEnergy(track, pfos, final, missing, .011, .004).status == 1);
  auto secondPfo = pfos.create(); secondPfo.setCharge(-1); secondPfo.addToTracks(track);
  require(collectChargedEnergy(track, pfos, final, final, .011, .004).status == 3);
}
