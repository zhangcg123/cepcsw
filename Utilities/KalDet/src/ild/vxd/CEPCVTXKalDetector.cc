
#include "kaldet/CEPCVTXKalDetector.h"
#include "kaldet/MaterialDataBase.h"

#include "kaldet/ILDParallelPlanarMeasLayer.h"
#include "kaldet/ILDCylinderMeasLayer.h"
#include "kaldet/CEPCCylinderMeasLayer.h"
#include "kaldet/ILDDiscMeasLayer.h"

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>

#include "DetInterface/IGeomSvc.h"
#include "DD4hep/Detector.h"
#include "DDRec/DetectorData.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "DD4hep/DD4hepUnits.h"

#include <gear/GEAR.h>
#include "gear/BField.h"
#include <gearimpl/ZPlanarParametersImpl.h>
#include <gear/VXDParameters.h>
#include <gear/VXDLayerLayout.h>
#include "gearimpl/Util.h"
#include "DetInterface/IGeomSvc.h"

#include "TMath.h"

#include "math.h"
#include <sstream>

// #include "streamlog/streamlog.h"

CEPCVTXKalDetector::CEPCVTXKalDetector( const gear::GearMgr& gearMgr, IGeomSvc* geoSvc )
: TVKalDetector(300) // SJA:FIXME initial size, 300 looks reasonable for ILD, though this would be better stored as a const somewhere
{
  
  // streamlog_out(DEBUG1) << "CEPCVTXKalDetector building VXD detector using GEAR " << std::endl ;
  
  MaterialDataBase::Instance().registerForService(gearMgr, geoSvc);
  
  TMaterial & air       = *MaterialDataBase::Instance().getMaterial("air");
  TMaterial & silicon   = *MaterialDataBase::Instance().getMaterial("silicon");
  TMaterial & carbon    = *MaterialDataBase::Instance().getMaterial("VXDSupportMaterial");

  //TMaterial & aluminium = *MaterialDataBase::Instance().getMaterial("aluminium");
  
  if(geoSvc){
    this->setupGearGeom(geoSvc) ;
  }
  else{
    this->setupGearGeom(gearMgr) ;
  }

  //--The Ladder structure (realistic ladder)--
  int nLadders;
  
  Bool_t active = true;
  Bool_t dummy  = false;
  
  const double eps      = 1e-6;
  const double edge_eps = 1e-6;
  
  UTIL::BitField64 encoder( lcio::ILDCellID0::encoder_string ) ; 

  double front_distance = 0;
  for (int layer=0; layer<_nLayers[0]; ++layer) {
    nLadders = _VXDgeo[layer].nLadders ;
    
    double phi0 = _VXDgeo[layer].phi0 ;
    
    double ladder_distance       = _VXDgeo[layer].supRMin;
    double ladder_thickness      = _VXDgeo[layer].supThickness;
    double ladder_outer_distance = ladder_distance + ladder_thickness;

    if (layer%2 == 0) front_distance = ladder_distance;
    
    double sensitive_distance       = _VXDgeo[layer].senRMin;
    double sensitive_thickness      = _VXDgeo[layer].senThickness;
    double sensitive_outer_distance = sensitive_distance + sensitive_thickness;
    
    double width = _VXDgeo[layer].width ;
    double length = _VXDgeo[layer].length;
    double offset = _VXDgeo[layer].offset;

    // FIXME: generally, more then half nonoverlap
    double pos_xi_nonoverlap_width = width / 2.0; //(2.0 * (( width / 2.0 ) - fabs(offset)));
    double nonoverlap_region_offset = offset - offset/fabs(offset) * (width/2.0 - pos_xi_nonoverlap_width/2.0);
    
    double currPhi;
    double dphi = _VXDgeo[layer].dphi ;
    
    static const double z_offset = 0.0; // all VXD planes are centred at zero 
    
    for (int ladder=0; ladder<nLadders; ++ladder) {
      
      currPhi = phi0 + (dphi * ladder);
      
      encoder.reset() ;  // reset to 0
      
      encoder[lcio::ILDCellID0::subdet] = lcio::ILDDetID::VXD ;
      encoder[lcio::ILDCellID0::side] = 0 ;
      encoder[lcio::ILDCellID0::layer]  = layer + _nLayers[1];
      encoder[lcio::ILDCellID0::module] = ladder ;
      encoder[lcio::ILDCellID0::sensor] = 0 ;
      
      int CellID = encoder.lowWord() ;

      int nsort = offset > 0 ? (nLadders - ladder)%nLadders : ladder;
      
      // even layers have the senstive side facing the IP
      if(layer%2 == 0 ){ // overlap section of ladder0 is defined after the last ladder,

        double sen_front_sorting_policy         = ladder_distance + (100 * nsort + 0) * eps ;
        double measurement_plane_sorting_policy = ladder_distance + (100 * nsort + 1) * eps ;
        double sen_back_sorting_policy          = ladder_distance + (100 * nsort + 2) * eps ;
        double sup_back_sorting_policy          = ladder_distance + (100 * nsort + 3) * eps ;
        
        if(ladder==0){   // bacause overlap section of ladder0 is further outer than the last ladder.
          
          // streamlog_out(DEBUG0) << "CEPCVTXKalDetector add surface with CellID = "
          // << CellID
          // << std::endl ;

          // non overlapping region
          // air - sensitive boundary
          Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, dummy, -1, "VXDSenFront_non_overlap_even"));
          // measurement plane defined as the middle of the sensitive volume  - unless "relative_position_of_measurement_surface" parameter given in GEAR
          Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*_relative_position_of_measurement_surface,
					     currPhi, _bZ, measurement_plane_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, active, CellID, "VXDMeasLayer_non_overlap_even"));
          // sensitive - support boundary
          Add(new ILDParallelPlanarMeasLayer(silicon, carbon, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, dummy, -1, "VXDSenSuppportIntf_non_overlap_even"));
          // support - air boundary
          Add(new ILDParallelPlanarMeasLayer(carbon, air, ladder_distance+ladder_thickness, currPhi, _bZ, sup_back_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, dummy, -1, "VXDSupRear_non_overlap_even"));
          
          // overlapping region
          double overlap_region_width  = width - pos_xi_nonoverlap_width ;
          double overlap_region_offset = offset + offset/fabs(offset) * (width/2.0 - overlap_region_width/2.0);// -(overlap_region_width/2.0) - (pos_xi_nonoverlap_width)/2.0 ;
          
          // overlap sorting policy uses nLadders as the overlapping "ladder" is the order i.e. there will now be nLadders+1 
          double overlap_front_sorting_policy                = ladder_distance + (100* nLadders+0) * eps;
          double overlap_measurement_plane_sorting_policy    = ladder_distance + (100* nLadders+1) * eps;
          double overlap_back_sorting_policy                 = ladder_distance + (100* nLadders+2) * eps;
          double overlap_sup_back_sorting_policy             = ladder_distance + (100* nLadders+3) * eps;
          
          // streamlog_out(DEBUG0) << "CEPCVTXKalDetector add surface with CellID = "
          // << CellID
          // << std::endl ;
          
          // air - sensitive boundary
          Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, overlap_front_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, dummy, -1, "VXDSenFront_overlap_even"));
          // measurement plane defined as the middle of the sensitive volume  - unless "relative_position_of_measurement_surface" parameter given in GEAR
          Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*_relative_position_of_measurement_surface,
					     currPhi, _bZ, overlap_measurement_plane_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, active, CellID, "VXDMeasLayer_overlap_even"));
          // sensitive - support boundary
          Add(new ILDParallelPlanarMeasLayer(silicon, carbon, sensitive_distance+sensitive_thickness, currPhi, _bZ, overlap_back_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, dummy, -1, "VXDSenSuppportIntf_overlap_even"));
          // support - air boundary
          Add(new ILDParallelPlanarMeasLayer(carbon, air, ladder_distance+ladder_thickness, currPhi, _bZ, overlap_sup_back_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, dummy, -1, "VXDSupRear_overlap_even"));
        }
        else{
          // streamlog_out(DEBUG0) << "CEPCVTXKalDetector (ILDParallelPlanarMeasLayer) add surface with CellID = "
          // << CellID
          // << std::endl ;                                        
          
          // air - sensitive boundary
          Add(new ILDParallelPlanarMeasLayer(air, silicon, sensitive_distance, currPhi, _bZ, sen_front_sorting_policy, width, length,
					     offset, z_offset, offset, dummy, -1, "VXDSenFront_even"));
          // measurement plane defined as the middle of the sensitive volume  - unless "relative_position_of_measurement_surface" parameter given in GEAR - even layers face outwards ! 
          Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*(1.-_relative_position_of_measurement_surface),
					     currPhi, _bZ, measurement_plane_sorting_policy, width, length, offset, z_offset, offset, active, CellID, "VXDMeaslayer_even"));
          // sensitive - support boundary 
          Add(new ILDParallelPlanarMeasLayer(silicon, carbon, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, width, length,
					     offset, z_offset, offset, dummy, -1, "VXDSenSuppportIntf_even"));
          // support - air boundary
          Add(new ILDParallelPlanarMeasLayer(carbon, air, ladder_distance+ladder_thickness, currPhi, _bZ, sup_back_sorting_policy, width, length,
					     offset, z_offset, offset, dummy, -1, "VXDSupRear_even"));
        }        
      }
      else{ // counting from 0, odd numbered layers are placed with the support closer to the IP than the sensitive
        
        double sup_forward_sorting_policy        = front_distance + (100 * nsort + 10) * eps;
        double sup_back_sorting_policy           = front_distance + (100 * nsort + 11) * eps;
        double measurement_plane_sorting_policy  = front_distance + (100 * nsort + 12) * eps;
        double sen_back_sorting_policy           = front_distance + (100 * nsort + 13) * eps;
        
	if (ladder==0) {
          // non overlapping region
          // air - support boundary
          Add(new ILDParallelPlanarMeasLayer(air, carbon, ladder_distance, currPhi, _bZ, sup_forward_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, dummy, -1, "VXDSupFront_non_overlap_odd"));
          // support - sensitive boundary
          Add(new ILDParallelPlanarMeasLayer(carbon, silicon, ladder_distance+ladder_thickness, currPhi, _bZ, sup_back_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, dummy, -1, "VXDSenSuppportIntf_non_overlap_odd"));
          // measurement plane defined as the middle of the sensitive volume  - unless "relative_position_of_measurement_surface" parameter given in GEAR
          Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*(1.-_relative_position_of_measurement_surface),
					     currPhi, _bZ, measurement_plane_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, active, CellID, "VXDMeasLayer_non_overlap_odd"));
	  // sensitive - air boundary
          Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, sen_back_sorting_policy, pos_xi_nonoverlap_width, length,
					     nonoverlap_region_offset, z_offset, offset, dummy, -1, "VXDSenRear_non_overlap_odd"));
          // overlapping region
          double overlap_region_width  = width - pos_xi_nonoverlap_width ;
          double overlap_region_offset = offset + offset/fabs(offset) * (width/2.0 - overlap_region_width/2.0);

          // overlap sorting policy uses nLadders as the overlapping "ladder" is the order i.e. there will now be nLadders+1
          double overlap_sup_front_sorting_policy         = front_distance + (100* nLadders+10) * eps;
          double overlap_sup_back_sorting_policy          = front_distance + (100* nLadders+11) * eps;
          double overlap_measurement_plane_sorting_policy = front_distance + (100* nLadders+12) * eps;
          double overlap_sen_back_sorting_policy          = front_distance + (100* nLadders+13) * eps;

	  // air - spport boundary
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, ladder_distance, currPhi, _bZ, overlap_sup_front_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, dummy, -1, "VXDSupFront_overlap_odd"));
	  // support - sensitive boundary
          Add(new ILDParallelPlanarMeasLayer(carbon, silicon, ladder_distance+ladder_thickness, currPhi, _bZ, overlap_sup_back_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, dummy, -1, "VXDSenSuppportIntf_overlap_odd"));
          // measurement plane defined as the middle of the sensitive volume  - unless "relative_position_of_measurement_surface" parameter given in GEAR
          Add(new ILDParallelPlanarMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*(1.-_relative_position_of_measurement_surface),
					     currPhi, _bZ, overlap_measurement_plane_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, active, CellID, "VXDMeasLayer_overlap_odd"));
          // sensitive - air boundary
          Add(new ILDParallelPlanarMeasLayer(silicon, air, sensitive_distance+sensitive_thickness, currPhi, _bZ, overlap_sen_back_sorting_policy, overlap_region_width, length,
					     overlap_region_offset, z_offset, offset, dummy, -1, "VXDSenRear_overlap_odd"));
        }
	else {
	  // air - support boundary
	  Add(new ILDParallelPlanarMeasLayer(air, carbon, ladder_distance, currPhi, _bZ, sup_forward_sorting_policy, width, length,
					     offset, z_offset, offset, dummy, -1, "VXDSupFront_odd"));
	  // support - sensitive boundary
	  Add(new ILDParallelPlanarMeasLayer(carbon, silicon, (ladder_distance+ladder_thickness), currPhi, _bZ, sup_back_sorting_policy, width, length,
					     offset, z_offset, offset, dummy, -1, "VXDSenSuppportIntf_odd"));
	  // measurement plane defined as the middle of the sensitive volume
	  Add(new ILDParallelPlanarMeasLayer(silicon, silicon, (sensitive_distance+sensitive_thickness*(1.-_relative_position_of_measurement_surface)),
					     currPhi, _bZ, measurement_plane_sorting_policy, width, length,
					     offset, z_offset, offset, active, CellID, "VXDMeaslayer_odd"));
	  // sensitive air - sensitive boundary
	  Add(new ILDParallelPlanarMeasLayer(silicon, air, (sensitive_distance+sensitive_thickness), currPhi, _bZ, sen_back_sorting_policy, width, length,
					     offset, z_offset, offset, dummy, -1, "VXDSenRear_odd"));
	}
      }
    }
    if (layer%2 == 1) {
      double redge = sqrt(sensitive_outer_distance*sensitive_outer_distance + (fabs(offset) + 0.5*width) * (fabs(offset) + 0.5*width)) + edge_eps;
      Add(new ILDCylinderMeasLayer(air, air, redge, 0.5*length, 0, 0, 0, _bZ, dummy, -1, "VXDOuterEdge"));
    }
  }

  if (_nLayers[1]>0) {
   TMaterial & bentmat   = *MaterialDataBase::Instance().getMaterial("VXDBentSupportMaterial");
   for (int layer=0; layer<_nLayers[1]; ++layer) {
    //std::cout << "add stitching ... " << layer << std::endl;
    double phi0 = _STTgeo[layer].phi0;
    double width = _STTgeo[layer].width;

    double ladder_distance = _STTgeo[layer].supRMin;
    double ladder_thickness = _STTgeo[layer].supThickness;

    double sensitive_distance = _STTgeo[layer].senRMin;
    double sensitive_thickness = _STTgeo[layer].senThickness;

    //std::cout << "sensor radius: " << sensitive_distance << " sensitive_thickness: " << sensitive_thickness << std::endl;
    //double width = _STTgeo[layer].width;
    double halfz = _STTgeo[layer].length/2.0;

    double currPhi;
    double dphi = _STTgeo[layer].dphi ;

    static const double z_offset = 0.0; // all VXD planes are centred at zero
    double x0(0), y0(0), z0(0);

    for (int im=0; im<2; im++) {
      currPhi = phi0 + (dphi * im);

      encoder.reset() ;  // reset to 0
      encoder[lcio::ILDCellID0::subdet] = lcio::ILDDetID::VXD;
      encoder[lcio::ILDCellID0::side] = 0;
      encoder[lcio::ILDCellID0::layer]  = layer;
      encoder[lcio::ILDCellID0::module] = im;
      encoder[lcio::ILDCellID0::sensor] = 0;

      int CellID = encoder.lowWord() ;

      if (im==1) {
	sensitive_distance += _STTgeo[layer].rgap;
	ladder_distance += _STTgeo[layer].rgap;
      }

      // air - sensitive boundary
      Add(new CEPCCylinderMeasLayer(air, silicon, sensitive_distance, halfz, currPhi, width, x0, y0, z0, _bZ, dummy, -1, "STTMeasL_0"));
      // measurement plane defined as the middle of the sensitive volume
      // - unless "relative_position_of_measurement_surface" parameter given in GEAR - even layers face outwards !
      Add(new CEPCCylinderMeasLayer(silicon, silicon, sensitive_distance+sensitive_thickness*(1.-_relative_position_of_measurement_surface),
				    halfz, currPhi, width, x0, y0, z0, _bZ, active, CellID, "STTMeaslayer_0"));
      // sensitive - support boundary
      Add(new CEPCCylinderMeasLayer(silicon, bentmat, sensitive_distance+sensitive_thickness, halfz, currPhi, width, x0, y0, z0, _bZ, dummy, -1, "STTSenSuppportIntf_0"));
      // support - air boundary
      Add(new CEPCCylinderMeasLayer(bentmat, air, ladder_distance+ladder_thickness, halfz, currPhi, width, x0, y0, z0, _bZ, dummy, -1, "STTSupRear_0" )) ;
    }
    Add(new ILDCylinderMeasLayer(air, air, ladder_distance+ladder_thickness+edge_eps, halfz, x0, y0, z0, _bZ, dummy, -1, "STTOuterEdge"));
   }
  }

  if (_shellInnerR > 0 && _shellOuterR > _shellInnerR) {
    TMaterial& shell = *MaterialDataBase::Instance().getMaterial("VXDShellMaterial");

    Add(new ILDCylinderMeasLayer(air, shell, _shellInnerR, _shellHalfZ, 0, 0, 0, _bZ, dummy, -1, "VXDShellInnerWall"));
    Add(new ILDCylinderMeasLayer(shell, air, _shellOuterR, _shellHalfZ, 0, 0, 0, _bZ, dummy, -1, "VXDShellOuterWall"));

    TVector3 normal(0, 0, 1);
    TVector3 xc(0.0, 0.0, _shellHalfZ);
    Add(new ILDDiscMeasLayer(shell, air,  xc,  normal, _bZ, _shellOuterR + eps, _shellInnerR, _shellOuterR, dummy, -1, "VXDShellSidePositiveZ"));
    Add(new ILDDiscMeasLayer(shell, air, -xc, -normal, _bZ, _shellOuterR + eps, _shellInnerR, _shellOuterR, dummy, -1, "VXDShellSideNegativeZ"));
  }

  SetOwner();                   
}

void CEPCVTXKalDetector::setupGearGeom( const gear::GearMgr& gearMgr ){
  const gear::VXDParameters& pVXDDetMain = gearMgr.getVXDParameters();
  const gear::VXDLayerLayout& pVXDLayerLayout = pVXDDetMain.getVXDLayerLayout();

  _bZ = gearMgr.getBField().at( gear::Vector3D( 0.,0.,0.)  ).z() ;

  _shellInnerR = pVXDDetMain.getShellInnerRadius();
  _shellOuterR = pVXDDetMain.getShellOuterRadius();
  _shellHalfZ  = pVXDDetMain.getShellHalfLength();

  _nLayers[0] = pVXDLayerLayout.getNLayers();
  _VXDgeo.resize(_nLayers[0]);

  for( int layer=0; layer < _nLayers[0]; ++layer){
    _VXDgeo[layer].nLadders = pVXDLayerLayout.getNLadders(layer);
    _VXDgeo[layer].phi0 = pVXDLayerLayout.getPhi0(layer);
    _VXDgeo[layer].dphi = 2*M_PI / _VXDgeo[layer].nLadders;
    _VXDgeo[layer].senRMin = pVXDLayerLayout.getSensitiveDistance(layer);
    _VXDgeo[layer].supRMin = pVXDLayerLayout.getLadderDistance(layer);
    _VXDgeo[layer].length = pVXDLayerLayout.getSensitiveLength(layer) * 2.0 ; // note: gear for historical reasons uses the halflength
    _VXDgeo[layer].width = pVXDLayerLayout.getSensitiveWidth(layer);
    _VXDgeo[layer].offset = pVXDLayerLayout.getSensitiveOffset(layer);
    _VXDgeo[layer].senThickness = pVXDLayerLayout.getSensitiveThickness(layer);
    _VXDgeo[layer].supThickness = pVXDLayerLayout.getLadderThickness(layer);
    //std::cout << layer << ": " << _VXDgeo[layer].nLadders << " " << _VXDgeo[layer].phi0 << " " << _VXDgeo[layer].dphi << " " << _VXDgeo[layer].senRMin
    //          << " " << _VXDgeo[layer].supRMin << " " << _VXDgeo[layer].length << " " << _VXDgeo[layer].width << " " << _VXDgeo[layer].offset
    //          << " " << _VXDgeo[layer].senThickness << " " << _VXDgeo[layer].supThickness << std::endl;
  }
  
  _relative_position_of_measurement_surface = 0.5 ;

  try {
    _relative_position_of_measurement_surface =  pVXDDetMain.getDoubleVal( "relative_position_of_measurement_surface"  );
  }
  catch (gear::UnknownParameterException& e) {}

  _nLayers[1] = 0;
  try {
    const std::vector<int> ids = pVXDDetMain.getIntVals("VTXLayerIds");
    const std::vector<double> zhalfs = pVXDDetMain.getDoubleVals("VTXLayerHalfLengths");
    const std::vector<double> rsens  = pVXDDetMain.getDoubleVals("VTXLayerSensitiveRadius");
    const std::vector<double> tsens  = pVXDDetMain.getDoubleVals("VTXLayerSensitiveThickness");
    const std::vector<double> rsups  = pVXDDetMain.getDoubleVals("VTXLayerSupperRadius");
    const std::vector<double> tsups  = pVXDDetMain.getDoubleVals("VTXLayerSupperThickness");
    const std::vector<double> phi0s  = pVXDDetMain.getDoubleVals("VTXLayerPhi0");
    const std::vector<double> rgaps  = pVXDDetMain.getDoubleVals("VTXLayerRadialGap");
    const std::vector<double> dphis  = pVXDDetMain.getDoubleVals("VTXLayerDeltaPhi");
    const std::vector<double> widths = pVXDDetMain.getDoubleVals("VTXLayerWidth");
  
    _nLayers[1] = ids.size();
    _STTgeo.resize(_nLayers[1]);
    for (int ilayer=0; ilayer<_nLayers[1]; ilayer++) {
      _STTgeo[ilayer].id = ids[ilayer];
      _STTgeo[ilayer].length = zhalfs[ilayer]*2;
      _STTgeo[ilayer].width = widths[ilayer];
      _STTgeo[ilayer].senRMin = rsens[ilayer];
      _STTgeo[ilayer].senThickness = tsens[ilayer];
      _STTgeo[ilayer].supRMin = rsups[ilayer];
      _STTgeo[ilayer].supThickness = tsups[ilayer];
      _STTgeo[ilayer].phi0 = phi0s[ilayer];
      _STTgeo[ilayer].rgap = rgaps[ilayer];
      _STTgeo[ilayer].dphi = dphis[ilayer];
    }
  }
  catch (gear::UnknownParameterException& e) {}

  //std::cout << "planar: " << _nLayers[0] << " bent: " << _nLayers[1] << std::endl;
}


void CEPCVTXKalDetector::setupGearGeom( IGeomSvc* geoSvc){
  dd4hep::DetElement world = geoSvc->getDD4HepGeo();
  dd4hep::DetElement vxd;
  const std::map<std::string, dd4hep::DetElement>& subs = world.children();
  for(std::map<std::string, dd4hep::DetElement>::const_iterator it=subs.begin();it!=subs.end();it++){
    if(it->first!="VXD") continue;
    vxd = it->second;
  }
  dd4hep::rec::ZPlanarData* vxdData = nullptr;
  try{
    vxdData = vxd.extension<dd4hep::rec::ZPlanarData>();
  }
  catch(std::runtime_error& e){
    std::cout << e.what() << " " << vxdData << std::endl;
    throw GaudiException(e.what(), "FATAL", StatusCode::FAILURE);
  }
  
  const dd4hep::Direction& field = geoSvc->lcdd()->field().magneticField(dd4hep::Position(0,0,0));
  _bZ = field.z()/dd4hep::tesla;

  const std::vector<dd4hep::rec::ZPlanarData::LayerLayout>& layers = vxdData->layers;
  
  _nLayers[0] = layers.size(); 
  _VXDgeo.resize(_nLayers[0]);
  
  //SJA:FIXME: for now the support is taken as the same size the sensitive
  //           if this is not done then the exposed areas of the support would leave a carbon - air boundary,
  //           which if traversed in the reverse direction to the next boundary then the track will be propagated through carbon
  //           for a significant distance 
  
  for( int layer=0; layer < _nLayers[0]; ++layer){
    const dd4hep::rec::ZPlanarData::LayerLayout& thisLayer = layers[layer];
    _VXDgeo[layer].nLadders = thisLayer.ladderNumber; 
    _VXDgeo[layer].phi0 = thisLayer.phi0; 
    _VXDgeo[layer].dphi = 2*M_PI / _VXDgeo[layer].nLadders; 
    _VXDgeo[layer].senRMin = thisLayer.distanceSensitive; 
    _VXDgeo[layer].supRMin = thisLayer.distanceSupport; 
    _VXDgeo[layer].length = thisLayer.zHalfSensitive * 2.0 ; // note: gear for historical reasons uses the halflength 
    _VXDgeo[layer].width = thisLayer.widthSensitive; 
    _VXDgeo[layer].offset = thisLayer.offsetSensitive; 
    _VXDgeo[layer].senThickness = thisLayer.thicknessSensitive; 
    _VXDgeo[layer].supThickness = thisLayer.thicknessSupport;
    //std::cout << layer << ": " << _VXDgeo[layer].nLadders << " " << _VXDgeo[layer].phi0 << " " << _VXDgeo[layer].dphi << " " << _VXDgeo[layer].senRMin 
    //	      << " " << _VXDgeo[layer].supRMin << " " << _VXDgeo[layer].length << " " << _VXDgeo[layer].width << " " << _VXDgeo[layer].offset
    //	      << " " << _VXDgeo[layer].senThickness << " " << _VXDgeo[layer].supThickness << std::endl; 
  }
  // by default, we put the measurement surface in the middle of the sensitive
  // layer, this can optionally be changed, e.g. in the case of the FPCCD where the 
  // epitaxial layer is 15 mu thick (in a 50 mu wafer)
  _relative_position_of_measurement_surface = 0.5 ;
  
}
