#include "TruthBHLossEventData.h"
#include "TruthLayerOwnership.h"
#include "DD4hep/DD4hepUnits.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "TGeoManager.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoVolume.h"
#include "TGeoNavigator.h"
#include "TGeoMatrix.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

void check(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
int main() {
  TGeoManager geometry("truth-test", "truth-test");
  auto* material = new TGeoMaterial("test-air", 14., 7., .001);
  auto* medium = new TGeoMedium("test", 1, material);
  auto* world = geometry.MakeBox("world", medium, 100., 100., 100.);
  world->AddNode(geometry.MakeBox("BeamPipe_IPInnerTube", medium, .03, .03, .03), 2,
                new TGeoTranslation(20.*dd4hep::mm, 0., 0.));
  world->AddNode(geometry.MakeBox("TPCinnerWallLog", medium, .03, .03, .03), 0,
                new TGeoTranslation(40.*dd4hep::mm, 0., 0.));
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

  // A diagnostic ECAL entry must not create an emission or change hook losses.
  auto ecal = steps.create();
  ecal.setTrackID(1); ecal.setParentID(0); ecal.setPdg(11); ecal.setStepNumber(100);
  ecal.setFirstEcalEntry(1); ecal.setEcalEntryOnly(1); ecal.setProcessSubtype(3);
  ecal.setPrePosition({9.,0.,0.}); ecal.setPostPosition({10.,0.,0.});
  ecal.setPreMomentum({10.f,0.f,0.f}); ecal.setPostMomentum({1.f,0.f,0.f});
  ecal.setMomentumLoss(9.); ecal.setStepTX0(10.);
  check(reader.prepare(&steps,&links,{&associations},error), error.c_str());
  TruthBreakpointMatch withEntry;
  check(reader.matchBreakpoints(hits,1.,navigator,withEntry,error), error.c_str());
  check(withEntry.emissions.size()==match.emissions.size() &&
        withEntry.breakpoints.size()==match.breakpoints.size(), "ECAL entry changed ownership");
  check(withEntry.breakpoints.front().ebremLoss==match.breakpoints.front().ebremLoss,
        "ECAL entry changed breakpoint loss");
  check(withEntry.hooks.retainedFractions==old.retainedFractions, "ECAL entry changed hook losses");

  gsftruth::G4MaterialStepCollection effectiveSteps; effectiveSteps.setID(11);
  gsftruth::SimTrackerHitG4StepLinkCollection effectiveLinks; effectiveLinks.setID(12);
  edm4hep::SimTrackerHitCollection effectiveSim; effectiveSim.setID(13);
  edm4hep::TrackerHitCollection effectiveReco; effectiveReco.setID(14);
  edm4hep::MCRecoTrackerAssociationCollection effectiveAssociations; effectiveAssociations.setID(15);
  const double positions[] = {20.,30.,35.,40.,50.,55.,60.};
  for (int i=0; i<7; ++i) {
    auto s = effectiveSteps.create(); s.setTrackID(1); s.setParentID(0); s.setPdg(11); s.setStepNumber(i);
    s.setPrePosition({positions[i],0.,0.}); s.setPostPosition({positions[i]+.1,0.,0.});
    s.setProcessSubtype(i==0 || i==3 ? 3 : 0); s.setPreMomentum({10.f,0.f,0.f});
    s.setPostMomentum({9.9f,0.f,0.f}); s.setMomentumLoss(i==0 || i==3 ? .1 : 0.);
    s.setStepTX0(.001); s.setMaterialRadiationLength(material->GetRadLen()/dd4hep::mm);
  }
  auto effectiveHit = [&](int step, int system, int layer) {
    const unsigned cell = unsigned(system) | (unsigned(layer)<<7);
    auto s = effectiveSim.create(); s.setCellID(system==4 ? layer+1 : cell);
    s.setPosition({positions[step]+.05,0.,0.});
    auto link = effectiveLinks.create(); link.setTrackID(1); link.setSimHit(s);
    link.setFirstStepNumber(step); link.setLastStepNumber(step); link.setHookStepNumber(step);
    link.setHookFraction(.5); link.setStatus(31); link.setHookKind(4); link.setProvenanceType(system==4 ? 3 : 2);
    link.setFirstStep(effectiveSteps[step]); link.setLastStep(effectiveSteps[step]); link.setHookStep(effectiveSteps[step]);
    auto h = effectiveReco.create(); h.setCellID(cell); h.setPosition(s.getPosition());
    auto a = effectiveAssociations.create(); a.setRec(h); a.setSim(s); a.setWeight(1.);
  };
  effectiveHit(1,1,0); effectiveHit(2,1,1); effectiveHit(4,4,0);
  effectiveHit(5,4,1); effectiveHit(6,5,0);
  std::vector<edm4hep::TrackerHit> effectiveHits;
  for (const auto& h : effectiveReco) effectiveHits.push_back(h);
  TruthBHLossEventData effectiveReader;
  check(effectiveReader.prepare(&effectiveSteps,&effectiveLinks,{&effectiveAssociations},error), error.c_str());
  check(effectiveReader.matchBreakpoints(effectiveHits,1.,navigator,match,error), error.c_str());
  check(match.breakpoints.size()==2 && match.emissions.size()==2, "both effective owners must be eligible");
  for (int i=0; i<2; ++i) {
    const auto& e = match.emissions[i];
    check(e.status==TruthBreakpointStatus::Eligible && e.hitIndex==2*i && e.layer==0,
          "effective loss must use the exact first-layer hit");
    check(breakpoint::effectiveOwnerRule(e.ownerKey)==i+1, "effective rule must persist in key");
    check(e.position.x==effectiveSteps[3*i].getPostPosition().x &&
          e.momentumLoss==effectiveSteps[3*i].getMomentumLoss() && !e.materialPath.empty(),
          "effective assignment must not move or replace physical truth");
  }
  auto missing = effectiveHits; missing.erase(missing.begin()+2); missing.erase(missing.begin());
  check(effectiveReader.matchBreakpoints(missing,1.,navigator,match,error), error.c_str());
  check(match.breakpoints.empty() && match.emissions[0].status==TruthBreakpointStatus::MissingHit &&
        match.emissions[1].status==TruthBreakpointStatus::MissingHit, "never substitute another accepted layer");
  auto outer = effectiveHits; outer.resize(3);
  check(effectiveReader.matchBreakpoints(outer,1.,navigator,match,error), error.c_str());
  check(match.emissions[1].status==TruthBreakpointStatus::OutermostHit, "effective owner still needs downstream interval");
  effectiveSteps[3].setPrePosition({20.,0.,0.}); effectiveSteps[3].setPostPosition({20.1,0.,0.});
  check(effectiveReader.prepare(&effectiveSteps,&effectiveLinks,{&effectiveAssociations},error), error.c_str());
  check(effectiveReader.matchBreakpoints(effectiveHits,1.,navigator,match,error), error.c_str());
  check(match.emissions[1].status==TruthBreakpointStatus::MissingHit,
        "returning beam-pipe emission must not attach to an earlier VXD0 visit");
  effectiveSteps[3].setPrePosition({40.,0.,0.}); effectiveSteps[3].setPostPosition({40.1,0.,0.});
  effectiveSim[0].setCellID(1u | (1u<<7)); effectiveSim[1].setCellID(1u);
  check(effectiveReader.prepare(&effectiveSteps,&effectiveLinks,{&effectiveAssociations},error), error.c_str());
  check(effectiveReader.matchBreakpoints(effectiveHits,1.,navigator,match,error), error.c_str());
  check(match.emissions[0].status==TruthBreakpointStatus::MissingHit,
        "effective assignment must not cross an intervening different sensor");
  effectiveSteps[0].setMaterialRadiationLength(1.);
  check(effectiveReader.prepare(&effectiveSteps,&effectiveLinks,{&effectiveAssociations},error), error.c_str());
  check(effectiveReader.matchBreakpoints(effectiveHits,1.,navigator,match,error), error.c_str());
  check(match.emissions[0].owner==TruthLossOwner::GeometryMismatch && match.emissions[0].hitIndex==-1,
        "effective assignment must not bypass geometry/G4 consistency");
  std::cout << "Truth breakpoint full association test passed\n";
}
