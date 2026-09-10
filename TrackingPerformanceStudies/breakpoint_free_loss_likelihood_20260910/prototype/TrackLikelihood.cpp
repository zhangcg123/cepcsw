#include "TrackLikelihood.h"
#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>

namespace breakpoint_probe {
namespace {
using Matrix=Eigen::MatrixXd;
using Vector=Eigen::VectorXd;
Matrix eigen(const TMatrixD& value) {
  Matrix out(value.GetNrows(),value.GetNcols());
  for(int i=0;i<out.rows();++i) for(int j=0;j<out.cols();++j) out(i,j)=value(i,j);
  return out;
}

// A square root with only supported stochastic directions: no inverse of
// singular multiple-scattering Q, no artificial noise in deterministic modes.
Matrix rootPSD(const Matrix& value) {
  Matrix c=.5*(value+value.transpose());
  Vector scale=c.diagonal().cwiseMax(0).cwiseSqrt();
  Matrix normalized=Matrix::Zero(c.rows(),c.cols());
  for(int i=0;i<c.rows();++i) for(int j=0;j<c.cols();++j) {
    if(scale(i)>0 && scale(j)>0) normalized(i,j)=c(i,j)/(scale(i)*scale(j));
    else if(std::abs(c(i,j))>1.e-20) throw std::runtime_error("Unsupported PSD zero diagonal");
  }
  Eigen::SelfAdjointEigenSolver<Matrix> solver(normalized);
  if(solver.info()!=Eigen::Success || solver.eigenvalues().minCoeff() < -1.e-8)
    throw std::runtime_error("Likelihood noise is not PSD");
  int rank=0;
  for(int j=0;j<c.cols();++j) if(solver.eigenvalues()(j)>1.e-12) ++rank;
  Matrix root(c.rows(),rank);
  int k=0;
  for(int j=0;j<c.cols();++j) if(solver.eigenvalues()(j)>1.e-12)
    root.col(k++)=scale.asDiagonal()*solver.eigenvectors().col(j)*std::sqrt(solver.eigenvalues()(j));
  if((root*root.transpose()-c).norm()>1.e-8*std::max(c.norm(),1.e-25))
    throw std::runtime_error("Likelihood PSD root closure failure");
  return root;
}

// Factor the measurement covariance WITHOUT forming its potentially ill-
// conditioned normal matrix. QR([I,A]^T) gives C=R^T R, C=I+A A^T.
// Ascending/descending hit-row order implement the two conditional orders.
std::pair<double,double> orderedScore(const Matrix& a,const Vector& d) {
  const int n=a.rows();
  Matrix generator(n,n+a.cols());
  generator.leftCols(n).setIdentity(); generator.rightCols(a.cols())=a;
  Eigen::HouseholderQR<Matrix> qr(generator.transpose());
  Matrix r=qr.matrixQR().topLeftCorner(n,n).template triangularView<Eigen::Upper>();
  Vector innovation=r.transpose().template triangularView<Eigen::Lower>().solve(d);
  return {innovation.squaredNorm(),2*r.diagonal().array().abs().log().sum()};
}
}

LikelihoodScores likelihoodScores(const breakpoint::FitResult& fit) {
  const int n=fit.predicted.size();
  if(n==0 || int(fit.likelihoodMeasurements.size())!=n || int(fit.likelihoodTransport.size())!=n-1)
    throw std::runtime_error("Missing frozen likelihood model");
  std::vector<Matrix> roots;
  roots.push_back(rootPSD(eigen(fit.predicted.front().covariance)));
  int latent=roots.front().cols(), measurements=0;
  for(int i=1;i<n;++i) { roots.push_back(rootPSD(eigen(fit.likelihoodNoise[i-1]))); latent+=roots.back().cols(); }
  for(const auto& m:fit.likelihoodMeasurements) measurements+=m.residual.GetNrows();
  Matrix a=Matrix::Zero(measurements,latent), state=Matrix::Zero(5,latent);
  state.leftCols(roots[0].cols())=roots[0];
  Vector offset=Vector::Zero(5), d(measurements);
  int column=roots[0].cols(), row=0;
  double logdetV=0;
  for(int i=0;i<n;++i) {
    if(i) {
      const Matrix f=eigen(fit.likelihoodTransport[i-1]);
      // x_i = xpred_i + F_i (x_(i-1)-xfiltered_(i-1)) + w_i.
      offset=f*(offset+eigen(breakpoint::stateDifference(fit.predicted[i-1].mean,fit.filtered[i-1].mean)));
      state=(f*state).eval();
      state.middleCols(column,roots[i].cols())=roots[i];
      column+=roots[i].cols();
    }
    const auto& m=fit.likelihoodMeasurements[i];
    const Matrix h=eigen(m.derivative), v=eigen(m.noise);
    Eigen::LLT<Matrix> factor(v);
    if(factor.info()!=Eigen::Success) throw std::runtime_error("Non-positive hit noise");
    const Matrix l=factor.matrixL();
    const int size=h.rows();
    a.middleRows(row,size)=l.template triangularView<Eigen::Lower>().solve(h*state);
    d.segment(row,size)=l.template triangularView<Eigen::Lower>().solve(eigen(m.residual)-h*offset);
    logdetV+=2*l.diagonal().array().log().sum(); row+=size;
  }
  LikelihoodScores out;
  out.measurements=measurements; out.latentDimensions=latent;
  const auto forward=orderedScore(a,d);
  const auto backward=orderedScore(a.colwise().reverse().eval(),d.reverse().eval());
  out.forwardQuadratic=forward.first; out.backwardQuadratic=backward.first;
  out.forwardLogdet=forward.second+logdetV; out.backwardLogdet=backward.second+logdetV;

  // Joint trajectory smoothing in independent whitened seed/process variables u:
  // min_u ||d-Au||^2+||u||^2. SVD gives the joint posterior mean without
  // inverting Q. Marginalization adds logdet(I+A^T A), computed independently
  // here, NOT a sum of independent smoothed-residual likelihoods.
  Eigen::BDCSVD<Matrix> svd(a,Eigen::ComputeThinU|Eigen::ComputeThinV);
  if(svd.info()!=Eigen::Success) throw std::runtime_error("Likelihood smoothing SVD failed");
  const Vector singular=svd.singularValues();
  const Vector projection=svd.matrixU().transpose()*d;
  const Vector coefficients=(singular.array()/(1+singular.array().square())*projection.array()).matrix();
  const Vector u=svd.matrixV()*coefficients;
  const Vector residual=d-svd.matrixU()*(singular.array()*coefficients.array()).matrix();
  out.hitPenalty=residual.squaredNorm();
  out.seedPenalty=u.head(roots[0].cols()).squaredNorm();
  out.processPenalty=u.tail(u.size()-roots[0].cols()).squaredNorm();
  out.smoothedQuadratic=out.hitPenalty+out.seedPenalty+out.processPenalty;
  out.smoothedLogdet=(1+singular.array().square()).log().sum()+logdetV;
  const double normalization=measurements*std::log(2*std::acos(-1.));
  out.forward=out.forwardQuadratic+out.forwardLogdet+normalization;
  out.backward=out.backwardQuadratic+out.backwardLogdet+normalization;
  out.smoothed=out.smoothedQuadratic+out.smoothedLogdet+normalization;
  const double discrepancy=std::max(std::abs(out.forward-out.backward),std::abs(out.forward-out.smoothed));
  if(!std::isfinite(out.forward) || discrepancy>1.e-4)
    throw std::runtime_error("Three likelihood formulations disagree: "+std::to_string(discrepancy));
  return out;
}
}
