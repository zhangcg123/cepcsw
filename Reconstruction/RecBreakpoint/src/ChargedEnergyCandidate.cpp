#include "ChargedEnergyCandidate.h"
#include <cmath>
#include <set>
#include <stdexcept>
#include <tuple>

namespace breakpoint {
namespace {
using HitKey = std::tuple<unsigned long long, float, float, float>;
std::set<HitKey> hitKeys(const edm4hep::Cluster& cluster) {
  std::set<HitKey> keys;
  for (const auto& hit : cluster.getHits()) {
    const auto& p = hit.getPosition();
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z))
      throw std::invalid_argument("Nonfinite charged ECAL hit position");
    keys.emplace(hit.getCellID(), p.x, p.y, p.z);
  }
  return keys;
}
}

ChargedEnergyCandidate collectChargedEnergy(const edm4hep::Track& track,
    const edm4hep::ReconstructedParticleCollection& pfos,
    const edm4hep::ClusterCollection& finalClusters,
    const edm4hep::ClusterCollection& inputClusters,
    double stochasticError, double constantError) {
  if (!std::isfinite(stochasticError) || stochasticError < 0 ||
      !std::isfinite(constantError) || constantError < 0 ||
      stochasticError + constantError <= 0)
    throw std::invalid_argument("Invalid charged ECAL energy error");
  ChargedEnergyCandidate result;
  std::set<edm4hep::Cluster> final, selected;
  for (const auto& cluster : finalClusters) final.insert(cluster);
  int linkedPfos = 0;
  double variance = 0;
  for (const auto& pfo : pfos) {
    if (pfo.getCharge() == 0) continue;
    bool ownsTrack = false;
    for (const auto& linked : pfo.getTracks())
      if (linked == track) { ownsTrack = true; break; }
    if (!ownsTrack) continue;
    if (++linkedPfos > 1) return {3, 0, 0, {}};
    for (const auto& cluster : pfo.getClusters()) {
      if (!final.count(cluster) || cluster.hits_size() == 0 ||
          !std::isfinite(cluster.getEnergy()) || cluster.getEnergy() <= 0) continue;
      auto source = cluster;
      if (&inputClusters != &finalClusters) {
        const auto reference = hitKeys(cluster);
        int matches = 0;
        for (const auto& candidate : inputClusters) {
          if (candidate.hits_size() == 0 || !std::isfinite(candidate.getEnergy()) ||
              candidate.getEnergy() <= 0) continue;
          const auto keys = hitKeys(candidate);
          bool contains = true;
          for (const auto& key : reference)
            if (!keys.count(key)) { contains = false; break; }
          if (contains) { source = candidate; ++matches; }
        }
        if (matches != 1) return {matches > 1 ? 3 : 1, 0, 0, {}};
      }
      if (!selected.insert(source).second) continue;
      const double value = source.getEnergy();
      result.clusterIndices.push_back(source.id().index);
      result.energy += value;
      const double sigma = stochasticError * std::sqrt(value) + constantError * value;
      variance += sigma * sigma;
    }
  }
  result.sigma = std::sqrt(variance);
  if (linkedPfos == 1 && result.energy > 0) result.status = 2;
  return result;
}
} // namespace breakpoint
