#include "PhotonCaloProjection.h"
#include "kaltest/TCylinder.h"
#include "kaltest/TPlane.h"
#include "TDecompChol.h"
#include "TMatrixDSym.h"
#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>

namespace {
void require(bool ok, const char* message) {
  if (!ok) throw std::runtime_error(message);
}
}

int main() {
  using namespace breakpoint;
  TrackState source;
  source.pivot={100,0,20};
  source.mean.Zero(); source.mean(1,0)=-std::acos(-1.)/2;
  source.mean(2,0)=.05; source.mean(4,0)=.2;
  source.covariance.Zero();
  const double sigma[]={.01,.0001,.00001,.02,.0001};
  for (int i=0;i<5;++i) source.covariance(i,i)=sigma[i]*sigma[i];
  source.covariance(0,1)=source.covariance(1,0)=.3*sigma[0]*sigma[1];
  source.covariance(3,4)=source.covariance(4,3)=-.4*sigma[3]*sigma[4];
  source.covariance(1,4)=source.covariance(4,1)=.25*sigma[1]*sigma[4];
  const auto unchanged=source.mean;
  TCylinder barrel(1830,3000);
  TPlane endcap(TVector3(0,0,2500),TVector3(0,0,1));
  const std::vector<const TVSurface*> faces{&barrel,&endcap};
  const auto nominal=projectPhotonToCalorimeter(source,faces);
  require(nominal.surface==0,"Wrong first forward surface");
  require((nominal.position-TVector3(1830,0,366)).Mag()<1.e-5,"Wrong ray intersection");
  require((source.mean-unchanged).E2Norm()==0,"Source state mutated");
  for (int j=0;j<5;++j) {
    auto plus=source,minus=source;
    const double step=j==0 || j==3 ? 1.e-4 : 1.e-6;
    plus.mean(j,0)+=step;minus.mean(j,0)-=step;
    const auto p=projectPhotonToCalorimeter(plus,faces).position;
    const auto m=projectPhotonToCalorimeter(minus,faces).position;
    for (int i=0;i<3;++i)
      require(std::abs((p[i]-m[i])/(2*step)-nominal.derivative(i,j))<1.e-3,
              "Surface-aware Jacobian disagrees with finite differences");
  }
  // Validate the full correlated covariance independently with sampled rays.
  TMatrixDSym covariance(5);
  for(int i=0;i<5;++i) for(int j=0;j<5;++j) covariance(i,j)=source.covariance(i,j);
  TDecompChol chol(covariance);
  require(chol.Decompose(),"Test covariance is not positive definite");
  const TMatrixD lower(TMatrixD::kTransposed,chol.GetU());
  std::mt19937 generator(731);
  std::normal_distribution<double> normal;
  TMatrixD second(2,2),sum(2,1);
  second.Zero();sum.Zero();
  constexpr int samples=6000;
  for(int n=0;n<samples;++n) {
    TMatrixD random(5,1);
    for(int i=0;i<5;++i) random(i,0)=normal(generator);
    auto sampled=source;sampled.mean+=lower*random;
    const auto projected=projectPhotonToCalorimeter(sampled,faces);
    TMatrixD delta(2,1);
    delta(0,0)=projected.position.Z()-nominal.position.Z();
    delta(1,0)=std::remainder(projected.position.Phi()-nominal.position.Phi(),2*std::acos(-1.));
    sum+=delta;second+=delta*TMatrixD(TMatrixD::kTransposed,delta);
  }
  const TMatrixD empirical=(second-sum*TMatrixD(TMatrixD::kTransposed,sum)*(1./samples))*(1./(samples-1));
  for(int i=0;i<2;++i)
    require(std::abs(empirical(i,i)/nominal.zPhiCovariance(i,i)-1)<.06,
            "Sampled ray covariance disagrees with projected covariance");
  const double correlationScale=std::sqrt(nominal.zPhiCovariance(0,0)*nominal.zPhiCovariance(1,1));
  require(std::abs(empirical(0,1)-nominal.zPhiCovariance(0,1))<.05*correlationScale,
          "Sampled ray z/phi correlation disagrees with projected covariance");
  bool failed=false;
  try { projectPhotonToCalorimeter(source,{}); }
  catch(const std::runtime_error&) { failed=true; }
  require(failed,"Missing geometry did not fail explicitly");
  source.mean(4,0)=4.;
  require(projectPhotonToCalorimeter(source,faces).surface==1,"Forward endcap not selected");
  std::cout<<"PASS: native ray geometry, analytic Jacobian, correlated covariance, "
             "6000 sampled rays, missing surfaces and endcap selection\n";
}
