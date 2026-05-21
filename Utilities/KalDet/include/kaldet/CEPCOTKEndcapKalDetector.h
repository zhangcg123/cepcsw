#ifndef __CEPCOTKEndcapDETECTOR__
#define __CEPCOTKEndcapDETECTOR__

/** Petal based OTKEndcap to be used for CEPC TDR studies 
 * WARNING: Still very experimental
 *
 * @author
 */

#include "kaltest/TVKalDetector.h"

class TNode;
class TVector3;
class IGeomSvc;

namespace gear{
  class GearMgr ;
}

class CEPCOTKEndcapKalDetector : public TVKalDetector {
public:
  /** Initialize the OTKEndcap from GEAR */
  CEPCOTKEndcapKalDetector( const gear::GearMgr& gearMgr, IGeomSvc* geoSvc  );

private:
  
  struct OTKEndcap_Petal {
    int    ipetal;
    double phi;
    double alpha;
    double rInner;
    double height;
    double innerBaseLength;
    double outerBaseLength;
    double senThickness;
    double supThickness;
    double senZPos;
    bool faces_ip;
  };
  
  struct OTKEndcap_Disk {
    int nPetals;
    double phi0;
    double dphi;
    
    double alpha;
    double rInner;
    double height;
    double innerBaseLength;
    double outerBaseLength;
    double senThickness;
    double supThickness;
    
    double stripAngle;
    
    double senZPos_even_front;
    double senZPos_odd_front;
    
    bool isDoubleSided;
    bool isStripReadout;
    
    int nSensors;
  };
 
  void build_staggered_design();
  
  //void create_petal(TVector3 measurement_plane_centre, OTKEndcap_Petal petal, int CellID);
  /**
   * @param zpos the z position of the front measurement surface (middle of front sensitive)
   */
  void create_segmented_disk_layers(int idisk, int nsegments, bool even_petals, double phi0, double zpos );
  
  void setupGearGeom(const gear::GearMgr& gearMgr);
  void setupGearGeom(IGeomSvc* geoSvc);
  
  int _nDisks;
  double _bZ;
   
  std::vector<OTKEndcap_Disk> _OTKEndcapgeo;
};
#endif
