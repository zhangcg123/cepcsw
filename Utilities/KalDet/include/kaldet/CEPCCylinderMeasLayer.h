#ifndef CEPCCYLINDERMEASLAYER_H
#define CEPCCYLINDERMEASLAYER_H

/** CEPCCylinderMeasLayer: User defined KalTest measurement layer class 
 *
 * @author 
 */

#include "ILDCylinderMeasLayer.h"
#include <iostream>
#include <cmath>
/* #include "streamlog/streamlog.h" */


class CEPCCylinderMeasLayer : public ILDCylinderMeasLayer/*, public TCylinder */{
  
 public:
  
  /** Constructor Taking inner and outer materials, radius and half length, B-Field, whether the layer is sensitive, Cell ID, and an optional name */
  CEPCCylinderMeasLayer(TMaterial &min,
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
			Int_t      CellID = -1,
			const Char_t    *name = "CEPCCylinderMeasL");
  //    : ILDCylinderMeasLayer(min, mout, r0, lhalf, x0, y0, z0, Bz, is_active, CellID, name),
  //_phi0(phi0), _dphi(dphi)
  //{ /* no op */ }

  virtual Bool_t IsOnSurface(const TVector3 &xx) const;// {

  //    bool z = (xx.Z() >= GetZmin() && xx.Z() <= GetZmax());
  // bool r = std::fabs( (xx-this->GetXc()).Perp() - this->GetR() ) < 1.e-3; // for very short, very stiff tracks this can be poorly defined, so we relax this here a bit to 1 micron
  // Double_t phi = (xx-this->GetXc()).Phi();
  //dphi = phi - phi0;
  //if (dphi>M_PI) dphi -= M_PI*2;
  //else if (dphi<-M_PI) dphi += M_PI*2;
//    streamlog_out(DEBUG0) << "CEPCCylinderMeasLayer IsOnSurface for " << this->TVMeasLayer::GetName() << " R =  " << this->GetR() << "  GetZmin() = " << GetZmin() << " GetZmax() = " << GetZmax()
//    << " dr = " << std::fabs( (xx-this->GetXc()).Perp() - this->GetR() ) << " r = " << r << " z = " << z 
//    << std::endl;
    
  //return r && z && (fabs(dphi)<_dphi);
  //}
  
private:

  Double_t _phi0;
  Double_t _dphi;
};
#endif
