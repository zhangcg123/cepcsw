#include "TruthElectronSelector.h"
#include "TrackTruthMatch.h"

#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <set>
#include <stdexcept>

namespace breakpoint {
bool TruthElectronSelector::Result::accepts(int index) const {
  return index >= 0 && std::find(trackIndices.begin(), trackIndices.end(), index) != trackIndices.end();
}

TruthElectronSelector::Result TruthElectronSelector::select(
    const edm4hep::MCParticleCollection& particles,
    const edm4hep::TrackCollection& tracks,
    const edm4hep::MCRecoTrackParticleAssociationCollection& associations) const {
  Result result;
  bool hasHiggs = false;
  for (const auto& particle : particles)
    if (particle.getPDG() == 25 && particle.getGeneratorStatus() != 0) hasHiggs = true;
  if (!hasHiggs) {
    result.status = -1; result.error = "No generator Higgs record";
    return result;
  }
  std::map<edm4hep::MCParticle, bool> higgsAncestor;
  std::set<edm4hep::MCParticle> visiting;
  std::function<bool(edm4hep::MCParticle)> fromHiggs = [&](auto particle) {
    if (!particle.isAvailable()) throw std::runtime_error("Unavailable MC parent");
    if (const auto found = higgsAncestor.find(particle); found != higgsAncestor.end()) return found->second;
    if (!visiting.insert(particle).second) throw std::runtime_error("Cyclic MC ancestry");
    bool foundHiggs = particle.getPDG() == 25;
    for (const auto& parent : particle.getParents()) foundHiggs = fromHiggs(parent) || foundHiggs;
    visiting.erase(particle);
    higgsAncestor[particle] = foundHiggs;
    return foundHiggs;
  };
  std::array<edm4hep::MCParticle, 2> pair;
  try {
    for (const auto& particle : particles) {
      if (std::abs(particle.getPDG()) != 11 || particle.getGeneratorStatus() != 1 || fromHiggs(particle)) continue;
      const int charge = particle.getPDG() == 11 ? 0 : 1;
      ++result.candidateCounts[charge];
      pair[charge] = particle;
    }
  } catch (const std::exception& failure) {
    result.status = -4; result.error = failure.what();
    return result;
  }
  // Some eeH generators omit the production Z, so do not require a PDG-23 parent.
  // shortcut: uniqueness outside Higgs identifies the production pair only in eeH;
  // use process-specific provenance if additional non-Higgs electrons are present.
  if (result.candidateCounts != std::vector<int>{1, 1}) {
    result.status = -2; result.error = "Require exactly one stable generator e- and e+ outside Higgs ancestry";
    return result;
  }
  for (int charge = 0; charge < 2; ++charge) result.mcIndices[charge] = pair[charge].id().index;

  std::array<int, 2> bestCount{}, multiplicity{};
  std::array<double, 2> bestPurity{};
  int index = -1;
  for (const auto& track : tracks) {
    ++index;
    const auto match = matchTrackTruth(track, associations);
    if (match.multiplicity != 1 || track.trackerHits_size() == 0) continue;
    for (int charge = 0; charge < 2; ++charge) {
      if (!(match.particle == pair[charge])) continue;
      const double purity = double(match.count) / track.trackerHits_size();
      if (match.count > bestCount[charge] || (match.count == bestCount[charge] && purity > bestPurity[charge])) {
        bestCount[charge] = match.count; bestPurity[charge] = purity;
        result.trackIndices[charge] = index; multiplicity[charge] = 1;
      } else if (match.count == bestCount[charge] && purity == bestPurity[charge]) ++multiplicity[charge];
    }
  }
  int count = 0;
  for (int charge = 0; charge < 2; ++charge) {
    if (multiplicity[charge] != 1) result.trackIndices[charge] = -1;
    if (result.trackIndices[charge] >= 0) ++count;
  }
  result.status = count == 2 ? 1 : count == 1 ? 2 : -3;
  if (count != 2) result.error = "Missing or tied reconstructed match for one or both production electrons";
  return result;
}
} // namespace breakpoint
