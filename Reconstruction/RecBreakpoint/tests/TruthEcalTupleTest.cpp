#include "RecoAssociationTuple.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackCollection.h"
#include "TLeaf.h"
#include "TTree.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
void require(bool ok, const char* message) {
  if (!ok) throw std::runtime_error(message);
}
}

int main() {
  edm4hep::TrackCollection tracks;
  edm4hep::MCParticleCollection particles;
  edm4hep::SimTrackerHitCollection hits;
  edm4hep::MCRecoTrackParticleAssociationCollection associations;
  edm4hep::ReconstructedParticleCollection pfos;
  edm4hep::ClusterCollection clusters;
  gsftruth::G4MaterialStepCollection steps;
  gsftruth::SimTrackerHitG4StepLinkCollection links;
  auto first = tracks.create(), second = tracks.create(), unmatched = tracks.create();
  auto electron = particles.create(), muon = particles.create();
  electron.setPDG(11); muon.setPDG(13);
  auto a = associations.create(); a.setRec(first); a.setSim(electron); a.setWeight(4);
  auto b = associations.create(); b.setRec(second); b.setSim(muon); b.setWeight(3);
  auto eh = hits.create(), mh = hits.create(); eh.setMCParticle(electron); mh.setMCParticle(muon);
  auto el = links.create(), ml = links.create();
  el.setSimHit(eh); el.setTrackID(42); ml.setSimHit(mh); ml.setTrackID(7);
  // Deliberately reverse step order and use nontrivial IDs; no index/ID-1 shortcut.
  auto ms = steps.create(), es = steps.create();
  ms.setTrackID(7); ms.setPdg(13); ms.setFirstEcalEntry(1); ms.setStepNumber(200);
  ms.setPostPosition({10, 20, 2930}); ms.setPostMomentum({1, 2, 3});
  es.setTrackID(42); es.setPdg(11); es.setFirstEcalEntry(1); es.setStepNumber(300);
  es.setPostPosition({1831, 30, 40}); es.setPostMomentum({4, 5, 6});

  TTree tree("test", "test");
  breakpoint::TrackRecoTuple tuple;
  tuple.book(tree);
  auto value = [&](const char* branch) { return tree.GetLeaf(branch)->GetValue(); };
  auto check = [&](edm4hep::Track track, int status,
                   const gsftruth::G4MaterialStepCollection* inputSteps,
                   const gsftruth::SimTrackerHitG4StepLinkCollection* inputLinks) {
    tuple.assign(track, associations, pfos, clusters, inputSteps, inputLinks);
    tree.Fill(); tree.GetEntry(tree.GetEntries() - 1);
    if (value("truth_ecal_entry_status") != status)
      throw std::runtime_error("Row " + std::to_string(tree.GetEntries() - 1) +
          ": expected status " + std::to_string(status) + ", got " +
          std::to_string(value("truth_ecal_entry_status")));
    if (status != 1) {
      for (const char* branch : {"truth_ecal_entry_x", "truth_ecal_entry_y", "truth_ecal_entry_z",
                                "truth_ecal_entry_px", "truth_ecal_entry_py", "truth_ecal_entry_pz"})
        require(std::isnan(value(branch)), "Invalid row retained old entry kinematics");
      require(value("truth_ecal_entry_step_number") == -1, "Invalid row retained step number");
    }
  };
  check(first, 1, &steps, &links);
  require(value("truth_ecal_entry_g4_track_id") == 42 && value("truth_ecal_entry_x") == 1831 &&
          value("truth_ecal_entry_pz") == 6 && value("truth_ecal_entry_step_number") == 300,
          "Electron entry did not follow its own MC/G4 identity");
  check(second, 1, &steps, &links);
  require(value("truth_ecal_entry_g4_track_id") == 7 && value("truth_ecal_entry_z") == 2930 &&
          value("truth_ecal_entry_px") == 1, "Muon entry mixed with electron");
  check(unmatched, -1, &steps, &links);
  check(first, -5, nullptr, &links);
  check(first, -5, &steps, nullptr);
  gsftruth::SimTrackerHitG4StepLinkCollection emptyLinks;
  check(first, -2, &steps, &emptyLinks);
  es.setFirstEcalEntry(0); check(first, 0, &steps, &links);
  require(value("truth_ecal_entry_g4_track_id") == 42, "Known ID lost without entry");
  es.setFirstEcalEntry(1);
  es.setPostPosition({std::numeric_limits<double>::quiet_NaN(), 30, 40});
  check(first, -4, &steps, &links); es.setPostPosition({1831, 30, 40});
  es.setPdg(13); check(first, -3, &steps, &links); es.setPdg(11);
  auto duplicate = steps.create(); duplicate.setTrackID(42); duplicate.setFirstEcalEntry(1);
  duplicate.setPdg(11); check(first, -3, &steps, &links); duplicate.setFirstEcalEntry(0);
  auto ambiguous = links.create(); ambiguous.setSimHit(eh); ambiguous.setTrackID(99);
  check(first, -3, &steps, &links); ambiguous.setTrackID(42);
  check(first, 1, &steps, &links); // Repeated links to the same ID are not ambiguity.
  auto tie = associations.create(); tie.setRec(first); tie.setSim(muon); tie.setWeight(4);
  check(first, -1, &steps, &links);
}
