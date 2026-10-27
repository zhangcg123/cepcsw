#include "DelayedNeutralConditioning.h"
#include "TrackLikelihood.h"
#include <cassert>
#include <iostream>

using namespace breakpoint;
using E=Eigen::MatrixXd;
E eigen(const TMatrixD& x) { E y(x.GetNrows(),x.GetNcols()); for(int i=0;i<y.rows();++i) for(int j=0;j<y.cols();++j)y(i,j)=x(i,j);return y; }
int main() {
  const std::vector<int> dim{5,6,6,6}, offset{0,5,11,17};
  std::vector<TMatrixD> f,q,h,pm,pc,fm,fc,g;
  TMatrixD seed(5,1),seedCov(5,5);seed.Zero();seedCov.Zero();seed(2,0)=-.2;seed(4,0)=.4;
  for(int j=0;j<5;++j)seedCov(j,j)=.001*(j+1);
  for(int i=1;i<4;++i){
    TMatrixD a(dim[i],dim[i-1]);a.Zero(); for(int j=0;j<dim[i-1];++j)a(j,j)=1;
    if(i==1)a(2,2)=1.1; else a(0,2)=.2;
    f.push_back(a);
    TMatrixD noise(dim[i],dim[i]);noise.Zero();
    if(i==1){TMatrixD v(6,1);v.Zero();v(2,0)=-.22;v(5,0)=1;noise=.0001*v*transpose(v);}
    else {noise(0,0)=1.e-5;noise(2,2)=1.e-6;noise(4,4)=2.e-6;}
    q.push_back(noise);
  }
  E jointCov=E::Zero(23,23),jointMean=E::Zero(23,1);
  jointCov.block(0,0,5,5)=eigen(seedCov);jointMean.block(0,0,5,1)=eigen(seed);
  for(int i=1;i<4;++i){
    const E a=eigen(f[i-1]);const int o=offset[i],prev=offset[i-1];
    jointCov.block(o,0,dim[i],o)=a*jointCov.block(prev,0,dim[i-1],o);
    jointCov.block(0,o,o,dim[i])=jointCov.block(o,0,dim[i],o).transpose();
    jointCov.block(o,o,dim[i],dim[i])=a*jointCov.block(prev,prev,dim[i-1],dim[i-1])*a.transpose()+eigen(q[i-1]);
    jointMean.block(o,0,dim[i],1)=a*jointMean.block(prev,0,dim[i-1],1);
    if(i==1)jointMean(o+5,0)=.1;
  }
  const E priorCov=jointCov,priorMean=jointMean;
  std::vector<double> y{.02,.11,-.215,-.19},variance{.0004,.00009,.0001,.0002};
  for(int i=0;i<4;++i){TMatrixD hi(1,dim[i]);hi.Zero();hi(0,i==0?0:2)=1;
    if(i==1){hi(0,2)=.05;hi(0,4)=.01;hi(0,5)=1;}
    if(i==3)hi(0,5)=.2;
    h.push_back(hi);
  }
  auto conditionFull=[&](int i){
    E hi=E::Zero(1,23);hi.block(0,offset[i],1,dim[i])=eigen(h[i]);
    E cross=jointCov*hi.transpose();double s=(hi*cross)(0,0)+variance[i];
    jointMean+=cross*((y[i]-(hi*jointMean)(0,0))/s);
    jointCov-=cross*cross.transpose()/s;
  };
  for(int i:{0,2,3})conditionFull(i);
  for(int i=0;i<4;++i){
    LossTrackState x;
    if(!i){x.mean.ResizeTo(seed);x.mean=seed;x.covariance.ResizeTo(seedCov);x.covariance=seedCov;}
    else {const auto m=f[i-1]*fm.back();x.mean.ResizeTo(m);x.mean=m;if(i==1)x.mean(5,0)=.1;
      x.covariance=f[i-1]*fc.back()*transpose(f[i-1])+q[i-1];}
    pm.push_back(x.mean);pc.push_back(x.covariance);
    if(i!=1){GaussianHitModel obs{h[i],TMatrixD(1,1),TMatrixD(1,1)};obs.noise(0,0)=variance[i];obs.residual(0,0)=y[i]-(h[i]*x.mean)(0,0);x=updateScalarState(x,obs);}
    fm.push_back(x.mean);fc.push_back(x.covariance);
  }
  auto sm=fm,sc=fc;
  for(int i=0;i<3;++i)g.push_back(fc[i]*transpose(f[i])*inverseCovariance(pc[i+1]));
  for(int i=2;i>=0;--i){sm[i]=fm[i]+g[i]*(sm[i+1]-pm[i+1]);sc[i]=fc[i]+g[i]*(sc[i+1]-pc[i+1])*transpose(g[i]);}
  for(int i=0;i<4;++i){assert((eigen(sm[i])-jointMean.block(offset[i],0,dim[i],1)).norm()<1.e-10);assert((eigen(sc[i])-jointCov.block(offset[i],offset[i],dim[i],dim[i])).norm()<1.e-10);}
  GaussianHitModel neutral{h[1],TMatrixD(1,1),TMatrixD(1,1)};
  neutral.noise(0,0)=variance[1];neutral.residual(0,0)=y[1]-(h[1]*sm[1])(0,0);
  conditionTrajectory(sm,sc,g,1,neutral);conditionFull(1);
  for(int i=0;i<4;++i){assert((eigen(sm[i])-jointMean.block(offset[i],0,dim[i],1)).norm()<1.e-10);assert((eigen(sc[i])-jointCov.block(offset[i],offset[i],dim[i],dim[i])).norm()<1.e-10);}
  GaussianTrackModel model;model.seedCovariance.ResizeTo(seedCov);model.seedCovariance=seedCov;
  double score=supportedQuadratic(sm[0]-pm[0],seedCov);
  E H=E::Zero(4,23),Y(4,1),V=E::Zero(4,4);
  for(int i=0;i<4;++i){GaussianHitModel obs{h[i],TMatrixD(1,1),TMatrixD(1,1)};
    obs.noise(0,0)=variance[i];obs.residual(0,0)=y[i]-(h[i]*pm[i])(0,0);model.hits.push_back(obs);
    score+=supportedQuadratic(obs.residual-h[i]*(sm[i]-pm[i]),obs.noise);
    if(i){model.transitions.push_back({f[i-1],q[i-1],pm[i-1]-fm[i-1]});score+=supportedQuadratic(sm[i]-pm[i]-f[i-1]*(sm[i-1]-fm[i-1]),q[i-1]);}
    H.block(i,offset[i],1,dim[i])=eigen(h[i]);Y(i,0)=y[i];V(i,i)=variance[i];
  }
  const E S=H*priorCov*H.transpose()+V, residual=Y-H*priorMean;
  Eigen::LLT<E> llt(S);const double dense=(residual.transpose()*llt.solve(residual))(0,0);
  const auto likelihood=evaluateTrackLikelihood(model);
  assert(std::abs(score-dense)<1.e-8);
  assert(std::abs(likelihood.quadratic-dense)<1.e-8);
  std::cout<<"PASS: delayed native update equals dense joint conditioning at ALL nodes, covariances, rectangular birth, singular Q, complete trajectory score and likelihood"<<std::endl;
}
