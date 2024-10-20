#include "kaldet/CEPCSegmentedDiscMeasLayer.h"
#include "kaldet/ILDPlanarHit.h"

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>
#include "DetIdentifier/CEPCConf.h"

#include "kaltest/TVTrack.h"
#include "TVector3.h"
#include "TMath.h"
#include "TRotMatrix.h"
#include "TBRIK.h"
#include "TNode.h"
#include "TString.h"

//#include <EVENT/TrackerHitPlane.h>
#include <bitset>
#include <math.h>
#include <assert.h>
#include <algorithm>

// #include "streamlog/streamlog.h"
//#define DEBUGPRINT

CEPCSegmentedDiscMeasLayer::CEPCSegmentedDiscMeasLayer(TMaterial &min,
						       TMaterial &mout,
						       double   Bz,
						       double   sortingPolicy,
						       int      nsegments,
						       double   zpos,
						       double   phi0, // defined by the axis of symmerty of the first petal
						       double   rmin,
						       double   rmax,
						       double   halfPetal,
						       std::vector<int> nsensors,
						       bool     is_active,
						       std::vector<int>      CellIDs,
						       const Char_t    *name) : 
  ILDVMeasLayer(min, mout, Bz, CellIDs, is_active, name),
  TPlane(TVector3(0.,0.,zpos), TVector3(0.,0.,zpos)),
  _sortingPolicy(sortingPolicy),_nsegments(nsegments),_rmin(rmin),_rmax(rmax),_halfPetal(halfPetal),_nsensors(nsensors) {
  
  _segment_dphi = 2.0*M_PI / _nsegments; 
  
  phi0 = angular_range_2PI(phi0);
  
  _start_phi = phi0 - _halfPetal;
  
  _start_phi = angular_range_2PI(_start_phi);

  _nrow = _nsensors.size();
  // now check for constistency
  if (_halfPetal*2 > _segment_dphi ) {
    std::cout << "CEPCSegmentedDiscMeasLayer::CEPCSegmentedDiscMeasLayer overlaps: exit(1) called from " << __FILE__ << "   line " << __LINE__ << std::endl; 
    exit(1);
  }
}

CEPCSegmentedDiscMeasLayer::CEPCSegmentedDiscMeasLayer(TMaterial &min,
						       TMaterial &mout,
						       double   Bz,
						       double   sortingPolicy,
						       int      nsegments,
						       double   zpos,
						       double   phi0, // defined by the axis of symmerty of the first petal
						       double   rmin,
						       double   rmax,
						       double   halfPetal,
						       bool     is_active,
						       const Char_t    *name) : 
  ILDVMeasLayer(min, mout, Bz, is_active, -1, name),
  TPlane(TVector3(0.,0.,zpos), TVector3(0.,0.,zpos)),
  _sortingPolicy(sortingPolicy),_nsegments(nsegments),_rmin(rmin),_rmax(rmax),_halfPetal(halfPetal) {
  
  _segment_dphi = 2.0*M_PI / _nsegments; 
  
  phi0 = angular_range_2PI(phi0);
  
  _start_phi = phi0 - _halfPetal;
  
  _start_phi = angular_range_2PI(_start_phi);
  
  // now check for constistency
  if (_halfPetal*2 > _segment_dphi ) {
    std::cout << "delta phi between two segments: " << _segment_dphi << " half petal: " << _halfPetal << std::endl;
    std::cout << "CEPCSegmentedDiscMeasLayer::CEPCSegmentedDiscMeasLayer overlaps: exit(1) called from " << __FILE__ << "   line " << __LINE__ << std::endl; 
    exit(1);
  }
}


TKalMatrix CEPCSegmentedDiscMeasLayer::XvToMv(const TVector3 &xv) const {
  // Calculate measurement vector (hit coordinates) from global coordinates:
  // coordinate matrix to return
  TKalMatrix mv(ILDPlanarHit_DIM,1);

  int segmentIndex = get_segment_index(xv.Phi());
  
  TVector3 XC = this->get_segment_centre(segmentIndex);
  double rc = XC.Perp();
  double phic = XC.Phi();
  
  double u = rc*(xv.Phi() - phic);
  double v = xv.Perp() - rc;

  mv(0,0) = u;
  mv(1,0) = v;

#ifdef DEBUGPRINT
  std::cout << "CEPCSegmentedDiscMeasLayer::XvToMv: phic = " << phic << " phi = " << xv.Phi() << " rc = " << rc << " r = " << xv.Perp() << std::endl;
  std::cout << "CEPCSegmentedDiscMeasLayer::XvToMv: "
	    << " mv(0,0) = " << mv(0,0) 
	    << " mv(1,0) = " << mv(1,0) 
	    << std::endl;
#endif
  return mv;
}


TVector3 CEPCSegmentedDiscMeasLayer::HitToXv(const TVTrackHit &vht) const {
  // std::cout << "CEPCSegmentedDiscMeasLayer::HitToXv: "
  // << " vht(0,0) = " << vht(0,0) << " vht(1,0) = " << vht(1,0) << std::endl;
  
  const ILDPlanarHit &mv = dynamic_cast<const ILDPlanarHit &>(vht);
    
//  double x =   mv(0,0) ;
//  double y =   mv(1,0) ;
//  
//  double z = this->GetXc().Z() ;

  UTIL::BitField64 encoder(lcio::ILDCellID0::encoder_string);
  edm4hep::TrackerHit hit = mv.getLCIOTrackerHit();
  encoder.setValue(hit.getCellID());
  int segmentIndex = encoder[lcio::ILDCellID0::module];
  
  TVector3 XC = this->get_segment_centre(segmentIndex);
  double rc = XC.Perp();
  double phic = XC.Phi();
  double zc = XC.Z();
  
  double u = mv(0,0);
  double v = mv(1,0);

  double phi = u/rc + phic;
  double r = v + rc;

  double x = r*cos(phi); 
  double y = r*sin(phi);
  double z = zc;
  
  // std::cout << "CEPCSegmentedDiscMeasLayer::HitToXv: "
  // << " x = " << x 
  // << " y = " << y 
  // << " z = " << z 
  // << std::endl;

  return TVector3(x,y,z);
}

void CEPCSegmentedDiscMeasLayer::CalcDhDa(const TVTrackHit &vht,
					  const TVector3   &xxv,
					  const TKalMatrix &dxphiada,
					  TKalMatrix &H) const {
  // Calculate
  //    H = (@h/@a) = (@phi/@a, @z/@a)^t
  // where
  //        h(a) = (phi, z)^t: expected meas vector
  //        a = (drho, phi0, kappa, dz, tanl, t0)
  //
  Int_t sdim = H.GetNcols();
  Int_t hdim = TMath::Max(5,sdim-1);

  // assume cylinder center at (0,0)
  Double_t phiv = xxv.Phi();
  Double_t xv   = xxv.X();
  Double_t yv   = xxv.Y();
  Double_t xxyy = xv * xv + yv * yv;

  for (Int_t i = 0; i < hdim; i++) {
    H(0, i)  = - (yv / xxyy) * dxphiada(0, i) + (xv / xxyy) * dxphiada(1, i);
    H(0, i) *= xxv.Perp();

    H(1, i)  = cos(phiv) * dxphiada(0, i) + sin(phiv) * dxphiada(1, i);
  }

  if (sdim == 6) {
    H(0,sdim-1) = 0.0;
    H(1,sdim-1) = 0.;
  }
}

Int_t CEPCSegmentedDiscMeasLayer::CalcXingPointWith(const TVTrack  &hel,
                                                    TVector3 &xx,
                                                    Double_t &phi,
                                                    Int_t     mode,
                                                    Double_t  eps) const{

  //  streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::CalcXingPointWith" << std::endl;
  
  phi = 0.0;
  
  xx.SetX(0.0);
  xx.SetY(0.0);
  xx.SetZ(0.0);
  
  // check that direction has one of the correct values
  if( !( mode == 0 || mode == 1 || mode == -1) ) return -1 ;
  
//  
  
  // get helix parameters
  Double_t dr     = hel.GetDrho();
  Double_t phi0   = hel.GetPhi0(); //
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
    // streamlog_out(ERROR) << ">>>> Error >>>> CEPCSegmentedDiscMeasLayer::CalcXingPointWith" << std::endl
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
  
  
//  streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::CalcXingPointWith "
//  << " ref_point.z()  = " << ref_point.z()
//  << " z = " << z 
//  << " z0  = " << z0
//  << " z_pca  = " << z_pca
//  << " tanl  = " << tanl
//  << " z - z_pca  = " << z - z_pca
//  << std::endl;
//  
//  TVector3 xx_n;
//  int cuts = TVSurface::CalcXingPointWith(hel, xx_n, phi, mode, eps);
//  streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::CalcXingPointWith from Newton: cuts = " << cuts << " x = " << xx_n.x() << " y = "<< xx_n.y() << " z = " << xx_n.z() << " r = " << xx_n.Perp() << " phi = " << xx_n.Phi() << " dphi = " <<  phi << std::endl;

  
  phi = -omega * s;
  
  const double delta_phi_half = -phi/2.0 ;

  double x;
  double y;
  
  if( fabs(s) > FLT_MIN ){ // protect against starting on the plane

    x = x_pca - s * ( sin(delta_phi_half) / delta_phi_half ) *  sin( phi0 - delta_phi_half ) ;
    
    y = y_pca + s * ( sin(delta_phi_half) / delta_phi_half ) *  cos( phi0 - delta_phi_half ) ;
  
  }
  else{
    // streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::CalcXingPointWith Using PCA values " << std::endl;
    x = x_pca;
    y = y_pca;
    phi = 0.0;
  }

  
  xx.SetXYZ(x, y, z);
 
  
  // streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::CalcXingPointWith            : cuts = " << (IsOnSurface(xx) && (chg*phi*mode)<0) << " x = " << xx.x() << " y = "<< xx.y() << " z = " << xx.z() << " r = " << xx.Perp() << " phi = " << xx.Phi() << " dphi = " <<  phi << " s = " << s << " " << this->TVMeasLayer::GetName() << std::endl;
//
//  streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::CalcXingPointWith :  xdiff = " << xx.x() - xx_n.x() << " ydiff = "<< xx.y() - xx_n.y() << " zdiff = " << xx.z() - xx_n.z() << std::endl;

  // check if intersection with plane is within boundaries
  
  if( mode!=0 && fabs(phi)>1.e-10 ){ // (+1,-1) = (fwd,bwd)
    if( chg*phi*mode > 0){
      return 0;
    }
  }

  
  return (IsOnSurface(xx) ? 1 : 0);
  
  
}

Bool_t CEPCSegmentedDiscMeasLayer::IsOnSurface(const TVector3 &xx) const {
  bool onSurface = false ;
  
  if (TMath::Abs(xx.Z()-GetXc().Z()) < 1e-4) {
    //std::cout << "CEPCSegmentedDiscMeasLayer::IsOnSurface z passed " << std::endl;
    double r2 = xx.Perp2();
    
    // quick check to see weather the hit lies inside the min max r 
    if (r2 <= _rmax*_rmax && r2 >= _rmin*_rmin) { 
      //std::cout << "CEPCSegmentedDiscMeasLayer::IsOnSurface r2 passed " << std::endl;
      
      double phi_point = angular_range_2PI(xx.Phi());
      
      // get the angle in the local system 
      double gamma = angular_range_2PI(phi_point - _start_phi);
      
      // the angle local to the sector
      double local_phi = fmod(gamma, _segment_dphi);
      
      if (local_phi < 2*_halfPetal) {
	//std::cout << "CEPCSegmentedDiscMeasLayer::IsOnSurface dphi passed " << std::endl;
	onSurface = true ;
      }
    }    
  }
#ifdef DEBUGPRINT
  if (!onSurface) {
    std::cout << "IsOnSurface: zc = " << GetXc().Z() << " z = " << xx.Z() << " r = " << xx.Perp() << " phi = " << xx.Phi() << " phi0 = " << _start_phi
	      << " dphi = " << _segment_dphi << " local = " << fmod(xx.Phi()-_start_phi, _segment_dphi) << " half = " << _halfPetal << std::endl;
  }
#endif
  return onSurface;
}


ILDVTrackHit* CEPCSegmentedDiscMeasLayer::ConvertLCIOTrkHit(edm4hep::TrackerHit trkhit) const {
  std::bitset<32> type(trkhit.getType());
  // remember here the "position" of the hit in fact defines the origin of the plane it defines so u and v are per definition 0. 
  const edm4hep::Vector3d& pos=trkhit.getPosition();
  const TVector3 hit(pos.x, pos.y, pos.z) ;
  
  // convert to layer coordinates       
  TKalMatrix h(ILDPlanarHit_DIM,1); 
  
  h    = this->XvToMv(hit);
  
  double  x[ILDPlanarHit_DIM] ;
  double dx[ILDPlanarHit_DIM] ;
  
  x[0] = h(0, 0);
  x[1] = h(1, 0);
  
  if (type[CEPCConf::TrkHitTypeBit::PLANAR]) {
    dx[0] = trkhit.getCovMatrix(2);
    dx[1] = trkhit.getCovMatrix(5);
  }
  else if (type[CEPCConf::TrkHitTypeBit::CYLINDER]) {
    dx[0] = sqrt(trkhit.getCovMatrix(0)+trkhit.getCovMatrix(2));
    dx[1] = sqrt(trkhit.getCovMatrix(5));
  }
  else {
    dx[0] = sqrt(trkhit.getCovMatrix(0)+trkhit.getCovMatrix(2));
    dx[1] = sqrt(trkhit.getCovMatrix(5));
  }
  
  bool hit_on_surface = IsOnSurface(hit);

#ifdef  DEBUGPRINT  
  std::cout << "CEPCSegmentedDiscMeasLayer::ConvertLCIOTrkHit: ILDPlanarHit created" 
	    << " for CellID " << trkhit.getCellID()
	    << " u = "  <<  x[0]
	    << " v = "  <<  x[1]
	    << " du = " << dx[0]
	    << " dv = " << dx[1]
	    << " x = "  << pos.x
	    << " y = "  << pos.y
	    << " z = "  << pos.z
	    << " onSurface = " << hit_on_surface
	    << std::endl;
#endif
  //ILDPlanarHit hh( *this , x, dx, this->GetBz(),trkhit);
  
  //this->HitToXv(hh);
  
  return hit_on_surface ? new ILDPlanarHit( *this , x, dx, this->GetBz(),trkhit) : NULL; 
  
}


/** Get the intersection and the CellID, needed for multilayers */
int CEPCSegmentedDiscMeasLayer::getIntersectionAndCellID(const TVTrack  &hel,
							 TVector3 &xx,
							 Double_t &phi,
							 Int_t    &CellID,
							 Int_t     mode,
							 Double_t  eps) const {
  int crosses = this->CalcXingPointWith(hel, xx, phi, mode, eps);
  
  if ( crosses != 0 ) {
    unsigned int segment = this->get_segment_index(xx.Phi());
    
    const std::vector<int>& cellIds = this->getCellIDs();
    
    lcio::BitField64 bf(UTIL::ILDCellID0::encoder_string);
    bf.setValue(this->getCellIDs()[0]); // get the first cell_ID, module = 0, sensor = 0
    
    double phic = get_segment_phi(segment);
    double local_phi = xx.Phi()-phic;
    if (local_phi>M_PI) local_phi -= 2.0 * M_PI;
    if (local_phi<-M_PI) local_phi += 2.0 * M_PI;
    unsigned int isensor = get_sensor_index(xx.Perp(), local_phi);
    bf[lcio::ILDCellID0::module] = segment;//cellIds.at(segment);
    bf[lcio::ILDCellID0::sensor] = isensor;
    CellID = bf.lowWord();
  }

  return crosses;
}

unsigned int CEPCSegmentedDiscMeasLayer::get_segment_index(double phi) const {
  phi = angular_range_2PI(phi-_start_phi);
  return unsigned(floor(phi/_segment_dphi));
}

unsigned int CEPCSegmentedDiscMeasLayer::get_sensor_index(double r, double phi) const {
  unsigned int irow = floor((r-_rmin)/(_rmax-_rmin)*_nrow);
  if (irow >= _nrow) {
    std::cout << "CEPCSegmentedDiscMeasLayer::get_sensor_index wrong row range: " << irow << std::endl;
    exit(1);
  }
  unsigned int isensor = irow;
  int nphi = _nsensors[irow];
  if (nphi>1) {
    int iphi = floor((phi+_halfPetal)/(2*_halfPetal)*nphi);
    for (int i=1; i<iphi; i++) {
      for (int j=0; j<_nrow; j++) {
	if (_nsensors[j]>i) isensor++;
      }
    }
  }

  return isensor;
}

double CEPCSegmentedDiscMeasLayer::get_segment_phi(unsigned int index) const{
  return angular_range_2PI(_start_phi + 0.5*_segment_dphi + index * _segment_dphi);
}

TVector3 CEPCSegmentedDiscMeasLayer::get_segment_centre(unsigned int index) const{
  
//  streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::get_segment_centre index = " << index << std::endl; 
  
  double phi = this->get_segment_phi(index);

//  streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::get_segment_centre phi = " << phi << std::endl; 
   
  
  double rc = 0.5*(_rmin + _rmax);

//  streamlog_out(DEBUG0) << "CEPCSegmentedDiscMeasLayer::get_segment_centre rc = " << rc << std::endl; 
  
  double xc = rc * cos(phi);
  double yc = rc * sin(phi);;

  double zc = this->GetXc().Z();

  TVector3 XC(xc,yc,zc);
  
  return XC;
}

double CEPCSegmentedDiscMeasLayer::angular_range_2PI(double phi) const {
  //bring phi_point into range 0 < phi < +2PI
  while (phi < 0) {
    phi += 2.0 * M_PI;
  }
  while (phi >= 2.0*M_PI) {
    phi -= 2.0 * M_PI;
  }
  
  return phi;
}

