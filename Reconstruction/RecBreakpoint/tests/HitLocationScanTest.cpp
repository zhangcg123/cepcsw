#include "HitLocationScan.h"
#include <cassert>
#include <iostream>

using breakpoint::HitLocationScan;
int main() {
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
  std::cout<<"PASS: silicon-only candidates, last-hit exclusion, null test, threshold, ties and failed fits\n";
}
