#ifndef RECBREAKPOINT_ENERGYMEASUREMENTSITE_H
#define RECBREAKPOINT_ENERGYMEASUREMENTSITE_H

#include "GaussianTrackModel.h"
#include "LossTrackState.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/TKalTrackState.h"

namespace breakpoint {
/// One linearized scalar energy observation; the update itself is native KalTest.
class EnergyMeasurementSite final : public TKalTrackSite {
public:
  EnergyMeasurementSite(const LossTrackState& predicted, const GaussianHitModel& observation)
      : TKalTrackSite(1, 6), m_reference(predicted.mean), m_derivative(observation.derivative) {
    SetOwner();
    SetPivot(TVector3(predicted.pivot.x, predicted.pivot.y, predicted.pivot.z));
    GetMeasVec() = observation.residual;
    GetMeasNoiseMat() = observation.noise;
    Add(new TKalTrackState(TKalMatrix(predicted.mean), TKalMatrix(predicted.covariance),
                          *this, TVKalSite::kPredicted, 6));
  }
  Int_t CalcExpectedMeasVec(const TVKalState& state, TKalMatrix& expected) override {
    expected = m_derivative * stateDifference(state, m_reference);
    return 1;
  }
  Int_t CalcMeasVecDerivative(const TVKalState&, TKalMatrix& derivative) override {
    derivative = m_derivative;
    return 1;
  }
  Bool_t IsAccepted() override { return std::isfinite(GetDeltaChi2()) && GetDeltaChi2() >= 0; }

private:
  TVKalState& CreateState(const TKalMatrix& mean, Int_t type) override {
    return *new TKalTrackState(mean, *this, type, 6);
  }
  TVKalState& CreateState(const TKalMatrix& mean, const TKalMatrix& covariance, Int_t type) override {
    return *new TKalTrackState(mean, covariance, *this, type, 6);
  }
  TMatrixD m_reference, m_derivative;
};

inline LossTrackState updateEnergyState(const LossTrackState& predicted,
                                       const GaussianHitModel& observation) {
  auto result = predicted;
  validateCovariance(result.covariance);
  if (!std::isfinite(observation.residual(0, 0)) ||
      !std::isfinite(observation.noise(0, 0)) || observation.noise(0, 0) <= 0)
    throw std::invalid_argument("Invalid scalar energy observation");
  EnergyMeasurementSite site(predicted, observation);
  if (!site.Filter()) throw std::runtime_error("KalTest rejected energy measurement");
  const auto& updated = site.GetState(TVKalSite::kFiltered);
  result.mean = updated;
  result.covariance = updated.GetCovMat();
  for (int i = 0; i < 6; ++i)
    if (!std::isfinite(result.mean(i, 0)))
      throw std::runtime_error("Nonfinite energy-updated state");
  if (result.mean(2, 0) * predicted.mean(2, 0) <= 0)
    throw std::runtime_error("Energy update crosses zero curvature");
  validateCovariance(result.covariance);
  return result;
}
} // namespace breakpoint
#endif
