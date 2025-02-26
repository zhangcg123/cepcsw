/** User defined KalTest measurement layer class 
 *
 * @author 
 */


#include "kaltest/TKalTrack.h" 

#include "kaldet/CEPCCylinderMeasLayer.h"
#include "kaldet/ILDCylinderHit.h"

#include <lcio.h>
#include <edm4hep/TrackerHit.h>
//#include <EVENT/TrackerHitZCylinder.h>

#include "GaudiKernel/CommonMessaging.h"
// #include "streamlog/streamlog.h"

#include "TMath.h"
#include <cmath>

CEPCCylinderMeasLayer::CEPCCylinderMeasLayer(TMaterial &min,
					     TMaterial &mout,
					     Double_t   r0,
					     Double_t   lhalf,
					     Double_t   phi0,
					     Double_t   width,
					     Double_t   x0,
					     Double_t   y0,
					     Double_t   z0,
					     Double_t   Bz,
					     Bool_t     is_active,
					     Int_t      CellID,
					     const Char_t    *name)
: ILDCylinderMeasLayer(min, mout, r0, lhalf, x0, y0, z0, Bz, is_active, CellID, name),
  _phi0(phi0) {
  _dphi = width/r0/2.0;
}

Bool_t CEPCCylinderMeasLayer::IsOnSurface(const TVector3 &xx) const {

  bool z = (xx.Z() >= GetZmin() && xx.Z() <= GetZmax());
  bool r = std::fabs( (xx-this->GetXc()).Perp() - this->GetR() ) < 1.e-3; // for very short, very stiff tracks this can be poorly defined, so we relax this here a bit to 1 micron
  Double_t phi = (xx-this->GetXc()).Phi();
  Double_t dphi = phi - _phi0;
  if (dphi>M_PI) dphi -= M_PI*2;
  else if (dphi<-M_PI) dphi += M_PI*2;
  //    streamlog_out(DEBUG0) << "CEPCCylinderMeasLayer IsOnSurface for " << this->TVMeasLayer::GetName() << " R =  " << this->GetR() << "  GetZmin() = " << GetZmin() << " GetZmax() = " << GetZmax()
  //    << " dr = " << std::fabs( (xx-this->GetXc()).Perp() - this->GetR() ) << " r = " << r << " z = " << z
  //    << std::endl;

  return r && z && (fabs(dphi)<_dphi);
}
