#include "TrackLikelihood.h"
#include <Eigen/Dense>
#include <iostream>
#include <cmath>

int main() {
  // Three repeated 2D measurements of a 5D state. Unobserved/deterministic
  // directions and singular Q are intentional. Independent dense-C reference.
  for(double noise:{0.,.1}) {
    breakpoint::FitResult fit;
    Eigen::MatrixXd covariance=Eigen::MatrixXd::Zero(6,6);
    Eigen::VectorXd residual(6);
    for(int i=0;i<3;++i) {
      breakpoint::TrackState state;
      state.mean.Zero(); state.covariance.UnitMatrix();
      fit.predicted.push_back(state); fit.filtered.push_back(state);
      TMatrixD h(2,5),v(2,2),r(2,1); h.Zero();v.Zero();
      h(0,0)=1;h(1,2)=1;v(0,0)=.2;v(1,1)=.3;v(0,1)=v(1,0)=.02;
      r(0,0)=.3+i*.2;r(1,0)=-.4+i*.1;
      fit.likelihoodMeasurements.push_back({h,v,r});
      if(i) {TMatrixD f(5,5),q(5,5);f.UnitMatrix();q.Zero();q(0,0)=noise;
        fit.likelihoodTransport.push_back(f);fit.likelihoodNoise.push_back(q);}
      residual(2*i)=r(0,0);residual(2*i+1)=r(1,0);
      for(int j=0;j<3;++j) {
        covariance(2*i,2*j)=1+std::min(i,j)*noise;
        covariance(2*i+1,2*j+1)=1;
        if(i==j) {covariance(2*i,2*j)+=.2;covariance(2*i+1,2*j+1)+=.3;
          covariance(2*i,2*j+1)=covariance(2*i+1,2*j)=.02;}
      }
    }
    Eigen::LLT<Eigen::MatrixXd> llt(covariance);
    Eigen::MatrixXd l=llt.matrixL();
    const double expected=residual.dot(llt.solve(residual))+2*l.diagonal().array().log().sum()+6*std::log(2*std::acos(-1.));
    const auto result=breakpoint_probe::likelihoodScores(fit);
    for(double score:{result.forward,result.backward,result.smoothed})
      if(std::abs(score-expected)>1.e-10) throw std::runtime_error("Dense Gaussian reference mismatch");
    std::cout << "Q=" << noise << " analytic dense reference and three likelihoods agree\n";
  }
}
