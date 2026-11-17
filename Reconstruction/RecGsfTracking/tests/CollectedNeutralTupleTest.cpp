#include "CollectedNeutralTuple.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "TMemFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {
void near(double a, double b) { assert(std::abs(a-b) < 1.e-12); }
template<class Function> void throws(Function function) {
  bool failed = false;
  try { function(); } catch (const std::exception&) { failed = true; }
  assert(failed);
}
}

int main() {
  edm4hep::TrackCollection tracks;
  tracks.setID(100);
  auto first = tracks.create(), second = tracks.create(), noReference = tracks.create();
  auto badReference = tracks.create(), noMatch = tracks.create(), overlapping = tracks.create();
  edm4hep::TrackState state{};
  state.location = edm4hep::TrackState::AtCalorimeter;
  state.referencePoint = {1000, 0, 0}; first.addToTrackStates(state); overlapping.addToTrackStates(state);
  state.referencePoint = {0, 1000, 0}; second.addToTrackStates(state);
  first.addToTrackStates(state); // The first AtCalorimeter state owns the window.
  state.referencePoint = {-1000, 0, 0}; noMatch.addToTrackStates(state);
  state.referencePoint = {0, 0, 0}; badReference.addToTrackStates(state);
  edm4hep::ClusterCollection ecal, hcal;
  edm4hep::CalorimeterHitCollection hits;
  edm4hep::ReconstructedParticleCollection pfos;
  ecal.setID(101); hcal.setID(102); hits.setID(103); pfos.setID(104);
  auto neutral = [&](float energy, float x, float y, float z, bool supported=true) {
    auto cluster = ecal.create(); cluster.setEnergy(energy); cluster.setPosition({x,y,z});
    if (supported) cluster.addToHits(hits.create());
    auto pfo = pfos.create(); pfo.setCharge(0); pfo.addToClusters(cluster);
    return pfo;
  };
  auto a = neutral(4,1000,10,0), b = neutral(9,1000,-10,0), c = neutral(16,0,1000,0);
  auto duplicate = pfos.create(); duplicate.setCharge(0); duplicate.addToClusters(a.getClusters()[0]);
  neutral(25,1000,0,100); // Outside theta.
  neutral(36,1000,400,0); // Outside phi.
  neutral(49,1000,0,0,false);
  auto charged = neutral(50,1000,0,0); charged.setCharge(-1); charged.addToTracks(first);
  auto tracked = neutral(64,1000,0,0); tracked.addToTracks(first);
  auto hcCharged = hcal.create(); hcCharged.setEnergy(.5); hcCharged.setPosition({2000,0,0});
  auto hcFirst = hcal.create(); hcFirst.setEnergy(.25); hcFirst.setPosition({2000,10,0});
  auto hcSecond = hcal.create(); hcSecond.setEnergy(2); hcSecond.setPosition({0,2000,0});
  auto hcOnly = hcal.create(); hcOnly.setEnergy(50); hcOnly.setPosition({2000,0,0});
  charged.addToClusters(hcCharged);
  a.addToClusters(hcCharged); a.addToClusters(hcFirst); a.addToClusters(hcFirst);
  duplicate.addToClusters(hcFirst); c.addToClusters(hcSecond);
  auto hcalOnly = pfos.create(); hcalOnly.setCharge(0); hcalOnly.addToClusters(hcOnly);

  gsf::CollectedNeutralTuple tuple;
  tuple.settings.validate();
  TMemFile file("gsf-neutral-test.root", "RECREATE");
  TTree tree("test","test"); tuple.book(tree);
  tuple.assign(tracks,{0,1,2,3,4,5},&pfos,&ecal,&hcal);
  assert(tuple.inputIndices == std::vector<int>({0,1,2,3,4,5}));
  assert(tuple.status == std::vector<int>({2,2,0,-1,1,2}));
  assert(tuple.clusterIndices[0] == std::vector<int>({0,1}));
  assert(tuple.clusterIndices[1] == std::vector<int>({2}));
  near(tuple.ecalEnergy[0],13); near(tuple.hcalEnergy[0],1); near(tuple.caloEnergy[0],14);
  near(tuple.ecalError[0],std::hypot(.011*2+.004*4,.011*3+.004*9));
  near(tuple.hcalError[0],.37);
  near(tuple.caloError[0],std::hypot(tuple.ecalError[0],.37));
  assert(tuple.hcalIndices[0] == std::vector<int>({1}));
  assert(tuple.hcalRawEnergy[0] == std::vector<double>({.25}));
  assert(tuple.hcalClusterEnergy[0] == std::vector<double>({1}));
  assert(tuple.hcalPfoLink[0] == std::vector<int>({a.id().index,duplicate.id().index}));
  assert(tuple.hcalClusterLink[0] == std::vector<int>({1,1}));
  near(tuple.hcalX[0][0],2000); near(tuple.hcalY[0][0],10);
  near(tuple.ecalEnergy[1],16); near(tuple.hcalEnergy[1],8);
  assert(tuple.referenceStatus == std::vector<int>({1,1,0,-1,1,1}));
  assert(std::isnan(tuple.ecalEnergy[3]) && !tuple.errors[3].empty());
  near(tuple.caloEnergy[2],0); near(tuple.caloEnergy[4],0);
  // Shared observations stay keyed to each track; downstream event unions must deduplicate.
  assert(tuple.clusterIndices[5] == tuple.clusterIndices[0]);
  assert(tuple.hcalIndices[5] == std::vector<int>({0,1})); // Different charged owner.
  tree.Fill();

  tuple.assign(tracks,{1},nullptr,nullptr,nullptr);
  assert(tuple.inputIndices == std::vector<int>({1}) && !tuple.snapshotAvailable);
  assert(tuple.status == std::vector<int>({0}) && tuple.clusterIndices[0].empty());
  assert(std::isnan(tuple.ecalEnergy[0]) && std::isnan(tuple.caloEnergy[0]));
  tuple.assign(tracks,{0},&pfos,&ecal,nullptr);
  assert(tuple.snapshotAvailable && !tuple.hcalAvailable);
  near(tuple.ecalEnergy[0],13);
  assert(std::isnan(tuple.hcalEnergy[0]) && std::isnan(tuple.caloEnergy[0]));
  tree.Fill(); tree.Write();
  // Check nested-vector persistence after the writer's vectors have been replaced.
  TTreeReader reader(&tree);
  TTreeReaderValue<std::vector<int>> indices(reader,"neutral_collection_track_input_index");
  TTreeReaderValue<std::vector<std::vector<int>>> selected(reader,"collected_neutral_hcal_cluster_indices");
  TTreeReaderValue<std::vector<double>> energy(reader,"collected_neutral_calo_energy");
  assert(reader.Next()); assert(indices->size()==6 && (*selected)[0]==std::vector<int>({1}));
  near((*energy)[0],14);
  assert(reader.Next()); assert(indices->size()==1 && (*selected)[0].empty());
  assert(std::isnan((*energy)[0]));
  assert(!reader.Next());

  throws([&]{tuple.assign(tracks,{0},&pfos,nullptr,nullptr);});
  throws([&]{tuple.assign(tracks,{0,0},&pfos,&ecal,&hcal);});
  throws([&]{tuple.assign(tracks,{-1},&pfos,&ecal,&hcal);});
  hcFirst.setEnergy(-1);
  throws([&]{tuple.assign(tracks,{0},&pfos,&ecal,&hcal);});
  hcFirst.setEnergy(.25);
  tuple.settings.thetaMrad = 0; throws([&]{tuple.settings.validate();});
  tuple.settings.thetaMrad = 3141; throws([&]{tuple.settings.validate();});
  tuple.settings.thetaMrad = 25; tuple.settings.hcalScale = 0;
  throws([&]{tuple.settings.validate();});
}
