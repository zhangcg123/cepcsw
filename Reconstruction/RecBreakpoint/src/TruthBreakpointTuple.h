#ifndef RECBREAKPOINT_TRUTH_BREAKPOINT_TUPLE_H
#define RECBREAKPOINT_TRUTH_BREAKPOINT_TUPLE_H
#include "TruthBHLossEventData.h"
class TTree;
namespace breakpoint {
struct TruthBreakpointTuple {
  void book(TTree& tree);
  void reset();
  void assign(const TruthBreakpointMatch& match);
  int status = 0, noUniqueOwner = 0, ineligible = 0;
  std::vector<int> step, owner, eligibility, system, layer, hit, candidateOffset, candidateLayer;
  std::vector<int> effectiveRule;
  std::vector<double> x, y, z, momentumBefore, loss;
  std::vector<std::string> ownerKey, materialPath;
};
}
#endif
