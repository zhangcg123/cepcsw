#ifndef RECBREAKPOINT_TRACKTRUTHMATCH_H
#define RECBREAKPOINT_TRACKTRUTHMATCH_H

#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#include <cmath>
#include <limits>
#include <map>

namespace breakpoint {
struct TrackTruthMatch {
  edm4hep::MCParticle particle;
  int count = 0, multiplicity = 0, linkedCount = 0;
};

// Association weights are matched-hit counts, not probabilities.
inline TrackTruthMatch matchTrackTruth(
    const edm4hep::Track& track,
    const edm4hep::MCRecoTrackParticleAssociationCollection& associations) {
  std::map<edm4hep::MCParticle, int> counts;
  for (const auto& association : associations) {
    if (!(association.getRec() == track) || !association.getSim().isAvailable()) continue;
    const double weight = association.getWeight();
    if (!std::isfinite(weight) || weight <= 0 || weight > std::numeric_limits<int>::max()) continue;
    const int count = static_cast<int>(std::lround(weight));
    if (count > 0) counts[association.getSim()] += count;
  }
  TrackTruthMatch result;
  for (const auto& [particle, count] : counts) {
    result.linkedCount += count;
    if (count > result.count) {
      result.particle = particle; result.count = count; result.multiplicity = 1;
    } else if (count == result.count) ++result.multiplicity;
  }
  return result;
}
} // namespace breakpoint
#endif
