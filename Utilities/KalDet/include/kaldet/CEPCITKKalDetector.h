#ifndef __CEPCITKKALDETECTOR
#define __CEPCITKKALDETECTOR__

/** Ladder based ITK to be used for ILD DBD studies 
 *
 * @author S.Aplin DESY
 */

#include "kaltest/TVKalDetector.h"

#include "TMath.h"

class TNode;

namespace gear{
  class GearMgr ;
}

class IGeomSvc;

class CEPCITKKalDetector : public TVKalDetector {
  
public:
  
  /** Initialize the ITK from GEAR */
  CEPCITKKalDetector( const gear::GearMgr& gearMgr, IGeomSvc* geoSvc );
  
  
private:
  
  void setupGearGeom( const gear::GearMgr& gearMgr ) ;
  void setupGearGeom( IGeomSvc* geoSvc );
  
  int _nLayers ;
  double _bZ ;
  
  bool _isStripDetector;
  
  struct ITK_Layer {
    int nLadders;
    int nSensorsPerLadder;
    double phi0;
    double dphi;
    double senRMin;
    double supRMin;
    double length;
    double width;
    double offset;
    double senThickness;
    double supThickness;
    double sensorLength;
    double stripAngle;
  };
  std::vector<ITK_Layer> _ITKgeo;
  
};



#endif
