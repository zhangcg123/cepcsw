#include "DiffuseLossState.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace breakpoint {
namespace {
double dot(const TMatrixD& left, const TMatrixD& right) {
  if (left.GetNcols() != 1 || right.GetNcols() != 1 ||
      left.GetNrows() != right.GetNrows())
    throw std::invalid_argument("Diffuse dot-product dimensions differ");
  double value = 0;
  for (int i = 0; i < left.GetNrows(); ++i) value += left(i, 0) * right(i, 0);
  return value;
}

TMatrixD whitenedRows(const TMatrixD& input, const TMatrixD& noise) {
  const int count = input.GetNrows();
  if (count != noise.GetNrows() || count != noise.GetNcols() ||
      (count != 1 && count != 2))
    throw std::invalid_argument("Diffuse update requires one or two native hit coordinates");
  if (!(noise(0, 0) > 0) || !std::isfinite(noise(0, 0)))
    throw std::runtime_error("Invalid diffuse hit noise");
  TMatrixD result(input);
  const double first = std::sqrt(noise(0, 0));
  for (int j = 0; j < input.GetNcols(); ++j) result(0, j) = input(0, j) / first;
  if (count == 2) {
    const double cross = .5 * (noise(0, 1) + noise(1, 0)) / first;
    const double remaining = noise(1, 1) - cross * cross;
    if (!(remaining > 0) || !std::isfinite(remaining))
      throw std::runtime_error("Diffuse hit noise is not positive definite");
    const double second = std::sqrt(remaining);
    for (int j = 0; j < input.GetNcols(); ++j)
      result(1, j) = (input(1, j) - cross * result(0, j)) / second;
  }
  return result;
}

TMatrixD row(const TMatrixD& matrix, int index) {
  TMatrixD result(1, matrix.GetNcols());
  for (int j = 0; j < matrix.GetNcols(); ++j) result(0, j) = matrix(index, j);
  return result;
}

void symmetrize(TMatrixD& matrix) {
  for (int i = 0; i < matrix.GetNrows(); ++i)
    for (int j = i + 1; j < matrix.GetNcols(); ++j)
      matrix(i, j) = matrix(j, i) = .5 * (matrix(i, j) + matrix(j, i));
}
} // namespace

bool DiffuseLossState::unresolved() const {
  return dot(diffuseDirection, diffuseDirection) > 0;
}

DiffuseHitUpdate updateDiffuseHit(DiffuseLossState& state,
                                  const TMatrixD& derivative,
                                  const TMatrixD& noise,
                                  const TMatrixD& innovation) {
  if (derivative.GetNcols() != 6 || innovation.GetNcols() != 1 ||
      innovation.GetNrows() != derivative.GetNrows())
    throw std::invalid_argument("Invalid diffuse measurement dimensions");
  const TMatrixD h = whitenedRows(derivative, noise);
  const TMatrixD r = whitenedRows(innovation, noise);
  TMatrixD increment(6, 1); increment.Zero();
  DiffuseHitUpdate score;
  for (int i = 0; i < h.GetNrows(); ++i) {
    const auto hi = row(h, i);
    const double residual = r(i, 0) - (hi * increment)(0, 0);
    const TMatrixD finiteCross = state.finiteCovariance * transpose(hi);
    const double finiteVariance = (hi * finiteCross)(0, 0) + 1.;
    if (!(finiteVariance > 0) || !std::isfinite(finiteVariance))
      throw std::runtime_error("Invalid finite diffuse-hit innovation variance");
    const double diffuseProjection = (hi * state.diffuseDirection)(0, 0);
    if (state.unresolved() && std::abs(diffuseProjection) >
          1.e-12 * std::sqrt(dot(state.diffuseDirection, state.diffuseDirection) *
                             (hi * transpose(hi))(0, 0))) {
      const TMatrixD direction(state.diffuseDirection);
      const TMatrixD cross = direction * diffuseProjection;
      const double infiniteVariance = diffuseProjection * diffuseProjection;
      const TMatrixD gain = cross * (1. / infiniteVariance);
      increment += gain * residual;
      state.mean += gain * residual;
      state.finiteCovariance -=
          (cross * transpose(finiteCross) + finiteCross * transpose(cross)) *
              (1. / infiniteVariance) -
          (cross * transpose(cross)) *
              (finiteVariance / (infiniteVariance * infiniteVariance));
      state.diffuseDirection.Zero(); // exactly one unresolved scalar direction
      ++score.diffuseDimensionsConsumed;
    } else {
      const TMatrixD gain = finiteCross * (1. / finiteVariance);
      increment += gain * residual;
      state.mean += gain * residual;
      state.finiteCovariance -= finiteCross * transpose(finiteCross) * (1. / finiteVariance);
      score.finiteChi2 += residual * residual / finiteVariance;
    }
    symmetrize(state.finiteCovariance);
    validateCovariance(state.finiteCovariance);
  }
  return score;
}

TMatrixD diffuseRtsGain(const TMatrixD& sourceFiniteCovariance,
                        const TMatrixD& sourceDiffuseDirection,
                        const TMatrixD& transport,
                        const TMatrixD& targetPredictedFiniteCovariance,
                        const TMatrixD& targetPredictedDiffuseDirection) {
  const int source = sourceFiniteCovariance.GetNrows();
  const int target = targetPredictedFiniteCovariance.GetNrows();
  if (sourceFiniteCovariance.GetNcols() != source ||
      sourceDiffuseDirection.GetNrows() != source || sourceDiffuseDirection.GetNcols() != 1 ||
      transport.GetNrows() != target || transport.GetNcols() != source ||
      targetPredictedFiniteCovariance.GetNcols() != target ||
      targetPredictedDiffuseDirection.GetNrows() != target ||
      targetPredictedDiffuseDirection.GetNcols() != 1)
    throw std::invalid_argument("Invalid diffuse RTS dimensions");
  const TMatrixD finiteCross = sourceFiniteCovariance * transpose(transport);
  const TMatrixD inverse = inverseCovariance(targetPredictedFiniteCovariance);
  if (dot(targetPredictedDiffuseDirection, targetPredictedDiffuseDirection) == 0)
    return finiteCross * inverse;
  const TMatrixD weighted = inverse * targetPredictedDiffuseDirection;
  const double denominator = dot(targetPredictedDiffuseDirection, weighted);
  if (!(denominator > 0) || !std::isfinite(denominator))
    throw std::runtime_error("Invalid diffuse RTS direction information");
  const TMatrixD projected = inverse - weighted * transpose(weighted) * (1. / denominator);
  return finiteCross * projected +
         sourceDiffuseDirection * transpose(weighted) * (1. / denominator);
}

TMatrixD diffuseRtsCovariance(const TMatrixD& sourceFiniteCovariance,
                              const TMatrixD& gain, const TMatrixD& transport,
                              const TMatrixD& finiteProcessNoise,
                              const TMatrixD& targetSmoothedCovariance) {
  const int source = sourceFiniteCovariance.GetNrows();
  const int target = targetSmoothedCovariance.GetNrows();
  if (sourceFiniteCovariance.GetNcols() != source ||
      gain.GetNrows() != source || gain.GetNcols() != target ||
      transport.GetNrows() != target || transport.GetNcols() != source ||
      finiteProcessNoise.GetNrows() != target || finiteProcessNoise.GetNcols() != target ||
      targetSmoothedCovariance.GetNcols() != target)
    throw std::invalid_argument("Invalid diffuse RTS covariance dimensions");
  TMatrixD remaining(source, source);
  remaining.UnitMatrix(); remaining -= gain * transport;
  TMatrixD covariance = remaining * sourceFiniteCovariance * transpose(remaining) +
      gain * (finiteProcessNoise + targetSmoothedCovariance) * transpose(gain);
  symmetrize(covariance);
  validateCovariance(covariance);
  return covariance;
}

} // namespace breakpoint
