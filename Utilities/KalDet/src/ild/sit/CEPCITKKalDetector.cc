
#include "kaldet/CEPCITKKalDetector.h"
#include "kaldet/MaterialDataBase.h"
#include "kaldet/ILDParallelPlanarStripMeasLayer.h"

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>

#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "DD4hep/DD4hepUnits.h"

#include <gear/GEAR.h>
#include "gear/BField.h"
#include <gear/ZPlanarParameters.h>
#include <gear/ZPlanarLayerLayout.h>
#include "gearimpl/Util.h"

#include "TMath.h"

#include "math.h"
#include <sstream>

// #include "streamlog/streamlog.h"

CEPCITKKalDetector::CEPCITKKalDetector( const gear::GearMgr& gearMgr, IGeomSvc* geoSvc )
: TVKalDetector(300) // SJA:FIXME initial size, 300 looks reasonable for ILD, though this would be better stored as a const somewhere
{
  // std::cout << "CEPCITKKalDetector building ITK detector using GEAR " << std::endl ;
  
  MaterialDataBase::Instance().registerForService(gearMgr, geoSvc);
  
  TMaterial & air       = *MaterialDataBase::Instance().getMaterial("air");
  //TMaterial & silicon   = *MaterialDataBase::Instance().getMaterial("silicon");
  TMaterial & silicon   = *MaterialDataBase::Instance().getMaterial("ITKBarrelSensorMaterial");
  //TMaterial & carbon    = *MaterialDataBase::Instance().getMaterial("carbon");
  TMaterial & carbon    = *MaterialDataBase::Instance().getMaterial("ITKBarrelSupportMaterial");

  if(geoSvc){
    this->setupGearGeom(geoSvc);
  }
  else{
    this->setupGearGeom(gearMgr) ;
  }
  
  if (_isStripDetector) {
    // streamlog_out(DEBUG4) << "\t\t building ITK detector as STRIP Detector." << std::endl ;    
  } else {
    // streamlog_out(DEBUG4) << "\t\t building ITK detector as PIXEL Detector." << std::endl ;    
  }
  
  //--The Ladder structure (realistic ladder)--
  int nLadders;
  
  //  Bool_t active = true;
  Bool_t dummy  = false;
  
  static const double eps_layer  = 1e-6; 
  static const double eps_sensor = 1e-8; 
  
  UTIL::BitField64 encoder( lcio::ILDCellID0::encoder_string ) ; 
  
  for (int layer=0; layer<_nLayers; ++layer) {
    
    double offset   = _ITKgeo[layer].offset ;
    double xioffset = 0.0;
    double z_centre_support = 0.0;

    nLadders = _ITKgeo[layer].nLadders ;
    
    const double phi0 = _ITKgeo[layer].phi0 ;
    
    const double ladder_distance  = _ITKgeo[layer].supRMin ;
    const double ladder_thickness = _ITKgeo[layer].supThickness ;
    
    const double sensitive_distance  = _ITKgeo[layer].senRMin ;
    const double sensitive_thickness = _ITKgeo[layer].senThickness ;
    
    const double width  = _ITKgeo[layer].width ;
    const double length = _ITKgeo[layer].length;
    
    double currPhi;
    const double dphi = _ITKgeo[layer].dphi ;
    
    const double stripAngle = pow(-1,layer) *_ITKgeo[layer].stripAngle;
    
    const int nsensors = _ITKgeo[layer].nSensorsPerLadder;
    
    const double sensor_length = _ITKgeo[layer].sensorLength;

    const int    nrow = std::floor((sensor_length*nsensors)/length+0.001);
    const double gap  = length - sensor_length*nsensors/nrow;
    // TODO: sorting_policy for overlap region like VXD
    for (int ladder=0; ladder<nLadders; ++ladder) {
      
      currPhi = phi0 + (dphi * ladder);
      
      encoder.reset() ;  // reset to 0
      
      encoder[lcio::ILDCellID0::subdet] = lcio::ILDDetID::SIT;
      encoder[lcio::ILDCellID0::side] = 0 ;
      encoder[lcio::ILDCellID0::layer]  = layer ;
      encoder[lcio::ILDCellID0::module] = ladder ;
      
      // check if the sensitive is inside or outside for the support 
      if( sensitive_distance < ladder_distance  ) {
        
        double sen_front_sorting_policy         = sensitive_distance  + (4 * ladder+0) * eps_layer ;
        double sen_back_sorting_policy          = sensitive_distance  + (4 * ladder+2) * eps_layer ;
        double sup_back_sorting_policy          = ladder_distance     + (4 * ladder+3) * eps_layer ;
        
        // air - sensitive boundary
        Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, width, length, offset, z_centre_support, offset, dummy,-1,"ITKSenFront")) ;
        
        for (int isensor=0; isensor<nsensors; ++isensor) {

          encoder[lcio::ILDCellID0::sensor] = isensor ;          
          int CellID = encoder.lowWord() ;
          
          double measurement_plane_sorting_policy = sensitive_distance  + (4 * ladder+1) * eps_layer + eps_sensor * isensor ;
          
          double z_centre_sensor = -0.5*length + (0.5*sensor_length) + (isensor%(nsensors/nrow))*sensor_length;
	  if (z_centre_sensor>0) z_centre_sensor += gap;

          if (_isStripDetector) {
            // measurement plane defined as the middle of the sensitive volume
            Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, measurement_plane_sorting_policy, width, sensor_length, offset, z_centre_sensor, offset, stripAngle, CellID, "ITKStripMeaslayer")) ;
          }
	  else {
            // measurement plane defined as the middle of the sensitive volume
            Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, measurement_plane_sorting_policy, width, sensor_length, offset, z_centre_sensor, offset, true, CellID, "ITKMeaslayer")) ;
          }
          
	  //std::cout << "CEPCITKKalDetector add surface with CellID = " << CellID << std::endl ;
	}
       
        // sensitive - support boundary 
        Add(new ILDParallelPlanarMeasLayer(silicon, carbon, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, width, length, offset, z_centre_support, offset, dummy,-1,"ITKSenSupportIntf" )) ; 
        
        // support - air boundary
        Add(new ILDParallelPlanarMeasLayer(carbon, air, ladder_distance+ladder_thickness, currPhi, _bZ, sup_back_sorting_policy, width, length, offset, z_centre_support, offset, dummy,-1,"ITKSupRear" )) ; 
      }
      else {
        
        double sup_front_sorting_policy         = ladder_distance     + (4 * ladder+0) * eps_layer ;
        double sen_front_sorting_policy         = sensitive_distance  + (4 * ladder+1) * eps_layer ;
        double sen_back_sorting_policy          = sensitive_distance  + (4 * ladder+3) * eps_layer ;
        
        // air - support boundary
        Add(new ILDParallelPlanarMeasLayer(air, carbon, ladder_distance, currPhi, _bZ, sup_front_sorting_policy, width, length, offset, z_centre_support, offset, dummy,-1,"ITKSupFront")) ;
        
        // support boundary - sensitive
        Add(new ILDParallelPlanarMeasLayer(carbon, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, width, length, offset, z_centre_support, offset, dummy,-1,"ITKSenSupportIntf" )) ; 
        
        for (int isensor=0; isensor<nsensors; ++isensor) {

          encoder[lcio::ILDCellID0::sensor] = isensor ;          
          int CellID = encoder.lowWord() ;
          
          double measurement_plane_sorting_policy = sensitive_distance  + (4 * ladder+2) * eps_layer + eps_sensor * isensor ;

          //double z_centre_sensor = -0.5*length + (0.5*sensor_length) + (isensor*sensor_length) ;
	  double z_centre_sensor = -0.5*length + (0.5*sensor_length) + (isensor%(nsensors/nrow))*sensor_length;
          if (z_centre_sensor>0) z_centre_sensor += gap;

          if (_isStripDetector) {
	    // measurement plane defined as the middle of the sensitive volume
	    Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, measurement_plane_sorting_policy, width, sensor_length, offset, z_centre_sensor, offset, stripAngle, CellID, "ITKStripMeaslayer")) ;
          } else {
            // measurement plane defined as the middle of the sensitive volume
            Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, measurement_plane_sorting_policy, width, sensor_length, offset, z_centre_sensor, offset, true, CellID, "ITKMeaslayer")) ;
          }
	  
	  //std::cout << "CEPCITKKalDetector add surface with CellID = " << CellID << std::endl ;
        }
                
        // support - air boundary
        Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, width, length, offset, z_centre_support, offset, dummy,-1,"ITKSenRear" )) ;  
      }
    }    
  }
  
  SetOwner();                   
}



void CEPCITKKalDetector::setupGearGeom( const gear::GearMgr& gearMgr ){
  
  const gear::ZPlanarParameters& pITKDetMain = gearMgr.getSITParameters();
  const gear::ZPlanarLayerLayout& pITKLayerLayout = pITKDetMain.getZPlanarLayerLayout();
  
  _bZ = gearMgr.getBField().at( gear::Vector3D( 0.,0.,0.)  ).z() ;
  
  _nLayers = pITKLayerLayout.getNLayers(); 
  _ITKgeo.resize(_nLayers);
  
  bool n_sensors_per_ladder_present = true;
  
  try {

    std::vector<int> v = pITKDetMain.getIntVals("n_sensors_per_ladder");

  } catch (gear::UnknownParameterException& e) {

    n_sensors_per_ladder_present = false;

  }

  double strip_angle_deg = 0.0;
  _isStripDetector = false;
  
  try {
    
    strip_angle_deg = pITKDetMain.getDoubleVal("strip_angle_deg");
    if (strip_angle_deg !=0 ) _isStripDetector = true;
    
  } catch (gear::UnknownParameterException& e) {
    
    _isStripDetector = false;
    
  }
  
  
  //SJA:FIXME: for now the support is taken as the same size the sensitive
  //           if this is not done then the exposed areas of the support would leave a carbon - air boundary,
  //           which if traversed in the reverse direction to the next boundary then the track would be propagated through carbon
  //           for a significant distance 
  //std::cout << "=============ITK strip angle: " << strip_angle_deg << "==============" << std::endl;
  for( int layer=0; layer < _nLayers; ++layer){
      
    _ITKgeo[layer].nLadders = pITKLayerLayout.getNLadders(layer); 
    _ITKgeo[layer].phi0 = pITKLayerLayout.getPhi0(layer); 
    _ITKgeo[layer].dphi = 2*M_PI / _ITKgeo[layer].nLadders; 
    _ITKgeo[layer].senRMin = pITKLayerLayout.getSensitiveDistance(layer); 
    _ITKgeo[layer].supRMin = pITKLayerLayout.getLadderDistance(layer); 
    _ITKgeo[layer].length = pITKLayerLayout.getSensitiveLength(layer)*2.0; // note: gear for historical reasons uses the halflength 
    _ITKgeo[layer].width = pITKLayerLayout.getSensitiveWidth(layer); 
    _ITKgeo[layer].offset = pITKLayerLayout.getSensitiveOffset(layer); 
    _ITKgeo[layer].senThickness = pITKLayerLayout.getSensitiveThickness(layer); 
    _ITKgeo[layer].supThickness = pITKLayerLayout.getLadderThickness(layer); 

    if (n_sensors_per_ladder_present) {
      _ITKgeo[layer].nSensorsPerLadder =   pITKDetMain.getIntVals("n_sensors_per_ladder")[layer];
    }
    else{
      _ITKgeo[layer].nSensorsPerLadder = 1 ;
    }
    
    _ITKgeo[layer].sensorLength = pITKDetMain.getDoubleVals("length_sensors")[layer];//_ITKgeo[layer].length / _ITKgeo[layer].nSensorsPerLadder;

    if (_isStripDetector) {
      _ITKgeo[layer].stripAngle = strip_angle_deg * M_PI/180 ;
    } else {
      _ITKgeo[layer].stripAngle = 0.0 ;
    }
    //std::cout << _ITKgeo[layer].nLadders << " " << _ITKgeo[layer].phi0 << " "<< _ITKgeo[layer].dphi << " "
    //	      << _ITKgeo[layer].senRMin << " " << _ITKgeo[layer].supRMin << " " << _ITKgeo[layer].length << " "
    //	      << _ITKgeo[layer].width << " " << _ITKgeo[layer].offset << " " << _ITKgeo[layer].senThickness << " "
    //	      << _ITKgeo[layer].supThickness << " " << _ITKgeo[layer].nSensorsPerLadder << " " << _ITKgeo[layer].sensorLength << std::endl;
  }
  //std::cout << " _isStripDetector  = " << _isStripDetector << std::endl;
}

void CEPCITKKalDetector::setupGearGeom( IGeomSvc* geoSvc ){

  dd4hep::DetElement world = geoSvc->getDD4HepGeo();
  dd4hep::DetElement sit;
  const std::map<std::string, dd4hep::DetElement>& subs = world.children();
  for(std::map<std::string, dd4hep::DetElement>::const_iterator it=subs.begin();it!=subs.end();it++){
    if(it->first!="ITK") continue;
    sit = it->second;
  }
  dd4hep::rec::ZPlanarData* sitData = nullptr;
  try{
    sitData = sit.extension<dd4hep::rec::ZPlanarData>();
  }
  catch(std::runtime_error& e){
    std::cout << e.what() << " " << sitData << std::endl;
    throw GaudiException(e.what(), "FATAL", StatusCode::FAILURE);
  }

  const dd4hep::Direction& field = geoSvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
  _bZ = field.z()/dd4hep::tesla;

  std::vector<dd4hep::rec::ZPlanarData::LayerLayout>& sitlayers = sitData->layers;
  _nLayers = sitlayers.size();
  _ITKgeo.resize(_nLayers);

  _isStripDetector = false;
  double strip_angle_deg = sitData->angleStrip/CLHEP::degree;
  if(strip_angle_deg!=0){
    _isStripDetector = true;
  }
  //std::cout << "=============ITK strip angle: " << strip_angle_deg << "==============" << std::endl;
  for( int layer=0; layer < _nLayers; ++layer){
    dd4hep::rec::ZPlanarData::LayerLayout& pITKLayerLayout = sitlayers[layer];
    
    _ITKgeo[layer].nLadders = pITKLayerLayout.ladderNumber;
    _ITKgeo[layer].phi0 = pITKLayerLayout.phi0;
    _ITKgeo[layer].dphi = 2*M_PI / _ITKgeo[layer].nLadders;
    _ITKgeo[layer].senRMin = pITKLayerLayout.distanceSensitive*CLHEP::cm;
    _ITKgeo[layer].supRMin = pITKLayerLayout.distanceSupport*CLHEP::cm;
    _ITKgeo[layer].length = pITKLayerLayout.zHalfSensitive*2.0*CLHEP::cm; // note: gear for historical reasons uses the halflength
    _ITKgeo[layer].width = pITKLayerLayout.widthSensitive*CLHEP::cm;
    _ITKgeo[layer].offset = pITKLayerLayout.offsetSensitive*CLHEP::cm;
    _ITKgeo[layer].senThickness = pITKLayerLayout.thicknessSensitive*CLHEP::cm;
    _ITKgeo[layer].supThickness = pITKLayerLayout.thicknessSupport*CLHEP::cm;
    _ITKgeo[layer].nSensorsPerLadder = pITKLayerLayout.sensorsPerLadder;
    _ITKgeo[layer].sensorLength = pITKLayerLayout.lengthSensor*CLHEP::cm;//_ITKgeo[layer].length / _ITKgeo[layer].nSensorsPerLadder;

    if (_isStripDetector) {
      _ITKgeo[layer].stripAngle = strip_angle_deg * M_PI/180 ;
    } else {
      _ITKgeo[layer].stripAngle = 0.0 ;
    }
    //std::cout << _ITKgeo[layer].nLadders << " " << _ITKgeo[layer].phi0 << " "<< _ITKgeo[layer].dphi << " " << _ITKgeo[layer].senRMin << " " << _ITKgeo[layer].supRMin << " "
    //	      << _ITKgeo[layer].length << " " << _ITKgeo[layer].width << " " << _ITKgeo[layer].offset << " " << _ITKgeo[layer].senThickness << " " << _ITKgeo[layer].supThickness << " "
    //	      << _ITKgeo[layer].nSensorsPerLadder << " " << _ITKgeo[layer].sensorLength << " " << pITKLayerLayout.lengthSensor << std::endl; 
  }
}
