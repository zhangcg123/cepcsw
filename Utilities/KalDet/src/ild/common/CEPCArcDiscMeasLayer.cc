
#include <iostream>

#include "kaldet/CEPCArcDiscMeasLayer.h"
#include "kaldet/ILDPlanarHit.h"

#include "kaltest/TVTrack.h"
#include "TVector3.h"
#include "TMath.h"
#include "TRotMatrix.h"
#include "TBRIK.h"
#include "TNode.h"
#include "TString.h"

#include "edm4hep/EDM4hepVersion.h"
#include <edm4hep/TrackerHit.h>

#include "gearimpl/Vector3D.h"

#include "DetIdentifier/CEPCConf.h"
#include <bitset>
// #include "streamlog/streamlog.h"


TKalMatrix CEPCArcDiscMeasLayer::XvToMv(const TVector3 &xv) const
{
  
  // Calculate measurement vector (hit coordinates) from global coordinates:
  
  TKalMatrix mv(kMdim,1);
  
  mv(0,0)  = xv.X() ;
  
  
  mv(1,0)  = xv.Y() ;
  return mv;
  
}


TVector3 CEPCArcDiscMeasLayer::HitToXv(const TVTrackHit &vht) const
{
  //  const ILDPlanarHit &mv = dynamic_cast<const ILDPlanarHit &>(vht);
  
  double x =   vht(0,0) ;
  double y =   vht(1,0) ;
  
  double z = GetXc().Z() ;
  
  return TVector3(x,y,z);
}

void CEPCArcDiscMeasLayer::CalcDhDa(const TVTrackHit &vht,
                                const TVector3   &xxv,
                                const TKalMatrix &dxphiada,
                                TKalMatrix &H)  const
{
  // Calculate
  //    H = (@h/@a) = (@phi/@a, @z/@a)^t
  // where
  //        h(a) = (phi, z)^t: expected meas vector
  //        a = (drho, phi0, kappa, dz, tanl, t0)
  //
  
  Int_t sdim = H.GetNcols();
  Int_t hdim = TMath::Max(5,sdim-1);
  
  // Set H = (@h/@a) = (@d/@a, @z/@a)^t
  
  for (Int_t i=0; i<hdim; i++) {
    
    H(0,i) = dxphiada(0,i);
    H(1,i) = dxphiada(1,i) ;
    
  }
  if (sdim == 6) {
    H(0,sdim-1) = 0.;
    H(1,sdim-1) = 0.;
  }
  
}

Int_t CEPCArcDiscMeasLayer::CalcXingPointWith(const TVTrack  &hel,
                                          TVector3 &xx,
                                          Double_t &phi,
                                          Int_t     mode,
                                          Double_t  eps) const{
    
  phi = 0.0;
  
  xx.SetX(0.0);
  xx.SetY(0.0);
  xx.SetZ(0.0);

  
  // check that direction has one of the correct values
  if( !( mode == 0 || mode == 1 || mode == -1) ) return -1 ;
  
  // get helix parameters
  Double_t dr     = hel.GetDrho();
  Double_t phi0   = hel.GetPhi0();  //
  Double_t kappa  = hel.GetKappa();
  Double_t rho    = hel.GetRho();
  Double_t omega  = 1.0 / rho;
  Double_t z0     = hel.GetDz();
  Double_t tanl   = hel.GetTanLambda();
  
  TVector3 ref_point = hel.GetPivot();
  
  
  //
  // Check if charge is nonzero.
  //
  
  Int_t    chg = (Int_t)TMath::Sign(1.1,kappa);
  if (!chg) {
    // streamlog_out(ERROR) << ">>>> Error >>>> CEPCArcDiscMeasLayer::CalcXingPointWith" << std::endl
    // << "      Kappa = 0 is invalid for a helix "          << std::endl;
    return -1;
  }
  
  const double sin_phi0 = sin(phi0); 
  const double cos_phi0 = cos(phi0); 
  
  const double x_pca = ref_point.x() + dr * cos_phi0 ; 
  const double y_pca = ref_point.y() + dr * sin_phi0 ; 
  const double z_pca = ref_point.z() + z0 ;
  
  const double z = this->GetXc().Z() ;
  // get path length to crossing point 
  
  const double s = ( z - z_pca ) / tanl ;
  
//  streamlog_out(DEBUG0) << "CEPCArcDiscMeasLayer::CalcXingPointWith "
//  << " ref_point.z()  = " << ref_point.z()
//  << " z = " << z
//  << " z0  = " << z0
//  << " z_pca  = " << z_pca
//  << " tanl  = " << tanl
//  << " z - z_pca  = " << z - z_pca
//  << std::endl;
  
//  TVector3 xx_n;
//  int cuts = TVSurface::CalcXingPointWith(hel, xx_n, phi, 0, eps);
//  streamlog_out(DEBUG0) << "CEPCArcDiscMeasLayer::CalcXingPointWith from Newton: cuts = " << cuts << " x = " << xx_n.x() << " y = "<< xx_n.y() << " z = " << xx_n.z() << " r = " << xx_n.Perp() << " phi = " << xx_n.Phi() << " dphi = " <<  phi << std::endl;

  
  phi = -omega * s;
  
  const double delta_phi_half = -phi/2.0 ;
  
  
  double x;
  double y;
  
  if( fabs(s) > FLT_MIN ){ // protect against starting on the plane

    x = x_pca - s * ( sin(delta_phi_half) / delta_phi_half ) *  sin( phi0 - delta_phi_half ) ;
    
    y = y_pca + s * ( sin(delta_phi_half) / delta_phi_half ) *  cos( phi0 - delta_phi_half ) ;

  }
  else{
    // streamlog_out(DEBUG0) << "CEPCArcDiscMeasLayer::CalcXingPointWith Using PCA values " << std::endl;
    x = x_pca;
    y = y_pca;
    phi = 0;
  }
  
  
  // check if intersection with plane is within boundaries
  
  xx.SetXYZ(x, y, z);
  
  
  // streamlog_out(DEBUG0) << "CEPCArcDiscMeasLayer::CalcXingPointWith            : cuts = " << (IsOnSurface(xx) ? 1 : 0) << " x = " << xx.x() << " y = "<< xx.y() << " z = " << xx.z() << " r = " << xx.Perp() << " phi = " << xx.Phi() << " dphi = " <<  phi << " s = " << s << " " << this->TVMeasLayer::GetName() << std::endl;  

  if( mode!=0 && fabs(phi)>1.e-10 ){ // (+1,-1) = (fwd,bwd)
    if( chg*phi*mode > 0){
      return 0;
    }
  }
  
  return (IsOnSurface(xx) ? 1 : 0);  
  
}


Bool_t CEPCArcDiscMeasLayer::IsOnSurface(const TVector3 &xx) const
{
    
  bool onSurface = false ;
  
  TKalMatrix mv = XvToMv(xx);
  
  // check whether the hit lies in the same plane as the surface
  if (TMath::Abs((xx.X()-GetXc().X())*GetNormal().X() + (xx.Y()-GetXc().Y())*GetNormal().Y() + (xx.Z()-GetXc().Z())*GetNormal().Z()) < 1e-4) {
    // check whether the hit lies within the boundary of the surface 
    
    double r2 = mv(0,0) * mv(0,0) + mv(1,0) * mv(1,0) ;
    
    if (r2 <= _rMax*_rMax && r2 >= _rMin*_rMin) {
      onSurface = true ;
    }
    else {
      std::cout << "r2: " << r2 << " r2min: " << _rMin*_rMin << " r2max: " << _rMax*_rMax << std::endl;
    }
  }
  else {
    std::cout << "Xc: " << GetXc().X() << " " << GetXc().Y() << " " << GetXc().Z() << " Normal: " << GetNormal().X() << " " << GetNormal().Y() << " " << GetNormal().Z() << std::endl;
  }
  
  return onSurface;
  
}


ILDVTrackHit* CEPCArcDiscMeasLayer::ConvertLCIOTrkHit(edm4hep::TrackerHit trkhit) const {
  
  //edm4hep::TrackerHitPlane* plane_hit = dynamic_cast<EVENT::TrackerHitPlane*>( trkhit ) ;
  //edm4hep::TrackerHitPlane* plane_hit = trkhit;
  std::cout << "CEPCArcDiscMeasLayer::ConvertLCIOTrkHit type = " << trkhit.getType() << std::endl;
  std::bitset<32> type(trkhit.getType());
  //if (!type[CEPCConf::TrkHitTypeBit::PLANAR]) return NULL;
  
  //edm4hep::TrackerHit plane_hit = trkhit;
  //if( plane_hit == NULL )  return NULL; // SJA:FIXME: should be replaced with an exception  
  
  //gear::Vector3D U(1.0,plane_hit.getU()[1],plane_hit.getU()[0],gear::Vector3D::spherical);
  //gear::Vector3D V(1.0,plane_hit.getV()[1],plane_hit.getV()[0],gear::Vector3D::spherical);
  if (type[CEPCConf::TrkHitTypeBit::PLANAR]) {
      float phiU = 0.0;
      float thetaU = 0.0;
      float phiV = 0.0;
      float thetaV = 0.0;
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      if (trkhit.isA<edm4hep::TrackerHitPlane>()) {
          auto hitPlane = trkhit.as<edm4hep::TrackerHitPlane>();
          // TODO
          thetaU = hitPlane.getU().a;
          phiU   = hitPlane.getU().b;
          thetaV = hitPlane.getV().a;
          phiV   = hitPlane.getV().b;
      } else if (trkhit.isA<edm4hep::TrackerHit3D>()) {
          auto hit3d = trkhit.as<edm4hep::TrackerHit3D>();
          auto covmat = hit3d.getCovMatrix();
          phiU = covmat[1];
          thetaU = covmat[0];
          phiV = covmat[4];
          thetaV = covmat[3];
      } else {
          throw std::runtime_error("Unsupported concrete TrackerHit type in CEPCArcDiscMeasLayer::ConvertLCIOTrkHit");
      }
#else
      phiU = trkhit.getCovMatrix(1);
      thetaU = trkhit.getCovMatrix(0);
      phiV = trkhit.getCovMatrix(4);
      thetaV = trkhit.getCovMatrix(3);
#endif
    gear::Vector3D U(1.0,phiU,thetaU,gear::Vector3D::spherical);
    gear::Vector3D V(1.0,phiV,thetaV,gear::Vector3D::spherical);
    // gear::Vector3D U(1.0,trkhit.getCovMatrix(1),trkhit.getCovMatrix(0),gear::Vector3D::spherical);
    // gear::Vector3D V(1.0,trkhit.getCovMatrix(4),trkhit.getCovMatrix(3),gear::Vector3D::spherical);
    gear::Vector3D X(1.0,0.0,0.0);
    gear::Vector3D Y(0.0,1.0,0.0);
    gear::Vector3D Z(0.0,0.0,1.0);
    
    const float eps = 1.0e-07;
    // only require vertical to Z axis
    if (U.dot(Z) > eps) {
      std::cout << "CEPCArcDiscMeasLayer: TrackerHit measurment vectors U is not vertical to the global Z axis. \n exit(1) called from file " << __FILE__ << " and line " << __LINE__ << std::endl;
      exit(1);
    }
    if (V.dot(Z) > eps) {
      std::cout << "CEPCArcDiscMeasLayer: TrackerHit measurment vectors V is not vertical to the global Z axis. \n exit(1) called from file " << __FILE__ << " and line " << __LINE__ << std::endl;
      exit(1);
    }
  }
  /*
  // U must be the global X axis 
  if( fabs(1.0 - U.dot(Y)) > eps ) {
    std::cout << "CEPCArcDiscMeasLayer: TrackerHitPlane measurment vectors U is not equal to the global Y axis. \n exit(1) called from file " << __FILE__ << " and line " << __LINE__ << std::endl;
    exit(1);
  }
  
  // V must be the global X axis 
  if( fabs(1.0 - V.dot(X)) > eps ) {
    std::cout << "CEPCArcDiscMeasLayer: TrackerHitPlane measurment vectors V is not equal to the global X axis. \n exit(1) called from file " << __FILE__ << " and line " << __LINE__ << std::endl;
    exit(1);
  }
  */
  const edm4hep::Vector3d& pos=trkhit.getPosition();
  const TVector3 hit(pos.x, pos.y, pos.z);
  
  // convert to layer coordinates       
  TKalMatrix h    = this->XvToMv(hit);
  
  double  x[2] ;
  double dx[2] ;
  
  x[0] = h(0, 0);
  x[1] = h(1, 0);

  if (type[CEPCConf::TrkHitTypeBit::PLANAR]) {
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      if (trkhit.isA<edm4hep::TrackerHit3D>()) {
          auto hit3d = trkhit.as<edm4hep::TrackerHit3D>();
          auto covmat = hit3d.getCovMatrix();
          dx[0] = covmat[2];
          dx[1] = covmat[5];
      } else if (trkhit.isA<edm4hep::TrackerHitPlane>()) {
          auto hitPlane = trkhit.as<edm4hep::TrackerHitPlane>();
          dx[0] = hitPlane.getDu();
          dx[1] = hitPlane.getDv();
      } else {
          throw std::runtime_error("Unsupported concrete TrackerHit type in CEPCArcDiscMeasLayer::ConvertLCIOTrkHit");
      }
#else
    dx[0] = trkhit.getCovMatrix(2);
    dx[1] = trkhit.getCovMatrix(5);
#endif
  }
  else if (type[CEPCConf::TrkHitTypeBit::CYLINDER]) {
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      if (trkhit.isA<edm4hep::TrackerHit3D>()) {
          auto hit3d = trkhit.as<edm4hep::TrackerHit3D>();
          auto covmat = hit3d.getCovMatrix();
          dx[0] = sqrt(covmat[0]+covmat[2]);
          dx[1] = sqrt(covmat[5]);
      } else if (trkhit.isA<edm4hep::TrackerHitPlane>()) {
          // TODO: fixme
          auto hitPlane = trkhit.as<edm4hep::TrackerHitPlane>();
          dx[0] = sqrt(hitPlane.getU().a+hitPlane.getDu());
          dx[1] = sqrt(hitPlane.getDv());
      } else {
          throw std::runtime_error("Unsupported concrete TrackerHit type in CEPCArcDiscMeasLayer::ConvertLCIOTrkHit");
      }
#else
    dx[0] = sqrt(trkhit.getCovMatrix(0)+trkhit.getCovMatrix(2));
    dx[1] = sqrt(trkhit.getCovMatrix(5));
#endif
  }
  else {
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
      if (trkhit.isA<edm4hep::TrackerHit3D>()) {
          auto hit3d = trkhit.as<edm4hep::TrackerHit3D>();
          auto covmat = hit3d.getCovMatrix();
          dx[0] = sqrt(covmat[0]+covmat[2]);
          dx[1] = sqrt(covmat[5]);
      } else if (trkhit.isA<edm4hep::TrackerHitPlane>()) {
          // TODO: fixme
          auto hitPlane = trkhit.as<edm4hep::TrackerHitPlane>();
          dx[0] = sqrt(hitPlane.getU().a+hitPlane.getDu());
          dx[1] = sqrt(hitPlane.getDv());
      } else {
          throw std::runtime_error("Unsupported concrete TrackerHit type in CEPCArcDiscMeasLayer::ConvertLCIOTrkHit");
      }
#else
    dx[0] = sqrt(trkhit.getCovMatrix(0)+trkhit.getCovMatrix(2));
    dx[1] = sqrt(trkhit.getCovMatrix(5));
#endif
  }

  bool hit_on_surface = IsOnSurface(hit);
#define DEBUG_CONVERT 1
#ifdef  DEBUG_CONVERT
  std::cout << "CEPCArcDiscMeasLayer::ConvertLCIOTrkHit ILDPlanarHit created" 
	    << " u = "  <<  x[0]
	    << " v = "  <<  x[1]
	    << " du = " << dx[0]
	    << " dv = " << dx[1]
	    << " x = " << pos.x
	    << " y = " << pos.y
	    << " z = " << pos.z
	    << " onSurface = " << hit_on_surface
	    << std::endl ;
#endif
  return hit_on_surface ? new ILDPlanarHit( *this , x, dx, this->GetBz(), trkhit) : NULL; 
}
