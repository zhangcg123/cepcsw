#ifndef __CEPCVTXKALDETECTOR__
#define __CEPCVTXKALDETECTOR__

/** Ladder based VXD to be used for ILD DBD studies 
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


class CEPCVTXKalDetector : public TVKalDetector {
  
public:
  
  CEPCVTXKalDetector( const gear::GearMgr& gearMgr, IGeomSvc* geoSvc);
  
  
private:
  
  void setupGearGeom( const gear::GearMgr& gearMgr );
  void setupGearGeom( IGeomSvc* geoSvc) ;
  
  int _nLayers[2];
  double _bZ;

  double _relative_position_of_measurement_surface;

  double _shellInnerR;
  double _shellOuterR;
  double _shellHalfZ;

  struct VXD_Layer {
    int nLadders;
    double phi0;
    double dphi;
    double senRMin;
    double supRMin;
    double length;
    double width;
    double offset;
    double senThickness;
    double supThickness;
  };
  std::vector<VXD_Layer> _VXDgeo;

  struct STT_Layer {
    int    id;
    double length;
    double width;
    double senRMin;
    double senThickness;
    double supRMin;
    double supThickness;
    double phi0;
    double rgap;
    double dphi;
  };
  std::vector<STT_Layer> _STTgeo;
};
#endif
