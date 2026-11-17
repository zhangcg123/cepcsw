#ifndef RECBREAKPOINT_HCALENERGYCANDIDATE_H
#define RECBREAKPOINT_HCALENERGYCANDIDATE_H

#include "edm4hep/ClusterCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include <cmath>
#include <set>
#include <stdexcept>
#include <vector>

namespace breakpoint {
struct HcalEnergyCandidate {
  double energy = 0, sigma = 0;
  std::vector<int> clusterIndices;
  std::vector<double> rawEnergies, energies, errors;
};

inline HcalEnergyCandidate collectHcalEnergy(
    const edm4hep::ReconstructedParticleCollection& pfos,
    const std::vector<int>& selectedPfos,
    const edm4hep::ClusterCollection& hcal, double scale, double stochastic,
    double constant, const std::vector<int>& excluded = {}) {
  if (!std::isfinite(scale) || scale <= 0 || !std::isfinite(stochastic) ||
      stochastic < 0 || !std::isfinite(constant) || constant < 0 ||
      stochastic + constant <= 0)
    throw std::invalid_argument("Invalid HCAL calibration/resolution");
  std::set<edm4hep::Cluster> members, selected;
  for (const auto& c : hcal) members.insert(c);
  const std::set<int> owners(selectedPfos.begin(), selectedPfos.end());
  const std::set<int> skip(excluded.begin(), excluded.end());
  HcalEnergyCandidate result;
  double variance = 0;
  for (const auto& pfo : pfos) {
    if (!owners.count(pfo.id().index)) continue;
    for (const auto& c : pfo.getClusters()) {
      if (!members.count(c) || skip.count(c.id().index) || !selected.insert(c).second) continue;
      const double raw = c.getEnergy();
      if (!std::isfinite(raw) || raw < 0)
        throw std::runtime_error("Invalid associated pre-HCAL energy");
      if (raw == 0) continue;
      const double energy = scale * raw;
      const double sigma = stochastic * std::sqrt(energy) + constant * energy;
      if (!std::isfinite(energy) || !std::isfinite(sigma))
        throw std::runtime_error("Nonfinite calibrated HCAL observation");
      result.clusterIndices.push_back(c.id().index);
      result.rawEnergies.push_back(raw);
      result.energies.push_back(energy);
      result.errors.push_back(sigma);
      result.energy += energy;
      variance += sigma * sigma;
    }
  }
  result.sigma = std::sqrt(variance);
  return result;
}
} // namespace breakpoint
#endif
