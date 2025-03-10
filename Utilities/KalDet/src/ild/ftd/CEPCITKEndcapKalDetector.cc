
#include "kaldet/CEPCITKEndcapKalDetector.h"
#include "kaldet/MaterialDataBase.h"

#include <sstream>

#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DD4hep/DD4hepUnits.h"
#include "DDSegmentation/BitField64.h"

#include "DetIdentifier/CEPCConf.h"

#include "gear/GEAR.h"
#include "gear/BField.h"
#include "gearimpl/Util.h"

#include "kaldet/ILDSegmentedDiscMeasLayer.h"
#include "kaldet/ILDDiscMeasLayer.h"
#include "kaldet/ILDCylinderMeasLayer.h"

#include "streamlog/streamlog.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "TVector3.h"

CEPCITKEndcapKalDetector::CEPCITKEndcapKalDetector( const gear::GearMgr& gearMgr, IGeomSvc* geoSvc ) : 
TVKalDetector(300) {
  streamlog_out(DEBUG1) << "CEPCITKEndcapKalDetector building ITKEndcap detector using GEAR " << std::endl;

  MaterialDataBase::Instance().registerForService(gearMgr, geoSvc);
  if(geoSvc){
    setupGearGeom(geoSvc); 
  }
  else{
    setupGearGeom(gearMgr);
  }

  this->build();

  SetOwner();
}

void CEPCITKEndcapKalDetector::build() {
  streamlog_out(DEBUG) << "CEPCITKEndcapKalDetector::build " << std::endl;

  double eps = 1e-9;
  double eps1 = 1.0e-04; // disk
  double eps2 = 1.0e-05; // odd or even
  double eps3 = 1.0e-06; // layer in disk
  //double eps4 = 1.0e-07; // ring
  double eps5 = 1.0e-08; // forward or backwards

  int ndisks = _disksData.layers.size();
  for (int idisk = 0; idisk < ndisks; idisk++) {
    streamlog_out(DEBUG) << "CEPCITKEndcapKalDetector::build disk " << idisk << std::endl;

    auto& disk  = _disksData.layers[idisk];
    auto& rings = disk.rings;

    double alphaPetal       = disk.alphaPetal;
    double zPosition        = disk.zPosition;
    double zOffset          = disk.zOffsetSupport;
    double rminSupport      = disk.rminSupport;
    double rmaxSupport      = disk.rmaxSupport;
    double thicknessSupport = disk.thicknessSupport;
    int nrings = rings.size();
    double thicknessTotal = 0;
    for (int iring = 0; iring < nrings; iring++) {
      auto& ring = rings[iring];

      int npetals       = ring.petalNumber;
      double phi0       = ring.phi0;
      double distance   = ring.distance;
      double widthInner = ring.widthInner;
      double widthOuter = ring.widthOuter;
      double length     = ring.length;
      double tSensitive = ring.thicknessSensitive;
      double tGlue      = ring.thicknessGlue;
      double tService   = ring.thicknessService;
      double thickness  = tSensitive + tGlue + tService;
      if (thickness > thicknessTotal) thicknessTotal = thickness;

      int nsegments = npetals/2;
      this->create_segmented_disk_layers(idisk, iring, nsegments, true, phi0,  zPosition);
      this->create_segmented_disk_layers(idisk, iring, nsegments, true, phi0, -zPosition);

      // odd segements
      // update phi0 by the angular distance of one petal
      phi0 += 2.0 * M_PI / npetals;
      this->create_segmented_disk_layers(idisk, iring, nsegments, false, phi0,  zPosition);
      this->create_segmented_disk_layers(idisk, iring, nsegments, false, phi0, -zPosition);
    }
    
    TMaterial& air     = *MaterialDataBase::Instance().getMaterial("air");
    TMaterial& support = *MaterialDataBase::Instance().getMaterial("ITKEndcapSupportMaterial");
    
    Bool_t dummy = false;

    double z_front = zPosition - 0.5*thicknessSupport + eps;

    double sort_policy = rmaxSupport + idisk * eps1 + 6 * eps3;
    streamlog_out(DEBUG) << "CEPCITKEndcapKalDetector::create air support disk at " << z_front << " sort_policy = " << sort_policy << std::endl;

    TVector3 normal_fwd(0, 0, 1);
    TVector3 normal_bwd(0, 0,-1);

    TVector3 xc_front_fwd(0.0, 0.0, z_front);
    Add(new ILDDiscMeasLayer(air, support, xc_front_fwd, normal_fwd, _bZ, sort_policy, rminSupport, rmaxSupport, dummy, -1, "ITKEAirSupportDiscPositiveZ"));

    TVector3 xc_front_bwd(0.0, 0.0, -z_front);
    Add(new ILDDiscMeasLayer(air, support, xc_front_bwd, normal_bwd, _bZ, sort_policy+eps5, rminSupport, rmaxSupport, dummy, -1, "ITKEAirSupportDiscNegativeZ"));

    double z_rear = zPosition + 0.5*thicknessSupport - eps;

    TVector3 xc_rear_fwd(0.0, 0.0, z_rear);
    Add(new ILDDiscMeasLayer(support, air, xc_rear_fwd, normal_fwd, _bZ, sort_policy+eps3, rminSupport, rmaxSupport, dummy, -1, "ITKESupportAirDiscPositiveZ"));

    TVector3 xc_rear_bwd(0.0, 0.0, -z_rear);
    Add(new ILDDiscMeasLayer(support, air, xc_rear_bwd, normal_bwd, _bZ, sort_policy+eps3+eps5, rminSupport, rmaxSupport, dummy, -1, "ITKESupportAirDiscNegativeZ"));

    double inner_radius = rminSupport - idisk * eps1;
    double outer_radius = rmaxSupport + idisk * eps1 + 2 * eps2;

    Add(new ILDCylinderMeasLayer(air, air, inner_radius, 0.5*thicknessSupport+thicknessTotal+eps, 0, 0, zPosition, _bZ, dummy, -1, "ITKEInnerEdgePositiveZ"));
    Add(new ILDCylinderMeasLayer(air, air, inner_radius, 0.5*thicknessSupport+thicknessTotal+eps, 0, 0, -zPosition, _bZ, dummy, -1, "ITKEInnerEdgeNegativeZ"));

    sort_policy = rmaxSupport + idisk * eps1;
    TVector3 xc_front_edge_fwd(0.0, 0.0, zPosition - 0.5*thicknessSupport - thicknessTotal - eps);
    Add(new ILDDiscMeasLayer(air, air, xc_front_edge_fwd, normal_fwd, _bZ, sort_policy, inner_radius, outer_radius, dummy, -1, "ITKEFrontEdgePositiveZ"));

    TVector3 xc_front_edge_bwd(0.0, 0.0, -zPosition + 0.5*thicknessSupport + thicknessTotal + eps);
    Add(new ILDDiscMeasLayer(air, air, xc_front_edge_bwd, normal_bwd, _bZ, sort_policy+eps5, inner_radius, outer_radius, dummy, -1, "ITKEFrontEdgeNegativeZ"));

    Add(new ILDCylinderMeasLayer(air, air, outer_radius, 0.5*thicknessSupport+thicknessTotal+eps, 0, 0, zPosition, _bZ, dummy, -1, "ITKEOuterEdgePositiveZ"));
    Add(new ILDCylinderMeasLayer(air, air, outer_radius, 0.5*thicknessSupport+thicknessTotal+eps, 0, 0, -zPosition, _bZ, dummy, -1, "ITKEOuterEdgeNegativeZ"));

    sort_policy = rmaxSupport + idisk * eps1 + 3 * eps2;
    TVector3 xc_rear_edge_fwd(0.0, 0.0, zPosition + 0.5*thicknessSupport + thicknessTotal + eps);
    Add(new ILDDiscMeasLayer(air, air, xc_rear_edge_fwd, normal_fwd, _bZ, sort_policy, inner_radius, outer_radius, dummy, -1, "ITKERearEdgePositiveZ"));

    TVector3 xc_rear_edge_bwd(0.0, 0.0, -zPosition - 0.5*thicknessSupport - thicknessTotal - eps);
    Add(new ILDDiscMeasLayer(air, air, xc_rear_edge_bwd, normal_bwd, _bZ, sort_policy+eps5, inner_radius, outer_radius, dummy, -1, "ITKERearEdgeNegativeZ"));
  }

  streamlog_out(DEBUG1) << "MultiRingsZDisk based ITKEndcap MeasLayer created" << std::endl;
}

void CEPCITKEndcapKalDetector::create_segmented_disk_layers(int idisk, int iring, int nsegments, bool even_petals, double phi0, double zpos) {
  streamlog_out(DEBUG1) << "create_segmented_disk_layers idisk = " << idisk << " iring = " << iring << " nsegments = " << nsegments
			<< " even = " << even_petals << " zpos = " << zpos << std::endl;
  Bool_t active = true;
  Bool_t dummy  = false;
  
  TMaterial& air     = *MaterialDataBase::Instance().getMaterial("air");
  TMaterial& silicon = *MaterialDataBase::Instance().getMaterial("silicon");
  TMaterial& service = *MaterialDataBase::Instance().getMaterial("ITKEndcapServiceMaterial");
  TMaterial& glue    = *MaterialDataBase::Instance().getMaterial("ITKEndcapGlueMaterial");

  int zsign       = zpos > 0 ? 1 : -1;
  int start_index = even_petals ? 0 : 1;

  dd4hep::DDSegmentation::BitField64 encoder("system:5,side:-2,layer:9,module:8,sensor:8");
  encoder.reset();
  encoder[CEPCConf::DetCellID::system] = CEPCConf::DetID::ITKEndcap;
  encoder[CEPCConf::DetCellID::side]   = zsign;
  encoder[CEPCConf::DetCellID::layer]  = idisk;
  encoder[CEPCConf::DetCellID::sensor] = iring;

  std::vector<int> module_ids;
  for (int i = 0; i < nsegments; i++) {
    encoder[CEPCConf::DetCellID::module] = even_petals ? 2*i : 2*i+1;
    module_ids.push_back(encoder.lowWord());
  }
  
  // create segmented disk
  double rmaxSupport = _disksData.layers[idisk].rmaxSupport;

  double eps1 = 1.0e-04; // disk
  double eps2 = 1.0e-05; // odd or even
  double eps3 = 1.0e-06; // layer in disk
  double eps4 = 1.0e-07; // ring
  double eps5 = 1.0e-08; // forward or backwards
  //double sort_policy = fabs(z);
  //double sort_policy = rInner+height + eps1 * idisk + eps3 * 1 ;
  //double sort_policy = eps1 * iring;
  double sort_policy = rmaxSupport + eps1 * idisk + eps3 + eps4 * iring;
  if (!even_petals) sort_policy += eps2;
  if (zpos < 0)     sort_policy += eps5;

  double tSupport   = _disksData.layers[idisk].thicknessSupport;
  double tSensitive = _disksData.layers[idisk].rings[iring].thicknessSensitive;
  double tGlue      = _disksData.layers[idisk].rings[iring].thicknessGlue;
  double tService   = _disksData.layers[idisk].rings[iring].thicknessService;
  double rInner     = _disksData.layers[idisk].rings[iring].distance;
  double height     = _disksData.layers[idisk].rings[iring].length;
  double widthInner = _disksData.layers[idisk].rings[iring].widthInner;
  double widthOuter = _disksData.layers[idisk].rings[iring].widthOuter;

  double tFront = even_petals ? tService : tGlue;
  double tRear  = even_petals ? tGlue : tService;
  TMaterial& front = even_petals ? service : glue;
  TMaterial& rear  = even_petals ? glue : service;

  double z = even_petals ? zpos - zsign*(0.5*tSupport + tRear + tSensitive + tFront) : zpos + zsign*(0.5*tSupport);
  streamlog_out(DEBUG1) << "CEPCITKEndcapKalDetector::create_segmented_disk add front face of sensitive at " << z << " sort_policy = " << sort_policy << std::endl;

  const char *name1 = z > 0 ? (even_petals ? "ITKEFrontFacePositiveZ_even" : "ITKEFrontFacePositiveZ_odd") : (even_petals ? "ITKEFrontFaceNegativeZ_even" : "ITKEFrontFaceNegativeZ_odd");
  Add(new ILDSegmentedDiscMeasLayer(air, front, _bZ, sort_policy, nsegments, z, phi0, rInner, height, widthInner, widthOuter, dummy, name1));

  z += zsign*tFront;
  const char *name2 = z > 0 ? (even_petals ? "ITKEFrontPositiveZ_even" : "ITKEFrontPositiveZ_odd") : (even_petals ? "ITKEFrontNegativeZ_even" : "ITKEFrontNegativeZ_odd");
  Add(new ILDSegmentedDiscMeasLayer(front, silicon, _bZ, sort_policy+eps3, nsegments, z, phi0, rInner, height, widthInner, widthOuter, dummy, name2));

  z += zsign*0.5*tSensitive;
  const char *name3 = z > 0 ? (even_petals ? "ITKESenPositiveZ_even" : "ITKESenPositiveZ_odd") : (even_petals ? "ITKESenNegativeZ_even" : "ITKESenNegativeZ_odd");
  Add( new ILDSegmentedDiscMeasLayer(silicon, silicon, _bZ, sort_policy+2*eps3, nsegments, z, phi0, rInner, height, widthInner, widthOuter, active, module_ids, name3));

  z += zsign*0.5*tSensitive;
  const char *name4 = z > 0 ? (even_petals ? "ITKERearPositiveZ_even" : "ITKERearPositiveZ_odd") : (even_petals ? "ITKERearNegativeZ_even" : "ITKERearNegativeZ_odd");
  Add( new ILDSegmentedDiscMeasLayer(silicon, rear, _bZ, sort_policy+3*eps3, nsegments, z, phi0, rInner, height, widthInner, widthOuter, dummy, name4));

  z += zsign*tRear;
  const char *name5 = z > 0 ? (even_petals ? "ITKERearFacePositiveZ_even" : "ITKERearFacePositiveZ_odd") : (even_petals ? "ITKERearFaceNegativeZ_even" : "ITKERearFaceNegativeZ_odd");
  Add(new ILDSegmentedDiscMeasLayer(rear, air, _bZ, sort_policy+4*eps3, nsegments, z, phi0, rInner, height, widthInner, widthOuter, dummy, name5));
}

void CEPCITKEndcapKalDetector::setupGearGeom(const gear::GearMgr& gearMgr) {

  const gear::GearParameters& params = gearMgr.getGearParameters("ITKEndcapParameters");
  streamlog_out(DEBUG) << params << std::endl;

  _bZ = gearMgr.getBField().at(gear::Vector3D(0.,0.,0.)).z();
  try {
    std::vector<double> alphas      = params.getDoubleVals("AlphaPetals");
    std::vector<double> zpositions  = params.getDoubleVals("ZPositions");
    std::vector<double> zoffsets    = params.getDoubleVals("ZOffsetSupport");
    std::vector<double> rminSups    = params.getDoubleVals("RMinSupports");
    std::vector<double> rmaxSups    = params.getDoubleVals("RMaxSupports");
    std::vector<double> tSupports   = params.getDoubleVals("ThicknessSupports");
    std::vector<double> tSensitives = params.getDoubleVals("ThicknessSensitives");
    std::vector<double> tGlues      = params.getDoubleVals("ThicknessGlues");
    std::vector<double> tServices   = params.getDoubleVals("ThicknessServices");
    std::vector<std::string> petalParNames      = params.getStringVals("PetalNumberNames");
    std::vector<std::string> phi0ParNames       = params.getStringVals("PetalPhi0Names");
    std::vector<std::string> distanceParNames   = params.getStringVals("PetalDistanceNames");
    std::vector<std::string> widthInnerParNames = params.getStringVals("PetalInnerWidthNames");
    std::vector<std::string> widthOuterParNames = params.getStringVals("PetalOuterWidthNames");
    std::vector<std::string> lengthParNames     = params.getStringVals("PetalLengthNames");

    for (int idisk = 0, N = alphas.size(); idisk < N; idisk++) {
      dd4hep::rec::MultiRingsZDiskData::LayerLayout disk;// = _disksData.layers[idisk];
      disk.alphaPetal = alphas[idisk];
      disk.zPosition  = zpositions[idisk];
      disk.zOffsetSupport = zoffsets[idisk];
      disk.rminSupport = rminSups[idisk];
      disk.rmaxSupport = rmaxSups[idisk];
      disk.thicknessSupport = tSupports[idisk];
      //std::string petalParName = petalParNames[idisk];
      //std::string phi0ParName = phi0ParNames[idisk];
      //std::string distanceParName = distanceParNames[idisk];
      //std::string widthInnerParName = widthInnerParNames[idisk];
      //std::string widthOuterParName = widthOuterParNames[idisk];
      //std::string lengthParName = lengthParNames[idisk];
      std::vector<int> petals         = params.getIntVals(petalParNames[idisk]);
      std::vector<double> phi0s       = params.getDoubleVals(phi0ParNames[idisk]);
      std::vector<double> distances   = params.getDoubleVals(distanceParNames[idisk]);
      std::vector<double> widthInners = params.getDoubleVals(widthInnerParNames[idisk]);
      std::vector<double> widthOuters = params.getDoubleVals(widthOuterParNames[idisk]);
      std::vector<double> lengths     = params.getDoubleVals(lengthParNames[idisk]);
      int nrings = petals.size();
      for (int iring = 0; iring < nrings; iring++) {
	dd4hep::rec::MultiRingsZDiskData::Ring ring;
	ring.petalNumber = petals[iring];
	ring.phi0 = phi0s[iring];
	ring.distance = distances[iring];
	ring.widthInner = widthInners[iring];
	ring.widthOuter = widthOuters[iring];
	ring.length = lengths[iring];
	ring.thicknessSensitive = tSensitives[idisk];
	ring.thicknessGlue = tGlues[idisk];
	ring.thicknessService = tServices[idisk];
	disk.rings.push_back(ring);
      }
      _disksData.layers.push_back(disk);
    }
  } catch (gear::UnknownParameterException& e) {
    std::cout << e.what() << std::endl;
  }

  streamlog_out(DEBUG) << _disksData << std::endl;
}

void CEPCITKEndcapKalDetector::setupGearGeom(IGeomSvc* geoSvc) {
  std::cout << "CEPCITKEndcapKalDetector::setupGearGeom(IGeomSvc* geoSvc) TODO" << std::endl;
  exit(1);
}
