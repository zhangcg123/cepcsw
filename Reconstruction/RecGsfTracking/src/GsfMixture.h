#ifndef RecGsfTracking_GsfMixture_h
#define RecGsfTracking_GsfMixture_h

#include <functional>
#include <string>
#include <vector>

struct GsfComponent;

namespace GsfMixture {

using RemovalObserver = std::function<void(const GsfComponent&)>;
using MergeObserver = std::function<void(
    GsfComponent& merged, int keepSourceNodeId, int dropSourceNodeId,
    double mergeCost)>;

/// Normalize component weights to sum to 1
void normalizeWeights(std::vector<GsfComponent*>& comps);

/// Remove normalized components below cutoff, preserving at least the largest.
void removeLowWeight(std::vector<GsfComponent*>& comps, double cutoff,
                     bool protectIdentity = true,
                     const RemovalObserver& observer = {});

/// KL-distance-based mixture reduction: prune to maxN components.
/// @param bz  B-field strength [T] for mean-vector extraction
void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz,
            bool protectIdentity = true);
void reduce(std::vector<GsfComponent*>& comps, int maxN, double bz,
            bool protectIdentity,
            const std::function<void(const std::string&)>& logger,
            const std::string& mergeCost = "SymmetricKL",
            const MergeObserver& observer = {});

} // namespace GsfMixture

#endif
