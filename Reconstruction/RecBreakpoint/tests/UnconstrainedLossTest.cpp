#include "UnconstrainedLoss.h"
#include <Eigen/Dense>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
TMatrixD root(const Eigen::MatrixXd& value) {
  TMatrixD result(value.rows(),value.cols());
  for(int i=0;i<value.rows();++i) for(int j=0;j<value.cols();++j) result(i,j)=value(i,j);
  return result;
}
void near(double a,double b) {
  if(!std::isfinite(a) || !std::isfinite(b) || std::abs(a-b)>2.e-9*(1.+std::abs(b)))
    throw std::runtime_error("Unconstrained/dense joint regression mismatch");
}
}

int main() {
  // Independent joint batch least squares in seed + process kicks + free b.
  // Compare every smoothed mean, covariance and Cov(state,b), not just b.
  for(double q:{0.,.05}) for(double reference:{0.,.3}) {
    constexpr int n=6,latent=5+5*(n-1)+1;
    using V=Eigen::Matrix<double,5,1>;using M=Eigen::Matrix<double,5,5>;
    M f=M::Identity();f(0,2)=.4;f(1,3)=.2;f(2,2)=.9;
    Eigen::Matrix<double,2,5> h;h.setZero();h(0,0)=1;h(1,1)=1;h(1,2)=.2;
    Eigen::Matrix2d v;v<<.2,.01,.01,.3;
    V g=V::Zero();g(2)=.7;
    V mean=V::Zero(),response=V::Zero(),direct=V::Zero();M p=M::Identity();
    std::vector<V> xp,xf,ap,af;std::vector<M> pp,pf;
    std::vector<Eigen::MatrixXd> design;
    Eigen::MatrixXd d=Eigen::MatrixXd::Zero(5,latent);d.leftCols(5).setIdentity();
    Eigen::MatrixXd normal=Eigen::MatrixXd::Identity(latent,latent);normal(latent-1,latent-1)=0;
    Eigen::VectorXd rhs=Eigen::VectorXd::Zero(latent);
    breakpoint::UnconstrainedLoss regression;
    double total=0;
    for(int i=0;i<n;++i) {
      if(i) {
        mean=f*mean;response=f*response;direct=f*direct;
        p=f*p*f.transpose()+q*M::Identity();d=f*d;
        d.block<5,5>(0,5+5*(i-1))=std::sqrt(q)*M::Identity();
        if(i==2) {response+=g;direct+=g;d.col(latent-1)+=g;}
      }
      Eigen::Vector2d y;y<<.1+.15*i,-.2+.04*i*i;
      const Eigen::Vector2d yReference=y-h*direct*reference;
      const Eigen::Vector2d residual=yReference-h*mean;
      const Eigen::Matrix2d s=h*p*h.transpose()+v;
      total+=regression.add({root(h),root(v),root(residual)},root(p),root(response));
      xp.push_back(mean);pp.push_back(p);ap.push_back(response);
      const Eigen::Matrix<double,5,2> k=p*h.transpose()*s.inverse();
      mean+=k*residual;response=(M::Identity()-k*h)*response;
      p=(M::Identity()-k*h)*p;
      xf.push_back(mean);pf.push_back(p);af.push_back(response);design.push_back(d);
      const Eigen::MatrixXd hd=h*d;
      normal+=hd.transpose()*v.inverse()*hd;rhs+=hd.transpose()*v.inverse()*y;
    }
    const Eigen::MatrixXd jointCov=normal.ldlt().solve(Eigen::MatrixXd::Identity(latent,latent));
    const Eigen::VectorXd jointMean=normal.ldlt().solve(rhs);
    near(reference+regression.shift(),jointMean(latent-1));
    near(regression.variance(),jointCov(latent-1,latent-1));
    near(total,regression.quadratic());
    auto xs=xf,as=af;auto ps=pf;
    for(int i=n-2;i>=0;--i) {
      const M gain=pf[i]*f.transpose()*pp[i+1].inverse();
      xs[i]=xf[i]+gain*(xs[i+1]-xp[i+1]);
      as[i]=af[i]+gain*(as[i+1]-ap[i+1]);
      ps[i]=pf[i]+gain*(ps[i+1]-pp[i+1])*gain.transpose();
    }
    direct.setZero();
    for(int i=0;i<n;++i) {
      if(i) direct=f*direct;if(i==2) direct+=g;
      const V actualMean=xs[i]+direct*reference+as[i]*regression.shift();
      const M actualCov=ps[i]+as[i]*as[i].transpose()*regression.variance();
      const V actualCross=as[i]*regression.variance();
      const V expectedMean=design[i]*jointMean;
      const M expectedCov=design[i]*jointCov*design[i].transpose();
      const V expectedCross=design[i]*jointCov.col(latent-1);
      for(int j=0;j<5;++j) {
        near(actualMean(j),expectedMean(j));near(actualCross(j),expectedCross(j));
        for(int k=0;k<5;++k) near(actualCov(j,k),expectedCov(j,k));
      }
    }
    std::cout<<"Q="<<q<<" reference="<<reference<<": b, variance, all RTS states/covariances/cross-covariances passed\n";
  }
  breakpoint::UnconstrainedLoss unidentified;
  TMatrixD h(1,5),v(1,1),r(1,1),p(5,5),a(5,1);
  h(0,0)=1;v(0,0)=1;r(0,0)=.3;p.UnitMatrix();
  unidentified.add({h,v,r},p,a);
  if(unidentified.identified()) throw std::runtime_error("Zero response was called identifiable");
  bool rejected=false;try{unidentified.variance();}catch(const std::runtime_error&){rejected=true;}
  if(!rejected) throw std::runtime_error("Diffuse unidentifiable variance was not rejected");
  std::cout<<"Unidentifiable loss rejected, no invented finite covariance\n";
}
