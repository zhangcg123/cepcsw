#ifndef RECBREAKPOINT_TRACKSYSTEM_H
#define RECBREAKPOINT_TRACKSYSTEM_H

#include "MarlinKalTest.h"

namespace breakpoint {
/// Expose the existing protected layer lookup through a narrow adapter.
/// Geometry construction, material and measurement classes are unchanged.
class BreakpointTrackSystem : public MarlinTrk::MarlinKalTest {
public:
  explicit BreakpointTrackSystem(const gear::GearMgr& gear)
      : MarlinTrk::MarlinKalTest(gear) {}
  const ILDVMeasLayer* layer(edm4hep::TrackerHit hit) const {
    return findMeasLayer(hit);
  }
};
} // namespace breakpoint
#endif
