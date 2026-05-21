
#include "kaldet/CEPCOTKEndcapKalDetector.h"

#include "kaldet/MaterialDataBase.h"

#include <sstream>

#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"
#include "DD4hep/DD4hepUnits.h"

#include "gear/GEAR.h"
#include "gear/BField.h"
#include "gearimpl/Util.h"

#include "kaldet/ILDRotatedTrapMeaslayer.h"
//#include "kaldet/ILDSegmentedDiscMeasLayer.h"
#include "kaldet/ILDSegmentedDiscStripMeasLayer.h"
#include "kaldet/CEPCSegmentedDiscMeasLayer.h"

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>
//#include "Identifier/CEPCConf.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "TVector3.h"

CEPCOTKEndcapKalDetector::CEPCOTKEndcapKalDetector(const gear::GearMgr& gearMgr, IGeomSvc* geoSvc) : 
TVKalDetector(300), _nDisks(0) // SJA:FIXME initial size, 300 looks reasonable for ILD, though this would be better stored as a const somewhere
{
  //std::cout << "CEPCOTKEndcapKalDetector building OTKEndcap detector using GEAR " << std::endl ;
  
  MaterialDataBase::Instance().registerForService(gearMgr, geoSvc);
  if(geoSvc){
    setupGearGeom(geoSvc); 
  }
  else{
    setupGearGeom(gearMgr);
  }

  this->build_staggered_design();
  
  SetOwner();
  
}

void CEPCOTKEndcapKalDetector::build_staggered_design() {
  //std::cout << "CEPCOTKEndcapKalDetector::build_staggered_design " << std::endl;
  std::string name = "OTKEndcap";
  
  UTIL::BitField64 encoder(lcio::ILDCellID0::encoder_string);
  
  for (int idisk = 0; idisk < _nDisks; ++idisk) {
    //std::cout << "CEPCOTKEndcapKalDetector::build_staggered_design build disk " << idisk << std::endl;
    int npetals =  _OTKEndcapgeo[idisk].nPetals;
    double phi0 =  _OTKEndcapgeo[idisk].phi0;
    
    double senZPos_even_front = _OTKEndcapgeo[idisk].senZPos_even_front;
    double senZPos_odd_front  = _OTKEndcapgeo[idisk].senZPos_odd_front;
    
    // check that the number of petals is divisible by 2
    int nsegments = npetals/2;
    
    // even segments forward
    this->create_segmented_disk_layers(idisk, npetals, false, phi0,  senZPos_odd_front);
    
    // even segments backwards
    this->create_segmented_disk_layers(idisk, npetals, false, phi0, -senZPos_odd_front);
 
    // odd segements 
    // update phi0 by the angular distance of one petal
    phi0 += 2.0 * M_PI / npetals; 
    
    // odd segments forward
    //this->create_segmented_disk_layers(idisk, nsegments, true, phi0,  senZPos_even_front);
    
    // odd segments backward
    //this->create_segmented_disk_layers(idisk, nsegments, true, phi0, -senZPos_even_front);
    /*
    // make the air disks
    TMaterial & air = *MaterialDataBase::Instance().getMaterial("air") ;
    
    Bool_t dummy  = false;
    
    // place air discs to help transport during track extrapolation
    if (idisk < _nDisks-1) {
      // place the disc half way between the two discs 
      double z = (_OTKEndcapgeo[idisk].senZPos_even_front + _OTKEndcapgeo[idisk+1].senZPos_even_front) * 0.5;
      
      TVector3 xc_fwd(0.0, 0.0, z);
      TVector3 normal_fwd(xc_fwd);
      normal_fwd.SetMag(1.0);
      
      double eps1 = 1.0e-04; // offset for disk number 
      double eps2 = 1.0e-05; // odd or even 
      double eps4 = 1.0e-08; // offset for forwards and backwards
      
      double height = _OTKEndcapgeo[idisk].height;
      double rInner = _OTKEndcapgeo[idisk].rInner;

      double sort_policy = rInner+height + eps1 * idisk + eps2 * 2; // we need to be after the even and odd
      
      //std::cout << "CEPCOTKEndcapKalDetector::create air support disk at " << xc_fwd.z() << " sort_policy = " << sort_policy << std::endl;
      
      Add(new ILDDiscMeasLayer(air, air, xc_fwd, normal_fwd, _bZ, sort_policy,
			       rInner, rInner+height, dummy, -1, "OTKEndcapAirSupportDiscFront"));
      
      TVector3 xc_bwd(0.0, 0.0, -z);
      TVector3 normal_bwd(xc_bwd);
      normal_bwd.SetMag(1.0);
      
      // offset needed for rear disks 
      sort_policy += eps4;
      
      //std::cout << "CEPCOTKEndcapKalDetector::create air support disk at " <<  xc_bwd.z() << " sort_policy = " << sort_policy << std::endl;
      Add(new ILDDiscMeasLayer(air, air, xc_bwd, normal_bwd, _bZ, sort_policy,
                               rInner, rInner+height, dummy, -1, "OTKEndcapAirSupportDiscRear"));
    }
    */
  }
}

void CEPCOTKEndcapKalDetector::create_segmented_disk_layers(int idisk, int nsegments, bool even_petals, double phi0, double zpos) {
  
  Bool_t active = true;
  Bool_t dummy  = false;
  
  TMaterial & air          = *MaterialDataBase::Instance().getMaterial("air");
  TMaterial & silicon      = *MaterialDataBase::Instance().getMaterial("silicon");
  TMaterial & carbon       = *MaterialDataBase::Instance().getMaterial("carbon");
  TMaterial & stripsupport = *MaterialDataBase::Instance().getMaterial("OTKEndcapSupportMaterial");

  double halfPetal       = _OTKEndcapgeo[idisk].dphi/2.0;
  double senThickness    = _OTKEndcapgeo[idisk].senThickness;
  double supThickness    = _OTKEndcapgeo[idisk].supThickness;
  double innerBaseLength = _OTKEndcapgeo[idisk].innerBaseLength;
  double outerBaseLength = _OTKEndcapgeo[idisk].outerBaseLength;
  double height          = _OTKEndcapgeo[idisk].height;
  double rInner          = _OTKEndcapgeo[idisk].rInner;
  bool isDoubleSided     = _OTKEndcapgeo[idisk].isDoubleSided;
  int nSensors           = _OTKEndcapgeo[idisk].nSensors;
  int zsign              = zpos > 0 ? +1 : -1;
  double stripAngle      = _OTKEndcapgeo[idisk].stripAngle;
  bool isStripReadout    = _OTKEndcapgeo[idisk].isStripReadout;
  
  //SJA:FIXME: due to the space frame design of the strip layers there is far too much support so just leave it out for now ...
  TMaterial & support   = isStripReadout == false ? carbon : stripsupport;
  
  UTIL::BitField64 encoder(lcio::ILDCellID0::encoder_string);
  encoder.reset();  // reset to 0
  
  encoder[lcio::ILDCellID0::subdet] = lcio::ILDDetID::ETD;
  encoder[lcio::ILDCellID0::side]   = zsign;
  encoder[lcio::ILDCellID0::layer]  = idisk;
  
  int start_index = even_petals ? 0 : 1 ;
  std::vector<int> sensors_front;
  std::vector<int> sensors_back;
  std::vector<int> module_ids_front;
  std::vector<int> module_ids_back;
  std::vector<int> nsensors(10,0); // FIXME: assume 10 row 
  
  if (isDoubleSided) { // sensors on front and back    double supZPos_odd_front = _OTKEndcapgeo[idisk].supZPos_odd;
    // first half is on the front, second half on the back, sensors start with sensor number 1
    for (int iSensor=0; iSensor < nSensors/2; iSensor++) {
      sensors_front.push_back(iSensor);
      int irow = std::fmod(iSensor, 10);
      nsensors[10-1-irow]++;
    }
    for (int iSensor=nSensors/2; iSensor < nSensors; iSensor++) sensors_back.push_back(iSensor);
  }
  else { // only sensors on the front
    for (int iSensor=0; iSensor < nSensors; iSensor++) {
      sensors_front.push_back(iSensor);
      int irow = std::fmod(iSensor, 10);
      nsensors[10-1-irow]++;
    }
  }

  for (int i=0; i<nsegments; ++i) {
    encoder[lcio::ILDCellID0::module] = i;//start_index + i*2 ;

    for (unsigned j=0; j<sensors_front.size(); j++) {
      encoder[lcio::ILDCellID0::sensor] = sensors_front[j];
      module_ids_front.push_back(encoder.lowWord());
      //std::cout << "will add front cell id: " << encoder.lowWord() << std::endl;
    }
    
    for (unsigned j=0; j<sensors_back.size(); j++) {
      encoder[lcio::ILDCellID0::sensor] = sensors_back[j];
      module_ids_back.push_back(encoder.lowWord());
      //std::cout << "will add back cell id: " << encoder.lowWord() << std::endl;
    }
  }
  //std::cout << "total " << nsegments*nSensors << " cell IDs added, module: 0-" << nsegments-1 << " sensor: 0-" << nSensors-1 << std::endl;
  
  // create segmented disk
  // strip: SegmentedDisc, trapezoid
  // pixel: Disc, tube
  
  // front face of sensitive  
  double z = zpos - zsign*0.5*(senThickness);  
  //  double sort_policy = fabs(z) ;
  double eps1 = 1.0e-04 ; // disk  
  double eps2 = 1.0e-05 ; // odd or even 
  double eps3 = 1.0e-06 ; // layer in disk 
  double eps4 = 1.0e-08 ; // forward or backwards
  
  //double sort_policy = rInner+height + eps1 * idisk + eps3 * 1;
  double sort_policy = fabs(z) + eps1 * idisk + eps3 * 1;
  if (!even_petals) sort_policy += eps2;
  
  // if this is the negative z disk add epsilon to the policy
  if (z < 0) sort_policy += eps4 ; 
  const char *name1 = z > 0 ? "OTKEndcapSenFrontPositiveZ" : "OTKEndcapSenFrontNegativeZ";  

  //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add front face of sensitive at " << z << " sort_policy = " << sort_policy << std::endl;
  if (isStripReadout) {
    Add(new ILDSegmentedDiscMeasLayer(air, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, dummy, name1));
  }
  else {
    TVector3 xc(0.0, 0.0, z);
    TVector3 normal(xc);
    normal.SetMag(1.0);
    Add(new CEPCSegmentedDiscMeasLayer(air, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, dummy, name1));
  }

  // measurement plane
  z += zsign*0.5*senThickness;
  //sort_policy = fabs(z) ;
  sort_policy = fabs(z) + eps1 * idisk + eps3 * 2 ;
  if (z < 0) sort_policy += eps4 ;

  if (!even_petals) sort_policy += eps2;
  if (!even_petals) {
    const char *name2 = z > 0 ? "OTKEndcapMeasLayerFrontPositiveZOdd" : "OTKEndcapMeasLayerFrontNegativeZOdd";
    //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add measurement plane at " << z << " sort_policy = " << sort_policy << " Strip Readout = " << isStripReadout << " number of module_ids = " << module_ids_front.size();
    if (isStripReadout) {
      Add(new ILDSegmentedDiscStripMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, stripAngle, active, module_ids_front, name2));
    //std::cout << " stripAngle = " << stripAngle ; 
    }
    else {
      TVector3 xc(0.0, 0.0, z);
      TVector3 normal(xc);
      normal.SetMag(1.0);
      Add(new CEPCSegmentedDiscMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, nsensors, active, module_ids_front, name2));
    }
    //std::cout << std::endl;*/
  }
  else {
    const char *name2 = z > 0 ? "OTKEndcapMeasLayerFrontPositiveZEven" : "OTKEndcapMeasLayerFrontNegativeZEven";
    std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add measurement plane at " << z << " sort_policy = " << sort_policy << " Strip Readout = " << isStripReadout << " number of module_ids = " << module_ids_front.size();
    if (isStripReadout) {
      Add(new ILDSegmentedDiscStripMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, stripAngle, active, module_ids_front, name2));
      //std::cout << " stripAngle = " << stripAngle;
    }
    else {
      TVector3 xc(0.0, 0.0, z);
      TVector3 normal(xc);
      normal.SetMag(1.0);
      Add(new CEPCSegmentedDiscMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, nsensors, active, module_ids_front, name2));
    }
    //std::cout << std::endl;
  }
  
  // interface between sensitive and support
  z += zsign*0.5*senThickness;
  //  sort_policy = fabs(z) ;
  sort_policy = fabs(z) + eps1 * idisk + eps3 * 3;
  if (z < 0) sort_policy += eps4;
  if (!even_petals) sort_policy += eps2;
  
  const char *name3 = z > 0 ? "OTKEndcapSenSupportIntfPositiveZ" : "OTKEndcapSenSupportIntfNegativeZ";
  
  //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add interface between sensitive and support at " << z << " sort_policy = " << sort_policy << std::endl;
  if (isStripReadout) {
    Add(new ILDSegmentedDiscMeasLayer(silicon, support, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, dummy, name3));
  }
  else {
    TVector3 xc(0.0, 0.0, z);
    TVector3 normal(xc);
    normal.SetMag(1.0);
    Add(new CEPCSegmentedDiscMeasLayer(silicon, support, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, dummy, name3));
    //Add(new CEPCSegmentedDiscMeasLayer(silicon, support, xc, normal, _bZ, sort_policy, rInner, rInner+height, dummy, -1, name3));
    //std::cout << "add ILDDiscMeasLayer at z = " << xc[2] << std::endl;
  }
  
  if (isDoubleSided) {
    // interface between support and sensitive
    z += zsign*supThickness;   
    //  sort_policy = fabs(z) ;
    sort_policy = fabs(z) + eps1 * idisk + eps3 * 4;
    if (z < 0) sort_policy += eps4;
    if (!even_petals) sort_policy += eps2;
    
    const char *name4 = z > 0 ? "OTKEndcapSupportSenIntfPositiveZ" : "OTKEndcapSupportSenIntfNegativeZ";
    
    //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add interface between support and sensitive at " << z << " sort_policy = " << sort_policy << std::endl;
    if (isStripReadout) {
      Add(new ILDSegmentedDiscMeasLayer(support, silicon , _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, dummy, name4));
    }
    else {
      TVector3 xc(0.0, 0.0, z);
      TVector3 normal(xc);
      normal.SetMag(1.0);
      Add(new CEPCSegmentedDiscMeasLayer(support, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, dummy, name4));
      //Add(new CEPCSegmentedDiscMeasLayer(support, silicon, xc, normal, _bZ, sort_policy, rInner, rInner+height, dummy, -1, name4));
    }
    // measurement plane at the back
    z += zsign*0.5*senThickness;   
    //  sort_policy = fabs(z) ;
    sort_policy = fabs(z) + eps1 * idisk + eps3 * 5;
    if (z < 0) sort_policy += eps4;
    if (!even_petals) {
      sort_policy += eps2;
      
      const char *name5 = z > 0 ? "OTKEndcapMeasLayerBackPositiveZOdd" : "OTKEndcapMeasLayerBackNegativeZOdd";
      //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add measurement plane at " << z << " sort_policy = " << sort_policy << " Strip Readout = " << isStripReadout << " number of module_ids = " << module_ids_back.size() ;
      if (isStripReadout) {
        Add(new ILDSegmentedDiscStripMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, -stripAngle, active, module_ids_back, name5));
        //std::cout << " stripAngle = " << -stripAngle;
      }
      else {
	TVector3 xc(0.0, 0.0, z);
	TVector3 normal(xc);
	normal.SetMag(1.0);
	Add(new CEPCSegmentedDiscMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, nsensors, active, module_ids_back, name5));
	//Add(new CEPCSegmentedDiscMeasLayer(silicon, silicon, xc, normal, _bZ, sort_policy, rInner, rInner+height, module_ids_back, active, name5));
      }
      //std::cout << std::endl;
    }
    else{
      const char *name5 = z > 0 ? "OTKEndcapMeasLayerBackPositiveZEven" : "OTKEndcapMeasLayerBackNegativeZEven";
      //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add measurement plane at " << z << " sort_policy = " << sort_policy<< " Strip Readout = " << isStripReadout << " number of module_ids = " << module_ids_back.size();
      if (isStripReadout) {
        Add(new ILDSegmentedDiscStripMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, -stripAngle, active, module_ids_back, name5));
        //std::cout << " stripAngle = " << -stripAngle ; 
      }
      else {
	TVector3 xc(0.0, 0.0, z);
	TVector3 normal(xc);
	normal.SetMag(1.0);
	Add(new CEPCSegmentedDiscMeasLayer(silicon, silicon, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, nsensors, active, module_ids_back, name5));
	//Add(new CEPCSegmentedDiscMeasLayer(silicon, silicon, xc, normal, _bZ, sort_policy, rInner, rInner+height, module_ids_back, active, name5));
      }
      //std::cout << std::endl;
    }
    
    // rear face of sensitive
    z += zsign*0.5*senThickness;  
    //  sort_policy = fabs(z) ;
    sort_policy = fabs(z) + eps1 * idisk + eps3 * 6;
    if (z < 0) sort_policy += eps4;
    if (!even_petals) sort_policy += eps2;
    
    const char *name6 = z > 0 ? "OTKEndcapSenRearPositiveZ" : "OTKEndcapSenRearNegativeZ";
    
    //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add rear face of sensitive at " << z << " sort_policy = " << sort_policy <<  std::endl;
    if (isStripReadout) {
      Add(new ILDSegmentedDiscMeasLayer(silicon, air, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, dummy, name6));
    }
    else {
      TVector3 xc(0.0, 0.0, z);
      TVector3 normal(xc);
      normal.SetMag(1.0);
      Add(new CEPCSegmentedDiscMeasLayer(silicon, air, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, dummy, name6));
      //Add(new CEPCSegmentedDiscMeasLayer(silicon, air, xc, normal, _bZ, sort_policy, rInner, rInner+height, dummy, -1, name6));
    }
  }
  else{
    // rear face of support
    z += zsign*supThickness;
    //  sort_policy = fabs(z) ;
    sort_policy = fabs(z) + eps1 * idisk + eps3 * 4;
    if (z < 0) sort_policy += eps4;
    if (!even_petals) sort_policy += eps2;
    
    const char *name4 = z > 0 ? "OTKEndcapSupRearPositiveZ" : "OTKEndcapSupRearNegativeZ";
    
    //std::cout << "CEPCOTKEndcapKalDetector::create_segmented_disk_layers add rear face of support at " << z << " sort_policy = " << sort_policy << std::endl;
    if (isStripReadout) {
      Add(new ILDSegmentedDiscMeasLayer(support, air, _bZ, sort_policy, nsegments, z, phi0, rInner, height, innerBaseLength, outerBaseLength, dummy, name4));
    }
    else {
      TVector3 xc(0.0, 0.0, z);
      TVector3 normal(xc);
      normal.SetMag(1.0);
      Add(new CEPCSegmentedDiscMeasLayer(support, air, _bZ, sort_policy, nsegments, z, phi0, rInner, rInner+height, halfPetal, dummy, name4));
      //Add(new CEPCSegmentedDiscMeasLayer(support, air, xc, normal, _bZ, sort_policy, rInner, rInner+height, dummy, -1, name4));
    }
  }
}

void CEPCOTKEndcapKalDetector::setupGearGeom(const gear::GearMgr& gearMgr) {
  const gear::GearParameters& etdParams = gearMgr.getGearParameters("ETDParameters");
  //std::cout << etdParams << std::endl;
  
  _bZ = gearMgr.getBField().at(gear::Vector3D(0.,0.,0.)).z();
  
  double strip_angle_deg = 0.0;
  bool strip_angle_present = true;
  try {
    strip_angle_deg = etdParams.getDoubleVal("strip_angle_deg");
  }
  catch (gear::UnknownParameterException& e) {
    strip_angle_present = false;
  }
  //std::cout << "=============OTKEndcap strip angle: " << strip_angle_deg << "==============" << std::endl;
  try {
    std::vector<int>    nPetals       = etdParams.getIntVals("ETDPetalNumber");
    std::vector<int>    nSensors      = etdParams.getIntVals("ETDSensorNumber");
    std::vector<double> petalAngles   = etdParams.getDoubleVals("ETDPetalAngle");
    std::vector<double> phi0s         = etdParams.getDoubleVals("ETDDiskPhi0");
    std::vector<double> alphas        = etdParams.getDoubleVals("ETDDiskAlpha");
    std::vector<double> zpositions    = etdParams.getDoubleVals("ETDDiskPosition");
    std::vector<double> zoffsets      = etdParams.getDoubleVals("ETDDiskOffset");
    std::vector<double> supRinners    = etdParams.getDoubleVals("ETDSupportRmin");
    std::vector<double> supThicknesss = etdParams.getDoubleVals("ETDSupportThickness");
    std::vector<double> supHeights    = etdParams.getDoubleVals("ETDSupportHeight");
    std::vector<double> senRinners    = etdParams.getDoubleVals("ETDSensitiveRmin");
    std::vector<double> senThicknesss = etdParams.getDoubleVals("ETDSensitiveThickness");
    std::vector<double> senHeights    = etdParams.getDoubleVals("ETDSensitiveHeight");

    _nDisks = nPetals.size();
    _OTKEndcapgeo.resize(_nDisks);
  
    double eps = 1.0e-08;
  
    for(int disk=0; disk< _nDisks; ++disk){
      _OTKEndcapgeo[disk].nPetals         = nPetals      [disk];
      _OTKEndcapgeo[disk].nSensors        = nSensors     [disk];
      _OTKEndcapgeo[disk].dphi            = petalAngles  [disk];
      _OTKEndcapgeo[disk].phi0            = phi0s        [disk];
      _OTKEndcapgeo[disk].alpha           = alphas       [disk];
      _OTKEndcapgeo[disk].rInner          = senRinners   [disk];
      _OTKEndcapgeo[disk].height          = senHeights   [disk];
      _OTKEndcapgeo[disk].innerBaseLength = 0;
      _OTKEndcapgeo[disk].outerBaseLength = 0;
      _OTKEndcapgeo[disk].senThickness    = senThicknesss[disk];
      _OTKEndcapgeo[disk].supThickness    = supThicknesss[disk];
      
      _OTKEndcapgeo[disk].senZPos_even_front = zpositions[disk] - zoffsets[disk];
      _OTKEndcapgeo[disk].senZPos_odd_front  = zpositions[disk] + zoffsets[disk];
      
      // fixed single pixel layer (single long strip dealed as pixel), if design changed, to update
      _OTKEndcapgeo[disk].isDoubleSided  = false;
      _OTKEndcapgeo[disk].isStripReadout = false;
      
      if (strip_angle_present) {
	_OTKEndcapgeo[disk].stripAngle = strip_angle_deg * M_PI/180 ;
      }
      else {
	_OTKEndcapgeo[disk].stripAngle = 0.0 ;
      }
    
      //std::cout << _OTKEndcapgeo[disk].nPetals << " " << _OTKEndcapgeo[disk].dphi << " " << _OTKEndcapgeo[disk].phi0 << " " << _OTKEndcapgeo[disk].alpha << " "
      //          << _OTKEndcapgeo[disk].rInner << " " << _OTKEndcapgeo[disk].height << " " << _OTKEndcapgeo[disk].innerBaseLength << " " << _OTKEndcapgeo[disk].outerBaseLength << " "
      //          << _OTKEndcapgeo[disk].senThickness << " " <<  _OTKEndcapgeo[disk].supThickness << " " << _OTKEndcapgeo[disk].senZPos_even_front << " " << _OTKEndcapgeo[disk].senZPos_odd_front << " "
      //          << _OTKEndcapgeo[disk].isDoubleSided << " " << _OTKEndcapgeo[disk].isStripReadout << " " << _OTKEndcapgeo[disk].nSensors << " " << _OTKEndcapgeo[disk].stripAngle << std::endl;
      ////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Assertions       ////////////////////////////////////////////////////////////////////////////////////
      
      // there should be an even number of petals per disk if offset not zero
      if (zoffsets[disk]!=0)  assert(_OTKEndcapgeo[disk].nPetals%2 == 0);
      
      // support and sensitive should have the same size an position in xy
      assert(fabs(supRinners[disk] - senRinners[disk]) < eps);
      assert(fabs(supHeights[disk] - senHeights[disk]) < eps);
      
      // double sided petals should have as many sensors on the front as on the back
      if(_OTKEndcapgeo[disk].isDoubleSided) assert(_OTKEndcapgeo[disk].nSensors%2 == 0); 
      
      // petals should not be rotated around their symmetry axis
      assert(fabs(alphas[disk]) < eps); // not still support alpha
    }
  }
  catch (gear::UnknownParameterException& e) {
    std::cout << e.what() << std::endl;
    _nDisks = 0;
  }
}

void CEPCOTKEndcapKalDetector::setupGearGeom(IGeomSvc* geoSvc) {
  dd4hep::DetElement world = geoSvc->getDD4HepGeo();
  dd4hep::DetElement ftd;
  const std::map<std::string, dd4hep::DetElement>& subs = world.children();
  for(std::map<std::string, dd4hep::DetElement>::const_iterator it=subs.begin();it!=subs.end();it++){
    if(it->first!="OTKEndcap") continue;
    ftd = it->second;
  }
  dd4hep::rec::ZDiskPetalsData* ftdData = nullptr;
  try{
    ftdData = ftd.extension<dd4hep::rec::ZDiskPetalsData>();
  }
  catch(std::runtime_error& e){
    std::cout << e.what() << " " << ftdData << std::endl;
    throw GaudiException(e.what(), "FATAL", StatusCode::FAILURE);
  }
  
  const dd4hep::Direction& field = geoSvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
  _bZ = field.z()/dd4hep::tesla;

  double strip_angle_deg = ftdData->angleStrip/CLHEP::degree;
  bool strip_angle_present = true;

  std::vector<dd4hep::rec::ZDiskPetalsData::LayerLayout>& ftdlayers = ftdData->layers;
  _nDisks = ftdlayers.size() ;

  _OTKEndcapgeo.resize(_nDisks);

  double eps = 1.0e-08;
  //std::cout << "=============OTKEndcap strip angle: " << strip_angle_deg << "==============" << std::endl; 
  for(int disk=0; disk< _nDisks; ++disk){
    dd4hep::rec::ZDiskPetalsData::LayerLayout& ftdlayer = ftdlayers[disk];
    _OTKEndcapgeo[disk].nPetals = ftdlayer.petalNumber;
    _OTKEndcapgeo[disk].dphi = ftdlayer.petalHalfAngle*2;
    _OTKEndcapgeo[disk].phi0 = ftdlayer.phi0;
    _OTKEndcapgeo[disk].alpha = ftdlayer.alphaPetal;
    _OTKEndcapgeo[disk].rInner = ftdlayer.distanceSensitive*CLHEP::cm;
    _OTKEndcapgeo[disk].height = ftdlayer.lengthSensitive*CLHEP::cm;
    _OTKEndcapgeo[disk].innerBaseLength =  ftdlayer.widthInnerSensitive*CLHEP::cm;
    _OTKEndcapgeo[disk].outerBaseLength =  ftdlayer.widthOuterSensitive*CLHEP::cm;
    _OTKEndcapgeo[disk].senThickness =  ftdlayer.thicknessSensitive*CLHEP::cm;
    _OTKEndcapgeo[disk].supThickness =  ftdlayer.thicknessSupport*CLHEP::cm;

    _OTKEndcapgeo[disk].senZPos_even_front = ftdlayer.zPosition*CLHEP::cm - ftdlayer.zOffsetSensitive*CLHEP::cm;
    _OTKEndcapgeo[disk].senZPos_odd_front = ftdlayer.zPosition*CLHEP::cm - ftdlayer.zOffsetSensitive*CLHEP::cm - 2*ftdlayer.zOffsetSupport*CLHEP::cm;

    _OTKEndcapgeo[disk].isDoubleSided  = ftdlayer.typeFlags[dd4hep::rec::ZDiskPetalsData::SensorType::DoubleSided];
    _OTKEndcapgeo[disk].isStripReadout = !((bool)ftdlayer.typeFlags[dd4hep::rec::ZDiskPetalsData::SensorType::Pixel]);
    _OTKEndcapgeo[disk].nSensors = ftdlayer.sensorsPerPetal;

    if (strip_angle_present) {
      _OTKEndcapgeo[disk].stripAngle = strip_angle_deg * M_PI/180 ;
    } else {
      _OTKEndcapgeo[disk].stripAngle = 0.0 ;
    }
    
    assert(_OTKEndcapgeo[disk].nPetals%2 == 0);
    assert(fabs(ftdlayer.widthInnerSupport - ftdlayer.widthInnerSensitive) < eps);
    assert(fabs(ftdlayer.widthOuterSupport - ftdlayer.widthOuterSensitive) < eps);
    assert(fabs(ftdlayer.lengthSupport - ftdlayer.lengthSensitive) < eps);
    assert(fabs(ftdlayer.distanceSupport - ftdlayer.distanceSensitive) < eps);
    if (_OTKEndcapgeo[disk].isDoubleSided) assert(_OTKEndcapgeo[disk].nSensors%2 == 0);
    assert(fabs(ftdlayer.alphaPetal) < eps); // not still support alpha
  }
}
