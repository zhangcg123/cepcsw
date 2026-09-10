#include "TrackLikelihood.h"

#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>

namespace breakpoint {
namespace {
using Matrix = Eigen::MatrixXd;
using Vector = Eigen::VectorXd;

Matrix toEigen(const TMatrixD& value) {
  Matrix result(value.GetNrows(), value.GetNcols());
  for (int row = 0; row < result.rows(); ++row)
    for (int column = 0; column < result.cols(); ++column)
      result(row, column) = value(row, column);
  return result;
}

/// Retain only supported stochastic directions. In particular, Q_MS can be
/// singular. Scaling before decomposition avoids comparing unlike state units.
Matrix supportedCovarianceRoot(const Matrix& value) {
  const Matrix covariance = .5 * (value + value.transpose());
  const Vector scale = covariance.diagonal().cwiseMax(0).cwiseSqrt();
  Matrix normalized = Matrix::Zero(covariance.rows(), covariance.cols());
  for (int row = 0; row < covariance.rows(); ++row) {
    for (int column = 0; column < covariance.cols(); ++column) {
      if (scale(row) > 0 && scale(column) > 0)
        normalized(row, column) = covariance(row, column) / (scale(row) * scale(column));
      else if (std::abs(covariance(row, column)) > 1.e-20)
        throw std::runtime_error("Gaussian covariance has an unsupported zero diagonal");
    }
  }
  Eigen::SelfAdjointEigenSolver<Matrix> decomposition(normalized);
  if (decomposition.info() != Eigen::Success || decomposition.eigenvalues().minCoeff() < -1.e-8)
    throw std::runtime_error("Gaussian covariance is not positive semidefinite");
  int rank = 0;
  for (int column = 0; column < covariance.cols(); ++column)
    if (decomposition.eigenvalues()(column) > 1.e-12) ++rank;
  Matrix root(covariance.rows(), rank);
  int retained = 0;
  for (int column = 0; column < covariance.cols(); ++column) {
    if (decomposition.eigenvalues()(column) > 1.e-12)
      root.col(retained++) = scale.asDiagonal() * decomposition.eigenvectors().col(column)
          * std::sqrt(decomposition.eigenvalues()(column));
  }
  if ((root * root.transpose() - covariance).norm() > 1.e-8 * std::max(covariance.norm(), 1.e-25))
    throw std::runtime_error("Gaussian covariance square-root closure failed");
  return root;
}

struct WhitenedModel {
  Matrix response; // d = response * u + epsilon; u and epsilon are standard Gaussian
  Vector residual;
  double measurementLogDeterminant = 0;
};

WhitenedModel whitenModel(const GaussianTrackModel& model) {
  const int hitCount = model.hits.size();
  if (!hitCount || int(model.transitions.size()) != hitCount - 1)
    throw std::runtime_error("Incomplete Gaussian track model");
  std::vector<Matrix> roots{supportedCovarianceRoot(toEigen(model.seedCovariance))};
  int latentDimensions = roots.front().cols(), measurementDimensions = 0;
  for (const auto& edge : model.transitions) {
    roots.push_back(supportedCovarianceRoot(toEigen(edge.noise)));
    latentDimensions += roots.back().cols();
  }
  for (const auto& hit : model.hits) measurementDimensions += hit.residual.GetNrows();
  WhitenedModel result{Matrix::Zero(measurementDimensions, latentDimensions),
                      Vector(measurementDimensions), 0};
  Matrix stateResponse = Matrix::Zero(5, latentDimensions);
  stateResponse.leftCols(roots[0].cols()) = roots[0];
  Vector meanOffset = Vector::Zero(5);
  int column = roots[0].cols(), row = 0;
  for (int i = 0; i < hitCount; ++i) {
    if (i) {
      const auto& edge = model.transitions[i - 1];
      const Matrix transport = toEigen(edge.transport);
      meanOffset = transport * (meanOffset + toEigen(edge.sourceShift));
      stateResponse = (transport * stateResponse).eval();
      stateResponse.middleCols(column, roots[i].cols()) = roots[i];
      column += roots[i].cols();
    }
    const auto& hit = model.hits[i];
    const Matrix derivative = toEigen(hit.derivative);
    Eigen::LLT<Matrix> decomposition(toEigen(hit.noise));
    if (decomposition.info() != Eigen::Success)
      throw std::runtime_error("Gaussian hit noise is not positive definite");
    const Matrix lower = decomposition.matrixL();
    const int dimensions = derivative.rows();
    result.response.middleRows(row, dimensions) = lower.triangularView<Eigen::Lower>().solve(derivative * stateResponse);
    result.residual.segment(row, dimensions) = lower.triangularView<Eigen::Lower>().solve(
        toEigen(hit.residual) - derivative * meanOffset);
    result.measurementLogDeterminant += 2 * lower.diagonal().array().log().sum();
    row += dimensions;
  }
  return result;
}

struct GaussianScore { double quadratic, logDeterminant; };

/// QR([I,A]^T) gives C=R^T R for C=I+A A^T, avoiding an ill-conditioned
/// normal matrix. The triangular solve is the conditional hit factorization.
GaussianScore orderedScore(const Matrix& response, const Vector& residual) {
  const int dimensions = response.rows();
  Matrix generator(dimensions, dimensions + response.cols());
  generator.leftCols(dimensions).setIdentity();
  generator.rightCols(response.cols()) = response;
  Eigen::HouseholderQR<Matrix> decomposition(generator.transpose());
  const Matrix upper = decomposition.matrixQR().topLeftCorner(dimensions, dimensions)
      .triangularView<Eigen::Upper>();
  const Vector innovation = upper.transpose().triangularView<Eigen::Lower>().solve(residual);
  return {innovation.squaredNorm(), 2 * upper.diagonal().array().abs().log().sum()};
}

} // namespace

TrackLikelihoodResult evaluateTrackLikelihood(const GaussianTrackModel& model) {
  const auto whitened = whitenModel(model);
  const auto score = orderedScore(whitened.response, whitened.residual);
  TrackLikelihoodResult result;
  result.measurementDimensions = whitened.residual.size();
  result.latentDimensions = whitened.response.cols();
  const double normalization = result.measurementDimensions * std::log(2 * std::acos(-1.));
  result.quadratic = score.quadratic;
  result.logDeterminant = score.logDeterminant + whitened.measurementLogDeterminant;
  result.nll2 = result.quadratic + result.logDeterminant + normalization;
  if (!std::isfinite(result.nll2)) throw std::runtime_error("Nonfinite Gaussian likelihood");
  return result;
}
} // namespace breakpoint
