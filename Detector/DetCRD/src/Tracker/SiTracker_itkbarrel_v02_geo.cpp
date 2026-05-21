//====================================================================
//  cepcsitbgeo - CEPC silicon inner tracker barrel models in DD4hep 
//--------------------------------------------------------------------
//  v02, Peripheral electronics on the back of staves
//       Single module is divided into sensors 
//  Xiaojie Jiang, IHEP
//  email: jiangxj@ihep.ac.cn
//  $Id$
//====================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetType.h"
#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "XML/Utilities.h"
#include <cmath>

using namespace std;

using dd4hep::Box;
using dd4hep::Tube;
using dd4hep::DetElement;
using dd4hep::Material;
using dd4hep::Position;
using dd4hep::RotationY;
using dd4hep::RotationZYX;
using dd4hep::Transform3D;
using dd4hep::Rotation3D;
using dd4hep::Volume;
using dd4hep::_toString;
using dd4hep::rec::volSurfaceList;
using dd4hep::rec::ZPlanarData;
using dd4hep::mm;


double findMax_arr(double arr[], int size) {
    if (size == 0) {
        return 0;
    }
    
    double max_value = arr[0];
    
    for (int i = 1; i < size; i++) {
        if (arr[i] > max_value) {
            max_value = arr[i]; 
        }
    }
    
    return max_value;
}


/** helper struct */
struct SIT_Layer {
  int     n_staves;
  int     n_modules_per_ladder;
  double  module_length;
  double  half_z;
  double  sensitive_radius ;
  double  support_radius ;
  double  stave_width ;
  double  stave_dphi ;
};    


/** Construction of the SIT detector, ported from SITBStaggeredStave_v01_geo.cpp
 *
 *  Modify History:
 *  Steve Aplin, Feb 7th 2011 - original version
 *  F.Gaede, Jan 2014, DESY   - dd4hep SIT pixel
 *  Hao Zeng, July 2021, IHEP - dd4hep VXD pixel
 *  Xiaojie Jiang, July 2024, IHEP - dd4hep SIT pixel v01
 *  Xiaojie Jiang, Aug 2024, IHEP - dd4hep SIT pixel v02
 */

static dd4hep::Ref_t create_element(dd4hep::Detector& theDetector, xml_h e, dd4hep::SensitiveDetector sens)  {  

  xml_det_t  x_det    = e;
  Material   air      = theDetector.air();  
  int        det_id   = x_det.id();
  string     name     = x_det.nameStr();
  DetElement sit(name, det_id);

  Volume envelope = dd4hep::xml::createPlacedEnvelope(theDetector, e, sit);
  dd4hep::xml::setDetectorTypeFlag(e, sit) ;
  if(theDetector.buildType()==dd4hep::BUILD_ENVELOPE) return sit;
  envelope.setVisAttributes(theDetector.visAttributes("SeeThrough"));

  sens.setType("tracker");
  std::cout << " ** building SITBStaggeredStave_v02..." << std::endl ;

  dd4hep::rec::ZPlanarData* zPlanarData = new dd4hep::rec::ZPlanarData;

  //No fetch global paras, actually it's better to place sensor info @ global now
  //But if the module will be modified, and become non-uniform, we will need diff sensor settings 
  //fetch the display parameters
  xml_comp_t x_display(x_det.child(_Unicode(display)));  
  std::string staveVis      = x_display.attr<string>(_Unicode(stave));
  std::string tubeTiVis      = x_display.attr<string>(_Unicode(tubeTi));
  std::string supportVis     = x_display.attr<string>(_Unicode(support));
  std::string flexVis        = x_display.attr<string>(_Unicode(flex));
  std::string sensEnvVis     = x_display.attr<string>(_Unicode(sens_env));
  std::string sensVis        = x_display.attr<string>(_Unicode(sens));
  std::string deadsensVis    = x_display.attr<string>(_Unicode(deadsensor));
  std::string portsVis       = x_display.attr<string>(_Unicode(ports));
  std::string dataaggregationVis      = x_display.attr<string>(_Unicode(dataaggregation));
  std::string lpGBTxVis      = x_display.attr<string>(_Unicode(lpGBTx));
  std::string opticalconnectorVis    = x_display.attr<string>(_Unicode(opticalconnector));
  std::string DCDCVis      = x_display.attr<string>(_Unicode(DCDC));

  for(xml_coll_t layer_i(x_det,_U(layer)); layer_i; ++layer_i){
    xml_comp_t x_layer(layer_i); 
   
    dd4hep::PlacedVolume pv;
    int layer_id                 = x_layer.attr<int>(_Unicode(layer_id));
    std::cout << "*******************************" << endl;
    std::cout << "layer_id: " << layer_id << endl;

    double sensitive_radius      = x_layer.attr<double>(_Unicode(sensitive_radius));
    int n_staves                = x_layer.attr<int>(_Unicode(n_staves)) ;
    double stave_offset           = x_layer.attr<int>(_Unicode(stave_offset)) ;
    double stave_phi0           = -atan(stave_offset/sensitive_radius);
    double stave_sens_radius         = sqrt(stave_offset*stave_offset + sensitive_radius*sensitive_radius); 
  
    std::cout << "stave_sens_radius: " << stave_sens_radius/mm <<" mm" << endl;
    std::cout << "sensitive_radius: " << sensitive_radius/mm << " mm" << endl;
    std::cout << "stave_offset: " << stave_offset/mm << " mm" << endl;
    std::cout << "n_staves: " << n_staves << endl;

    std::string layerName = dd4hep::_toString( layer_id , "layer_%d"  );
    dd4hep::Assembly layer_assembly( layerName ) ;
    pv = envelope.placeVolume( layer_assembly ) ;
    dd4hep::DetElement layerDE( sit , layerName  , x_det.id() );
    layerDE.setPlacement( pv ) ;
   
    const double stave_dphi = ( dd4hep::twopi / n_staves ) ;
    std::cout << "stave_dphi: " << stave_dphi << endl;

    //fetch the stave parameters
    xml_comp_t x_stave(x_layer.child(_Unicode(stave)));
    //  db = XMLHandlerDB(x_stave);
   
    //fetch the titanium tube parameters
    xml_comp_t x_tubeTi(x_stave.child(_Unicode(tubeTi)));
    double tube_half_length        = x_tubeTi.attr<double>(_Unicode(length));
    double tube_inner_radius       = x_tubeTi.attr<double>(_Unicode(innerradius));
    double tube_outer_radius       = x_tubeTi.attr<double>(_Unicode(outerradius));
    Material tube_mat;
    if(x_tubeTi.hasAttr(_Unicode(mat)))
      {
	tube_mat = theDetector.material(x_tubeTi.attr<string>(_Unicode(mat)));
      }
    else
      {
	tube_mat = theDetector.material(x_tubeTi.materialStr());  //?
      }
    std::cout << "tube_half_length: " << tube_half_length/mm << " mm" << endl;
    std::cout << "tube_inner_radius: " << tube_inner_radius/mm << " mm" << endl;
    std::cout << "tube_outer_radius: " << tube_outer_radius/mm << " mm" << endl;
    
    //fetch the support parameters
    double support_thickness(0);
    double support_width(0);
    double support_half_length(0);
    xml_comp_t x_stave_support(x_stave.child(_Unicode(staveSupport)));
    for(xml_coll_t stave_support_i(x_stave_support,_U(slice)); stave_support_i; ++stave_support_i){
      xml_comp_t x_support_slice(stave_support_i);
      double x_support_slice_thickness = x_support_slice.attr<double>(_Unicode(thickness));
      double x_support_slice_width = x_support_slice.attr<double>(_Unicode(width));
      double x_support_slice_half_length = x_support_slice.attr<double>(_Unicode(length));
      support_thickness += x_support_slice_thickness;
      if (x_support_slice_width > support_width) support_width = x_support_slice_width;
      if (x_support_slice_half_length > support_half_length) support_half_length = x_support_slice_half_length;
      std::cout << "x_support_slice_thickness: " << x_support_slice_thickness/mm << " mm" << endl;
    }
    std::cout << "support_thickness: " << support_thickness/mm << " mm" << endl;
    std::cout << "support_width: " << support_width/mm << " mm" << endl;
    std::cout << "support_half_length: " << support_half_length/mm << " mm" << endl;

    //fetch the flex parameters
    double flex_thickness(0);  //not a normal declaration
    double flex_width(0);
    double flex_half_length(0);
    xml_comp_t x_flex(x_stave.child(_Unicode(flex)));
    for(xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i){
      xml_comp_t x_flex_slice(flex_i);
      double x_flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
      double x_flex_slice_width = x_flex_slice.attr<double>(_Unicode(width));
      double x_flex_slice_half_length = x_flex_slice.attr<double>(_Unicode(length));
      flex_thickness += x_flex_slice_thickness;
      if (x_flex_slice_width > flex_width) flex_width = x_flex_slice_width;
      if (x_flex_slice_half_length > flex_half_length) flex_half_length = x_flex_slice_half_length;
      std::cout << "x_flex_slice_thickness: " << x_flex_slice_thickness/mm << " mm" << endl;
    }
    std::cout << "flex_thickness: " << flex_thickness/mm << " mm" << endl;
    std::cout << "flex_width: " << flex_width/mm << " mm" << endl;
    std::cout << "flex_half_length: " << flex_half_length/mm << " mm" << endl;

    //fetch the module parameters
    xml_comp_t x_module(x_stave.child(_Unicode(module)));
    int n_modules_per_stave                  = x_module.attr<int>(_Unicode(n_modules));
    int n_sensors_per_module                  = x_module.attr<int>(_Unicode(n_sensors));
    double dead_gap                          = x_module.attr<double>(_Unicode(gap));
    double dead_sensor_gap                   = x_module.attr<double>(_Unicode(sensor_gap));
    double module_thickness                  = x_module.attr<double>(_Unicode(thickness));
    double sensor_thickness                  = x_module.attr<double>(_Unicode(sensor_thickness));
    double module_length              = x_module.attr<double>(_Unicode(length));
    double sensor_active_length             = x_module.attr<double>(_Unicode(sensor_sensitive_length));
    double module_width              = x_module.attr<double>(_Unicode(width));
    double sensor_active_width              = x_module.attr<double>(_Unicode(sensor_sensitive_width));
    double sensor_dead_width                = x_module.attr<double>(_Unicode(dead_width));
    double sensor_dead_length               = x_module.attr<double>(_Unicode(dead_length));
    Material sensor_mat                     = theDetector.material(x_module.attr<string>(_Unicode(sensor_mat)));

    std::cout << "n_modules_per_stave: " << n_modules_per_stave << endl;
    std::cout << "n_sensors_per_module: " << n_sensors_per_module << endl;
    std::cout << "dead_gap_btw_modules: " << dead_gap/mm << " mm" << endl;
    std::cout << "dead_gap_btw_sensors: " << dead_sensor_gap/mm << " mm" << endl;
    std::cout << "module_thickness: " << module_thickness/mm << " mm" << endl;
    std::cout << "sensor_thickness: " << sensor_thickness/mm << " mm" << endl;
    std::cout << "module_length: " << module_length/mm << " mm" << endl;
    std::cout << "sensor_active_length: " << sensor_active_length/mm << " mm" << endl;
    std::cout << "module_width: " << module_width/mm << " mm" << endl;
    std::cout << "sensor_active_width: " << sensor_active_width/mm << " mm" << endl;
    std::cout << "sensor_dead_width: " << sensor_dead_width/mm << " mm" << endl;
    std::cout << "sensor_dead_length: " << sensor_dead_length/mm << " mm" << endl;
    
    //fetch the lpGBTx parameters
    double lpGBTx_thickness(0);
    double lpGBTx_width(0);
    double lpGBTx_length(0);
    xml_comp_t x_lpGBTx(x_stave.child(_Unicode(lpGBTx)));
    for(xml_coll_t lpGBTx_i(x_lpGBTx,_U(slice)); lpGBTx_i; ++lpGBTx_i){
      xml_comp_t x_lpGBTx_slice(lpGBTx_i);
      double x_lpGBTx_slice_thickness = x_lpGBTx_slice.attr<double>(_Unicode(thickness));
      double x_lpGBTx_slice_width = x_lpGBTx_slice.attr<double>(_Unicode(width));
      double x_lpGBTx_slice_length = x_lpGBTx_slice.attr<double>(_Unicode(length));
      lpGBTx_thickness += x_lpGBTx_slice_thickness;
      if (x_lpGBTx_slice_width > lpGBTx_width) lpGBTx_width = x_lpGBTx_slice_width;
      if (x_lpGBTx_slice_length > lpGBTx_length) lpGBTx_length = x_lpGBTx_slice_length;
      std::cout << "x_lpGBTx_slice_thickness: " << x_lpGBTx_slice_thickness/mm << " mm" << endl;
    }
    std::cout << "lpGBTx_thickness: " << lpGBTx_thickness/mm << " mm" << endl;
    std::cout << "lpGBTx_width: " << lpGBTx_width/mm << " mm" << endl;
    std::cout << "lpGBTx_length: " << lpGBTx_length/mm << " mm" << endl;

    //fetch the opticalconnector parameters
    double opticalconnector_thickness(0);
    double opticalconnector_width(0);
    double opticalconnector_length(0);
    xml_comp_t x_opticalconnector(x_stave.child(_Unicode(opticalconnector)));
    for(xml_coll_t opticalconnector_i(x_opticalconnector,_U(slice)); opticalconnector_i; ++opticalconnector_i){
      xml_comp_t x_opticalconnector_slice(opticalconnector_i);
      double x_opticalconnector_slice_thickness = x_opticalconnector_slice.attr<double>(_Unicode(thickness));
      double x_opticalconnector_slice_width = x_opticalconnector_slice.attr<double>(_Unicode(width));
      double x_opticalconnector_slice_length = x_opticalconnector_slice.attr<double>(_Unicode(length));
      opticalconnector_thickness += x_opticalconnector_slice_thickness;
      if (x_opticalconnector_slice_width > opticalconnector_width) opticalconnector_width = x_opticalconnector_slice_width;
      if (x_opticalconnector_slice_length > opticalconnector_length) opticalconnector_length = x_opticalconnector_slice_length;
      std::cout << "x_opticalconnector_slice_thickness: " << x_opticalconnector_slice_thickness/mm << " mm" << endl;
    }
    std::cout << "opticalconnector_thickness: " << opticalconnector_thickness/mm << " mm" << endl;
    std::cout << "opticalconnector_width: " << opticalconnector_width/mm << " mm" << endl;
    std::cout << "opticalconnector_length: " << opticalconnector_length/mm << " mm" << endl;

    //fetch the data aggregation parameters
    double dataaggregation_thickness(0);
    double dataaggregation_width(0);
    double dataaggregation_length(0);
    xml_comp_t x_dataaggregation(x_stave.child(_Unicode(dataaggregation)));
    for(xml_coll_t dataaggregation_i(x_dataaggregation,_U(slice)); dataaggregation_i; ++dataaggregation_i){
      xml_comp_t x_dataaggregation_slice(dataaggregation_i);
      double x_dataaggregation_slice_thickness = x_dataaggregation_slice.attr<double>(_Unicode(thickness));
      double x_dataaggregation_slice_width = x_dataaggregation_slice.attr<double>(_Unicode(width));
      double x_dataaggregation_slice_length = x_dataaggregation_slice.attr<double>(_Unicode(length));
      dataaggregation_thickness += x_dataaggregation_slice_thickness;
      if (x_dataaggregation_slice_width > dataaggregation_width) dataaggregation_width = x_dataaggregation_slice_width;
      if (x_dataaggregation_slice_length > dataaggregation_length) dataaggregation_length = x_dataaggregation_slice_length;
      std::cout << "x_dataaggregation_slice_thickness: " << x_dataaggregation_slice_thickness/mm << " mm" << endl;
    }
    std::cout << "dataaggregation_thickness: " << dataaggregation_thickness/mm << " mm" << endl;
    std::cout << "dataaggregation_width: " << dataaggregation_width/mm << " mm" << endl;
    std::cout << "dataaggregation_length: " << dataaggregation_length/mm << " mm" << endl;

    //fetch the Direct Current Direct Current Converter parameters
    double DCDC_thickness(0);
    double DCDC_width(0);
    double DCDC_length(0);
    xml_comp_t x_DCDC(x_stave.child(_Unicode(DCDC)));
    for(xml_coll_t DCDC_i(x_DCDC,_U(slice)); DCDC_i; ++DCDC_i){
      xml_comp_t x_DCDC_slice(DCDC_i);
      double x_DCDC_slice_thickness = x_DCDC_slice.attr<double>(_Unicode(thickness));
      double x_DCDC_slice_width = x_DCDC_slice.attr<double>(_Unicode(width));
      double x_DCDC_slice_length = x_DCDC_slice.attr<double>(_Unicode(length));
      DCDC_thickness += x_DCDC_slice_thickness;
      if (x_DCDC_slice_width > DCDC_width) DCDC_width = x_DCDC_slice_width;
      if (x_DCDC_slice_length > DCDC_length) DCDC_length = x_DCDC_slice_length;
      std::cout << "x_DCDC_slice_thickness: " << x_DCDC_slice_thickness/mm << " mm" << endl;
    }
    std::cout << "DCDC_thickness: " << DCDC_thickness/mm << " mm" << endl;
    std::cout << "DCDC_width: " << DCDC_width/mm << " mm" << endl;
    std::cout << "DCDC_length: " << DCDC_length/mm << " mm" << endl;

    //create stave logical volume
    double arr_thic[] = {lpGBTx_thickness, opticalconnector_thickness, dataaggregation_thickness, DCDC_thickness};
    double max_connector_thickness = findMax_arr(arr_thic, 4);
    double stave_thickness = tube_outer_radius*2. + support_thickness + module_thickness + flex_thickness + max_connector_thickness;
    Box StaveSolid(stave_thickness / 2.0, support_width / 2.0, support_half_length);
    Volume StaveLogical(name + dd4hep::_toString( layer_id, "_StaveLogical_%02d"),
			StaveSolid, air); 
    std::cout << "Total thickness: " << stave_thickness/mm << " mm" << endl; 
    
    // create flex envelope logical volume
    Box FlexEnvelopeSolid(flex_thickness / 2.0, flex_width / 2.0, flex_half_length);
    Volume FlexEnvelopeLogical(name + dd4hep::_toString( layer_id, "_FlexEnvelopeLogical_%02d"), FlexEnvelopeSolid, air);
    FlexEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    //sit.setVisAttributes(theDetector, flexVis, FlexEnvelopeLogical);

    //create the flex layers inside the flex envelope
    double flex_start_height(-flex_thickness/2.); 
    int index = 0;
    for(xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i){
      xml_comp_t x_flex_slice(flex_i);
      double x_flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
      double x_flex_slice_width = x_flex_slice.attr<double>(_Unicode(width));
      double x_flex_slice_half_length = x_flex_slice.attr<double>(_Unicode(length));
      Material x_flex_slice_mat;
      if(x_flex_slice.hasAttr(_Unicode(mat)))
	{
	  x_flex_slice_mat = theDetector.material(x_flex_slice.attr<string>(_Unicode(mat)));
	}
      else
	{
	  x_flex_slice_mat = theDetector.material(x_flex_slice.materialStr());
	}
      // Material x_flex_slice_mat = theDetector.material(x_flex_slice.attr<string>(_Unicode(mat)));
      Box FlexLayerSolid(x_flex_slice_thickness/2.0, x_flex_slice_width/2.0, x_flex_slice_half_length);
      Volume FlexLayerLogical(name + dd4hep::_toString( layer_id, "_FlexLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), FlexLayerSolid, x_flex_slice_mat);
      FlexLayerLogical.setVisAttributes(theDetector.visAttributes(flexVis));
      double flex_slice_height = flex_start_height + x_flex_slice_thickness/2.;
      pv = FlexEnvelopeLogical.placeVolume(FlexLayerLogical, Position(flex_slice_height, 0., 0.));
      std::cout << "flex slice" << index << " thickness = " << x_flex_slice_thickness/mm << "mm" << std::endl;
      std::cout << "flex slice" << index << " width = " << x_flex_slice_width/mm << "mm" << std::endl;
      std::cout << "flex slice" << index << " half length = " << x_flex_slice_half_length/mm << "mm" << std::endl;
      // std::cout << "flex material: " << x_flex_slice_mat << std::endl;
      flex_start_height += x_flex_slice_thickness;
      index++;
    }

    //place the flex envelope inside the stave envelope
    double flexenv_start_height(-stave_thickness/2.0 + tube_outer_radius*2. + support_thickness + module_thickness);
    double flexenv_slice_height=flexenv_start_height + (flex_thickness) / 2.0;
    pv = StaveLogical.placeVolume(FlexEnvelopeLogical, Position(flexenv_slice_height, 0., 0.));

    //create module envelope logical volume
    Box ModuleEnvelopeSolid(module_thickness / 2.0, module_width / 2.0, module_length / 2.0);
    Volume ModuleEnvelopeLogical(name + dd4hep::_toString( layer_id, "_ModuleEnvelopeLogical_%02d"), ModuleEnvelopeSolid, air);
    //ModuleLogical.setSensitiveDetector(sens);
    //sit.setVisAttributes(theDetector, deadsensVis, ModuleDeadLogical);
    ModuleEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));

    //create sensor logical volume
    Box SensorSolid(sensor_thickness / 2.0, sensor_active_width / 2.0, sensor_active_length / 2.0);
    Volume SensorLogical(name + dd4hep::_toString( layer_id, "_SensorLogical_%02d"), SensorSolid, sensor_mat);
    SensorLogical.setSensitiveDetector(sens);
    SensorLogical.setVisAttributes(theDetector.visAttributes(sensVis));

    //create sensor dead area logical volume
    //  The Sensor Diagram
    //-------------------
    //   |           | 
    // a |           | a
    //   |  Active   |
    // 2 |           | 2    w=20-2.59
    //   |           |
    //   |           |
    //-------------------  ---------
    //      a     1         w=2.59 
    //-------------------
    //0.4| l=20-0.8  |0.4         *mm
    
    Box SensorDeada1Solid(sensor_thickness / 2.0, sensor_dead_width / 2.0, (sensor_active_length + sensor_dead_length) / 2.0);
    Volume SensorDeada1Logical(name + dd4hep::_toString( layer_id, "_SensorDeada1Logical_%02d"), SensorDeada1Solid, sensor_mat);
    SensorDeada1Logical.setVisAttributes(theDetector.visAttributes(deadsensVis));

    Box SensorDeada2Solid(sensor_thickness / 2.0, sensor_active_width / 2.0, sensor_dead_length/2.0/2.);
    Volume SensorDeada2Logical(name + dd4hep::_toString( layer_id, "_SensorDeada2Logical_%a2d"), SensorDeada2Solid, sensor_mat);
    SensorDeada2Logical.setVisAttributes(theDetector.visAttributes(deadsensVis));

    // place the active sensor and dead sensor inside the module envelope, then place module inside stave
    //Positions are very complex
    //Actually, it's better to place a combined volume of active & dead sensor areas first
    //But, the dispaly will get mistakes when there are over 3 layers of nested volumes
    std::vector<dd4hep::PlacedVolume> Sensor_pv;
    //std::vector<dd4hep::PlacedVolume> RightSensor_pv;
    //for(int imodule=0; imodule < n_modules_per_stave; ++imodule){
    for(int isensor=0; isensor < n_sensors_per_module / 2.; ++isensor){
      double sensor_total_z = n_sensors_per_module / 2. * (sensor_active_length + sensor_dead_length) + (n_sensors_per_module / 2-1) * dead_sensor_gap + 2 * dead_gap;
      double xpos_active = -module_thickness / 2.0 + sensor_thickness / 2.0;  //==0 here
      double left_ypos_active = -module_width / 2.0 + sensor_dead_width + sensor_active_width / 2.0;
      double right_ypos_active = module_width / 2.0 - sensor_dead_width - sensor_active_width / 2.0;
      double xpos_deada1 = xpos_active;
      double left_ypos_deada1 = -module_width / 2.0 + sensor_dead_width/2.;
      double right_ypos_deada1 = module_width / 2.0 - sensor_dead_width/2.;
      double xpos_deada2 = xpos_active;
      double left_ypos_deada2 = left_ypos_active;
      double right_ypos_deada2 = right_ypos_active;
      double zpos_active = -sensor_total_z/2.0 + dead_gap + sensor_dead_length / 2. + sensor_active_length / 2. + isensor*(sensor_active_length + sensor_dead_length + dead_sensor_gap);
      double zpos_deada1 = -sensor_total_z/2.0 + dead_gap + (sensor_active_length + sensor_dead_length) / 2.0 + isensor*(sensor_active_length + sensor_dead_length + dead_sensor_gap);  //== zpos_active
      double zpos_deada2_0 = -sensor_total_z/2.0 + dead_gap + sensor_dead_length / 4.0 + isensor*(sensor_active_length + sensor_dead_length + dead_sensor_gap);
      double zpos_deada2_1 = -sensor_total_z/2.0 + dead_gap + sensor_dead_length / 2. + sensor_active_length + sensor_dead_length / 4.0 + isensor*(sensor_active_length + sensor_dead_length + dead_sensor_gap);
      pv = ModuleEnvelopeLogical.placeVolume(SensorLogical, Position(xpos_active,left_ypos_active,zpos_active));
      //pv.addPhysVolID("layer", layer_id).addPhysVolID("module", imodule).addPhysVolID("sensor", isensor);
      pv.addPhysVolID("layer", layer_id).addPhysVolID("sensor", isensor);
      //LeftSensor_pv.push_back(pv);
      Sensor_pv.push_back(pv);
      pv = ModuleEnvelopeLogical.placeVolume(SensorLogical, Position(xpos_active,right_ypos_active,zpos_active));
      //pv.addPhysVolID("layer", layer_id).addPhysVolID("module", imodule).addPhysVolID("sensor", isensor+7);
      pv.addPhysVolID("layer", layer_id).addPhysVolID("sensor", isensor+7);
      //RightSensor_pv.push_back(pv);
      Sensor_pv.push_back(pv);
      pv = ModuleEnvelopeLogical.placeVolume(SensorDeada1Logical, Position(xpos_deada1,left_ypos_deada1,zpos_deada1));
      pv = ModuleEnvelopeLogical.placeVolume(SensorDeada1Logical, Position(xpos_deada1,right_ypos_deada1,zpos_deada1));
      pv = ModuleEnvelopeLogical.placeVolume(SensorDeada2Logical, Position(xpos_deada2,left_ypos_deada2,zpos_deada2_0));
      pv = ModuleEnvelopeLogical.placeVolume(SensorDeada2Logical, Position(xpos_deada2,left_ypos_deada2,zpos_deada2_1));
      pv = ModuleEnvelopeLogical.placeVolume(SensorDeada2Logical, Position(xpos_deada2,right_ypos_deada2,zpos_deada2_0));
      pv = ModuleEnvelopeLogical.placeVolume(SensorDeada2Logical, Position(xpos_deada2,right_ypos_deada2,zpos_deada2_1));
      }
      // double module_total_z = n_modules_per_stave*module_length;
    //   double xpos = -stave_thickness/2.0 + tube_outer_radius*2. + support_thickness + module_thickness/2.;
    //   double ypos = 0.0;
    //   double zpos = -module_total_z/2. + module_length/2.0 + imodule*module_length;
    //   pv = StaveLogical.placeVolume(ModuleEnvelopeLogical, Position(xpos,ypos,zpos));
    // }

    //place the module envelope inside the stave envelope
    std::vector<dd4hep::PlacedVolume> Module_pv;
    for(int imodule=0; imodule < n_modules_per_stave; ++imodule){
      double module_total_z = n_modules_per_stave*module_length;
      double xpos = -stave_thickness/2.0 + tube_outer_radius*2. + support_thickness + module_thickness/2.;
      double ypos = 0.0;
      double zpos = -module_total_z/2. + module_length/2.0 + imodule*module_length;
      pv = StaveLogical.placeVolume(ModuleEnvelopeLogical, Position(xpos,ypos,zpos));
      pv.addPhysVolID("layer", layer_id).addPhysVolID("module", imodule) ;
      Module_pv.push_back(pv);
    }

    //create combine(lpGBTx + opticalconnector + dataaggregation + DCDC) envelope logical volume
    Box combineEnvelopeSolid(max_connector_thickness / 2.0, flex_width / 2., module_length/2.0);
    Volume combineEnvelopeLogical(name + dd4hep::_toString( layer_id, "_combineEnvelopeLogical_%02d"), combineEnvelopeSolid, air);
    combineEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    std::cout << "max connector thickness = " << max_connector_thickness/mm << "mm"  << std::endl;

    // create lpGBTx envelope logical volume
    Box LpGBTxEnvelopeSolid(lpGBTx_thickness / 2.0, lpGBTx_width/2., lpGBTx_length/2.);
    Volume LpGBTxEnvelopeLogical(name + dd4hep::_toString( layer_id, "_LpGBTxEnvelopeLogical_%02d"), LpGBTxEnvelopeSolid, air);
    LpGBTxEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    //sit.setVisAttributes(theDetector, lpGBTxVis, LpGBTxEnvelopeLogical);
    //std::cout << "lpGBTx Env thickness = " << lpGBTx_width/mm << "mm"  << std::endl;
    //std::cout << "lpGBTx Env length = " << lpGBTx_length/mm << "mm"  << std::endl;
    
    //create the lpGBTx layers inside the lpGBTx envelope
    double lpGBTx_start_height(-lpGBTx_thickness/2.); 
    index = 0;
    for(xml_coll_t lpGBTx_i(x_lpGBTx,_U(slice)); lpGBTx_i; ++lpGBTx_i){
      xml_comp_t x_lpGBTx_slice(lpGBTx_i);
      double x_lpGBTx_slice_thickness = x_lpGBTx_slice.attr<double>(_Unicode(thickness));
      double x_lpGBTx_slice_width = x_lpGBTx_slice.attr<double>(_Unicode(width));
      double x_lpGBTx_slice_length = x_lpGBTx_slice.attr<double>(_Unicode(length));
      Material x_lpGBTx_slice_mat;
      if(x_lpGBTx_slice.hasAttr(_Unicode(mat)))
	{
	  x_lpGBTx_slice_mat = theDetector.material(x_lpGBTx_slice.attr<string>(_Unicode(mat)));
	}
      else
	{
	  x_lpGBTx_slice_mat = theDetector.material(x_lpGBTx_slice.materialStr());
	}
      // Material x_lpGBTx_slice_mat = theDetector.material(x_lpGBTx_slice.attr<string>(_Unicode(mat)));
      Box LpGBTxLayerSolid(x_lpGBTx_slice_thickness/2.0, x_lpGBTx_slice_width/2.0, x_lpGBTx_slice_length/2.);
      Volume LpGBTxLayerLogical(name + dd4hep::_toString( layer_id, "_LpGBTxLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), LpGBTxLayerSolid, x_lpGBTx_slice_mat);
      LpGBTxLayerLogical.setVisAttributes(theDetector.visAttributes(lpGBTxVis));
      double lpGBTx_slice_height = lpGBTx_start_height + x_lpGBTx_slice_thickness/2.;
      pv = LpGBTxEnvelopeLogical.placeVolume(LpGBTxLayerLogical, Position(lpGBTx_slice_height, 0., 0.)); //not found the define of Position, why will it has a return
      std::cout << "lpGBTx slice" << index << " thickness = " << x_lpGBTx_slice_thickness/mm << "mm" << std::endl;
      std::cout << "lpGBTx slice" << index << " width = " << x_lpGBTx_slice_width/mm << "mm" << std::endl;
      std::cout << "lpGBTx slice" << index << " length = " << x_lpGBTx_slice_length/mm << "mm" << std::endl;
      // std::cout << "lpGBTx material: " << x_lpGBTx_slice_mat << std::endl;
      lpGBTx_start_height += x_lpGBTx_slice_thickness;
      index++;
    }

    // create opticalconnector envelope logical volume
    Box opticalconnectorEnvelopeSolid(opticalconnector_thickness / 2.0, opticalconnector_width / 2.0, opticalconnector_length / 2.0);
    Volume opticalconnectorEnvelopeLogical(name + dd4hep::_toString( layer_id, "_opticalconnectorEnvelopeLogical_%02d"), opticalconnectorEnvelopeSolid, air);
    opticalconnectorEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    //sit.setVisAttributes(theDetector, opticalconnectorVis, opticalconnectorEnvelopeLogical);

    //create the opticalconnector layers inside the opticalconnector envelope
    double opticalconnector_start_height(-opticalconnector_thickness/2.); 
    index = 0;
    for(xml_coll_t opticalconnector_i(x_opticalconnector,_U(slice)); opticalconnector_i; ++opticalconnector_i){
      xml_comp_t x_opticalconnector_slice(opticalconnector_i);
      double x_opticalconnector_slice_thickness = x_opticalconnector_slice.attr<double>(_Unicode(thickness));
      double x_opticalconnector_slice_width = x_opticalconnector_slice.attr<double>(_Unicode(width));
      double x_opticalconnector_slice_length = x_opticalconnector_slice.attr<double>(_Unicode(length));
      Material x_opticalconnector_slice_mat;
      if(x_opticalconnector_slice.hasAttr(_Unicode(mat)))
	{
	  x_opticalconnector_slice_mat = theDetector.material(x_opticalconnector_slice.attr<string>(_Unicode(mat)));
	}
      else
	{
	  x_opticalconnector_slice_mat = theDetector.material(x_opticalconnector_slice.materialStr());
	}
      // Material x_opticalconnector_slice_mat = theDetector.material(x_opticalconnector_slice.attr<string>(_Unicode(mat)));
      Box opticalconnectorLayerSolid(x_opticalconnector_slice_thickness/2.0, x_opticalconnector_slice_width/2.0, x_opticalconnector_slice_length/2.);
      Volume opticalconnectorLayerLogical(name + dd4hep::_toString( layer_id, "_opticalconnectorLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), opticalconnectorLayerSolid, x_opticalconnector_slice_mat);
      opticalconnectorLayerLogical.setVisAttributes(theDetector.visAttributes(opticalconnectorVis));
      double opticalconnector_slice_height = opticalconnector_start_height + x_opticalconnector_slice_thickness/2.;
      pv = opticalconnectorEnvelopeLogical.placeVolume(opticalconnectorLayerLogical, Position(opticalconnector_slice_height, 0., 0.));  //not found the define of Position, why will it has a return
      std::cout << "opticalconnector slice" << index << " thickness = " << x_opticalconnector_slice_thickness/mm << "mm"  << std::endl;
      std::cout << "opticalconnector slice" << index << " width = " << x_opticalconnector_slice_width/mm << "mm"  << std::endl;
      std::cout << "opticalconnector slice" << index << " length = " << x_opticalconnector_slice_length/mm << "mm"  << std::endl;
      // std::cout << "opticalconnector material: " << x_opticalconnector_slice_mat << std::endl;
      opticalconnector_start_height += x_opticalconnector_slice_thickness;
      index++;
    }

    // create dataaggregation envelope logical volume
    Box dataaggregationEnvelopeSolid(dataaggregation_thickness / 2.0, dataaggregation_width/2., dataaggregation_length/2.);
    Volume dataaggregationEnvelopeLogical(name + dd4hep::_toString( layer_id, "_dataaggregationEnvelopeLogical_%02d"), dataaggregationEnvelopeSolid, air);
    dataaggregationEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    //sit.setVisAttributes(theDetector, dataaggregationVis, dataaggregationEnvelopeLogical);
     
    //create the dataaggregation layers inside the dataaggregation envelope
    double dataaggregation_start_height(-dataaggregation_thickness/2.); 
    index = 0;
    for(xml_coll_t dataaggregation_i(x_dataaggregation,_U(slice)); dataaggregation_i; ++dataaggregation_i){
      xml_comp_t x_dataaggregation_slice(dataaggregation_i);
      double x_dataaggregation_slice_thickness = x_dataaggregation_slice.attr<double>(_Unicode(thickness));
      double x_dataaggregation_slice_width = x_dataaggregation_slice.attr<double>(_Unicode(width));
      double x_dataaggregation_slice_length = x_dataaggregation_slice.attr<double>(_Unicode(length));
      Material x_dataaggregation_slice_mat;
      if(x_dataaggregation_slice.hasAttr(_Unicode(mat)))
	{
	  x_dataaggregation_slice_mat = theDetector.material(x_dataaggregation_slice.attr<string>(_Unicode(mat)));
	}
      else
	{
	  x_dataaggregation_slice_mat = theDetector.material(x_dataaggregation_slice.materialStr());
	}
      // Material x_dataaggregation_slice_mat = theDetector.material(x_dataaggregation_slice.attr<string>(_Unicode(mat)));
      Box dataaggregationLayerSolid(x_dataaggregation_slice_thickness/2.0, x_dataaggregation_slice_width/2.0, x_dataaggregation_slice_length/2.);
      Volume dataaggregationLayerLogical(name + dd4hep::_toString( layer_id, "_dataaggregationLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), dataaggregationLayerSolid, x_dataaggregation_slice_mat);
      dataaggregationLayerLogical.setVisAttributes(theDetector.visAttributes(dataaggregationVis));
      double dataaggregation_slice_height = dataaggregation_start_height + x_dataaggregation_slice_thickness/2.;
      pv = dataaggregationEnvelopeLogical.placeVolume(dataaggregationLayerLogical, Position(dataaggregation_slice_height, 0., 0.)); //not found the define of Position, why will it has a return
      std::cout << "dataaggregation slice" << index << " thickness = " << x_dataaggregation_slice_thickness/mm << "mm" << std::endl;
      std::cout << "dataaggregation slice" << index << " width = " << x_dataaggregation_slice_width/mm << "mm" << std::endl;
      std::cout << "dataaggregation slice" << index << " length = " << x_dataaggregation_slice_length/mm << "mm" << std::endl;
      // std::cout << "dataaggregation material: " << x_dataaggregation_slice_mat << std::endl;
      dataaggregation_start_height += x_dataaggregation_slice_thickness;
      index++;
    }

    // create DCDC envelope logical volume
    Box DCDCEnvelopeSolid(DCDC_thickness / 2.0, DCDC_width / 2.0, DCDC_length / 2.0);
    Volume DCDCEnvelopeLogical(name + dd4hep::_toString( layer_id, "_DCDCEnvelopeLogical_%02d"), DCDCEnvelopeSolid, air);
    DCDCEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    //sit.setVisAttributes(theDetector, DCDCVis, DCDCEnvelopeLogical);

    //create the DCDC layers inside the DCDC envelope
    double DCDC_start_height(-DCDC_thickness/2.); 
    index = 0;
    for(xml_coll_t DCDC_i(x_DCDC,_U(slice)); DCDC_i; ++DCDC_i){
      xml_comp_t x_DCDC_slice(DCDC_i);
      double x_DCDC_slice_thickness = x_DCDC_slice.attr<double>(_Unicode(thickness));
      double x_DCDC_slice_width = x_DCDC_slice.attr<double>(_Unicode(width));
      double x_DCDC_slice_length = x_DCDC_slice.attr<double>(_Unicode(length));
      Material x_DCDC_slice_mat;
      if(x_DCDC_slice.hasAttr(_Unicode(mat)))
	{
	  x_DCDC_slice_mat = theDetector.material(x_DCDC_slice.attr<string>(_Unicode(mat)));
	}
      else
	{
	  x_DCDC_slice_mat = theDetector.material(x_DCDC_slice.materialStr());
	}
      // Material x_DCDC_slice_mat = theDetector.material(x_DCDC_slice.attr<string>(_Unicode(mat)));
      Box DCDCLayerSolid(x_DCDC_slice_thickness/2.0, x_DCDC_slice_width/2.0, x_DCDC_slice_length/2.);
      Volume DCDCLayerLogical(name + dd4hep::_toString( layer_id, "_DCDCLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), DCDCLayerSolid, x_DCDC_slice_mat);
      DCDCLayerLogical.setVisAttributes(theDetector.visAttributes(DCDCVis));
      double DCDC_slice_height = DCDC_start_height + x_DCDC_slice_thickness/2.;
      pv = DCDCEnvelopeLogical.placeVolume(DCDCLayerLogical, Position(DCDC_slice_height, 0., 0.));  //not found the define of Position, why will it has a return
      std::cout << "DCDC slice" << index << " thickness = " << x_DCDC_slice_thickness/mm << "mm"  << std::endl;
      std::cout << "DCDC slice" << index << " width = " << x_DCDC_slice_width/mm << "mm"  << std::endl;
      std::cout << "DCDC slice" << index << " length = " << x_DCDC_slice_length/mm << "mm"  << std::endl;
      // std::cout << "DCDC material: " << x_DCDC_slice_mat << std::endl;
      DCDC_start_height += x_DCDC_slice_thickness;
      index++;
    }

    //place the lpGBTx & opticalconnector envelope inside the combine envelope 
    double combine_offset = 5*mm;
    pv = combineEnvelopeLogical.placeVolume(LpGBTxEnvelopeLogical, Position(-max_connector_thickness/2.0 + lpGBTx_thickness / 2.0, -lpGBTx_width/2.0 - combine_offset , -module_length/2.0+module_length/4.0+lpGBTx_length / 2.0));
    pv = combineEnvelopeLogical.placeVolume(dataaggregationEnvelopeLogical, Position(-max_connector_thickness/2.0 + dataaggregation_thickness / 2.0, -dataaggregation_width/2.0 - combine_offset , dataaggregation_length / 2.0));
    pv = combineEnvelopeLogical.placeVolume(opticalconnectorEnvelopeLogical, Position(-max_connector_thickness/2.0 + opticalconnector_thickness / 2.0, opticalconnector_width / 2.0+combine_offset , module_length/4.0 + opticalconnector_length / 2.0));
    pv = combineEnvelopeLogical.placeVolume(DCDCEnvelopeLogical, Position(-max_connector_thickness/2.0 + DCDC_thickness / 2.0, DCDC_width / 2.0 + combine_offset, -module_length/2.0 + DCDC_length / 2.0));

    // place the combine envelope inside the Stave envelope
    double combine_start_height = -stave_thickness/2.0 + tube_outer_radius*2. + support_thickness + module_thickness + flex_thickness;
    for(int icomb=0; icomb < n_modules_per_stave; ++icomb){
      double comb_total_z = n_modules_per_stave*module_length;
      //double xpos = 0.0;
      //double ypos = 0.0;
      double xpos = combine_start_height+max_connector_thickness / 2.0;
      double ypos = 0;
      double zpos = -comb_total_z/2.0 + module_length/2.0 + icomb*module_length;
      pv = StaveLogical.placeVolume(combineEnvelopeLogical, Position(xpos,ypos,zpos)); 
      //pv = PEEnvelopeLogical.placeVolume(combineEnvelopeLogical, Position(xpos,ypos,zpos)); 
    }

    // create Stave Support envelope logical volume
    Box StaveSupportEnvelopeSolid(support_thickness / 2.0, support_width / 2.0, support_half_length);
    Volume StaveSupportEnvelopeLogical(name + dd4hep::_toString( layer_id, "_StaveSupportEnvelopeLogical_%02d"), StaveSupportEnvelopeSolid, air);
    StaveSupportEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    //sit.setVisAttributes(theDetector, StaveSupportVis, StaveSupportEnvelopeLogical);

    //create the StaveSupport layers inside the StaveSupport envelope
    double StaveSupport_start_height(-support_thickness/2.); 

    index = 0;
    for(xml_coll_t StaveSupport_i(x_stave_support,_U(slice)); StaveSupport_i; ++StaveSupport_i){
      xml_comp_t x_StaveSupport_slice(StaveSupport_i);
      double x_StaveSupport_slice_thickness = x_StaveSupport_slice.attr<double>(_Unicode(thickness));
      double x_StaveSupport_slice_width = x_StaveSupport_slice.attr<double>(_Unicode(width));
      double x_StaveSupport_slice_half_length = x_StaveSupport_slice.attr<double>(_Unicode(length));
      Material x_StaveSupport_slice_mat;
      if(x_StaveSupport_slice.hasAttr(_Unicode(mat)))
	{
	  x_StaveSupport_slice_mat = theDetector.material(x_StaveSupport_slice.attr<string>(_Unicode(mat)));
	}
      else
	{
	  x_StaveSupport_slice_mat = theDetector.material(x_StaveSupport_slice.materialStr());
	}
      Box StaveSupportLayerSolid(x_StaveSupport_slice_thickness/2.0, x_StaveSupport_slice_width/2.0, x_StaveSupport_slice_half_length);
      Volume StaveSupportLayerLogical(name + dd4hep::_toString( layer_id, "_StaveSupportLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), StaveSupportLayerSolid, x_StaveSupport_slice_mat);
      StaveSupportLayerLogical.setVisAttributes(theDetector.visAttributes(supportVis));
      double StaveSupport_slice_height = StaveSupport_start_height + x_StaveSupport_slice_thickness/2.;
      pv = StaveSupportEnvelopeLogical.placeVolume(StaveSupportLayerLogical, Position(StaveSupport_slice_height, 0., 0.));
      std::cout << "Stave Support slice" << index << " thickness = " << x_StaveSupport_slice_thickness/mm << "mm" << std::endl;
      std::cout << "Stave Support slice" << index << " width = " << x_StaveSupport_slice_width/mm << "mm" << std::endl;
      std::cout << "Stave Support slice" << index << " half length = " << x_StaveSupport_slice_half_length/mm << "mm" << std::endl;
      // std::cout << "StaveSupport material: " << x_StaveSupport_slice_mat << std::endl;
      StaveSupport_start_height += x_StaveSupport_slice_thickness;
      index++;
    }

    //place the StaveSupport envelope inside the stave envelope
    double StaveSupportenv_start_height(-stave_thickness/2.0 + tube_outer_radius*2.);
    pv = StaveLogical.placeVolume(StaveSupportEnvelopeLogical, Position(StaveSupportenv_start_height + support_thickness / 2.0, 0., 0.));

    // create titanium tube logical volume
    Tube TitubeSolid(tube_inner_radius, tube_outer_radius, tube_half_length);
    Volume TitubeLogical(name + _toString( layer_id,"_TitubeLogical_%02d"), TitubeSolid, tube_mat);
    TitubeLogical.setVisAttributes(theDetector.visAttributes(tubeTiVis));
    
    pv = StaveLogical.placeVolume(TitubeLogical, Position(-stave_thickness/2.0 + tube_outer_radius, -support_width / 4.0, 0.));
    pv = StaveLogical.placeVolume(TitubeLogical, Position(-stave_thickness/2.0 + tube_outer_radius, support_width / 4.0, 0.));

    double stave_radius = stave_sens_radius - stave_thickness/2 + (max_connector_thickness + flex_thickness + sensor_thickness/2);
    for(int i = 0; i < n_staves; i++){
      std::stringstream stave_enum; 
      stave_enum << "sit_stave_" << layer_id << "_" << i;
      DetElement staveDE(layerDE, stave_enum.str(), x_det.id());
      //std::cout << "start building " << stave_enum.str() << ":" << endl; 

      //====== create the meassurement surface ===================
      dd4hep::rec::Vector3D o(0,0,0);
      dd4hep::rec::Vector3D u( 0., 1., 0.);
      dd4hep::rec::Vector3D v( 0., 0., 1.);
      dd4hep::rec::Vector3D n( 1., 0., 0.);
      double inner_thick = tube_outer_radius*2. + support_thickness + module_thickness/2.0;
      double outer_thick = max_connector_thickness + flex_thickness + module_thickness / 2.0;
      dd4hep::rec::VolPlane surf( SensorLogical,
				  dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
				  inner_thick, outer_thick , u,v,n,o ) ;
    
      for(int imodule=0; imodule < n_modules_per_stave; ++imodule){
	std::stringstream module_str;
	module_str << stave_enum.str() << "_module_" << imodule;
	//std::cout << "\tstart building " << module_str.str() << ":" << endl;
	DetElement moduleDE(staveDE, module_str.str(), x_det.id());
	moduleDE.setPlacement(Module_pv[imodule]);
	//std::cout << "\t" << module_str.str() << " done." << endl;

	for (int isensor = 0; isensor < n_sensors_per_module; ++isensor) {
	  std::stringstream sensor_str;
	  sensor_str << module_str.str() << "_sensor_" << isensor;
	  //std::cout << "\tstart building " << sensor_str.str() << ":" << endl;
	  DetElement sensorDE(moduleDE, sensor_str.str(), x_det.id());
	  sensorDE.setPlacement(Sensor_pv[isensor]);
	  volSurfaceList(sensorDE)->push_back(surf);
	  //std::cout << "\t" << sensor_str.str() << " done." << endl;
	}
      }

      //double stave_radius = stave_sens_radius - stave_thickness/2 + (max_connector_thickness + flex_thickness + sensor_thickness/2);  
      Transform3D tr (RotationZYX(stave_dphi*i,0.,0.),Position(stave_radius*cos(stave_phi0+stave_dphi*i), stave_radius*sin(stave_phi0+stave_dphi*i), 0.));
      //std::cout << "1st Test!!!" << endl;
      pv = layer_assembly.placeVolume(StaveLogical,tr);
      //std::cout << "2ed Test!!!" << endl;
      pv.addPhysVolID("layer", layer_id).addPhysVolID("stave", i ) ;
      
      staveDE.setPlacement(pv);
      //std::cout << stave_enum.str() << " done." << endl;
      if(i==0) std::cout << "xy=" << stave_radius*cos(stave_phi0) << " " << stave_radius*sin(stave_phi0) << std::endl;
    }

    // package the reconstruction data
    dd4hep::rec::ZPlanarData::LayerLayout Layer;

    //Layer.staveNumber         = n_staves;
    Layer.ladderNumber         = n_staves;
    Layer.phi0                 = 0.;
    //Layer.modulesPerStave     = n_modules_per_stave;
    Layer.sensorsPerLadder     = n_modules_per_stave;
    //Layer.sensorsPerLadder     = n_modules_per_stave*n_sensors_per_module;
    //Layer.lengthModule         = module_active_length;
    Layer.lengthSensor         = module_length;//sensor_active_length;
    Layer.distanceSupport      = stave_radius*cos(stave_phi0) + StaveSupportenv_start_height; //sensitive_radius;
    Layer.thicknessSupport     = support_thickness;
    Layer.offsetSupport        = stave_radius*sin(stave_phi0);//-stave_offset;
    Layer.widthSupport         = support_width;
    Layer.zHalfSupport         = module_length*n_modules_per_stave/2.0;//support_half_length;
    Layer.distanceSensitive    = stave_radius*cos(stave_phi0) + StaveSupportenv_start_height + support_thickness;
                               //sensitive_radius + tube_outer_radius*2. + support_thickness;
    Layer.thicknessSensitive   = sensor_thickness;//module_thickness;
    Layer.offsetSensitive      = stave_radius*sin(stave_phi0);//-stave_offset;//stave_offset/2.0;
    Layer.widthSensitive       = module_width;
    Layer.zHalfSensitive       = module_length*n_modules_per_stave/2.0;//support_half_length;

    zPlanarData->layers.push_back(Layer);
 }
 std::cout << (*zPlanarData) << endl;
 //sit.addExtension< ZPlanarData >(zPlanarData);
 sit.addExtension<ZPlanarData>(zPlanarData);
 if ( x_det.hasAttr(_U(combineHits)) ) {
    sit.setCombineHits(x_det.attr<bool>(_U(combineHits)),sens);
 }
 std::cout << "sit done." << endl; 
 return sit;
}
DECLARE_DETELEMENT(SiTracker_itkbarrel_v02, create_element)
