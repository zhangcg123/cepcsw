#include "RecBreakpoint/AugmentedTransport.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace breakpoint {
namespace {
template <std::size_t N>
void checkFinite(const std::array<double, N>& values) {
  for (double value : values)
    if (!std::isfinite(value))
      throw std::invalid_argument("RecBreakpoint: non-finite transport input");
}

void checkSymmetric(const Matrix6& matrix) {
  for (std::size_t i = 0; i < 6; ++i)
    for (std::size_t j = i + 1; j < 6; ++j) {
      const double a = matrix[i * 6 + j], b = matrix[j * 6 + i];
      // Normalize to covariance units, not the off-diagonal itself: a
      // nominally zero correlation can contain harmless roundoff.
      const double scale = std::max({1.0e-30, std::abs(a), std::abs(b),
          std::sqrt(std::abs(matrix[i * 6 + i] * matrix[j * 6 + j]))});
      if (std::abs(a - b) > 1.0e-10 * scale)
        throw std::invalid_argument("RecBreakpoint: asymmetric covariance");
    }
}
} // namespace

Matrix6 AugmentedTransport::jacobian(const Matrix5& helix,
                                     const Vector5& derivative) {
  checkFinite(helix);
  checkFinite(derivative);
  Matrix6 result{};
  for (std::size_t i = 0; i < 5; ++i) {
    for (std::size_t j = 0; j < 5; ++j)
      result[i * 6 + j] = helix[i * 5 + j];
    result[i * 6 + 5] = derivative[i];
  }
  result[35] = 1.0;
  return result;
}

Matrix6 AugmentedTransport::processNoise(const Matrix5& helixNoise) {
  checkFinite(helixNoise);
  Matrix6 result{};
  for (std::size_t i = 0; i < 5; ++i)
    for (std::size_t j = 0; j < 5; ++j)
      result[i * 6 + j] = helixNoise[i * 5 + j];
  checkSymmetric(result);
  return result;
}

Matrix6 AugmentedTransport::covariance(const Matrix6& prior,
                                       const Matrix6& transport,
                                       const Matrix6& noise) {
  checkFinite(prior);
  checkFinite(transport);
  checkFinite(noise);
  checkSymmetric(prior);
  checkSymmetric(noise);
  Matrix6 intermediate{}, result = noise;
  for (std::size_t i = 0; i < 6; ++i)
    for (std::size_t j = 0; j < 6; ++j)
      for (std::size_t k = 0; k < 6; ++k)
        intermediate[i * 6 + j] += transport[i * 6 + k] * prior[k * 6 + j];
  for (std::size_t i = 0; i < 6; ++i)
    for (std::size_t j = 0; j < 6; ++j)
      for (std::size_t k = 0; k < 6; ++k)
        result[i * 6 + j] += intermediate[i * 6 + k] * transport[j * 6 + k];
  checkFinite(result);
  for (std::size_t i = 0; i < 6; ++i)
    for (std::size_t j = i + 1; j < 6; ++j) {
      const double average = 0.5 * result[i * 6 + j] + 0.5 * result[j * 6 + i];
      result[i * 6 + j] = result[j * 6 + i] = average;
    }
  return result;
}
} // namespace breakpoint
