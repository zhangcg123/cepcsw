#include "NeutralLossCandidate.h"

#include <cmath>
#include <set>
#include <stdexcept>

namespace breakpoint {
namespace {
constexpr double pi = 3.14159265358979323846;
struct Direction { double theta = 0, phi = 0; };

Direction direction(double x, double y, double z) {
  const double transverse = std::hypot(x, y);
  if (!std::isfinite(transverse) || !std::isfinite(z) ||
      !(std::hypot(transverse, z) > 0))
    throw std::invalid_argument("Invalid ECAL reference or cluster position");
  return {std::atan2(transverse, z), std::atan2(y, x)};
}
} // namespace

NeutralLossCandidate collectNeutralLoss(const edm4hep::Track& track,
    const edm4hep::ReconstructedParticleCollection& pfos,
    const edm4hep::ClusterCollection& ecalClusters,
    double thetaWindow, double phiWindow,
    double stochasticError, double constantError) {
  if (!std::isfinite(thetaWindow) || !std::isfinite(phiWindow) ||
      thetaWindow <= 0 || thetaWindow >= pi ||
      phiWindow <= 0 || phiWindow >= pi ||
      !std::isfinite(stochasticError) || stochasticError < 0 ||
      !std::isfinite(constantError) || constantError < 0 ||
      stochasticError + constantError <= 0)
    throw std::invalid_argument("Invalid neutral-loss window or energy error");

  NeutralLossCandidate result;
  Direction reference;
  bool hasReference = false;
  for (const auto& state : track.getTrackStates()) {
    if (state.location != edm4hep::TrackState::AtCalorimeter) continue;
    const auto& point = state.referencePoint;
    reference = direction(point.x, point.y, point.z);
    hasReference = true;
    break;
  }
  if (!hasReference) return result;
  result.hasReference = true;

  std::set<edm4hep::Cluster> ecal;
  for (const auto& cluster : ecalClusters) ecal.insert(cluster);
  std::set<edm4hep::Cluster> selected;
  double variance = 0;
  for (const auto& pfo : pfos) {
    if (pfo.getCharge() != 0 || !pfo.getTracks().empty()) continue;
    for (const auto& cluster : pfo.getClusters()) {
      if (!ecal.count(cluster) || cluster.hits_size() == 0 ||
          selected.count(cluster)) continue;
      const double energy = cluster.getEnergy();
      if (!std::isfinite(energy) || energy <= 0) continue;
      const auto& point = cluster.getPosition();
      Direction candidate;
      try { candidate = direction(point.x, point.y, point.z); }
      catch (const std::invalid_argument&) { continue; }
      const double dphi = std::remainder(candidate.phi - reference.phi, 2 * pi);
      if (std::abs(candidate.theta - reference.theta) > thetaWindow ||
          std::abs(dphi) > phiWindow) continue;
      selected.insert(cluster);
      result.clusterIndices.push_back(cluster.id().index);
      result.clusterEnergies.push_back(energy);
      result.energy += energy;
      const double sigma = stochasticError * std::sqrt(energy) + constantError * energy;
      result.clusterEnergyErrors.push_back(sigma);
      variance += sigma * sigma;
    }
  }
  result.sigmaEnergy = std::sqrt(variance);
  return result;
}
} // namespace breakpoint
