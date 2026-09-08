#ifndef RECBREAKPOINT_SEEDHITSELECTION_H
#define RECBREAKPOINT_SEEDHITSELECTION_H

#include <array>
#include <stdexcept>
#include <string>
#include <vector>

namespace breakpoint {

enum class SeedHitSelection { FirstMiddleLast, FirstThree };

inline SeedHitSelection parseSeedHitSelection(const std::string& name) {
  if (name == "FirstMiddleLast") return SeedHitSelection::FirstMiddleLast;
  if (name == "FirstThree") return SeedHitSelection::FirstThree;
  throw std::invalid_argument("SeedHitSelection must be FirstMiddleLast or FirstThree");
}

/// Input indices refer to usable 2D hits in outward radius order. Choosing
/// the middle by ordinal (N/2, upper middle for even N) is deterministic and
/// does not inspect momentum, truth, or the configured breakpoint intervals.
inline std::array<int, 3> selectSeedHitIndices(const std::vector<int>& usable,
                                             SeedHitSelection mode) {
  if (usable.size() < 3)
    throw std::runtime_error("Need at least three two-dimensional seed hits");
  if (mode == SeedHitSelection::FirstThree)
    return {usable[0], usable[1], usable[2]};
  return {usable.front(), usable[usable.size() / 2], usable.back()};
}

} // namespace breakpoint
#endif
