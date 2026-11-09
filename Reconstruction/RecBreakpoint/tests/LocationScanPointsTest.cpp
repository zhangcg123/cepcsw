#include "LocationFitScan.h"
#include <cassert>
#include <iostream>

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
  bool rejected=false; try { locationScanPoints({1,4},{0}); } catch(const std::invalid_argument&) { rejected=true; }
  assert(rejected);
  breakpoint::LocationFitScan all;
  all.status=1; all.setup={0,100,110,111}; all.nll2={4,3,2,1}; all.truthPoint=2;
  all.reset(); assert(all.status==0 && all.setup.empty() && all.nll2.empty() && all.truthPoint==-1);
  std::cout<<"PASS: scan-only silicon plus exact TPC row0; last-hit, truth reuse, null, failures\n";
}
