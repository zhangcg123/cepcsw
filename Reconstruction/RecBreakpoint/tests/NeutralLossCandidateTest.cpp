#include "NeutralLossCandidate.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/TrackCollection.h"

#include <cmath>
#include <stdexcept>

namespace {
void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
void near(double actual, double expected) {
  require(std::abs(actual - expected) < 1.e-12, "Neutral energy/error mismatch");
}
}

int main() {
  edm4hep::TrackCollection tracks;
  edm4hep::ClusterCollection clusters;
  edm4hep::CalorimeterHitCollection hits;
  edm4hep::ReconstructedParticleCollection pfos;
  auto first = tracks.create(), second = tracks.create(), noReference = tracks.create();
  edm4hep::TrackState state{};
  state.location = edm4hep::TrackState::AtCalorimeter;
  state.referencePoint = {1000, 0, 0};
  first.addToTrackStates(state);
  state.referencePoint = {0, 1000, 0};
  second.addToTrackStates(state);
  auto makeNeutral = [&](float energy, float x, float y, float z, bool hitSupported) {
    auto cluster = clusters.create();
    cluster.setEnergy(energy);
    cluster.setPosition({x, y, z});
    if (hitSupported) cluster.addToHits(hits.create());
    auto pfo = pfos.create();
    pfo.setCharge(0);
    pfo.addToClusters(cluster);
    return pfo;
  };
  auto shared = makeNeutral(4, 1000, 10, 0, true);
  makeNeutral(9, 1000, -10, 0, true);
  makeNeutral(16, 10, 1000, 0, true);
  makeNeutral(25, 1000, 0, 0, false); // synthetic: excluded even inside the window
  makeNeutral(36, 1000, 0, 100, true); // outside theta window
  makeNeutral(49, 1000, 400, 0, true); // outside phi window
  auto charged = makeNeutral(64, 1000, 0, 0, true);
  charged.setCharge(-1);
  auto tracked = makeNeutral(81, 1000, 0, 0, true);
  tracked.addToTracks(first);
  auto duplicate = pfos.create();
  duplicate.setCharge(0);
  duplicate.addToClusters(shared.getClusters()[0]);
  auto collect = [&](const edm4hep::Track& track) {
    return breakpoint::collectNeutralLoss(track, pfos, clusters, .01, .2, .011, .004);
  };

  const auto a = collect(first), b = collect(second), empty = collect(noReference);
  require(a.hasReference && b.hasReference && !empty.hasReference, "Reference status mismatch");
  require(a.clusterIndices == std::vector<int>({0, 1}), "First track selection mismatch/duplicate");
  require(b.clusterIndices == std::vector<int>({2}), "Second track reused first selection");
  require(a.clusterEnergies == std::vector<double>({4, 9}), "Per-cluster energies not aligned");
  require(b.clusterEnergies == std::vector<double>({16}), "Second-track energy mismatch");
  near(a.energy, 13);
  near(b.energy, 16);
  near(a.clusterEnergyErrors[0], .011 * 2 + .004 * 4);
  near(a.clusterEnergyErrors[1], .011 * 3 + .004 * 9);
  near(a.sigmaEnergy, std::hypot(a.clusterEnergyErrors[0], a.clusterEnergyErrors[1]));
  require(empty.clusterIndices.empty() && empty.clusterEnergies.empty() &&
          empty.clusterEnergyErrors.empty(), "No-reference row retained earlier clusters");
  near(empty.energy, 0);
  near(empty.sigmaEnergy, 0);
  auto noMatch = tracks.create();
  state.referencePoint = {-1000, 0, 0};
  noMatch.addToTrackStates(state);
  const auto none = collect(noMatch);
  require(none.hasReference && none.clusterIndices.empty(), "No-match/reference distinction lost");
  near(none.energy, 0);
  require(collect(first).clusterIndices == a.clusterIndices, "Selection depends on earlier track order");
}
