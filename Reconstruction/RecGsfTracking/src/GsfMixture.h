#ifndef RecGsfTracking_GsfMixture_h
#define RecGsfTracking_GsfMixture_h

#include <functional>
#include <string>
#include <vector>

struct GsfComponent;

namespace GsfMixture {

/// Normalize component weights to sum to 1
void normalizeWeights(std::vector<GsfComponent*>& comps);

/// Remove normalized components below cutoff, preserving at least the largest.
void removeLowWeight(std::vector<GsfComponent*>& comps, double cutoff);

/// KL-distance-based mixture reduction: prune to maxN components.
/// @param bz  B-field strength [T] for mean-vector extraction
void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz);
void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz,
            const std::function<void(const std::string&)>& logger);

/// Weight-rank reduction: keep the top maxN components by normalized weight.
void reduceTopN(std::vector<GsfComponent*>& comps, int maxN);
void reduceTopN(std::vector<GsfComponent*>& comps, int maxN,
                const std::function<void(const std::string&)>& logger);

} // namespace GsfMixture

#endif
