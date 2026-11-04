#include "HitLocationScan.h"
#include <cassert>
#include <iostream>

using breakpoint::HitLocationScan;
int main() {
  assert((breakpoint::locationScanSetups(false)==std::vector<int>{0,100,110}));
  assert((breakpoint::locationScanSetups(true)==std::vector<int>{0,100,110,111}));
  assert(breakpoint::checkedLocationScanSetups({}).empty());
  assert((breakpoint::checkedLocationScanSetups({110})==std::vector<int>{110}));
  assert((breakpoint::checkedLocationScanSetups({111,0,100})==std::vector<int>{0,100,111}));
  for (const auto& invalid : {std::vector<int>{110,110}, std::vector<int>{10}, std::vector<int>{-1}}) {
    bool bad=false;
    try { breakpoint::checkedLocationScanSetups(invalid); } catch (const std::invalid_argument&) { bad=true; }
    assert(bad);
  }
  using breakpoint::siliconScanHits;
  assert((siliconScanHits({1,1,2,3,4,5,6})==std::vector<int>{0,1,2,3}));
  assert((siliconScanHits({4,5,1})==std::vector<int>{})); // Last hit is never a candidate.
  assert(siliconScanHits({}).empty()); assert(siliconScanHits({1}).empty());
  HitLocationScan scan;
  scan.nullNll2=10; scan.hit={0,2,5}; scan.nll2={8,6,7}; scan.fitStatus={2,2,2};
  scan.select(); assert(scan.status==2 && scan.selectedHit==2 && scan.gain==4 && scan.secondNll2==7);
  scan.minNllGain=4; scan.select(); assert(scan.status==1 && scan.selectedHit==-1 && scan.bestHit==2);
  scan.minNllGain=0; scan.nll2={10,11,12}; scan.select(); assert(scan.status==1);
  scan.nll2={6,6,7}; scan.select(); assert(scan.selectedHit==0 && scan.secondNll2==6);
  scan.nll2[0]=std::numeric_limits<double>::quiet_NaN(); scan.fitStatus[2]=-2;
  scan.select(); assert(scan.selectedHit==2 && scan.failedCandidates==2);
  scan.fitStatus={-2,-2,-2}; scan.select(); assert(scan.status==-2 && scan.selectedHit==-1);
  scan.reset(0); scan.nullNll2=10; scan.select(); assert(scan.status==3);
  scan.nullNll2=std::numeric_limits<double>::infinity(); scan.select(); assert(scan.status==-1);
  scan.minNllGain=-1;
  bool rejected=false; try { scan.select(); } catch(const std::invalid_argument&) { rejected=true; }
  assert(rejected);
  scan.reset(0); scan.nullNll2=10; scan.hit={0};
  rejected=false; try { scan.select(); } catch(const std::invalid_argument&) { rejected=true; }
  assert(rejected);
  scan.constraintMask=3; scan.chargedEnergyMode="ElectronPhoton";
  scan.truthStatus=2; scan.truthHit=5; scan.truthNll2=1.; scan.truthReusedCandidate=true;
  scan.nullNll2=10; scan.hit={0,2}; scan.nll2={8,7}; scan.fitStatus={2,2};
  scan.select(); assert(scan.selectedHit==2); // Passive truth h5 never enters ranking.
  scan.reset(2.);
  assert(scan.constraintMask==0 && scan.chargedEnergyMode=="None" && scan.minNllGain==2.);
  assert(scan.truthStatus==0 && scan.truthHit==-1 && !scan.truthReusedCandidate && std::isnan(scan.truthNll2));
  using breakpoint::locationScanPoints;
  const std::vector<int> systems{1,1,2,3,4,4,5}, layers{0,1,0,0,0,1,0};
  assert((locationScanPoints(systems,layers)==std::vector<int>{-1,0,1,2,3,4}));
  assert((locationScanPoints(systems,layers,2)==std::vector<int>{-1,0,1,2,3,4}));
  assert((locationScanPoints(systems,layers,4)==std::vector<int>{-1,0,1,2,3,4})); // Reuse row0.
  assert((locationScanPoints(systems,layers,5)==std::vector<int>{-1,0,1,2,3,4,5})); // Other TPC truth only.
  assert((locationScanPoints({1,4,4,5},{0,1,2,0})==std::vector<int>{-1,0})); // Missing row0: no substitute.
  assert((locationScanPoints({1,4,4,5},{0,0,0,0})==std::vector<int>{-1,0,1})); // Only first row0.
  assert((locationScanPoints({1,4},{0,0})==std::vector<int>{-1,0})); // Last-hit guard applies to row0.
  assert((locationScanPoints({4,5,1},{1,0,0},0)==std::vector<int>{-1,0}));
  assert((locationScanPoints({4,5,1},{1,0,0},2)==std::vector<int>{-1})); // Invalid last owner ignored.
  assert((locationScanPoints({},{})==std::vector<int>{-1}));
  rejected=false; try { locationScanPoints({1,4},{0}); } catch(const std::invalid_argument&) { rejected=true; }
  assert(rejected);
  breakpoint::LocationFitScan all;
  all.status=1; all.setup={0,100,110,111}; all.nll2={4,3,2,1}; all.truthPoint=2;
  all.reset(); assert(all.status==0 && all.setup.empty() && all.nll2.empty() && all.truthPoint==-1);
  std::cout<<"PASS: legacy silicon scan; scan-only silicon plus exact TPC row0; last-hit, truth reuse, null, failures\n";
}
