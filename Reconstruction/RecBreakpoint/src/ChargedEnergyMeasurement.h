#ifndef RECBREAKPOINT_CHARGEDENERGYMEASUREMENT_H
#define RECBREAKPOINT_CHARGEDENERGYMEASUREMENT_H

#include "LossTrackState.h"
#include <string>

namespace breakpoint {
enum class ChargedEnergyMode { ElectronOnly, ElectronPhoton };

inline ChargedEnergyMode parseChargedEnergyMode(const std::string& value) {
  if (value == "ElectronOnly") return ChargedEnergyMode::ElectronOnly;
  if (value == "ElectronPhoton") return ChargedEnergyMode::ElectronPhoton;
  throw std::invalid_argument("ChargedEcalEnergyMode must be ElectronOnly or ElectronPhoton");
}

struct ChargedEnergyProjection {
  double energy = 0;
  TMatrixD derivative{1, 6};
};

inline ChargedEnergyMode effectiveChargedEnergyMode(ChargedEnergyMode requested,
                                                   bool hasBreakpoint, bool hasNeutral) {
  return !hasBreakpoint || hasNeutral ? ChargedEnergyMode::ElectronOnly : requested;
}

/// ECAL-face scalar energy; no loss coordinate is needed for an unbroken track.
template<class State>
ChargedEnergyProjection chargedEnergyProjection(const State& state,
                                                       ChargedEnergyMode mode) {
  if (mode == ChargedEnergyMode::ElectronPhoton && state.mean.GetNrows() != 6)
    throw std::invalid_argument("ElectronPhoton requires a breakpoint loss coordinate");
  const double kappa = state.mean(2, 0), tanl = state.mean(4, 0);
  const double momentum = std::hypot(1., tanl) / std::abs(kappa);
  ChargedEnergyProjection result;
  result.derivative.ResizeTo(1, state.mean.GetNrows());
  result.energy = mode == ChargedEnergyMode::ElectronPhoton
      ? momentum * std::exp(state.mean(5, 0)) : momentum;
  if (!std::isfinite(kappa) || kappa == 0 || !std::isfinite(tanl) ||
      !std::isfinite(result.energy) || result.energy <= 0)
    throw std::runtime_error("Invalid predicted charged ECAL energy");
  result.derivative.Zero();
  result.derivative(0, 2) = -result.energy / kappa;
  result.derivative(0, 4) = result.energy * tanl / (1. + tanl * tanl);
  if (mode == ChargedEnergyMode::ElectronPhoton)
    result.derivative(0, 5) = result.energy;
  // shortcut: the sum assumes one loss and no other energy change to ECAL;
  // transport photon energy separately before supporting additional losses.
  return result;
}
} // namespace breakpoint
#endif
