#ifndef RECBREAKPOINT_SCALARMEASUREMENTSITE_H
#define RECBREAKPOINT_SCALARMEASUREMENTSITE_H

#include "GaussianTrackModel.h"
#include "LossTrackState.h"
#include "kaltest/TKalTrackSite.h"
#include "kaltest/TKalTrackState.h"

namespace breakpoint {
/// One scalar observation of a 5D/6D state; the update itself is native KalTest.
class ScalarMeasurementSite final : public TKalTrackSite {
public:
  template<class State>
  ScalarMeasurementSite(const State& predicted, const GaussianHitModel& observation)
      : TKalTrackSite(1, predicted.mean.GetNrows()), m_reference(predicted.mean),
        m_derivative(observation.derivative), m_dimensions(predicted.mean.GetNrows()) {
    SetOwner();
    SetPivot(TVector3(predicted.pivot.x, predicted.pivot.y, predicted.pivot.z));
    GetMeasVec() = observation.residual;
    GetMeasNoiseMat() = observation.noise;
    Add(new TKalTrackState(TKalMatrix(predicted.mean), TKalMatrix(predicted.covariance),
                          *this, TVKalSite::kPredicted, m_dimensions));
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
    return *new TKalTrackState(mean, *this, type, m_dimensions);
  }
  TVKalState& CreateState(const TKalMatrix& mean, const TKalMatrix& covariance, Int_t type) override {
    return *new TKalTrackState(mean, covariance, *this, type, m_dimensions);
  }
  TMatrixD m_reference, m_derivative;
  int m_dimensions;
};

template<class State>
State updateScalarState(const State& predicted, const GaussianHitModel& observation) {
  auto result = predicted;
  validateCovariance(result.covariance);
  if ((predicted.mean.GetNrows() != 5 && predicted.mean.GetNrows() != 6) ||
      observation.derivative.GetNrows() != 1 ||
      observation.derivative.GetNcols() != predicted.mean.GetNrows() ||
      observation.residual.GetNrows() != 1 || observation.residual.GetNcols() != 1 ||
      observation.noise.GetNrows() != 1 || observation.noise.GetNcols() != 1)
    throw std::invalid_argument("Invalid scalar observation dimensions");
  if (!std::isfinite(observation.residual(0, 0)) ||
      !std::isfinite(observation.noise(0, 0)) || observation.noise(0, 0) <= 0)
    throw std::invalid_argument("Invalid scalar observation");
  ScalarMeasurementSite site(predicted, observation);
  if (!site.Filter()) throw std::runtime_error("KalTest rejected scalar measurement");
  const auto& updated = site.GetState(TVKalSite::kFiltered);
  result.mean = updated;
  result.covariance = updated.GetCovMat();
  for (int i = 0; i < predicted.mean.GetNrows(); ++i)
    if (!std::isfinite(result.mean(i, 0)))
      throw std::runtime_error("Nonfinite scalar-updated state");
  if (result.mean(2, 0) * predicted.mean(2, 0) <= 0)
    throw std::runtime_error("Scalar update crosses zero curvature");
  validateCovariance(result.covariance);
  return result;
}
} // namespace breakpoint
#endif
