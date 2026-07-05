#ifndef RecGsfTracking_GsfMixture_h
#define RecGsfTracking_GsfMixture_h

#include <vector>

struct GsfComponent;

namespace GsfMixture {

/// Normalize component weights to sum to 1
void normalizeWeights(std::vector<GsfComponent*>& comps);

/// KL-distance-based mixture reduction: prune to maxN components.
/// @param bz  B-field strength [T] for mean-vector extraction
void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz);

} // namespace GsfMixture

#endif
