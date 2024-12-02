#include "GearSvc.h"
#include "DetInterface/IGeomSvc.h"
#include "DetSegmentation/GridDriftChamber.h"
#include "DetIdentifier/CEPCDetectorData.h"

#include "gearxml/GearXML.h"
#include "gearimpl/GearMgrImpl.h"
#include "gearimpl/ConstantBField.h"
#include "gearimpl/GearParametersImpl.h"
#include "gearimpl/ZPlanarParametersImpl.h"
#include "gearimpl/ZPlanarLayerLayoutImpl.h"
#include "gearimpl/FTDParametersImpl.h"
#include "gearimpl/TPCParametersImpl.h"
#include "gearimpl/FixedPadSizeDiskLayout.h"
#include "gearimpl/CalorimeterParametersImpl.h"
#include "gearimpl/SimpleMaterialImpl.h"
#include "gear/VXDLayerLayout.h"
#include "gearxml/tinyxml.h"

#include "DD4hep/Detector.h"
#include "DD4hep/DetElement.h"
#include "DDRec/DetectorData.h"
#include "DDRec/MaterialManager.h"
#include "DD4hep/DD4hepUnits.h"
#include "CLHEP/Units/SystemOfUnits.h"

static const double deg_to_rad = dd4hep::degree/CLHEP::rad;
static const double rad_to_deg = dd4hep::rad/CLHEP::degree;

DECLARE_COMPONENT(GearSvc)

GearSvc::GearSvc(const std::string& name, ISvcLocator* svc)
    : base_class(name, svc),
      m_gearMgr(nullptr)
{
}

GearSvc::~GearSvc()
{
}

gear::GearMgr* GearSvc::getGearMgr()
{
    return m_gearMgr;
}

StatusCode GearSvc::initialize()
{
  StatusCode sc;
  
  if ( m_gearFile.size() > 0 ) {
    info() << "instantiated GEAR from file " << m_gearFile << endmsg;
    m_gearMgr = gear::GearXML(m_gearFile).createGearMgr();
  }
  else {
    warning() << "no GEAR XML file given ..." << endmsg;
    m_gearMgr = new gear::GearMgrImpl;
    
    auto geomSvc = service<IGeomSvc>("GeomSvc");
    if ( !geomSvc ) {
      info() << "Failed to find GeomSvc ..." << endmsg;
      return StatusCode::FAILURE;
    }
    info() << "Fill GEAR data from GeomSvc" << endmsg;
    m_gearMgr->setDetectorName("CRD_o1_v01");

    if (m_field.value()==0) {
      const dd4hep::Direction& field = geomSvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
      gear::ConstantBField* b = new gear::ConstantBField(gear::Vector3D(field.x()/dd4hep::tesla, field.y()/dd4hep::tesla, field.z()/dd4hep::tesla));
      m_gearMgr->setBField(b);
    }
    else {
      gear::ConstantBField* b = new gear::ConstantBField(gear::Vector3D(0, 0, m_field.value()));
      m_gearMgr->setBField(b);
    }

    dd4hep::DetElement world = geomSvc->getDD4HepGeo();
    const std::map<std::string, dd4hep::DetElement>& subs = world.children();
    for(std::map<std::string, dd4hep::DetElement>::const_iterator it=subs.begin();it!=subs.end();it++){
      dd4hep::DetElement sub = it->second;
      debug() << it->first << " " << sub.path() << " " << sub.placementPath() << endmsg;
      if(it->first=="Tube"||it->first=="BeamPipe"){
	sc = convertBeamPipe(sub);
      }
      else if(it->first=="VXD" || it->first=="VTX"){
	sc = convertVXD(sub);
	if (sc.isRecoverable()) sc = convertStitching(sub);
	if (sc.isRecoverable()) sc = convertComposite(sub);
	if (!sc.isSuccess()) {
	  error() << it->first << " extension not read" << endmsg;
	}
      }
      else if(it->first=="FTD" || it->first=="ITKEndcap"){
        sc = convertFTD(sub);
      }
      else if(it->first=="SIT" || it->first=="ITKBarrel"){
	sc = convertSIT(sub);
      }
      else if(it->first=="TPC"){
	sc = convertTPC(sub);
      }
      else if(it->first=="DriftChamber"){
        sc = convertDC(sub);
      }
      else if(it->first=="SET" || it->first=="OTKBarrel"){
	sc = convertSET(sub);
      }
      else if(it->first=="ETD" || it->first=="OTKEndcap"){
        sc = convertETD(sub);
      }
      else if(it->first=="EcalBarrel"||it->first=="EcalEndcap"||it->first=="EcalPlug"||
	      it->first=="HcalBarrel"||it->first=="HcalEndcap"||it->first=="HcalRing"||
	      it->first=="YokeBarrel"||it->first=="YokeEndcap"||it->first=="YokePlug"||
	      it->first=="Coil"){
        sc = convertCal(sub);
      }
      else{
	info() << it->first << " will convert in future!" << endmsg;
      }
      if (sc==StatusCode::FAILURE) return sc;
    }

    try {
      const auto& ecalB = m_gearMgr->getEcalBarrelParameters();
    }
    catch (...) {
      info() << "EcalBarrelParameters not create! create fake parameters (big size) now" << endmsg;
      gear::CalorimeterParametersImpl* barrelParam = new gear::CalorimeterParametersImpl(2500, 4500, 8, 0.);
      barrelParam->layerLayout().positionLayer(0, 5.25, 10, 10, 2.1);
      m_gearMgr->setEcalBarrelParameters(barrelParam);
    }
    try {
      const auto& ecalE = m_gearMgr->getEcalEndcapParameters();
    }
    catch (...) {
      info() << "EcalEndcapsParameters not create! create fake parameters (big size) now" << endmsg;
      gear::CalorimeterParametersImpl* endcapParam = new gear::CalorimeterParametersImpl(400., 3000, 4510, 2, 0.);
      endcapParam->layerLayout().positionLayer(0, 5.25, 10, 10, 2.1);
      m_gearMgr->setEcalEndcapParameters(endcapParam);
    }
    //gear::CalorimeterParametersImpl* barrelYokeParam = new gear::CalorimeterParametersImpl(4173.929932, 4072., 12, 0.0);
    //gear::CalorimeterParametersImpl* endcapYokeParam = new gear::CalorimeterParametersImpl(320., 7414.929932, 4072., 2, 0.0);
    //gear::CalorimeterParametersImpl* plugYokeParam   = new gear::CalorimeterParametersImpl(320., 2849.254326, 3781.43, 2, 0.0);
    //plugYokeParam->setDoubleVal("YokePlugThickness", 290.57) ;
    //m_gearMgr->setYokeBarrelParameters(barrelYokeParam) ;
    //m_gearMgr->setYokeEndcapParameters(endcapYokeParam) ;
    //m_gearMgr->setYokePlugParameters(plugYokeParam) ;

    if (m_outputFile.value()!="") gear::GearXML::createXMLFile(m_gearMgr, m_outputFile.value());
  }
  
  return StatusCode::SUCCESS;
}

StatusCode GearSvc::finalize()
{
    if ( m_gearMgr ) {
        delete m_gearMgr;
        m_gearMgr = nullptr;
    }

    return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertBeamPipe(dd4hep::DetElement& pipe){
  StatusCode sc;

  dd4hep::rec::ConicalSupportData* beamPipeData = nullptr;
  try{
    beamPipeData = pipe.extension<dd4hep::rec::ConicalSupportData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << beamPipeData << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<double> gearValRInner;
  std::vector<double> gearValROuter;
  std::vector<double> gearValZ;
  const std::vector<dd4hep::rec::ConicalSupportData::Section>& sections = beamPipeData->sections;
  for(int i=0;i<sections.size();i++){
    gearValZ.push_back(sections[i].zPos*CLHEP::cm );
    gearValRInner.push_back(sections[i].rInner*CLHEP::cm );
    gearValROuter.push_back(sections[i].rOuter*CLHEP::cm );
  }

  gear::GearParametersImpl* gearParameters = new gear::GearParametersImpl;
  gearParameters -> setDoubleVals( "Z" , gearValZ ) ;
  gearParameters -> setDoubleVals( "RInner" , gearValRInner ) ;
  gearParameters -> setDoubleVals( "ROuter" , gearValROuter ) ;

  m_gearMgr->setGearParameters("BeamPipe", gearParameters ) ;

  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertVXD(dd4hep::DetElement& vxd){
  StatusCode sc;
  // always use extension data
  dd4hep::rec::ZPlanarData* vxdData = nullptr;
  try{
    vxdData = vxd.extension<dd4hep::rec::ZPlanarData>();
  }
  catch(std::runtime_error& e){
    info() << e.what() << " " << vxdData << endmsg;
    return StatusCode::RECOVERABLE;
  }
  if(vxdData){
    int vxdType =  gear::ZPlanarParameters::CMOS;
    gear::ZPlanarParametersImpl* gearVXD = new gear::ZPlanarParametersImpl(vxdType, vxdData->rInnerShell/dd4hep::mm, vxdData->rOuterShell/dd4hep::mm,
									   vxdData->zHalfShell/dd4hep::mm, vxdData->gapShell/dd4hep::mm, 0.);
    for(unsigned i=0,n=vxdData->layers.size() ; i<n; ++i){
      const dd4hep::rec::ZPlanarData::LayerLayout& l = vxdData->layers[i];
      // FIXME set rad lengths to 0 -> need to get from dd4hep ....
      gearVXD->addLayer(l.ladderNumber, l.phi0,
			l.distanceSupport/dd4hep::mm, l.offsetSupport/dd4hep::mm, l.thicknessSupport/dd4hep::mm,
			l.zHalfSupport/dd4hep::mm, l.widthSupport/dd4hep::mm, 0.,
			l.distanceSensitive/dd4hep::mm, l.offsetSensitive/dd4hep::mm, l.thicknessSensitive/dd4hep::mm,
			l.zHalfSensitive/dd4hep::mm, l.widthSensitive/dd4hep::mm, 0.);
    }
    m_gearMgr->setVXDParameters(gearVXD);

    const dd4hep::rec::ZPlanarData::LayerLayout& l = vxdData->layers[0] ;
    double offset = l.offsetSupport;
    dd4hep::rec::Vector3D a( l.distanceSensitive + l.thicknessSensitive, offset, 2.*dd4hep::mm);
    dd4hep::rec::Vector3D b( l.distanceSupport   + l.thicknessSupport,   offset, 2.*dd4hep::mm);
    gear::SimpleMaterialImpl* VXDSupportMaterial = CreateGearMaterial(a, b, "VXDSupportMaterial");
    m_gearMgr->registerSimpleMaterial(VXDSupportMaterial);

    if (vxdData->rOuterShell>vxdData->rInnerShell) {
      dd4hep::rec::Vector3D a1( vxdData->rInnerShell, 0, 2.*dd4hep::mm);
      dd4hep::rec::Vector3D b1( vxdData->rOuterShell, 0, 2.*dd4hep::mm);
      gear::SimpleMaterialImpl* VXDShellMaterial = CreateGearMaterial(a1, b1, "VXDShellMaterial");
      m_gearMgr->registerSimpleMaterial(VXDShellMaterial);
    }

    info() << vxdData->rInnerShell << " " << vxdData->rOuterShell << " " << vxdData->zHalfShell << " " << vxdData->gapShell << endmsg;
    for(int i=0,n=vxdData->layers.size(); i<n; i++){
      const dd4hep::rec::ZPlanarData::LayerLayout& thisLayer = vxdData->layers[i];
      info() << i << ": " << thisLayer.ladderNumber << "," << thisLayer.phi0 << ","
	     << thisLayer.distanceSupport/dd4hep::mm << "," << thisLayer.offsetSupport/dd4hep::mm << "," << thisLayer.thicknessSupport/dd4hep::mm << ","
	     << thisLayer.zHalfSupport/dd4hep::mm << "," << thisLayer.widthSupport/dd4hep::mm << "," << "NULL,"
             << thisLayer.distanceSensitive/dd4hep::mm << "," << thisLayer.offsetSensitive/dd4hep::mm << "," << thisLayer.thicknessSensitive/dd4hep::mm << ","
	     << thisLayer.zHalfSensitive/dd4hep::mm << "," << thisLayer.widthSensitive/dd4hep::mm << ",NULL" << endmsg;
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertStitching(dd4hep::DetElement& vtx){
  dd4hep::rec::CylindricalData* vtxData = nullptr;
  try{
    vtxData = vtx.extension<dd4hep::rec::CylindricalData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << vtxData << endmsg;
    return StatusCode::RECOVERABLE;
  }
  if(vtxData){
    int vtxType =  gear::ZPlanarParameters::CMOS;
    gear::ZPlanarParametersImpl* gearVTX = new gear::ZPlanarParametersImpl(vtxType, vtxData->rInnerShell/dd4hep::mm, vtxData->rOuterShell/dd4hep::mm,
									   vtxData->zHalfShell/dd4hep::mm, vtxData->gapShell/dd4hep::mm, 0.);
    std::vector<int> ids;
    std::vector<double> zhalfs, rsens, tsens, rsups, tsups, phi0s, rgaps, dphis;

    for (unsigned i=0,n=vtxData->layers.size(); i<n; ++i) {
      const dd4hep::rec::CylindricalData::LayerLayout& l = vtxData->layers[i];

      ids.push_back(l.id);
      zhalfs.push_back(l.zHalf/dd4hep::mm);
      rsens.push_back(l.radiusSensitive/dd4hep::mm);
      tsens.push_back(l.thicknessSensitive/dd4hep::mm);
      rsups.push_back(l.radiusSupport/dd4hep::mm);
      tsups.push_back(l.thicknessSupport/dd4hep::mm);
      phi0s.push_back(l.phi0);
      rgaps.push_back(l.rgap/dd4hep::mm);
      dphis.push_back(l.dphi);
    }
    gearVTX->setIntVals("VTXLayerIds", ids);
    gearVTX->setDoubleVals("VTXLayerHalfLengths", zhalfs);
    gearVTX->setDoubleVals("VTXLayerSensitiveRadius", rsens);
    gearVTX->setDoubleVals("VTXLayerSensitiveThickness", tsens);
    gearVTX->setDoubleVals("VTXLayerSupperRadius", rsups);
    gearVTX->setDoubleVals("VTXLayerSupperThickness", tsups);
    gearVTX->setDoubleVals("VTXLayerPhi0", phi0s);
    gearVTX->setDoubleVals("VTXLayerRadialGap", rgaps);
    gearVTX->setDoubleVals("VTXLayerDeltaPhi", dphis);

    m_gearMgr->setVXDParameters(gearVTX);

    const dd4hep::rec::CylindricalData::LayerLayout& l = vtxData->layers[0] ;
    dd4hep::rec::Vector3D a( l.radiusSupport, l.phi0 , 0. ,  dd4hep::rec::Vector3D::cylindrical ) ;
    dd4hep::rec::Vector3D b( l.radiusSupport + l.thicknessSupport,   l.phi0 , 0. ,  dd4hep::rec::Vector3D::cylindrical ) ;
    gear::SimpleMaterialImpl* VXDSupportMaterial = CreateGearMaterial(a, b, "VXDSupportMaterial");
    m_gearMgr->registerSimpleMaterial(VXDSupportMaterial);

    if (vtxData->rOuterShell>vtxData->rInnerShell) {
      dd4hep::rec::Vector3D a1( vtxData->rInnerShell, 0, 2.*dd4hep::mm);
      dd4hep::rec::Vector3D b1( vtxData->rOuterShell, 0, 2.*dd4hep::mm);
      gear::SimpleMaterialImpl* VXDShellMaterial = CreateGearMaterial(a1, b1, "VXDShellMaterial");
      m_gearMgr->registerSimpleMaterial(VXDShellMaterial);
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertComposite(dd4hep::DetElement& vtx){
  dd4hep::rec::CompositeData* vtxData = nullptr;
  try{
    vtxData = vtx.extension<dd4hep::rec::CompositeData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << vtxData << endmsg;
    return StatusCode::RECOVERABLE;
  }
  if(vtxData){
    int vtxType =  gear::ZPlanarParameters::CMOS;
    gear::ZPlanarParametersImpl* gearVTX = new gear::ZPlanarParametersImpl(vtxType, vtxData->rInnerShell/dd4hep::mm, vtxData->rOuterShell/dd4hep::mm,
									   vtxData->zHalfShell/dd4hep::mm, vtxData->gapShell/dd4hep::mm, 0.);
    for (unsigned i=0,n=vtxData->layersPlanar.size(); i<n; ++i) {
      const dd4hep::rec::ZPlanarData::LayerLayout& l = vtxData->layersPlanar[i];
      // FIXME set rad lengths to 0 -> need to get from dd4hep ....
      gearVTX->addLayer(l.ladderNumber, l.phi0,
                        l.distanceSupport/dd4hep::mm, l.offsetSupport/dd4hep::mm, l.thicknessSupport/dd4hep::mm,
			l.zHalfSupport/dd4hep::mm, l.widthSupport/dd4hep::mm, 0.,
                        l.distanceSensitive/dd4hep::mm, l.offsetSensitive/dd4hep::mm, l.thicknessSensitive/dd4hep::mm,
			l.zHalfSensitive/dd4hep::mm, l.widthSensitive/dd4hep::mm, 0.);
    }

    {
      const dd4hep::rec::ZPlanarData::LayerLayout& l = vtxData->layersPlanar[0] ;
      double offset = l.offsetSupport;
      dd4hep::rec::Vector3D a( l.distanceSensitive + l.thicknessSensitive, offset, 2.*dd4hep::mm);
      dd4hep::rec::Vector3D b( l.distanceSupport   + l.thicknessSupport,   offset, 2.*dd4hep::mm);
      gear::SimpleMaterialImpl* VXDSupportMaterial = CreateGearMaterial(a, b, "VXDSupportMaterial");
      m_gearMgr->registerSimpleMaterial(VXDSupportMaterial);
    }

    std::vector<int> ids;
    std::vector<double> zhalfs, rsens, tsens, rsups, tsups, phi0s, rgaps, dphis;

    for (unsigned i=0,n=vtxData->layersBent.size(); i<n; ++i) {
      const dd4hep::rec::CylindricalData::LayerLayout& l = vtxData->layersBent[i];

      ids.push_back(l.id);
      zhalfs.push_back(l.zHalf/dd4hep::mm);
      rsens.push_back(l.radiusSensitive/dd4hep::mm);
      tsens.push_back(l.thicknessSensitive/dd4hep::mm);
      rsups.push_back(l.radiusSupport/dd4hep::mm);
      tsups.push_back(l.thicknessSupport/dd4hep::mm);
      phi0s.push_back(l.phi0);
      rgaps.push_back(l.rgap/dd4hep::mm);
      dphis.push_back(l.dphi);
    }

    gearVTX->setIntVals("VTXLayerIds", ids);
    gearVTX->setDoubleVals("VTXLayerHalfLengths", zhalfs);
    gearVTX->setDoubleVals("VTXLayerSensitiveRadius", rsens);
    gearVTX->setDoubleVals("VTXLayerSensitiveThickness", tsens);
    gearVTX->setDoubleVals("VTXLayerSupperRadius", rsups);
    gearVTX->setDoubleVals("VTXLayerSupperThickness", tsups);
    gearVTX->setDoubleVals("VTXLayerPhi0", phi0s);
    gearVTX->setDoubleVals("VTXLayerRadialGap", rgaps);
    gearVTX->setDoubleVals("VTXLayerDeltaPhi", dphis);

    m_gearMgr->setVXDParameters(gearVTX);

    {
      const dd4hep::rec::CylindricalData::LayerLayout& l = vtxData->layersBent[0] ;
      dd4hep::rec::Vector3D a(l.radiusSupport, l.phi0, 0., dd4hep::rec::Vector3D::cylindrical);
      dd4hep::rec::Vector3D b(l.radiusSupport + l.thicknessSupport, l.phi0, 0., dd4hep::rec::Vector3D::cylindrical);
      gear::SimpleMaterialImpl* VXDSupportMaterial = CreateGearMaterial(a, b, "VXDBentSupportMaterial");
      m_gearMgr->registerSimpleMaterial(VXDSupportMaterial);
    }

    if (vtxData->rOuterShell>vtxData->rInnerShell) {
      dd4hep::rec::Vector3D a1( vtxData->rInnerShell, 0, 2.*dd4hep::mm);
      dd4hep::rec::Vector3D b1( vtxData->rOuterShell, 0, 2.*dd4hep::mm);
      gear::SimpleMaterialImpl* VXDShellMaterial = CreateGearMaterial(a1, b1, "VXDShellMaterial");
      m_gearMgr->registerSimpleMaterial(VXDShellMaterial);
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertFTD(dd4hep::DetElement& ftd){
  dd4hep::rec::ZDiskPetalsData* ftdData = nullptr;
  try{
    ftdData = ftd.extension<dd4hep::rec::ZDiskPetalsData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << ftdData << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<dd4hep::rec::ZDiskPetalsData::LayerLayout>& ftdlayers = ftdData->layers;
  int nLayers = ftdlayers.size();

  gear::FTDParametersImpl* ftdParam = new gear::FTDParametersImpl();
  ftdParam->setDoubleVal("strip_width_mm", ftdData->widthStrip*CLHEP::cm);
  ftdParam->setDoubleVal("strip_length_mm", ftdData->lengthStrip*CLHEP::cm);
  ftdParam->setDoubleVal("strip_pitch_mm", ftdData->pitchStrip*CLHEP::cm);
  ftdParam->setDoubleVal("strip_angle_deg", ftdData->angleStrip*rad_to_deg);
  for(int layer = 0; layer < nLayers; layer++){
    dd4hep::rec::ZDiskPetalsData::LayerLayout& ftdlayer = ftdlayers[layer];
    int nPetals = ftdlayer.petalNumber;
    double dphi = CLHEP::twopi/nPetals;
    double phi0 = ftdlayer.phi0;
    double alpha = ftdlayer.alphaPetal;
    double zposition = ftdlayer.zPosition*CLHEP::cm;
    double zoffset = ftdlayer.zOffsetSupport*CLHEP::cm;
    int signoffset = ftdlayer.zOffsetSupport>0?1:-1;
    zoffset *= signoffset;

    double supRinner    = ftdlayer.distanceSupport*CLHEP::cm;
    double supThickness = ftdlayer.thicknessSupport*CLHEP::cm;
    double supLengthMin = ftdlayer.widthInnerSupport*CLHEP::cm;
    double supLengthMax = ftdlayer.widthOuterSupport*CLHEP::cm;
    double supWidth     = ftdlayer.lengthSupport*CLHEP::cm;
    double senRinner    = ftdlayer.distanceSensitive*CLHEP::cm;
    double senThickness = ftdlayer.thicknessSensitive*CLHEP::cm;
    double senLengthMin = ftdlayer.widthInnerSensitive*CLHEP::cm;
    double senLengthMax = ftdlayer.widthOuterSensitive*CLHEP::cm;
    double senWidth     = ftdlayer.lengthSensitive*CLHEP::cm;

    bool isDoubleSided  = ftdlayer.typeFlags[dd4hep::rec::ZDiskPetalsData::SensorType::DoubleSided];
    bool isPixelReadout = (bool)ftdlayer.typeFlags[dd4hep::rec::ZDiskPetalsData::SensorType::Pixel];
    int sensorType = (isPixelReadout)?gear::FTDParameters::PIXEL:gear::FTDParameters::STRIP;
    int nSensors = ftdlayer.sensorsPerPetal;
    double phalfangle = ftdlayer.petalHalfAngle;

    ftdParam->addLayer(nPetals, nSensors, isDoubleSided, sensorType, phalfangle, phi0, alpha, zposition, zoffset, signoffset,
                       supRinner, supThickness, supLengthMin, supLengthMax, supWidth, 0,
                       senRinner, senThickness, senLengthMin, senLengthMax, senWidth, 0);
  }
  m_gearMgr->setFTDParameters(ftdParam);
  info() << "nftd = " << nLayers << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertETD(dd4hep::DetElement& etd){
  dd4hep::rec::ZDiskPetalsData* etdData = nullptr;
  try{
    etdData = etd.extension<dd4hep::rec::ZDiskPetalsData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << etdData << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<dd4hep::rec::ZDiskPetalsData::LayerLayout>& etdlayers = etdData->layers;
  int nLayers = etdlayers.size();

  gear::GearParametersImpl* etdParam = new gear::GearParametersImpl();
  etdParam->setDoubleVal("strip_width_mm", etdData->widthStrip*CLHEP::cm);
  etdParam->setDoubleVal("strip_length_mm", etdData->lengthStrip*CLHEP::cm);
  etdParam->setDoubleVal("strip_pitch_mm", etdData->pitchStrip*CLHEP::cm);
  etdParam->setDoubleVal("strip_angle_deg", etdData->angleStrip*rad_to_deg);

  std::vector<int> nPetals, nSensors;
  std::vector<double> petalangles, phi0s, alphas, zpositions, zoffsets, supRinners, supThicknesss, supHeights, senRinners, senThicknesss, senHeights;
  for(int layer = 0; layer < nLayers; layer++){
    dd4hep::rec::ZDiskPetalsData::LayerLayout& etdlayer = etdlayers[layer];
    nPetals.push_back(etdlayer.petalNumber);
    petalangles.push_back(etdlayer.petalHalfAngle*2);
    phi0s.push_back(etdlayer.phi0);
    alphas.push_back(etdlayer.alphaPetal);
    zpositions.push_back(etdlayer.zPosition*CLHEP::cm);
    zoffsets.push_back(etdlayer.zOffsetSupport*CLHEP::cm);

    supRinners.push_back(etdlayer.distanceSupport*CLHEP::cm);
    supThicknesss.push_back(etdlayer.thicknessSupport*CLHEP::cm);
    supHeights.push_back(etdlayer.lengthSupport*CLHEP::cm);
    senRinners.push_back(etdlayer.distanceSensitive*CLHEP::cm);
    senThicknesss.push_back(etdlayer.thicknessSensitive*CLHEP::cm);
    senHeights.push_back(etdlayer.lengthSensitive*CLHEP::cm);

    nSensors.push_back(etdlayer.sensorsPerPetal);
  }
  etdParam->setIntVals("ETDPetalNumber", nPetals);
  etdParam->setIntVals("ETDSensorNumber", nSensors);
  etdParam->setDoubleVals("ETDPetalAngle", petalangles);
  etdParam->setDoubleVals("ETDDiskPhi0", phi0s);
  etdParam->setDoubleVals("ETDDiskAlpha", alphas);
  etdParam->setDoubleVals("ETDDiskPosition", zpositions);
  etdParam->setDoubleVals("ETDDiskOffset", zoffsets);
  etdParam->setDoubleVals("ETDSupportRmin", supRinners);
  etdParam->setDoubleVals("ETDSupportThickness", supThicknesss);
  etdParam->setDoubleVals("ETDSupportHeight", supHeights);
  etdParam->setDoubleVals("ETDSensitiveRmin", senRinners);
  etdParam->setDoubleVals("ETDSensitiveThickness", senThicknesss);
  etdParam->setDoubleVals("ETDSensitiveHeight", senHeights);

  const dd4hep::rec::ZDiskPetalsData::LayerLayout& l = etdData->layers[0];
  double z = l.zPosition;
  dd4hep::rec::Vector3D a(l.distanceSupport+0.5*l.lengthSupport, l.phi0, z-l.thicknessSupport, dd4hep::rec::Vector3D::cylindrical);
  dd4hep::rec::Vector3D b(l.distanceSupport+0.5*l.lengthSupport, l.phi0, z, dd4hep::rec::Vector3D::cylindrical);
  gear::SimpleMaterialImpl* ETDSupportMaterial = CreateGearMaterial(a, b, "OTKEndcapSupportMaterial");
  m_gearMgr->registerSimpleMaterial(ETDSupportMaterial);

  m_gearMgr->setGearParameters("ETDParameters", etdParam);
  info() << "nftd = " << nLayers << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertSIT(dd4hep::DetElement& sit){
  dd4hep::rec::ZPlanarData* sitData = nullptr;
  try{
    sitData = sit.extension<dd4hep::rec::ZPlanarData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << sitData << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<dd4hep::rec::ZPlanarData::LayerLayout>& sitlayers = sitData->layers;
  int nLayers = sitlayers.size();
  double strip_angle_deg = sitData->angleStrip*rad_to_deg;
  
  gear::ZPlanarParametersImpl* sitParams = new gear::ZPlanarParametersImpl(1, 0.0, 0.0, 0.0, 0.0, 0.0);
  sitParams->setDoubleVal("strip_width_mm",  sitData->widthStrip*CLHEP::cm);
  sitParams->setDoubleVal("strip_length_mm", sitData->lengthStrip*CLHEP::cm);
  sitParams->setDoubleVal("strip_pitch_mm",  sitData->pitchStrip*CLHEP::cm);
  sitParams->setDoubleVal("strip_angle_deg", strip_angle_deg);
  std::vector<int> n_sensors_per_ladder;
  for( int layer=0; layer < nLayers; layer++){
    dd4hep::rec::ZPlanarData::LayerLayout& layout = sitlayers[layer];

    int nLadders = layout.ladderNumber;
    double phi0 = layout.phi0;
    double supRMin = layout.distanceSupport*CLHEP::cm;
    double supOffset = layout.offsetSupport*CLHEP::cm;
    double supThickness = layout.thicknessSupport*CLHEP::cm;
    double supHalfLength = layout.zHalfSupport*CLHEP::cm;
    double supWidth = layout.widthSupport*CLHEP::cm;
    double senRMin = layout.distanceSensitive*CLHEP::cm;
    double senOffset = layout.offsetSensitive*CLHEP::cm;
    double senThickness = layout.thicknessSensitive*CLHEP::cm;
    double senHalfLength = layout.zHalfSensitive*CLHEP::cm;
    double senWidth = layout.widthSensitive*CLHEP::cm;
    int nSensorsPerLadder = layout.sensorsPerLadder;
    double stripAngle = strip_angle_deg*CLHEP::degree;
    n_sensors_per_ladder.push_back(nSensorsPerLadder);
    sitParams->addLayer(nLadders, phi0, supRMin, supOffset, supThickness, supHalfLength, supWidth, 0, senRMin, senOffset, senThickness, senHalfLength, senWidth, 0);
  }
  sitParams->setIntVals("n_sensors_per_ladder",n_sensors_per_ladder);
  m_gearMgr->setSITParameters( sitParams ) ;

  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertTPC(dd4hep::DetElement& tpc){
  dd4hep::rec::FixedPadSizeTPCData* tpcData = nullptr;
  try{
    tpcData = tpc.extension<dd4hep::rec::FixedPadSizeTPCData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << tpcData << endmsg;
    return StatusCode::FAILURE;
  }

  gear::TPCParametersImpl *tpcParameters = new gear::TPCParametersImpl();
  gear::PadRowLayout2D *padLayout = new gear::FixedPadSizeDiskLayout(tpcData->rMinReadout*CLHEP::cm, tpcData->rMaxReadout*CLHEP::cm,
                                                                     tpcData->padHeight*CLHEP::cm, tpcData->padWidth*CLHEP::cm, tpcData->maxRow, 0.0);
  tpcParameters->setPadLayout(padLayout);
  always() << tpcParameters->getPlaneExtent()[0] << " " << tpcParameters->getPlaneExtent()[1] << " " << tpcData->rMinReadout*CLHEP::cm << endmsg;
  tpcParameters->setMaxDriftLength(tpcData->driftLength*CLHEP::cm);
  tpcParameters->setDriftVelocity(    0.0); // SJA: not set in Mokka so set to 0.0
  tpcParameters->setReadoutFrequency( 0.0);
  tpcParameters->setDoubleVal( "tpcOuterRadius" , tpcData->rMax*CLHEP::cm ) ;
  tpcParameters->setDoubleVal( "tpcInnerRadius",  tpcData->rMin*CLHEP::cm ) ;
  tpcParameters->setDoubleVal( "tpcInnerWallThickness",  tpcData->innerWallThickness*CLHEP::cm ) ;
  tpcParameters->setDoubleVal( "tpcOuterWallThickness",  tpcData->outerWallThickness*CLHEP::cm ) ;

  dd4hep::rec::ConicalSupportData* supportData = nullptr;
  try{
    supportData = tpc.extension<dd4hep::rec::ConicalSupportData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << supportData << endmsg;
    return StatusCode::FAILURE;
  }
  tpcParameters->setDoubleVal( "tpcReadoutInnerRadius",  supportData->sections[0].rInner*CLHEP::cm );
  tpcParameters->setDoubleVal( "tpcReadoutOuterRadius",  supportData->sections[0].rOuter*CLHEP::cm );
  tpcParameters->setDoubleVal( "tpcReadoutZmin",         supportData->sections[0].zPos*CLHEP::cm );
  tpcParameters->setDoubleVal( "tpcReadoutZmax",         supportData->sections[1].zPos*CLHEP::cm );
  tpcParameters->setDoubleVal( "tpcEndplateInnerRadius", supportData->sections[2].rInner*CLHEP::cm );
  tpcParameters->setDoubleVal( "tpcEndplateOuterRadius", supportData->sections[2].rOuter*CLHEP::cm );
  tpcParameters->setDoubleVal( "tpcEndplateZmin",        supportData->sections[1].zPos*CLHEP::cm + 1*CLHEP::um );
  tpcParameters->setDoubleVal( "tpcEndplateZmax",        supportData->sections[2].zPos*CLHEP::cm );

  double x = (supportData->sections[0].rInner + supportData->sections[0].rOuter)/2.;
  // Readout
  {
    dd4hep::rec::Vector3D a(x, 0, supportData->sections[0].zPos);
    dd4hep::rec::Vector3D b(x, 0, supportData->sections[1].zPos);
    gear::SimpleMaterialImpl* TPCReadoutMaterial = CreateGearMaterial(a, b, "TPCReadoutMaterial");
    m_gearMgr->registerSimpleMaterial(TPCReadoutMaterial);
  }
  // Endplate
  {
    dd4hep::rec::Vector3D a(x, 0, supportData->sections[1].zPos);
    dd4hep::rec::Vector3D b(x, 0, supportData->sections[2].zPos);
    gear::SimpleMaterialImpl* TPCEndplateMaterial = CreateGearMaterial(a, b, "TPCEndplateMaterial");
    m_gearMgr->registerSimpleMaterial(TPCEndplateMaterial);
  }

  m_gearMgr->setTPCParameters(tpcParameters);

  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertDC(dd4hep::DetElement& dc){
  dd4hep::rec::FixedPadSizeTPCData* dcData = nullptr;
  try{
    dcData = dc.extension<dd4hep::rec::FixedPadSizeTPCData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << dcData << ", to search volume" << endmsg;
    // before extension ready, force to convert from volumes
    auto geomSvc = service<IGeomSvc>("GeomSvc");
    dd4hep::Readout readout = geomSvc->lcdd()->readout("DriftChamberHitsCollection");
    dd4hep::Segmentation seg = readout.segmentation();
    dd4hep::DDSegmentation::GridDriftChamber* grid = dynamic_cast< dd4hep::DDSegmentation::GridDriftChamber* > ( seg.segmentation() ) ;
    
    dcData = new dd4hep::rec::FixedPadSizeTPCData;
    dcData->rMinReadout = 99999;
    dcData->rMaxReadout = 0;
    std::vector<double> innerRadiusWalls,outerRadiusWalls;
    bool is_convert = true;
    dd4hep::Volume dc_vol = dc.volume();
    for(int i=0;i<dc_vol->GetNdaughters();i++){
      TGeoNode* daughter = dc_vol->GetNode(i);
      std::string nodeName = daughter->GetName();
      //info << nodeName << endmsg;
      if(nodeName.find("chamber_vol")!=-1||nodeName.find("assembly")!=-1){
	if(grid){
	  // if more than one chamber, just use the outer, TODO
	  dcData->rMinReadout = grid->DC_rbegin();
	  dcData->rMaxReadout = grid->DC_rend();
	  dcData->driftLength = grid->detectorLength();
	  dcData->padHeight   = grid->layer_width();
      dcData->padWidth    = dcData->padHeight;
	}
	else{
	  TGeoNode* next = daughter;
	  if(nodeName.find("assembly")!=-1){
	    // if more than one chamber, just use the outer, TODO 
	    next = daughter->GetDaughter(1);
	    std::string s = next->GetName();
	    if(s.find("chamber_vol")==-1){
	      error() << s << " not chamber_vol" << endmsg;
	      is_convert = false;
	    }
	  }
	  
	  //info() << next->GetName() << endmsg;
	  
	  const TGeoShape* chamber_shape = next->GetVolume()->GetShape();
	  if(chamber_shape->TestShapeBit(TGeoTube::kGeoTube)){
	    const TGeoTube* tube = (const TGeoTube*) chamber_shape;
	    double innerRadius = tube->GetRmin();
	    double outerRadius = tube->GetRmax();
	    double halfLength  = tube->GetDz();
	    dcData->driftLength = halfLength;
	  }
	  else{
	    error() << next->GetName() << " not TGeoTube::kGeoTube" << endmsg;
	    is_convert = false;
	  }
	  
	  dcData->maxRow = next->GetNdaughters();
	  if(dcData->maxRow>512){
	    error() << " layer number > 512, something wrong!" << endmsg;
	    is_convert = false;
	  }
	  for(int i=0;i<next->GetNdaughters();i++){
	    TGeoNode* layer = next->GetDaughter(i);
	    const TGeoShape* shape = layer->GetVolume()->GetShape();
	    if(shape->TestShapeBit(TGeoTube::kGeoTube)){
	      const TGeoTube* tube = (const TGeoTube*) shape;
	      double innerRadius = tube->GetRmin();
	      double outerRadius = tube->GetRmax();
	      double halfLength  = tube->GetDz();
	      if(innerRadius<dcData->rMinReadout) dcData->rMinReadout = innerRadius;
	      if(outerRadius>dcData->rMaxReadout) dcData->rMaxReadout = outerRadius;
	    }
	    else{
	      error() << layer->GetName() << " not TGeoTube::kGeoTube" << endmsg;
	      is_convert = false;
	    }
	  }
	  dcData->padHeight = (dcData->rMaxReadout-dcData->rMinReadout)/dcData->maxRow;
	  dcData->padWidth  = dcData->padHeight;
	}
      }
      else if(nodeName.find("wall_vol")!=-1){
	const TGeoShape* wall_shape = daughter->GetVolume()->GetShape();
        if(wall_shape->TestShapeBit(TGeoTube::kGeoTube)){
          const TGeoTube* tube = (const TGeoTube*) wall_shape;
          double innerRadius = tube->GetRmin();
          double outerRadius = tube->GetRmax();
          double halfLength  = tube->GetDz();
          innerRadiusWalls.push_back(innerRadius);
	  outerRadiusWalls.push_back(outerRadius);
        }
	else{
	  error() << nodeName << " not TGeoTube::kGeoTube" << endmsg;
	  is_convert = false;
	}
      }
    }
    if(innerRadiusWalls.size()<2||outerRadiusWalls.size()<2){
      error() << "wall number < 2" << endmsg;
      is_convert = false;
    }
    if(!is_convert){
      error() << "Cannot convert DC volume to extension data!" << endmsg;
      delete dcData;
      return StatusCode::FAILURE;
    }
    if(innerRadiusWalls[0]<innerRadiusWalls[innerRadiusWalls.size()-1]){
      dcData->rMin = innerRadiusWalls[0];
      dcData->rMax = outerRadiusWalls[outerRadiusWalls.size()-1];
      dcData->innerWallThickness = outerRadiusWalls[0]-innerRadiusWalls[0];
      dcData->outerWallThickness = outerRadiusWalls[outerRadiusWalls.size()-1]-innerRadiusWalls[innerRadiusWalls.size()-1];
    }
    else{
      dcData->rMin = innerRadiusWalls[innerRadiusWalls.size()-1];
      dcData->rMax = outerRadiusWalls[0];
      dcData->innerWallThickness = outerRadiusWalls[outerRadiusWalls.size()-1]-innerRadiusWalls[innerRadiusWalls.size()-1];
      dcData->outerWallThickness = outerRadiusWalls[0]-innerRadiusWalls[0];
    }
    info() << (*dcData) << endmsg;
  }
  debug() << dcData->maxRow << ": " << dcData->rMinReadout*CLHEP::cm << " " << dcData->rMaxReadout*CLHEP::cm << endmsg;
  // regard as TPCParameters, TODO: drift chamber parameters
  gear::TPCParametersImpl *tpcParameters = new gear::TPCParametersImpl();
  gear::PadRowLayout2D *padLayout = new gear::FixedPadSizeDiskLayout(dcData->rMinReadout*CLHEP::cm, dcData->rMaxReadout*CLHEP::cm,
                                                                     dcData->padHeight*CLHEP::cm, dcData->padWidth*CLHEP::cm, dcData->maxRow, 0.0);
  tpcParameters->setPadLayout(padLayout);
  tpcParameters->setMaxDriftLength(dcData->driftLength*CLHEP::cm);
  tpcParameters->setDriftVelocity(    0.0);
  tpcParameters->setReadoutFrequency( 0.0);
  tpcParameters->setDoubleVal( "tpcOuterRadius" , dcData->rMax*CLHEP::cm ) ;
  tpcParameters->setDoubleVal( "tpcInnerRadius",  dcData->rMin*CLHEP::cm ) ;
  tpcParameters->setDoubleVal( "tpcInnerWallThickness",  dcData->innerWallThickness*CLHEP::cm ) ;
  tpcParameters->setDoubleVal( "tpcOuterWallThickness",  dcData->outerWallThickness*CLHEP::cm ) ;

  m_gearMgr->setTPCParameters(tpcParameters);

  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertSET(dd4hep::DetElement& set){
  dd4hep::rec::ZPlanarData* setData = nullptr;
  try{
    setData = set.extension<dd4hep::rec::ZPlanarData>();
  }
  catch(std::runtime_error& e){
    warning() << e.what() << " " << setData << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<dd4hep::rec::ZPlanarData::LayerLayout>& setlayers = setData->layers;
  int nLayers = setlayers.size();
  double strip_angle_deg = setData->angleStrip*rad_to_deg;

  gear::ZPlanarParametersImpl* setParams = new gear::ZPlanarParametersImpl(1, 0.0 , 0.0 , 0.0 , 0.0 , 0.0);
  setParams->setDoubleVal("strip_width_mm",  setData->widthStrip*CLHEP::cm);
  setParams->setDoubleVal("strip_length_mm", setData->lengthStrip*CLHEP::cm);
  setParams->setDoubleVal("strip_pitch_mm",  setData->pitchStrip*CLHEP::cm);
  setParams->setDoubleVal("strip_angle_deg", strip_angle_deg);
  std::vector<int> n_sensors_per_ladder;
  for( int layer=0; layer < nLayers; layer++){
    dd4hep::rec::ZPlanarData::LayerLayout& layout = setlayers[layer];
    
    int nLadders = layout.ladderNumber;
    double phi0 = layout.phi0;
    double supRMin = layout.distanceSupport*CLHEP::cm;
    double supOffset = layout.offsetSupport*CLHEP::cm;
    double supThickness = layout.thicknessSupport*CLHEP::cm;
    double supHalfLength = layout.zHalfSupport*CLHEP::cm;
    double supWidth = layout.widthSupport*CLHEP::cm;
    double senRMin = layout.distanceSensitive*CLHEP::cm;
    double senOffset = layout.offsetSensitive*CLHEP::cm;
    double senThickness = layout.thicknessSensitive*CLHEP::cm;
    double senHalfLength = layout.zHalfSensitive*CLHEP::cm;
    double senWidth = layout.widthSensitive*CLHEP::cm;
    int nSensorsPerLadder = layout.sensorsPerLadder;
    double stripAngle = strip_angle_deg*CLHEP::degree;
    n_sensors_per_ladder.push_back(nSensorsPerLadder);
    setParams->addLayer(nLadders, phi0, supRMin, supOffset, supThickness, supHalfLength, supWidth, 0, senRMin, senOffset, senThickness, senHalfLength, senWidth, 0);
  }
  setParams->setIntVals("n_sensors_per_ladder", n_sensors_per_ladder);
  m_gearMgr->setSETParameters(setParams);

  return StatusCode::SUCCESS;
}

StatusCode GearSvc::convertCal(dd4hep::DetElement& cal) {
  std::string name = cal.name();

  dd4hep::rec::LayeredCalorimeterData* calData = nullptr;
  try{
    calData = cal.extension<dd4hep::rec::LayeredCalorimeterData>();
  }
  catch(std::runtime_error& e){
    info() << e.what() << " " << calData << endmsg;
  }
  if(calData){
    double rmin = calData->extent[0]/dd4hep::mm;
    double rmax = calData->extent[1]/dd4hep::mm;
    double zmin = calData->extent[2]/dd4hep::mm;
    double zmax = calData->extent[3]/dd4hep::mm;
    int inner_symmetry = calData->inner_symmetry;
    int outer_symmetry = calData->outer_symmetry;
    double phi0 = calData->phi0;
    gear::CalorimeterParametersImpl* param = nullptr;
    if (calData->layoutType==dd4hep::rec::LayeredCalorimeterData::BarrelLayout) {
      param = new gear::CalorimeterParametersImpl(rmin, zmax, inner_symmetry, phi0);
    }
    else if (calData->layoutType==dd4hep::rec::LayeredCalorimeterData::EndcapLayout) {
      param = new gear::CalorimeterParametersImpl(rmin, rmax, zmin, inner_symmetry, phi0);
    }
    else {
      error() << "Not BarrelLayout and EndcapLayout: " << calData->layoutType << endmsg;
    }
    auto layers = calData->layers;
    for (auto layer : layers) {
      double distance = layer.distance/dd4hep::mm;
      double thickness = layer.sensitive_thickness/dd4hep::mm;
      double absorberThickness = layer.absorberThickness/dd4hep::mm;
      double cellsize0 = layer.cellSize0/dd4hep::mm;
      double cellsize1 = layer.cellSize1/dd4hep::mm;
      param->layerLayout().positionLayer(0, thickness, cellsize0, cellsize1, absorberThickness);
    }
    if (calData->layoutType==dd4hep::rec::LayeredCalorimeterData::BarrelLayout) {
      if (name.find("Ecal")!=-1)      m_gearMgr->setEcalBarrelParameters(param);
      else if (name.find("Hcal")!=-1) m_gearMgr->setHcalBarrelParameters(param);
      else if (name.find("Yoke")!=-1) m_gearMgr->setYokeBarrelParameters(param);
      else m_gearMgr->setGearParameters("CoilParameters", param);

      info() << "BarrelParameters set " << name << endmsg;
    }
    else if (calData->layoutType==dd4hep::rec::LayeredCalorimeterData::EndcapLayout) {
      if (name.find("Endcap")) {
	if (name.find("Ecal")!=-1)      m_gearMgr->setEcalEndcapParameters(param);
	else if (name.find("Hcal")!=-1) m_gearMgr->setHcalEndcapParameters(param);
	else if (name.find("Yoke")!=-1) m_gearMgr->setYokeEndcapParameters(param);

	info() << "EndcapParameters set" << name << endmsg;
      }
      else {
	if (name.find("Ecal")!=-1)      m_gearMgr->setEcalPlugParameters(param);
	else if (name.find("Hcal")!=-1) m_gearMgr->setHcalRingParameters(param);
	else if (name.find("Yoke")!=-1) m_gearMgr->setYokePlugParameters(param);

	info() << "Plug(Ring)Parameters set" << name << endmsg;
      }
    }
  }
  else{
    info() << name << " will convert in future!" << endmsg;
  }

  return StatusCode::SUCCESS;
}

TGeoNode* GearSvc::FindNode(TGeoNode* mother, char* name) {
  TGeoNode* next = 0;
  if(mother->GetNdaughters()!=0){
    for(int i=0;i<mother->GetNdaughters();i++){
      TGeoNode* daughter = mother->GetDaughter(i);
      std::string s = daughter->GetName();
      //info() << "current: " << s << " search for" << name << endmsg;
      if(s.find(name)!=-1){
        next = daughter;
        break;
      }
      else{
        next = FindNode(daughter, name);
      }
    }
  }
  return next;
}

gear::SimpleMaterialImpl* GearSvc::CreateGearMaterial(const dd4hep::rec::Vector3D& a, const dd4hep::rec::Vector3D& b,
						      const std::string name) {
  // TODO: move to GeomSvc
  dd4hep::rec::MaterialManager matMgr( dd4hep::Detector::getInstance().world().volume() );

  const dd4hep::rec::MaterialVec& materials = matMgr.materialsBetween(a, b);
  dd4hep::rec::MaterialData mat = (materials.size() > 1) ? matMgr.createAveragedMaterial(materials) : materials[0].first;
  
  debug() << " ####### found materials between points : " << a << " and " << b << " ######" << endmsg;
  for (unsigned i=0,n=materials.size(); i<n; ++i) {
    debug() <<  materials[i].first.name() << " [" <<   materials[i].second << "]" << endmsg;
  }
  debug() << "   averaged material : " << mat << endmsg;
  gear::SimpleMaterialImpl* gearMaterial = new gear::SimpleMaterialImpl(name.c_str(), mat.A(), mat.Z(),
									mat.density()/(dd4hep::kg/(dd4hep::g*dd4hep::m3)),
									mat.radiationLength()/dd4hep::mm,
									mat.interactionLength()/dd4hep::mm);
  return gearMaterial;
}
