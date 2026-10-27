#ifndef BP_DELAYED_NEUTRAL_CONDITIONING_H
#define BP_DELAYED_NEUTRAL_CONDITIONING_H

#include "ScalarMeasurementSite.h"
#include <Eigen/Dense>
#include <vector>

namespace breakpoint {
// All gains/covariances refer to the SAME posterior before the new observation.
// Each marginal is conditioned once; updated marginals never feed this loop.
inline void conditionTrajectory(std::vector<TMatrixD>& means,
    std::vector<TMatrixD>& covariances, const std::vector<TMatrixD>& gains,
    int target, const GaussianHitModel& observation) {
  const auto oldMeans = means, oldCov = covariances;
  const int n = means.size(), d = means.at(target).GetNrows();
  if (gains.size()+1 != means.size()) throw std::runtime_error("Incomplete RTS gain chain");
  std::vector<TMatrixD> cross;
  for (const auto& mean : means) cross.emplace_back(mean.GetNrows(), d);
  cross[target] = oldCov[target];
  for (int i=target-1;i>=0;--i) cross[i] = gains[i] * cross[i+1];
  TMatrixD chain(d,d); chain.UnitMatrix();
  for (int i=target+1;i<n;++i) {
    const TMatrixD next = chain * gains[i-1];
    chain.ResizeTo(next); chain = next;
    cross[i] = oldCov[i] * transpose(chain);
  }
  const auto& h = observation.derivative;
  const double innovation = (h * oldCov[target] * transpose(h))(0,0) + observation.noise(0,0);
  for (int i=0;i<n;++i) {
    LossTrackState state;
    state.mean.ResizeTo(oldMeans[i]); state.mean = oldMeans[i];
    state.covariance.ResizeTo(oldCov[i]); state.covariance = oldCov[i];
    GaussianHitModel marginal = observation;
    if (i!=target) {
      // Equivalent observation for this marginal; other-state uncertainty is
      // integrated into its noise. The update itself stays native KalTest.
      const TMatrixD hi = h * transpose(cross[i]) * inverseCovariance(oldCov[i]);
      marginal.derivative.ResizeTo(hi); marginal.derivative = hi;
      marginal.noise(0,0) = innovation - (hi * oldCov[i] * transpose(hi))(0,0);
      if (marginal.noise(0,0) < observation.noise(0,0)*(1.-1.e-5))
        throw std::runtime_error("Invalid delayed conditional observation covariance");
    }
    const auto updated = updateScalarState(state, marginal);
    means[i] = updated.mean; covariances[i] = updated.covariance;
  }
}

// Complete trajectory score also needs singular process-noise penalties.
// Normalize units before taking the supported pseudoinverse; never invent Q.
inline double supportedQuadratic(const TMatrixD& residual, const TMatrixD& covariance) {
  const int n = residual.GetNrows();
  Eigen::MatrixXd normalized = Eigen::MatrixXd::Zero(n,n);
  Eigen::VectorXd r = Eigen::VectorXd::Zero(n);
  for (int i=0;i<n;++i) {
    if (covariance(i,i)>0) r(i)=residual(i,0)/std::sqrt(covariance(i,i));
    else if (std::abs(residual(i,0))>1.e-8)
      throw std::runtime_error("Delayed state violates a deterministic transition");
    for (int j=0;j<n;++j)
      if (covariance(i,i)>0 && covariance(j,j)>0)
        normalized(i,j)=covariance(i,j)/std::sqrt(covariance(i,i)*covariance(j,j));
  }
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(normalized);
  if (eig.info()!=Eigen::Success || eig.eigenvalues().minCoeff() < -1.e-8)
    throw std::runtime_error("Invalid delayed process covariance");
  const Eigen::VectorXd u=eig.eigenvectors().transpose()*r;
  double score=0, unsupported=0;
  for (int i=0;i<n;++i) {
    if (eig.eigenvalues()(i)>1.e-12) score+=u(i)*u(i)/eig.eigenvalues()(i);
    else unsupported+=u(i)*u(i);
  }
  if (unsupported>1.e-5*std::max(1.,r.squaredNorm()))
    throw std::runtime_error("Delayed state leaves process-noise support");
  return score;
}
}
#endif
