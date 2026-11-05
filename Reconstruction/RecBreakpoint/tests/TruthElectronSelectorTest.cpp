#include "TruthElectronSelector.h"
#include "TrackTruthMatch.h"
#include "edm4hep/TrackerHitCollection.h"
#include <cassert>
#include <limits>

int main() {
  edm4hep::MCParticleCollection particles;
  edm4hep::TrackCollection tracks;
  edm4hep::TrackerHitCollection hits;
  edm4hep::MCRecoTrackParticleAssociationCollection associations;
  auto particle = [&](int pdg, int status) {
    auto p = particles.create(); p.setPDG(pdg); p.setGeneratorStatus(status); return p;
  };
  auto link = [&](auto track, auto mc, float weight) {
    auto a = associations.create(); a.setRec(track); a.setSim(mc); a.setWeight(weight); return a;
  };
  auto track = [&](int count) {
    auto t = tracks.create();
    for (int i = 0; i < count; ++i) t.addToTrackerHits(hits.create());
    return t;
  };
  auto higgs = particle(25, 2), z = particle(23, 2);
  z.addToParents(higgs);
  auto higgsElectron = particle(11, 1), higgsPositron = particle(-11, 1);
  higgsElectron.addToParents(z); higgsPositron.addToParents(z);
  // The real generator record shares a shower intermediate and omits the production Z.
  auto rootElectron = particle(11, 2), rootPositron = particle(-11, 2);
  auto shower = particle(94, 2);
  shower.addToParents(rootElectron); shower.addToParents(rootPositron);
  auto electron = particle(11, 1), positron = particle(-11, 1);
  electron.addToParents(shower); positron.addToParents(shower);
  auto secondary = particle(11, 0);
  auto unrelated = track(4), electronTrack = track(4), positronTrack = track(4);
  link(unrelated, higgsElectron, 4);
  auto electronLink = link(electronTrack, electron, 3);
  link(positronTrack, positron, 4);
  link(track(4), secondary, 4);
  breakpoint::TruthElectronSelector selector;
  auto select = [&]() { return selector.select(particles, tracks, associations); };
  auto result = select();
  assert(result.status == 1 && result.trackIndices == std::vector<int>({1, 2}));
  assert(result.mcIndices == std::vector<int>({electron.id().index, positron.id().index}));
  assert(!result.accepts(0) && result.accepts(1) && result.accepts(2));
  assert(result.candidateCounts == std::vector<int>({1, 1}));

  auto duplicate = track(5);
  auto duplicateLink = link(duplicate, electron, 3);
  assert(select().trackIndices[0] == 1); // Equal count, lower purity loses.
  duplicateLink.setWeight(4);
  assert(select().trackIndices[0] == 4); // Higher hit count wins, no renumbering.
  electronLink.setWeight(4);
  assert(select().trackIndices[0] == 1); // Purity breaks equal-count tie.
  electronTrack.addToTrackerHits(hits.create());
  result = select();
  assert(result.status == 2 && result.trackIndices[0] == -1 && result.trackIndices[1] == 2);
  duplicateLink.setWeight(0);
  auto tie = link(electronTrack, higgsElectron, 4);
  assert(select().status == 2); // MC ambiguity is not a valid electron match.
  tie.setWeight(0);
  electronLink.setWeight(2);
  link(electronTrack, electron, 2); // Repeated association links aggregate.
  link(electronTrack, higgsElectron, std::numeric_limits<float>::quiet_NaN());
  assert(select().status == 1);
  assert(breakpoint::matchTrackTruth(electronTrack, associations).count == 4);

  auto extra = particle(11, 1);
  assert(select().status == -2); // Do not guess among extra non-Higgs electrons.
  extra.setGeneratorStatus(0);
  positron.setGeneratorStatus(2);
  assert(select().status == -2);
  positron.setGeneratorStatus(1);
  higgs.setGeneratorStatus(0);
  assert(select().status == -1);
  higgs.setGeneratorStatus(2);
  auto cycle = particle(11, 2);
  cycle.addToParents(cycle); electron.addToParents(cycle);
  assert(select().status == -4);

  edm4hep::MCRecoTrackParticleAssociationCollection emptyAssociations;
  electron.setGeneratorStatus(2); // Avoid the intentionally malformed candidate.
  auto replacement = particle(11, 1);
  result = selector.select(particles, tracks, emptyAssociations);
  assert(result.status == -3 && result.trackIndices == std::vector<int>({-1, -1}));
  assert(breakpoint::TruthElectronSelector::Result{}.status == 0);
}
