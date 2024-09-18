#ifndef __CEPCOTKKALDETECTOR__
#define __CEPCOTKKALDETECTOR__

/** Ladder based OTK to be used for ILD DBD studies 
 *
 * @author S.Aplin DESY
 */

#include "kaltest/TVKalDetector.h"

#include "TMath.h"

class TNode;

class IGeomSvc;

namespace gear{
  class GearMgr ;
}

class CEPCOTKKalDetector : public TVKalDetector {
 public:
  /** Initialize the OTK from GEAR */
  CEPCOTKKalDetector(const gear::GearMgr& gearMgr, IGeomSvc* geoSvc=0);
  
 private:
  void setupGearGeom(const gear::GearMgr& gearMgr);
  void setupGearGeom(IGeomSvc* geoSvc);
  
  int _nLayers ;
  double _bZ ;

  bool _isStripDetector;
    
  struct OTK_Layer {
    int nLadders;
    int nSensorsPerLadder;
    double phi0;
    double dphi;
    double senRMin;
    double supRMin;
    double senWidth;
    double supWidth;
    double senOffset;
    double supOffset;
    double senThickness;
    double supThickness;
    double senLength;
    double supLength;
    double sensorLength;
    double stripAngle;
  };
  std::vector<OTK_Layer> _OTKgeo;
};
#endif
