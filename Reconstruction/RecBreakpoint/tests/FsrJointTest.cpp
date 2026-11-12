#include "BeamSpotFsrFitter.h"
#include "TrackState.h"
#include <Eigen/Dense>
#include <cassert>
#include <cmath>
#include <iostream>
using namespace breakpoint;
using E=Eigen::MatrixXd;
TMatrixD root(const E& x){TMatrixD y(x.rows(),x.cols());for(int i=0;i<x.rows();++i)for(int j=0;j<x.cols();++j)y(i,j)=x(i,j);return y;}
E eigen(const TMatrixD& x){E y(x.GetNrows(),x.GetNcols());for(int i=0;i<y.rows();++i)for(int j=0;j<y.cols();++j)y(i,j)=x(i,j);return y;}
int main(){
 E pt=E::Identity(6,6);pt(1,1)=2.;pt(2,2)=.001;pt(4,4)=.5;pt(5,5)=.8;
 E g=E::Zero(5,6);g.block(0,0,5,5)=.7*E::Identity(5,5);g(0,5)=.15;
 E p0=g*pt*g.transpose()+.1*E::Identity(5,5),cross=g*pt;
 E prior=E::Zero(11,11);prior.block(0,0,5,5)=p0;prior.block(5,5,6,6)=pt;prior.block(0,5,5,6)=cross;prior.block(5,0,6,5)=cross.transpose();
 E h=E::Zero(2,6);h(0,0)=1.;h(0,5)=.2;h(0,4)=.05;h(1,0)=.3;h(1,5)=1.;h(1,4)=-.1;
 E noise=E::Zero(2,2);noise(0,0)=.3;noise(1,1)=.4;
 E s=h*pt*h.transpose()+noise;int count=0,seen=0;
 for(double a:{-2.,-.1,.5,2.})for(double b:{-2.,-.1,.5,2.}){
  E r(2,1);r<<a,b;auto profile=profileFsr(root(r),root(s));seen|=1<<profile.activeMask;
  E gradient=-2.*s.inverse()*(r-eigen(profile.energy));
  for(int i=0;i<2;++i){assert(profile.energy(i,0)>=0);if(profile.activeMask&(1<<i))assert(gradient(i,0)>-1.e-9);else assert(std::abs(gradient(i,0))<1.e-9);}
  int free=0;for(int i=0;i<2;++i)if(!(profile.activeMask&(1<<i)))++free;
  E design=E::Zero(2,11+free);design.block(0,5,2,6)=h;int slot=11;
  for(int i=0;i<2;++i)if(!(profile.activeMask&(1<<i)))design(i,slot++)=1.;
  E information=design.transpose()*noise.inverse()*design;information.block(0,0,11,11)+=prior.inverse();
  E cov=information.ldlt().solve(E::Identity(11+free,11+free));E mean=cov*design.transpose()*noise.inverse()*r;
  std::vector<TMatrixD> means{TMatrixD(5,1),TMatrixD(6,1)},covs{root(p0),root(pt)},gains{root(g)};
  means[0].Zero();means[1].Zero();means[0](2,0)=means[1](2,0)=10.;
  auto changed=conditionFsrTrajectory(means,covs,gains,root(h),root(noise),root(r),profile.activeMask);
  E delta0=eigen(means[0]),deltat=eigen(means[1]);delta0(2,0)-=10.;deltat(2,0)-=10.;
  assert((delta0-mean.block(0,0,5,1)).norm()<1.e-9);assert((deltat-mean.block(5,0,6,1)).norm()<1.e-9);
  assert((eigen(covs[0])-cov.block(0,0,5,5)).norm()<1.e-9);assert((eigen(covs[1])-cov.block(5,5,6,6)).norm()<1.e-9);assert((eigen(changed)-cov.block(0,5,5,6)).norm()<1.e-9);
  slot=11;for(int i=0;i<2;++i)if(!(profile.activeMask&(1<<i))){assert(std::abs(mean(slot,0)-profile.energy(i,0))<1.e-9);assert((cov.block(0,slot,5,1)+eigen(changed)*h.row(i).transpose()).norm()<1.e-9);assert(std::abs(cov(slot,slot)-(h.row(i)*cov.block(5,5,6,6)*h.row(i).transpose())(0,0)-noise(i,i))<1.e-9);++slot;}
  E x=mean.block(0,0,11,1),residual=r-design*mean;double q=(x.transpose()*prior.inverse()*x+residual.transpose()*noise.inverse()*residual)(0,0);assert(std::abs(q-profile.quadratic)<1.e-9);++count;
 }

 assert(seen==15);

 E lower=E::Identity(6,6);lower(5,2)=-.6;lower(5,1)=.2;lower(4,2)=.1;
 E scales=E::Zero(6,6);scales.diagonal()<<.01,.001,.0001,.02,.005,.5;
 E inputCov=scales*lower*lower.transpose()*scales;
 auto independent=[](const E& x) {
  const double pt=1./std::abs(x(2,0)),phi=x(1,0)+std::acos(-1.)/2.;
  E y(8,1);y(1,0)=pt*std::cos(phi);y(2,0)=pt*std::sin(phi);y(3,0)=pt*x(4,0);
  const double momentum=std::sqrt(y(1,0)*y(1,0)+y(2,0)*y(2,0)+y(3,0)*y(3,0));
  y(0,0)=std::sqrt(momentum*momentum+.00051099895*.00051099895);
  y(4,0)=x(5,0);for(int i=1;i<4;++i)y(4+i,0)=x(5,0)*y(i,0)/momentum;
  return y;
 };
 for(double kappa:{-.02,.02})for(double tanl:{-4.,.4,4.})for(double photon:{0.,.5,10.,50.}) {
  TrackState electron;electron.mean.Zero();electron.mean(1,0)=kappa>0 ? std::acos(-1.)-.001 : -std::acos(-1.)+.001;
  electron.mean(2,0)=kappa;electron.mean(4,0)=tanl;
  E x=E::Zero(6,1);for(int i=0;i<5;++i)x(i,0)=electron.mean(i,0);x(5,0)=photon;
  auto actual=electronAndFsrP4(electron,photon,root(inputCov));E value(8,1),covariance(8,8);
  for(int i=0;i<4;++i){value(i,0)=actual.electron[i];value(4+i,0)=actual.photon[i];}
  for(int i=0;i<8;++i)for(int j=0;j<8;++j)covariance(i,j)=actual.covariance[8*i+j];
  assert((value-independent(x)).norm()<1.e-10);
  assert(std::abs(value(4,0)*value(4,0)-value.block(5,0,3,1).squaredNorm())<1.e-9);
  assert(std::abs(value(0,0)*value(0,0)-value.block(1,0,3,1).squaredNorm()-.00051099895*.00051099895)<1.e-9);
  E numeric=E::Zero(8,6);
  for(int j=0;j<6;++j){E plus=x,minus=x;const double step=j==2 ? 1.e-7 : 1.e-5;plus(j,0)+=step;minus(j,0)-=step;numeric.col(j)=(independent(plus)-independent(minus))/(2*step);}
  const E expected=numeric*inputCov*numeric.transpose();
  assert((covariance-expected).norm()<2.e-6*std::max(1.,expected.norm()));
  assert((covariance-covariance.transpose()).norm()<1.e-12);
  assert(Eigen::SelfAdjointEigenSolver<E>(covariance).eigenvalues().minCoeff()>-1.e-8);
  assert(std::abs(covariance(4,4)-inputCov(5,5))<1.e-12);
  E sum=E::Zero(4,8);sum.block(0,0,4,4).setIdentity();sum.block(0,4,4,4).setIdentity();
  assert((sum*covariance*sum.transpose()-sum*expected*sum.transpose()).norm()<2.e-6*std::max(1.,expected.norm()));
  assert(covariance.block(0,4,4,4).norm()>1.e-6);++count;
 }
 TrackState electron;electron.mean.Zero();electron.mean(2,0)=.02;electron.mean(4,0)=.4;
 E fixed=inputCov;fixed.row(5).setZero();fixed.col(5).setZero();
 auto zero=electronAndFsrP4(electron,0.,root(fixed));
 for(int i=0;i<4;++i)assert(zero.photon[i]==0.);
 for(int i=4;i<8;++i)for(int j=0;j<8;++j)assert(zero.covariance[8*i+j]==0.);++count;
 try{electronAndFsrP4(electron,-1.,root(inputCov));assert(false);}catch(const std::invalid_argument&){++count;}
 try{electronAndFsrP4(electron,1.,root(E::Identity(5,5)));assert(false);}catch(const std::invalid_argument&){++count;}

 for(double r:{-3.,0.2,4.}){TMatrixD rr(1,1),ss(1,1);rr(0,0)=r;ss(0,0)=2.;auto p=profileFsr(rr,ss);assert(std::abs(p.energy(0,0)-std::max(0.,r))<1.e-12);assert(std::abs(p.quadratic-std::pow(std::min(0.,r),2)/2)<1.e-12);++count;}
 std::cout<<"PASS "<<count<<" dense independent joint-Gaussian tests: profile bounds, all active sets, native full-trajectory means/covariances, rectangular5->6 crosscov, photon-track covariance, likelihood closure, massless photon p4 and finite-difference joint-p4 covariance\n";
}
