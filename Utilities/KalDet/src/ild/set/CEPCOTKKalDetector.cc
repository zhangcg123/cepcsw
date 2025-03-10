
#include "kaldet/CEPCOTKKalDetector.h"

#include "kaldet/MaterialDataBase.h"

#include "kaldet/ILDParallelPlanarStripMeasLayer.h"


#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>

#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"

#include <gear/GEAR.h>
#include "gear/BField.h"
#include <gear/ZPlanarParameters.h>
#include <gear/ZPlanarLayerLayout.h>
#include "gearimpl/Util.h"

#include "CLHEP/Units/SystemOfUnits.h"
#include "DD4hep/DD4hepUnits.h"
#include "TMath.h"

#include "math.h"
#include <sstream>

// #include "streamlog/streamlog.h"

CEPCOTKKalDetector::CEPCOTKKalDetector( const gear::GearMgr& gearMgr, IGeomSvc* geoSvc )
: TVKalDetector(300) // SJA:FIXME initial size, 300 looks reasonable for ILD, though this would be better stored as a const somewhere
{
  
  
  // streamlog_out(DEBUG4) << "CEPCOTKKalDetector building OTK detector using GEAR " << std::endl ;
  
  MaterialDataBase::Instance().registerForService(gearMgr);
  
  TMaterial & air       = *MaterialDataBase::Instance().getMaterial("air");
  TMaterial & silicon   = *MaterialDataBase::Instance().getMaterial("silicon");
  TMaterial & carbon    = *MaterialDataBase::Instance().getMaterial("OTKBarrelSupportMaterial");
  
  if(geoSvc){
    setupGearGeom( geoSvc ) ;
  }
  else{
    setupGearGeom( gearMgr );
  }

  if (_isStripDetector) {
    // streamlog_out(DEBUG4) << "\t\t building OTK detector as STRIP Detector." << std::endl ;
  } else {
    // streamlog_out(DEBUG4) << "\t\t building OTK detector as PIXEL Detector." << std::endl ;
  }

  
  //--The Ladder structure (realistic ladder)--
  int nLadders;
  
//  Bool_t active = true;
  Bool_t dummy  = false;
  
  static const double eps_layer  = 1e-6; 
  static const double eps_sensor = 1e-8; 
  
  UTIL::BitField64 encoder( lcio::ILDCellID0::encoder_string ) ; 
  
  for (int layer=0; layer<_nLayers; ++layer) {
    nLadders = _OTKgeo[layer].nLadders;
    
    const double phi0 = _OTKgeo[layer].phi0;
    
    const double ladder_offset    = _OTKgeo[layer].supOffset;
    const double ladder_distance  = _OTKgeo[layer].supRMin;
    const double ladder_thickness = _OTKgeo[layer].supThickness;
    const double ladder_width     = _OTKgeo[layer].supWidth;
    const double ladder_length    = _OTKgeo[layer].supLength;
    
    const double sensitive_offset    = _OTKgeo[layer].senOffset;
    const double sensitive_distance  = _OTKgeo[layer].senRMin;
    const double sensitive_thickness = _OTKgeo[layer].senThickness;
    const double sensitive_width     = _OTKgeo[layer].senWidth;
    const double sensitive_length    = _OTKgeo[layer].senLength;

    if (ladder_distance+ladder_thickness-sensitive_distance>0)
      std::cout << "OTK ladder_distance =" << ladder_distance << " ladder_thickness = " << ladder_thickness << " sensitive_distance = " << sensitive_distance << std::endl; 

    double sign = ladder_offset > 0 ? 1 : -1;
    double nonoverlap_ladder_width     = ladder_width / 2.0;
    double nonoverlap_ladder_offset    = ladder_offset - sign * (ladder_width/2.0 - nonoverlap_ladder_width/2.0);
    double overlap_ladder_width        = ladder_width - nonoverlap_ladder_width;
    double overlap_ladder_offset       = ladder_offset + sign * (ladder_width/2.0 - overlap_ladder_width/2.0);
    double nonoverlap_sensitive_width  = sensitive_width / 2.0;
    double nonoverlap_sensitive_offset = sensitive_offset - sign * (sensitive_width/2.0 - nonoverlap_sensitive_width/2.0);
    double overlap_sensitive_width     = sensitive_width - nonoverlap_sensitive_width;
    double overlap_sensitive_offset    = sensitive_offset + sign * (sensitive_width/2.0 - overlap_sensitive_width/2.0);

    double currPhi;
    const double dphi = _OTKgeo[layer].dphi;
    
    const double stripAngle = pow(-1,layer) *_OTKgeo[layer].stripAngle;
    
    const int nsensors = _OTKgeo[layer].nSensorsPerLadder;
    
    const double sensor_length = _OTKgeo[layer].sensorLength;

    for (int ladder=0; ladder<nLadders; ++ladder) {
      currPhi = phi0 + (dphi * ladder);
      
      encoder.reset();  // reset to 0
      
      encoder[lcio::ILDCellID0::subdet] = lcio::ILDDetID::SET;
      encoder[lcio::ILDCellID0::side]   = 0;
      encoder[lcio::ILDCellID0::layer]  = layer;
      encoder[lcio::ILDCellID0::module] = ladder;
      
      double z_centre_support = 0.0;

      int nsort = sensitive_offset > 0 ? (nLadders - ladder)%nLadders : ladder;
      // check if the sensitive is inside or outside for the support 
      if (sensitive_distance < ladder_distance) {
	double gap_distance = sensitive_distance + sensitive_thickness + 1e-6;

        double sen_front_sorting_policy         = sensitive_distance + (5 * nsort + 0) * eps_layer;
        double sen_back_sorting_policy          = sensitive_distance + (5 * nsort + 2) * eps_layer;
	double sup_front_sorting_policy         = sensitive_distance + (5 * nsort + 3) * eps_layer;
        double sup_back_sorting_policy          = sensitive_distance + (5 * nsort + 4) * eps_layer;

	double overlap_sen_front_sorting_policy = sensitive_distance + (5 * nLadders + 0) * eps_layer;
        double overlap_sen_back_sorting_policy  = sensitive_distance + (5 * nLadders + 2) * eps_layer;
	double overlap_sup_front_sorting_policy = sensitive_distance + (5 * nLadders + 3) * eps_layer;
        double overlap_sup_back_sorting_policy  = sensitive_distance + (5 * nLadders + 4) * eps_layer;

	if (ladder == 0) {
	  // air - sensitive boundary
	  Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, nonoverlap_sensitive_width, sensitive_length,
					     nonoverlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenFront_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, overlap_sen_front_sorting_policy, overlap_sensitive_width, sensitive_length,
					     overlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenFront_overlap"));
	  // sensitive - air boundary
	  Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, nonoverlap_sensitive_width,
					     sensitive_length, nonoverlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenRear_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, overlap_sen_back_sorting_policy, overlap_sensitive_width,
					     sensitive_length, overlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenRear_overlap"));
	  // air - support boundary, very thin air gap added for different support and sensitive size
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, gap_distance, currPhi, _bZ, sen_back_sorting_policy+eps_layer*0.1, nonoverlap_ladder_width, ladder_length,
					     nonoverlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupFront_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, gap_distance, currPhi, _bZ, overlap_sup_front_sorting_policy, overlap_ladder_width, ladder_length,
					     overlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupFront_overlap"));
	  // support - air boundary
	  Add(new ILDParallelPlanarMeasLayer(carbon, air, ladder_distance+ladder_thickness, currPhi, _bZ, sup_back_sorting_policy, nonoverlap_ladder_width, ladder_length,
					     nonoverlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1 ,"OTKSupRear_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(carbon, air, ladder_distance+ladder_thickness, currPhi, _bZ, overlap_sup_back_sorting_policy, overlap_ladder_width, ladder_length,
					     overlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupRear_overlap"));
	}
	else {
	  // air - sensitive boundary
	  Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, sensitive_width, sensitive_length,
					     sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenFront"));
	  // sensitive - air boundary
	  Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, sensitive_width, sensitive_length,
					     sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenRear"));
	  // air - support boundary, very thin air gap added for different support and sensitive size
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, gap_distance, currPhi, _bZ, sup_front_sorting_policy, ladder_width, ladder_length,
					     ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupFront"));
	  // support - air boundary
	  Add(new ILDParallelPlanarMeasLayer(carbon, air, ladder_distance+ladder_thickness, currPhi, _bZ, sup_back_sorting_policy, ladder_width, ladder_length,
					     ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupRear"));
        }

        for (int isensor=0; isensor<nsensors; ++isensor) {
          encoder[lcio::ILDCellID0::sensor] = isensor;
          int CellID = encoder.lowWord();
          
          double measurement_plane_sorting_policy         = sensitive_distance + (5 * nsort + 1) * eps_layer + eps_sensor * isensor;
	  double overlap_measurement_plane_sorting_policy = sensitive_distance + (5 * nLadders + 1) * eps_layer + eps_sensor * isensor;
          
          double z_centre_sensor = -0.5*sensitive_length + (0.5*sensor_length) + (isensor*sensor_length) ;

          if (_isStripDetector) {
            // measurement plane defined as the middle of the sensitive volume
	    if (ladder == 0) {
	      Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
						      measurement_plane_sorting_policy, nonoverlap_sensitive_width, sensor_length, nonoverlap_sensitive_offset,
						      z_centre_sensor, sensitive_offset, stripAngle, CellID, "OTKStripMeaslayer_nonoverlap"));
	      Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
						      overlap_measurement_plane_sorting_policy, overlap_sensitive_width, sensor_length, overlap_sensitive_offset,
						      z_centre_sensor, sensitive_offset, stripAngle, CellID, "OTKStripMeaslayer_overlap"));
	    }
	    else {
	      Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
						      measurement_plane_sorting_policy, sensitive_width, sensor_length, sensitive_offset,
						      z_centre_sensor, sensitive_offset, stripAngle, CellID, "OTKStripMeaslayer"));
	    }
          }
	  else {
            // measurement plane defined as the middle of the sensitive volume
	    if (ladder == 0) {
	      Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
						 measurement_plane_sorting_policy, nonoverlap_sensitive_width, sensor_length, nonoverlap_sensitive_offset,
						 z_centre_sensor, sensitive_offset, true, CellID, "OTKMeaslayer_nonoverlap"));
	      Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
						 overlap_measurement_plane_sorting_policy, overlap_sensitive_width, sensor_length, overlap_sensitive_offset,
						 z_centre_sensor, sensitive_offset, true, CellID, "OTKMeaslayer_overlap"));
	    }
	    else {
	      Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
						 measurement_plane_sorting_policy, sensitive_width, sensor_length, sensitive_offset,
						 z_centre_sensor, sensitive_offset, true, CellID, "OTKMeaslayer"));
	    }
          }
	  //std::cout << "CEPCOTKKalDetector add surface with CellID = " << CellID
	  //          << " layer = " << layer << " ladder = " << ladder << " sensor = " << isensor << std::endl;
        }
      }
      else {
	double gap_distance = sensitive_distance - 1e-6;

        double sup_front_sorting_policy         = sensitive_distance + (5 * nsort + 0) * eps_layer;
	double sup_back_sorting_policy          = sensitive_distance + (5 * nsort + 1) * eps_layer;
        double sen_front_sorting_policy         = sensitive_distance + (5 * nsort + 2) * eps_layer;
        double sen_back_sorting_policy          = sensitive_distance + (5 * nsort + 4) * eps_layer ;
	double overlap_sup_front_sorting_policy = sensitive_distance + (5 * nLadders + 0) * eps_layer;
	double overlap_sup_back_sorting_policy  = sensitive_distance + (5 * nLadders + 1) * eps_layer;
        double overlap_sen_front_sorting_policy = sensitive_distance + (5 * nLadders + 2) * eps_layer;
        double overlap_sen_back_sorting_policy  = sensitive_distance + (5 * nLadders + 4) * eps_layer;

	if (ladder == 0) {
	  // air - support boundary
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, ladder_distance, currPhi, _bZ, sup_front_sorting_policy, nonoverlap_ladder_width, ladder_length,
					     nonoverlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupFront_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, ladder_distance, currPhi, _bZ, overlap_sup_front_sorting_policy, overlap_ladder_width, ladder_length,
					     overlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupFront_overlap"));
	  // support - air boundary, very thin air gap added for different support and sensitive size
	  Add(new ILDParallelPlanarMeasLayer(carbon, air, gap_distance, currPhi, _bZ, sup_back_sorting_policy, nonoverlap_ladder_width, ladder_length,
					     nonoverlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupRear_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(carbon, air, gap_distance, currPhi, _bZ, overlap_sup_back_sorting_policy, overlap_ladder_width, ladder_length,
					     overlap_ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupRear_overlap"));
	  // air boundary - sensitive
	  Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, nonoverlap_sensitive_width, sensitive_length,
					     nonoverlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenFront_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, overlap_sen_front_sorting_policy, overlap_sensitive_width, sensitive_length,
					     overlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenFront_overlap"));
	  // sensitive - air boundary
	  Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, nonoverlap_sensitive_width,
					     sensitive_length, nonoverlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenRear_nonoverlap"));
	  Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, overlap_sen_back_sorting_policy, overlap_sensitive_width,
					     sensitive_length, overlap_sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenRear_overlap"));
	}
	else {
	  // air - support boundary
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, ladder_distance, currPhi, _bZ, sup_front_sorting_policy, ladder_width, ladder_length,
					     ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupFront"));
	  // support - air boundary, very thin air gap added for different support and sensitive size
	  Add(new ILDParallelPlanarMeasLayer(carbon, air, gap_distance, currPhi, _bZ, sup_back_sorting_policy, ladder_width, ladder_length,
					     ladder_offset, z_centre_support, ladder_offset, dummy, -1, "OTKSupRear"));
	  // air boundary - sensitive
	  Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, sensitive_width, sensitive_length,
					     sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenFront"));
	  // sensitive - air boundary
	  Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, sensitive_width, sensitive_length,
					     sensitive_offset, z_centre_support, sensitive_offset, dummy, -1, "OTKSenRear"));
	}

        for (int isensor=0; isensor<nsensors; ++isensor) {
          encoder[lcio::ILDCellID0::sensor] = isensor;
          int CellID = encoder.lowWord() ;
          
          double measurement_plane_sorting_policy         = sensitive_distance  + (5 * nsort + 3) * eps_layer + eps_sensor * isensor;
	  double overlap_measurement_plane_sorting_policy = sensitive_distance  + (5 * nLadders + 3) * eps_layer + eps_sensor * isensor;

          double z_centre_sensor = -0.5*sensitive_length + (0.5*sensor_length) + (isensor*sensor_length) ;
          
          if (_isStripDetector) {
            // measurement plane defined as the middle of the sensitive volume
	    if (ladder == 0) {
              Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, measurement_plane_sorting_policy,
						      nonoverlap_sensitive_width, sensor_length, nonoverlap_sensitive_offset,
                                                      z_centre_sensor, sensitive_offset, stripAngle, CellID, "OTKStripMeaslayer_nonoverlap"));
              Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, overlap_measurement_plane_sorting_policy,
						      overlap_sensitive_width, sensor_length, overlap_sensitive_offset,
                                                      z_centre_sensor, sensitive_offset, stripAngle, CellID, "OTKStripMeaslayer_overlap"));
            }
            else {
              Add(new ILDParallelPlanarStripMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
                                                      measurement_plane_sorting_policy, sensitive_width, sensor_length, sensitive_offset,
                                                      z_centre_sensor, sensitive_offset, stripAngle, CellID, "OTKStripMeaslayer"));
            }
          }
	  else {
            // measurement plane defined as the middle of the sensitive volume
	    if (ladder == 0) {
              Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, measurement_plane_sorting_policy,
						 nonoverlap_sensitive_width, sensor_length, nonoverlap_sensitive_offset,
                                                 z_centre_sensor, sensitive_offset, true, CellID, "OTKMeaslayer_nonoverlap"));
              Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ, overlap_measurement_plane_sorting_policy,
						 overlap_sensitive_width, sensor_length, overlap_sensitive_offset,
                                                 z_centre_sensor, sensitive_offset, true, CellID, "OTKMeaslayer_overlap"));
            }
            else {
              Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*0.5, currPhi, _bZ,
                                                 measurement_plane_sorting_policy, sensitive_width, sensor_length, sensitive_offset,
                                                 z_centre_sensor, sensitive_offset, true, CellID, "OTKMeaslayer"));
            }
          }
	  //std::cout << "CEPCOTKKalDetector add surface with CellID = " << CellID
          //          << " layer = " << layer << " ladder = " << ladder << " sensor = " << isensor << std::endl ;
        }
      }
    }    
  }
  
  SetOwner();                   
}



void CEPCOTKKalDetector::setupGearGeom( const gear::GearMgr& gearMgr ){
  
  const gear::ZPlanarParameters& pOTKDetMain = gearMgr.getSETParameters();
  const gear::ZPlanarLayerLayout& pOTKLayerLayout = pOTKDetMain.getZPlanarLayerLayout();

  int hasOTKBarrel = pOTKDetMain.getIntVal("OTKBarrel");
  
  _bZ = gearMgr.getBField().at( gear::Vector3D( 0.,0.,0.)  ).z() ;
  
  _nLayers = pOTKLayerLayout.getNLayers(); 
  _OTKgeo.resize(_nLayers);
  
  bool n_sensors_per_ladder_present = true;
  
  try {

    std::vector<int> v = pOTKDetMain.getIntVals("n_sensors_per_ladder");

  } catch (gear::UnknownParameterException& e) {

    n_sensors_per_ladder_present = false;

  }

  double strip_angle_deg = 0.0;
  
  try {
    strip_angle_deg = pOTKDetMain.getDoubleVal("strip_angle_deg");
  }
  catch (gear::UnknownParameterException& e) {}

  if (strip_angle_deg==0) _isStripDetector = false;
  else                    _isStripDetector = true;
  //SJA:FIXME: for now the support is taken as the same size the sensitive
  //           if this is not done then the exposed areas of the support would leave a carbon - air boundary,
  //           which if traversed in the reverse direction to the next boundary then the track would be propagated through carbon
  //           for a significant distance 
  //std::cout << "=============OTK strip angle: " << strip_angle_deg << "==============" << std::endl;
  for( int layer=0; layer < _nLayers; ++layer){
    _OTKgeo[layer].nLadders = pOTKLayerLayout.getNLadders(layer); 
    _OTKgeo[layer].phi0 = pOTKLayerLayout.getPhi0(layer); 
    _OTKgeo[layer].dphi = 2*M_PI / _OTKgeo[layer].nLadders; 
    _OTKgeo[layer].senRMin = pOTKLayerLayout.getSensitiveDistance(layer); 
    _OTKgeo[layer].supRMin = pOTKLayerLayout.getLadderDistance(layer); 
    _OTKgeo[layer].senLength = pOTKLayerLayout.getSensitiveLength(layer)*2.0; // note: gear for historical reasons uses the halflength
    _OTKgeo[layer].supLength = pOTKLayerLayout.getLadderLength(layer)*2.0;
    _OTKgeo[layer].senWidth = pOTKLayerLayout.getSensitiveWidth(layer);
    _OTKgeo[layer].supWidth = pOTKLayerLayout.getLadderWidth(layer);
    _OTKgeo[layer].senOffset = pOTKLayerLayout.getSensitiveOffset(layer);
    _OTKgeo[layer].supOffset = pOTKLayerLayout.getLadderOffset(layer);
    _OTKgeo[layer].senThickness = pOTKLayerLayout.getSensitiveThickness(layer); 
    _OTKgeo[layer].supThickness = pOTKLayerLayout.getLadderThickness(layer); 

    if (n_sensors_per_ladder_present) {
      _OTKgeo[layer].nSensorsPerLadder = pOTKDetMain.getIntVals("n_sensors_per_ladder")[layer];
    }
    else{
      _OTKgeo[layer].nSensorsPerLadder = 1 ;
    }
    
    _OTKgeo[layer].sensorLength = _OTKgeo[layer].senLength / _OTKgeo[layer].nSensorsPerLadder;
     
    if (_isStripDetector) {
      _OTKgeo[layer].stripAngle = strip_angle_deg * M_PI/180 ;
    } else {
      _OTKgeo[layer].stripAngle = 0.0 ;
    }
    //std::cout << _OTKgeo[layer].nLadders << " " << _OTKgeo[layer].phi0 << " "<< _OTKgeo[layer].dphi << " " << _OTKgeo[layer].senRMin << " " << _OTKgeo[layer].supRMin << " "
    //          << _OTKgeo[layer].length << " " << _OTKgeo[layer].width << " " << _OTKgeo[layer].offset << " " << _OTKgeo[layer].senThickness << " " << _OTKgeo[layer].supThickness << " "
    //          << _OTKgeo[layer].nSensorsPerLadder << " " << _OTKgeo[layer].sensorLength << std::endl;
    // streamlog_out(DEBUG0) << " layer  = " << layer << std::endl;
    // streamlog_out(DEBUG0) << " nSensorsPerLadder  = " << _OTKgeo[layer].nSensorsPerLadder << std::endl;
    // streamlog_out(DEBUG0) << " sensorLength  = " << _OTKgeo[layer].sensorLength << std::endl;
    // streamlog_out(DEBUG0) << " stripAngle  = " << _OTKgeo[layer].stripAngle << std::endl;
    // streamlog_out(DEBUG0) << " _isStripDetector  = " << _isStripDetector << std::endl;

  }
}

void CEPCOTKKalDetector::setupGearGeom( IGeomSvc* geoSvc ){
  dd4hep::DetElement world = geoSvc->getDD4HepGeo();
  dd4hep::DetElement set;
  const std::map<std::string, dd4hep::DetElement>& subs = world.children();
  for(std::map<std::string, dd4hep::DetElement>::const_iterator it=subs.begin();it!=subs.end();it++){
    if(it->first!="OTK") continue;
    set = it->second;
  }
  dd4hep::rec::ZPlanarData* setData = nullptr;
  try{
    setData = set.extension<dd4hep::rec::ZPlanarData>();
  }
  catch(std::runtime_error& e){
    std::cout << e.what() << " " << setData << std::endl;
    throw GaudiException(e.what(), "FATAL", StatusCode::FAILURE);
  }

  const dd4hep::Direction& field = geoSvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
  _bZ = field.z()/dd4hep::tesla;

  std::vector<dd4hep::rec::ZPlanarData::LayerLayout>& setlayers = setData->layers;
  _nLayers = setlayers.size();
  _OTKgeo.resize(_nLayers);

  double strip_angle_deg = setData->angleStrip/CLHEP::degree;
  if (strip_angle_deg==0) _isStripDetector = false;
  else                    _isStripDetector = true;
  //std::cout << "=============OTK strip angle: " << strip_angle_deg << "==============" << std::endl;
  for( int layer=0; layer < _nLayers; ++layer){
    dd4hep::rec::ZPlanarData::LayerLayout& pOTKLayerLayout = setlayers[layer];

    _OTKgeo[layer].nLadders = pOTKLayerLayout.ladderNumber;
    _OTKgeo[layer].phi0 = pOTKLayerLayout.phi0;
    _OTKgeo[layer].dphi = 2*M_PI / _OTKgeo[layer].nLadders;
    _OTKgeo[layer].senRMin = pOTKLayerLayout.distanceSensitive*CLHEP::cm;
    _OTKgeo[layer].supRMin = pOTKLayerLayout.distanceSupport*CLHEP::cm;
    _OTKgeo[layer].senLength = pOTKLayerLayout.zHalfSensitive*2.0*CLHEP::cm; // note: gear for historical reasons uses the halflength
    _OTKgeo[layer].supLength = pOTKLayerLayout.zHalfSupport*2.0*CLHEP::cm;
    _OTKgeo[layer].senWidth = pOTKLayerLayout.widthSensitive*CLHEP::cm;
    _OTKgeo[layer].supWidth = pOTKLayerLayout.widthSupport*CLHEP::cm;
    _OTKgeo[layer].senOffset = pOTKLayerLayout.offsetSensitive*CLHEP::cm;
    _OTKgeo[layer].supOffset = pOTKLayerLayout.offsetSupport*CLHEP::cm;
    _OTKgeo[layer].senThickness = pOTKLayerLayout.thicknessSensitive*CLHEP::cm;
    _OTKgeo[layer].supThickness = pOTKLayerLayout.thicknessSupport*CLHEP::cm;
    _OTKgeo[layer].nSensorsPerLadder = pOTKLayerLayout.sensorsPerLadder;
    _OTKgeo[layer].sensorLength = _OTKgeo[layer].senLength / _OTKgeo[layer].nSensorsPerLadder;

    if (_isStripDetector) {
      _OTKgeo[layer].stripAngle = strip_angle_deg * M_PI/180 ;
    } else {
      _OTKgeo[layer].stripAngle = 0.0 ;
    }
    //std::cout << _OTKgeo[layer].nLadders << " " << _OTKgeo[layer].phi0 << " "<< _OTKgeo[layer].dphi << " " << _OTKgeo[layer].senRMin << " " << _OTKgeo[layer].supRMin << " "
    //        << _OTKgeo[layer].length << " " << _OTKgeo[layer].width << " " << _OTKgeo[layer].offset << " " << _OTKgeo[layer].senThickness << " " << _OTKgeo[layer].supThickness << " "
    //        << _OTKgeo[layer].nSensorsPerLadder << " " << _OTKgeo[layer].sensorLength << " " << pOTKLayerLayout.lengthSensor << std::endl;
  }
}
