#ifndef RECBREAKPOINT_TRUTH_LAYER_OWNERSHIP_H
#define RECBREAKPOINT_TRUTH_LAYER_OWNERSHIP_H

#include "TruthBHLossEventData.h"

namespace breakpoint {
struct MaterialOwner {
  TruthLossOwner kind = TruthLossOwner::Unassigned;
  std::string key;
  int system = -1, layer = -1;
  std::vector<int> candidates;
};

// Audited TDR_o1_v01 barrel assemblies. Unknown layouts fail closed, never
// substitute nearest-radius ownership. Sensitive provenance works separately.
MaterialOwner materialOwner(const std::string& path);
// Persisted rule codes: 0 physical ownership, 1 beam pipe -> VXD0,
// 2 TPC inner wall -> row0. Effective keys are not physical assembly keys.
int effectiveOwnerRule(const std::string& ownerKey);
}
#endif
