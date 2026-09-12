#include "UnconstrainedLoss.h"
#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>

namespace breakpoint {
namespace {
Eigen::MatrixXd eigen(const TMatrixD& input) {
  Eigen::MatrixXd output(input.GetNrows(), input.GetNcols());
  for (int i=0;i<input.GetNrows();++i)
    for (int j=0;j<input.GetNcols();++j) output(i,j)=input(i,j);
  return output;
}
}

double UnconstrainedLoss::add(const GaussianHitModel& hit,
    const TMatrixD& predictedCovariance, const TMatrixD& predictedResponse) {
  const auto h=eigen(hit.derivative);
  const Eigen::MatrixXd s=h*eigen(predictedCovariance)*h.transpose()+eigen(hit.noise);
  const Eigen::VectorXd r=eigen(hit.residual), response=h*eigen(predictedResponse);
  if (!s.allFinite() || !r.allFinite() || !response.allFinite())
    throw std::runtime_error("Nonfinite unconstrained-loss regression input");
  const Eigen::LLT<Eigen::MatrixXd> factor(s);
  if (factor.info()!=Eigen::Success)
    throw std::runtime_error("Unconstrained-loss innovation covariance is not positive definite");
  const double previous=quadratic();
  m_information+=response.dot(factor.solve(response));
  m_score+=response.dot(factor.solve(r));
  m_referenceQuadratic+=r.dot(factor.solve(r));
  const double increment=quadratic()-previous;
  if (!std::isfinite(increment) || increment < -1.e-7*(1.+m_referenceQuadratic))
    throw std::runtime_error("Invalid unconstrained-loss prefix quadratic");
  return std::max(0.,increment);
}

bool UnconstrainedLoss::identified() const {
  return m_information>0 && std::isfinite(m_information) && std::isfinite(1./m_information);
}
double UnconstrainedLoss::shift() const {
  if (!identified()) throw std::runtime_error("Loss is unidentifiable: zero data information");
  return m_score/m_information;
}
double UnconstrainedLoss::variance() const {
  if (!identified()) throw std::runtime_error("Loss is unidentifiable: zero data information");
  return 1./m_information;
}
double UnconstrainedLoss::quadratic() const {
  return m_referenceQuadratic-(identified() ? m_score*(m_score/m_information) : 0.);
}
} // namespace breakpoint
