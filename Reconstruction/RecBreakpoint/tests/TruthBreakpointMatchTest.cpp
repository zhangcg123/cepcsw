#include "TruthBHLossEventData.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TGeoNavigator.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
  TGeoManager geometry("truth-test", "truth-test");
  auto* material = new TGeoMaterial("test-air", 14., 7., .001);
  auto* medium = new TGeoMedium("test", 1, material);
  auto* world = geometry.MakeBox("world", medium, 100., 100., 100.);
  geometry.SetTopVolume(world); geometry.CloseGeometry();
  TGeoNavigator navigator(&geometry);
  navigator.BuildCache(); navigator.GetCache()->BuildInfoBranch();
  gsftruth::G4MaterialStepCollection steps; steps.setID(1);
  gsftruth::SimTrackerHitG4StepLinkCollection links; links.setID(2);
  edm4hep::SimTrackerHitCollection sim; sim.setID(3);
  edm4hep::TrackerHitCollection reco; reco.setID(4);
  edm4hep::MCRecoTrackerAssociationCollection associations; associations.setID(5);
  for (int i = 0; i < 9; ++i) {
    auto s = steps.create(); s.setTrackID(1); s.setParentID(0); s.setPdg(11); s.setStepNumber(i);
    s.setPrePosition({double(i),0.,0.}); s.setPostPosition({double(i+1),0.,0.});
    const bool emission = i == 1 || i == 3 || i == 4 || i == 5 || i == 7;
    s.setProcessSubtype(emission ? 3 : 0); s.setPreMomentum({10.f,0.f,0.f});
    s.setPostMomentum({emission ? 9.9f : 10.f,0.f,0.f}); s.setMomentumLoss(emission ? .1 : 0.);
    s.setStepTX0(.001); s.setMaterialRadiationLength(1000.);
  }
  auto addHit = [&](int layer, int first, int last, int hook, bool accepted) {
    auto s = sim.create(); s.setCellID(1u | (unsigned(layer)<<7));
    s.setPosition({double(hook)+.5,0.,0.});
    auto link = links.create(); link.setTrackID(1); link.setSimHit(s);
    link.setFirstStepNumber(first); link.setLastStepNumber(last); link.setHookStepNumber(hook);
    link.setHookFraction(.5); link.setStatus(31); link.setHookKind(4); link.setProvenanceType(2);
    link.setFirstStep(steps[first]); link.setLastStep(steps[last]); link.setHookStep(steps[hook]);
    if (accepted) {
      auto h = reco.create(); h.setCellID(s.getCellID()); h.setPosition(s.getPosition());
      auto a = associations.create(); a.setRec(h); a.setSim(s); a.setWeight(1.);
    }
  };
  addHit(0,0,0,0,true);
  addHit(1,1,3,2,true); // emissions before AND after this hook belong to hit1
  addHit(2,4,4,4,false); // known physical layer, no accepted reconstructed hit
  addHit(3,6,7,6,true); // outermost accepted hit cannot start an interval
  sim[2].setCellID(77); links[2].setProvenanceType(3); // TPC SD one-based row
  std::vector<edm4hep::TrackerHit> hits;
  for (const auto& h : reco) hits.push_back(h);
  TruthBHLossEventData reader;
  std::string error;
  check(reader.prepare(&steps,&links,{&associations},error), error.c_str());
  TruthBHLossEventDataMatch old;
  check(reader.matchTrack(hits,1.,true,old,error), error.c_str());
  TruthBreakpointMatch match;
  check(reader.matchBreakpoints(hits,1.,navigator,match,error), error.c_str());
  check(match.emissions.size()==5 && match.breakpoints.size()==1, "retain all emissions, select only eligible owner");
  const auto& group = match.breakpoints.front();
  check(group.hitIndex==1 && group.emissionCount==2, "both halves map to SAME upstream breakpoint");
  check(std::abs(group.ebremLoss-.2)<1.e-7 && std::abs(group.retainedFraction-.98)<1.e-7,
        "truth override must use the owner-group loss and pre-emission momentum");
  check(match.emissions[2].owner==TruthLossOwner::Sensitive &&
        match.emissions[2].status==TruthBreakpointStatus::MissingHit, "missing hit is not unknown owner");
  check(match.emissions[2].system==4 && match.emissions[2].layer==76,
        "TPC raw row number must not be decoded as a silicon CellID");
  check(match.emissions[3].status==TruthBreakpointStatus::NoUniqueOwner &&
        match.emissions[3].hitIndex==-1, "gap emission cannot select a breakpoint");
  check(match.emissions[4].status==TruthBreakpointStatus::OutermostHit &&
        match.emissions[4].hitIndex==2, "outermost owner retained but ineligible");
  check(match.hooks.materialIntervals.size()==old.materialIntervals.size(), "material intervals preserved");
  for (std::size_t i=0;i<old.materialIntervals.size();++i)
    check(old.materialIntervals[i].truthTX0==match.hooks.materialIntervals[i].truthTX0 &&
          old.retainedFractions[i]==match.hooks.retainedFractions[i], "legacy hook results changed");
  std::cout << "Truth breakpoint full association test passed\n";
}
