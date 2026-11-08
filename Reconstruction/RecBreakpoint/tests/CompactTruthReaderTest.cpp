#include "TruthBHLossEventData.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include <cassert>
#include <cmath>

int main() {
  gsftruth::G4MaterialStepCollection full, compact;
  gsftruth::G4MaterialStepSpanCollection spans;
  gsftruth::SimTrackerHitG4StepLinkCollection links, compactLinks;
  edm4hep::SimTrackerHitCollection sim;
  edm4hep::TrackerHitCollection reco;
  edm4hep::MCRecoTrackerAssociationCollection associations;
  full.setID(1);compact.setID(2);links.setID(3);compactLinks.setID(4);sim.setID(5);reco.setID(6);associations.setID(7);
  for (int i=0;i<30;++i) {
    auto s=full.create();s.setTrackID(1);s.setPdg(11);s.setStepNumber(i);
    s.setPrePosition({double(i),0,0});s.setPostPosition({double(i+1),0,0});
    s.setPreMomentum({10,0,0});s.setPostMomentum({9.9f,0,0});
    s.setStepTX0(.001f*(i+1));s.setProcessSubtype(i==5 || i==20 ? 3 : 0);
    s.setMomentumLoss(i==5 ? .1f : i==20 ? .2f : 0.f);
    if (i==0 || i==5 || i==12 || i==20 || i==29) compact.push_back(s.clone());
  }
  for (auto range : {std::pair<int,int>{1,4},{6,11},{13,19},{21,28}}) {
    auto span=spans.create();span.setTrackID(1);span.setFirstStepNumber(range.first);
    span.setLastStepNumber(range.second);span.setStepCount(range.second-range.first+1);
    double tx0=0;for(int i=range.first;i<=range.second;++i) tx0+=full[i].getStepTX0();
    span.setIntegratedTX0(tx0);
  }
  for (int n : {0,12,29}) {
    auto h=sim.create();h.setPosition({n+.5,0,0});h.setCellID(n);
    auto r=reco.create();r.setPosition(h.getPosition());r.setCellID(n);
    auto a=associations.create();a.setRec(r);a.setSim(h);a.setWeight(1);
    auto link=links.create();link.setSimHit(h);link.setTrackID(1);link.setStatus(31);
    link.setFirstStepNumber(n);link.setLastStepNumber(n);link.setHookStepNumber(n);
    link.setHookFraction(.5);link.setFirstStep(full[n]);link.setLastStep(full[n]);link.setHookStep(full[n]);
    auto copy=link.clone();
    for (const auto& s : compact) if(s.getStepNumber()==n) {copy.setFirstStep(s);copy.setLastStep(s);copy.setHookStep(s);}
    compactLinks.push_back(copy);
  }
  TruthBHLossEventData oldReader,newReader;std::string error;
  assert(oldReader.prepare(&full,&links,{&associations},error));
  assert(newReader.prepare(&compact,&compactLinks,{&associations},error,&spans));
  for (const auto& hits : {std::vector<edm4hep::TrackerHit>{reco[0],reco[1],reco[2]},
                          std::vector<edm4hep::TrackerHit>{reco[0],reco[2]}}) {
    TruthBHLossEventDataMatch oldResult,newResult;
    assert(oldReader.matchTrack(hits,1,true,oldResult,error));
    assert(newReader.matchTrack(hits,1,true,newResult,error));
    assert(oldResult.retainedFractions==newResult.retainedFractions);
    for(std::size_t i=0;i<oldResult.materialIntervals.size();++i) {
      const auto& a=oldResult.materialIntervals[i];const auto& b=newResult.materialIntervals[i];
      assert(a.ebremLoss==b.ebremLoss && a.momentumBefore==b.momentumBefore && a.stepCount==b.stepCount);
      assert(std::abs(a.truthTX0-b.truthTX0)<1.e-13);
    }
  }
  // Neither compact mode nor a summary is allowed to heal missing provenance.
  spans[0].setFirstStepNumber(2);spans[0].setStepCount(3);
  assert(newReader.prepare(&compact,&compactLinks,{&associations},error,&spans));
  TruthBHLossEventDataMatch result;
  assert(!newReader.matchTrack({reco[0],reco[2]},1,true,result,error));
  spans[0].setFirstStepNumber(0);spans[0].setStepCount(5);
  assert(!newReader.prepare(&compact,&compactLinks,{&associations},error,&spans));
}
