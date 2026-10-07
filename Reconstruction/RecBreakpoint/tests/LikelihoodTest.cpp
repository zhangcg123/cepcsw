#include "TrackLikelihood.h"
#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace {
void checkFiniteLossPrior() {
  // Independent one-breakpoint model: one seed curvature plus one Gaussian
  // loss kick. The kick affects BOTH downstream hits, once at birth. Direct
  // dense covariance and latent least squares independently check the capture
  // used for each shared-SigmaLogLoss Minuit prior-center trial.
  for (double sigma : {0., .001, .01, .05}) {
    for (double center : {0., .03, .1}) {
      breakpoint::GaussianTrackModel model;
      model.seedCovariance.ResizeTo(5,5);
      model.seedCovariance.UnitMatrix();
      model.seedCovariance(2,2)=.04;
      Eigen::Matrix3d measurement=Eigen::Matrix3d::Zero();
      measurement.diagonal()<<.01,.02,.03;
      Eigen::Vector3d residual;
      residual<<.03,.10-2*center,.09-2*center;
      Eigen::Matrix<double,3,2> response;
      response<<.2,0., .2,2*sigma, .2,2*sigma;
      for(int i=0;i<3;++i) {
        TMatrixD h(1,5),v(1,1),r(1,1);
        h.Zero();h(0,2)=1;v(0,0)=measurement(i,i);r(0,0)=residual(i);
        model.hits.push_back({h,v,r});
        if(i) {
          TMatrixD f(5,5),q(5,5),shift(5,1);
          f.UnitMatrix();q.Zero();shift.Zero();
          if(i==1) q(2,2)=4*sigma*sigma;
          model.transitions.push_back({f,q,shift});
        }
      }
      const Eigen::Matrix3d joint=measurement+response*response.transpose();
      const Eigen::LLT<Eigen::Matrix3d> jointFactor(joint);
      const Eigen::Matrix3d lower=jointFactor.matrixL();
      const double expectedChi2=residual.dot(jointFactor.solve(residual));
      const double expectedLogdet=2*lower.diagonal().array().log().sum();
      const Eigen::Matrix3d precision=measurement.inverse();
      const Eigen::Matrix2d normal=Eigen::Matrix2d::Identity()+response.transpose()*precision*response;
      const Eigen::Vector2d fitted=normal.ldlt().solve(response.transpose()*precision*residual);
      const Eigen::Vector3d hitResidual=residual-response*fitted;
      const double completeChi2=hitResidual.dot(precision*hitResidual)+fitted.squaredNorm();
      const auto scored=breakpoint::evaluateSmoothedTrackLikelihood(model,completeChi2);
      const auto marginal=breakpoint::evaluateTrackLikelihood(model);
      // The same loss retained as a sixth coordinate: rectangular 6x5 birth,
      // correlated track/loss noise at birth, then a static loss coordinate.
      // Its normalization must match both the 5D marginal and dense reference.
      breakpoint::GaussianTrackModel persistent;
      persistent.seedCovariance.ResizeTo(model.seedCovariance);
      persistent.seedCovariance=model.seedCovariance;
      for(int i=0;i<3;++i) {
        const int dimensions=i ? 6 : 5;
        TMatrixD h(1,dimensions),v(1,1),r(1,1);
        h.Zero();h(0,2)=1;v(0,0)=measurement(i,i);r(0,0)=residual(i);
        persistent.hits.push_back({h,v,r});
        if(i) {
          const int previousDimensions=i==1 ? 5 : 6;
          TMatrixD f(6,previousDimensions),q(6,6),shift(previousDimensions,1);
          f.Zero();q.Zero();shift.Zero();
          for(int j=0;j<previousDimensions;++j) f(j,j)=1;
          if(i==1) {
            q(2,2)=4*sigma*sigma;
            q(2,5)=q(5,2)=2*sigma*sigma;
            q(5,5)=sigma*sigma;
          }
          persistent.transitions.push_back({f,q,shift});
        }
      }
      const auto persistentScore=breakpoint::evaluateSmoothedTrackLikelihood(persistent,completeChi2);
      const auto persistentMarginal=breakpoint::evaluateTrackLikelihood(persistent);
      if(std::abs(completeChi2-expectedChi2)>1.e-10 ||
         std::abs(scored.logDeterminant-expectedLogdet)>1.e-10 ||
         std::abs(scored.nll2-marginal.nll2)>1.e-10 ||
         std::abs(persistentScore.nll2-scored.nll2)>1.e-10 ||
         std::abs(persistentMarginal.nll2-marginal.nll2)>1.e-10)
        throw std::runtime_error("Shared Gaussian loss variance/normalization mismatch");
    }
    std::cout<<"Shared loss sigma="<<sigma<<": dense and complete-smoothed objectives agree\n";
  }
}
} // namespace

int main() {
  checkFiniteLossPrior();
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
    // Independent full-trajectory least squares. The seven unit-prior latent
    // coordinates are five seed components and two scalar process kicks.
    // This represents singular Q without an inverse or artificial noise.
    Eigen::MatrixXd response = Eigen::MatrixXd::Zero(6, 7);
    Eigen::MatrixXd measurementCov = Eigen::MatrixXd::Zero(6, 6);
    for (int i = 0; i < 3; ++i) {
      response(2*i, 0) = 1; response(2*i+1, 2) = 1;
      for (int j = 0; j < i; ++j) response(2*i, 5+j) = std::sqrt(noise);
      measurementCov(2*i, 2*i) = .2;
      measurementCov(2*i+1, 2*i+1) = .3;
      measurementCov(2*i, 2*i+1) = measurementCov(2*i+1, 2*i) = .02;
    }
    const Eigen::MatrixXd precision = measurementCov.inverse();
    const Eigen::MatrixXd normal = Eigen::MatrixXd::Identity(7, 7)
        + response.transpose()*precision*response;
    const Eigen::VectorXd fitted = normal.ldlt().solve(response.transpose()*precision*residual);
    const Eigen::VectorXd measurementResidual = residual-response*fitted;
    const double smoothedPenalty = measurementResidual.dot(precision*measurementResidual)
        + fitted.squaredNorm(); // measurement + seed + process penalties

    // Separate forward Kalman factorization of exactly the same linear model.
    Eigen::Vector2d mean = Eigen::Vector2d::Zero();
    Eigen::Matrix2d stateCov = Eigen::Matrix2d::Identity();
    const Eigen::Matrix2d measurementNoise = measurementCov.topLeftCorner<2,2>();
    double forwardPenalty = 0;
    for (int i = 0; i < 3; ++i) {
      if (i) stateCov(0,0) += noise;
      const Eigen::Vector2d innovation = residual.segment<2>(2*i)-mean;
      const Eigen::Matrix2d innovationCov = stateCov+measurementNoise;
      forwardPenalty += innovation.dot(innovationCov.ldlt().solve(innovation));
      const Eigen::Matrix2d gain = stateCov*innovationCov.inverse();
      mean += gain*innovation;
      stateCov = (Eigen::Matrix2d::Identity()-gain)*stateCov;
    }
    if (std::abs(smoothedPenalty-result.quadratic) > 1.e-10 ||
        std::abs(forwardPenalty-result.quadratic) > 1.e-10)
      throw std::runtime_error("Forward / full smoothed / marginal quadratic mismatch");
    const auto smoothed = breakpoint::evaluateSmoothedTrackLikelihood(model, smoothedPenalty);
    if (smoothed.quadratic != smoothedPenalty || smoothed.logDeterminant != result.logDeterminant ||
        smoothed.measurementDimensions != 6 || smoothed.latentDimensions != result.latentDimensions ||
        std::abs(smoothed.nll2-expected) > 1.e-10)
      throw std::runtime_error("Direct complete smoothed likelihood / normalization mismatch");
    // Deliberately supply a different chi2 to prove the helper uses its RTS
    // argument, not a silently recomputed marginal quadratic.
    const auto supplied = breakpoint::evaluateSmoothedTrackLikelihood(model, smoothedPenalty+2.);
    if (supplied.quadratic != smoothedPenalty+2. || std::abs(supplied.nll2-smoothed.nll2-2.) > 1.e-10)
      throw std::runtime_error("Supplied complete smoothed chi2 was not used");
    const auto zero = breakpoint::evaluateSmoothedTrackLikelihood(model, 0.);
    if (zero.quadratic != 0. || std::abs(zero.nll2-result.logDeterminant-6*std::log(2*std::acos(-1.))) > 1.e-10)
      throw std::runtime_error("Zero smoothed chi2 dropped or changed normalization");
    for (double invalid : {-1., std::numeric_limits<double>::infinity(),
                           std::numeric_limits<double>::quiet_NaN()}) {
      bool rejected = false;
      try { breakpoint::evaluateSmoothedTrackLikelihood(model, invalid); }
      catch (const std::runtime_error&) { rejected = true; }
      if (!rejected) throw std::runtime_error("Invalid smoothed chi2 was accepted");
    }
    std::cout << "Q=" << noise << ": direct RTS objective and unchanged normalization passed\n";
    std::cout << "Q=" << noise << ": forward, full smoothed and marginal quadratics agree\n";
    std::cout << "Q=" << noise << ": independent dense reference passed\n";
  }
  try {
    breakpoint::evaluateTrackLikelihood({});
    throw std::logic_error("Empty Gaussian model was accepted");
  } catch (const std::runtime_error&) {}

  // Reference-origin invariance: nontrivial F, singular Q, and nonzero affine
  // offsets. Moving the expansion coordinates must not change the likelihood.
  breakpoint::GaussianTrackModel original;
  original.seedCovariance.ResizeTo(5,5); original.seedCovariance.UnitMatrix();
  for (int i=0;i<4;++i) {
    TMatrixD h(2,5), v(2,2), residual(2,1);
    h.Zero(); v.UnitMatrix(); h(0,0)=1;h(0,2)=.3;h(1,3)=1;h(1,4)=.2;
    residual(0,0)=.2*i;residual(1,0)=.4-.1*i;
    original.hits.push_back({h,v,residual});
    if (i) {
      TMatrixD f(5,5),q(5,5),shift(5,1);
      f.UnitMatrix();f(0,2)=.12;f(2,2)=1.3;f(3,4)=.2;
      q.Zero();q(1,1)=.01;q(4,4)=.02;shift.Zero();shift(2,0)=.13*i;
      original.transitions.push_back({f,q,shift});
    }
  }
  auto moved=original;
  TMatrixD previousShift(5,1);previousShift.Zero();
  for(int i=0;i<4;++i) {
    TMatrixD shift(5,1);shift.Zero();
    if(i) {shift(0,0)=.03*i;shift(2,0)=-.07*i;shift(4,0)=.02*i;}
    moved.hits[i].residual-=moved.hits[i].derivative*shift;
    if(i) {
      TMatrixD inverse(moved.transitions[i-1].transport);inverse.Invert();
      moved.transitions[i-1].sourceShift+=previousShift-inverse*shift;
    }
    previousShift=shift;
  }
  const auto a=breakpoint::evaluateTrackLikelihood(original);
  const auto b=breakpoint::evaluateTrackLikelihood(moved);
  if(std::abs(a.nll2-b.nll2)>1.e-10)
    throw std::runtime_error("Affine reference-origin invariance failed");
  std::cout << "Nonidentity transport/affine reference-origin invariance passed\n";
}
