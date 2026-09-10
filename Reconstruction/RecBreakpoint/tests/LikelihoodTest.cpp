#include "TrackLikelihood.h"
#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <stdexcept>

int main() {
  // Independent dense Gaussian reference. Includes unobserved state
  // coordinates, correlated measurement errors and singular/no process noise.
  for (double noise : {0., .1}) {
    breakpoint::GaussianTrackModel model;
    model.seedCovariance.ResizeTo(5, 5);
    model.seedCovariance.UnitMatrix();
    Eigen::MatrixXd covariance = Eigen::MatrixXd::Zero(6, 6);
    Eigen::VectorXd residual(6);
    for (int i = 0; i < 3; ++i) {
      TMatrixD derivative(2, 5), measurementNoise(2, 2), hitResidual(2, 1);
      derivative.Zero(); measurementNoise.Zero();
      derivative(0, 0) = 1; derivative(1, 2) = 1;
      measurementNoise(0, 0) = .2; measurementNoise(1, 1) = .3;
      measurementNoise(0, 1) = measurementNoise(1, 0) = .02;
      hitResidual(0, 0) = .3 + i * .2; hitResidual(1, 0) = -.4 + i * .1;
      model.hits.push_back({derivative, measurementNoise, hitResidual});
      if (i) {
        TMatrixD transport(5, 5), processNoise(5, 5), shift(5, 1);
        transport.UnitMatrix(); processNoise.Zero(); shift.Zero();
        processNoise(0, 0) = noise;
        model.transitions.push_back({transport, processNoise, shift});
      }
      residual(2 * i) = hitResidual(0, 0); residual(2 * i + 1) = hitResidual(1, 0);
      for (int j = 0; j < 3; ++j) {
        covariance(2 * i, 2 * j) = 1 + std::min(i, j) * noise;
        covariance(2 * i + 1, 2 * j + 1) = 1;
        if (i == j) {
          covariance(2 * i, 2 * j) += .2; covariance(2 * i + 1, 2 * j + 1) += .3;
          covariance(2 * i, 2 * j + 1) = covariance(2 * i + 1, 2 * j) = .02;
        }
      }
    }
    Eigen::LLT<Eigen::MatrixXd> decomposition(covariance);
    const Eigen::MatrixXd lower = decomposition.matrixL();
    const double expected = residual.dot(decomposition.solve(residual))
        + 2 * lower.diagonal().array().log().sum() + 6 * std::log(2 * std::acos(-1.));
    const auto result = breakpoint::evaluateTrackLikelihood(model);
    if (std::abs(result.nll2 - expected) > 1.e-10) throw std::runtime_error("Gaussian reference mismatch");
    std::cout << "Q=" << noise << ": independent dense reference passed\n";
  }
  try {
    breakpoint::evaluateTrackLikelihood({});
    throw std::logic_error("Empty Gaussian model was accepted");
  } catch (const std::runtime_error&) {}
}
