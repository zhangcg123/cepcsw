#include "TrackState.h"

#include "TDecompChol.h"
#include "TMatrixDSym.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace breakpoint {
namespace {
constexpr double curvaturePerTesla = 2.99792458e-4;
constexpr double pi = 3.14159265358979323846;
int packed(int i, int j) {
  if (i < j) std::swap(i, j);
  return i * (i + 1) / 2 + j;
}
} // namespace

TrackState fromEDM(const edm4hep::TrackState& state, double bz) {
  if (!std::isfinite(bz) || bz == 0) throw std::runtime_error("Invalid Bz");
  const double alpha = curvaturePerTesla * bz;
  const double scales[] = {-1, 1, 1 / alpha, 1, 1};
  TrackState result;
  result.pivot = {state.referencePoint.x, state.referencePoint.y, state.referencePoint.z};
  result.mean(0, 0) = -state.D0;
  result.mean(1, 0) = std::remainder(state.phi - pi / 2, 2 * pi);
  result.mean(2, 0) = state.omega / alpha;
  result.mean(3, 0) = state.Z0;
  result.mean(4, 0) = state.tanLambda;
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      result.covariance(i, j) = scales[i] * scales[j] * state.covMatrix[packed(i, j)];
  return result;
}

edm4hep::TrackState toEDM(const TrackState& state, double bz, int location) {
  const double alpha = curvaturePerTesla * bz;
  const double scales[] = {-1, 1, alpha, 1, 1};
  edm4hep::TrackState result{};
  result.location = location;
  result.referencePoint = {static_cast<float>(state.pivot.x), static_cast<float>(state.pivot.y),
                           static_cast<float>(state.pivot.z)};
  result.D0 = -state.mean(0, 0);
  result.phi = std::remainder(state.mean(1, 0) + pi / 2, 2 * pi);
  result.omega = alpha * state.mean(2, 0);
  result.Z0 = state.mean(3, 0);
  result.tanLambda = state.mean(4, 0);
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j <= i; ++j)
      result.covMatrix[packed(i, j)] = scales[i] * scales[j] * state.covariance(i, j);
  return result;
}

double transverseMomentumError(const edm4hep::TrackState& state, double bz) {
  // EDM packed covariance index (omega,omega) = 2*(2+1)/2 + 2 = 5.
  const double omega = state.omega;
  const double variance = state.covMatrix[5];
  if (!std::isfinite(bz) || !std::isfinite(omega) || omega == 0 ||
      !std::isfinite(variance) || variance < 0)
    return std::numeric_limits<double>::quiet_NaN();
  const double pt = std::abs(bz * curvaturePerTesla / omega);
  return pt * std::sqrt(variance) / std::abs(omega);
}

TMatrixD transpose(const TMatrixD& matrix) {
  return TMatrixD(TMatrixD::kTransposed, matrix);
}

TMatrixD inverseCovariance(const TMatrixD& matrix) {
  // Normalize units before Cholesky: loose seeds span many orders of magnitude.
  const int n = matrix.GetNrows();
  TMatrixDSym correlation(n);
  for (int i = 0; i < n; ++i) {
    if (!(matrix(i, i) > 0) || !std::isfinite(matrix(i, i)))
      throw std::runtime_error("Nonpositive covariance diagonal");
    for (int j = 0; j < n; ++j)
      correlation(i, j) = matrix(i, j) / std::sqrt(matrix(i, i) * matrix(j, j));
  }
  TDecompChol decomposition(correlation);
  if (!decomposition.Decompose() || !decomposition.Invert(correlation))
    throw std::runtime_error("Covariance is not positive definite");
  TMatrixD inverse(n, n);
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      inverse(i, j) = correlation(i, j) / std::sqrt(matrix(i, i) * matrix(j, j));
  return inverse;
}

TMatrixD stateDifference(const TMatrixD& left, const TMatrixD& right) {
  TMatrixD result = left - right;
  result(1, 0) = std::remainder(result(1, 0), 2 * pi);
  return result;
}

void validateCovariance(TMatrixD& matrix) {
  for (int i = 0; i < matrix.GetNrows(); ++i)
    for (int j = i; j < matrix.GetNcols(); ++j) {
      if (!std::isfinite(matrix(i, j)) || !std::isfinite(matrix(j, i)))
        throw std::runtime_error("Nonfinite covariance");
      matrix(i, j) = matrix(j, i) = 0.5 * matrix(i, j) + 0.5 * matrix(j, i);
    }
  inverseCovariance(matrix);
}
} // namespace breakpoint
