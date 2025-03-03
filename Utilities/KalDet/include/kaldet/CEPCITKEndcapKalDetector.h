#ifndef __CEPCITKENDCAPDETECTOR__
#define __CEPCITKENDCAPDETECTOR__

/** Multi rings by petals in each layer
 * WARNING: Still very experimental
 *
 * @author C.D.Fu IHEP
 */

#include "kaltest/TVKalDetector.h"
#include "DetIdentifier/CEPCDetectorData.h"

class IGeomSvc;

namespace gear{
  class GearMgr ;
}

class CEPCITKEndcapKalDetector : public TVKalDetector {
 public:
  /** Initialize the ITKEndcap from GEAR or GeomSvc*/
  CEPCITKEndcapKalDetector(const gear::GearMgr& gearMgr, IGeomSvc* geoSvc);

 private:

  void build();

  /**
   * @param zpos the z position of the center of support
   */
  void create_segmented_disk_layers(int idisk, int iring, int nsegments, bool even_petals, double phi0, double zpos);

  void setupGearGeom(const gear::GearMgr& gearMgr);
  void setupGearGeom(IGeomSvc* geoSvc);

  double _bZ;

  dd4hep::rec::MultiRingsZDiskData _disksData;
};
#endif
