#include "GsfTruthEventData/CompactTruth.h"
#include "GsfTruthEventData/GeneratorFsr.h"
#include <cassert>
#include <algorithm>
#include <limits>

int main() {
  // Outgoing copies, a generator FSR photon, and a simulation photon.
  auto tags=gsftruth::generatorFsr({11,94,11,11,22,22},{2,2,2,1,1,0},
                                  {{},{0},{1},{2},{2},{3}});
  assert(tags.size()==1 && tags[0].photon==4 && tags[0].electron==3 && tags[0].emitter==2);
  // An incoming-beam photon must not be assigned across the production node.
  assert(gsftruth::generatorFsr({11,94,11,22},{2,2,1,1},{{},{0},{1},{0}}).empty());
  tags=gsftruth::generatorFsr({11,11,11,22},{2,1,1,1},{{},{0},{0},{0}});
  assert(tags.size()==1 && tags[0].status==2 && tags[0].electron==-1);
  bool rejected=false;
  try { gsftruth::generatorFsr({11,11},{1,2},{{1},{0}}); } catch (...) { rejected=true; }
  assert(rejected);
  struct Step { int trackID=1,stepNumber=0,processSubtype=0,firstEcalEntry=0,ecalEntryOnly=0; float stepTX0=.01f; };
  std::vector<Step> steps(12);
  for (int i=0;i<12;++i) steps[i].stepNumber=i;
  steps[4].processSubtype=3; steps[11].firstEcalEntry=1;
  auto compact=gsftruth::compactSteps(steps,{{1,0},{1,9}});
  assert((compact.kept==std::vector<std::size_t>{0,4,9,11}));
  assert(compact.spans.size()==3 && compact.spans[0].first==1 && compact.spans[0].last==3);
  assert(compact.spans[0].count==3 && compact.spans[0].tx0==3*double(steps[0].stepTX0));
  // Photon parents remain checkpoints even when eBrem was not the step limiter.
  compact=gsftruth::compactSteps(steps,{{1,0},{1,6},{1,9}});
  assert(std::find(compact.kept.begin(),compact.kept.end(),6)!=compact.kept.end());
  steps.erase(steps.begin()+6); // A real missing step cannot disappear into a span.
  compact=gsftruth::compactSteps(steps,{{1,0},{1,9}});
  for (const auto& span : compact.spans) assert(!(span.first<=6 && span.last>=6));
  steps[2].stepTX0=std::numeric_limits<float>::quiet_NaN();
  compact=gsftruth::compactSteps(steps,{});
  assert(std::find(compact.kept.begin(),compact.kept.end(),2)!=compact.kept.end());
}
