//====================================================================
//  cepcsitbgeo - CEPC silicon inner tracker barrel models in DD4hep 
//--------------------------------------------------------------------
//  v01, Peripheral electronics on the side of staves
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

double findMax(double a, double b, double c) {
  if (a >= b && a >= c) {
    return a;
  } else if (b >= a && b >= c) {
    return b;
  } else {
    return c;
  }
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


/** Construction of the SIT detector, ported from Hao Zeng SiTrackerStaggeredLadder_v01_geo.cpp
 *
 *  Modify History:
 *  Steve Aplin, Feb 7th 2011 - original version
 *  F.Gaede, Jan 2014, DESY   - dd4hep SIT pixel
 *  Hao Zeng, July 2021, IHEP - dd4hep VXD pixel
 *  Xiaojie Jiang, July 2024, IHEP - dd4hep SIT pixel
 */

static dd4hep::Ref_t create_element(dd4hep::Detector& theDetector, xml_h e, dd4hep::SensitiveDetector sens)  {  

  xml_det_t  x_det    = e;
  Material   air      = theDetector.air();  // what's the air
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

  //No fetch global paras
  //fetch the display parameters
  xml_comp_t x_display(x_det.child(_Unicode(display)));  //where define the xml_comp_t?o
  std::string staveVis      = x_display.attr<string>(_Unicode(stave));
  std::string supportVis     = x_display.attr<string>(_Unicode(support));
  std::string flexVis        = x_display.attr<string>(_Unicode(flex));
  std::string sensEnvVis     = x_display.attr<string>(_Unicode(sens_env));
  std::string sensVis        = x_display.attr<string>(_Unicode(sens));
  std::string deadsensVis    = x_display.attr<string>(_Unicode(deadsensor));
  std::string deadwireVis    = x_display.attr<string>(_Unicode(deadwire));
  std::string portsVis       = x_display.attr<string>(_Unicode(ports));
  std::string lpGBTxVis      = x_display.attr<string>(_Unicode(lpGBTx));
  std::string opticalconnectorVis    = x_display.attr<string>(_Unicode(opticalconnector));

  for(xml_coll_t layer_i(x_det,_U(layer)); layer_i; ++layer_i){
    xml_comp_t x_layer(layer_i); 
   
    dd4hep::PlacedVolume pv;
    int layer_id                 = x_layer.attr<int>(_Unicode(layer_id));

    std::cout << "layer_id: " << layer_id << endl;

    double sensitive_radius      = x_layer.attr<double>(_Unicode(sensitive_radius));
    int n_staves                = x_layer.attr<int>(_Unicode(n_staves)) ;
    double stave_offset           = x_layer.attr<int>(_Unicode(stave_offset)) ;
    double stave_phi0           = -atan(stave_offset/sensitive_radius);
    double stave_radius         = sqrt(stave_offset*stave_offset + sensitive_radius*sensitive_radius); 
  
    std::cout << "stave_radius: " << stave_radius/mm <<" mm" << endl;
    std::cout << "sensitive_radius: " << sensitive_radius/mm << " mm" << endl;
    std::cout << "n_staves" << n_staves << endl;

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
   
    //fetch the stave support parameters
    xml_comp_t x_stave_support(x_stave.child(_Unicode(staveSupport)));
    double support_half_length        = x_stave_support.attr<double>(_Unicode(length));
    double support_thickness     = x_stave_support.attr<double>(_Unicode(thickness));
    double support_width         = x_stave_support.attr<double>(_Unicode(width));
    Material support_mat;
    if(x_stave_support.hasAttr(_Unicode(mat)))
      {
	support_mat = theDetector.material(x_stave_support.attr<string>(_Unicode(mat)));
      }
    else
      {
	support_mat = theDetector.material(x_stave_support.materialStr());  //?
      }
    std::cout << "support_half_length: " << support_half_length/mm << " mm" << endl;
    std::cout << "support_thickness: " << support_thickness/mm << " mm" << endl;
    std::cout << "support_width: " << support_width/mm << " mm" << endl;

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
    double dead_gap                         = x_module.attr<double>(_Unicode(gap));
    double module_thickness                 = x_module.attr<double>(_Unicode(thickness));
    double module_active_length                = x_module.attr<double>(_Unicode(active_length));
    double module_active_width              = x_module.attr<double>(_Unicode(active_width));
    double module_dead_width                = x_module.attr<double>(_Unicode(dead_width));
    double module_deadwire_length           = x_module.attr<double>(_Unicode(deadwire_length));
    double module_deadwire_width            = x_module.attr<double>(_Unicode(deadwire_width));
    double module_deadwire_thickness        = x_module.attr<double>(_Unicode(deadwire_thickness));
    Material module_mat                     = theDetector.material(x_module.attr<string>(_Unicode(module_mat)));
    Material module_deadwire_mat            = theDetector.material(x_module.attr<string>(_Unicode(deadwire_mat)));

    std::cout << "n_modules_per_stave: " << n_modules_per_stave << endl;
    std::cout << "dead_gap: " << dead_gap/mm << " mm" << endl;
    std::cout << "module_thickness: " << module_thickness/mm << " mm" << endl;
    std::cout << "module_active_length: " << module_active_length/mm << " mm" << endl;
    std::cout << "module_active_width: " << module_active_width/mm << " mm" << endl;
    std::cout << "module_dead_width: " << module_dead_width/mm << " mm" << endl;

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

    //create stave logical volume
    double max_thickness = findMax(module_thickness, lpGBTx_thickness, opticalconnector_thickness);
    double stave_thickness = support_thickness+flex_thickness+max_thickness;
    Box StaveSolid(stave_thickness / 2.0, support_width / 2.0, support_half_length);
    Volume StaveLogical(name + dd4hep::_toString( layer_id, "_StaveLogical_%02d"),
			StaveSolid, air); 
   
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
      pv = FlexEnvelopeLogical.placeVolume(FlexLayerLogical, Position(flex_slice_height, 0., 0.));  //not found the define of Position, why will it has a return
      std::cout << "flex thickness = " << x_flex_slice_thickness << std::endl;
      std::cout << "flex width = " << x_flex_slice_width << std::endl;
      std::cout << "flex half length = " << x_flex_slice_half_length << std::endl;
      // std::cout << "flex material: " << x_flex_slice_mat << std::endl;
      flex_start_height += x_flex_slice_thickness;
      index++;
    }

    //place the flex envelope inside the stave envelope
    double flexenv_start_height(-stave_thickness/2.0+support_thickness);
    double flexenv_slice_height=flexenv_start_height+(flex_thickness) / 2.0;
    pv = StaveLogical.placeVolume(FlexEnvelopeLogical, Position(flexenv_slice_height, 0., 0.)); 
    //define the transformation3D(only need a combination of translation and rotation)
    // Transform3D tran_mirro(RotationZYX(0., dd4hep::twopi/2.0, 0.), Position(-flexenv_slice_height, 0., 0.));
    // pv = StaveLogical.placeVolume(FlexEnvelopeLogical, tran_mirro); //bottom side

    //create module envelope logical volume
    double moduleenv_width = module_active_width + 2*module_dead_width + 2*module_deadwire_width;
    Box ModuleEnvelopeSolid(module_thickness / 2.0,  moduleenv_width/2., support_half_length);
    Volume ModuleEnvelopeLogical(name + dd4hep::_toString( layer_id, "_ModuleEnvelopeLogical_%02d"), ModuleEnvelopeSolid, air);
    ModuleEnvelopeLogical.setVisAttributes(theDetector.visAttributes(sensEnvVis));

    //create module logical volume
    Box ModuleSolid(module_thickness / 2.0, module_active_width / 2.0, module_active_length / 2.0);
    Volume ModuleLogical(name + dd4hep::_toString( layer_id, "_ModuleLogical_%02d"), ModuleSolid, module_mat);
    ModuleLogical.setSensitiveDetector(sens);
    //sit.setVisAttributes(theDetector, deadsensVis, ModuleDeadLogical);
    ModuleLogical.setVisAttributes(theDetector.visAttributes(sensVis));
    
    //create dead module logical volume
    Box ModuleDeadSolid(module_thickness / 2.0, module_dead_width / 2.0, module_active_length / 2.0);
    Volume ModuleDeadLogical(name + dd4hep::_toString( layer_id, "_ModuleDeadLogical_%02d"), ModuleDeadSolid, module_mat);
    ModuleDeadLogical.setVisAttributes(theDetector.visAttributes(deadsensVis));
    
    //create dead wire logical volume
    Box ModuleDeadWireSolid(module_deadwire_thickness / 2.0, module_deadwire_width / 2.0, module_deadwire_length / 2.0);
    Volume ModuleDeadWireLogical(name + dd4hep::_toString( layer_id, "_ModuleDeadWireLogical_%02d"), ModuleDeadWireSolid, module_deadwire_mat);
    ModuleDeadWireLogical.setVisAttributes(theDetector.visAttributes(deadwireVis));

    //place the dead wire in the module envelope
    // pv = ModuleEnvelopeLogical.placeVolume(ModuleDeadWireLogical, Position(0.0, (module_active_width-support_width/2.0) + module_dead_width/2.0 + module_deadwire_width/2.0, 0.0));
    // pv = ModuleBottomEnvelopeLogical.placeVolume(ModuleDeadWireLogical, Position(0.0, (module_active_width-support_width/2.0) + module_dead_width/2.0 + module_deadwire_width/2.0, (0.0));
    pv = ModuleEnvelopeLogical.placeVolume(ModuleDeadWireLogical, Position((- module_thickness+module_deadwire_thickness) / 2.0, (-moduleenv_width/2.0) + (module_deadwire_width/2.0), 0.0));  //left side, 
    pv = ModuleEnvelopeLogical.placeVolume(ModuleDeadWireLogical, Position((- module_thickness+module_deadwire_thickness) / 2.0, (moduleenv_width/2.0) - (module_deadwire_width/2.0), 0.0));  //right side

    // place the active module and dead module inside the module envelope
    std::vector<dd4hep::PlacedVolume> Module_pv;
    for(int imodule=0; imodule < n_modules_per_stave; ++imodule){
      double module_total_z = n_modules_per_stave*module_active_length + 2*dead_gap*n_modules_per_stave;
      double xpos = 0.0;
      double ypos_active = 0.0;
      double left_ypos_dead = (-moduleenv_width/2.0) + module_deadwire_width + (module_dead_width/2.0);
      double right_ypos_dead = (moduleenv_width/2.0) - module_deadwire_width - (module_dead_width/2.0);
      double zpos = -module_total_z/2.0 + module_active_length/2.0 + dead_gap + imodule*(module_active_length + 2*dead_gap);
      pv = ModuleEnvelopeLogical.placeVolume(ModuleLogical, Position(xpos,ypos_active,zpos));
      //pv.addPhysVolID("topmodule",  imodule ) ;
      pv.addPhysVolID("layer", layer_id).addPhysVolID("module", imodule) ;
      Module_pv.push_back(pv); 
      pv = ModuleEnvelopeLogical.placeVolume(ModuleDeadLogical, Position(xpos,left_ypos_dead,zpos));
      pv = ModuleEnvelopeLogical.placeVolume(ModuleDeadLogical, Position(xpos,right_ypos_dead,zpos));
    }
    
    //create Peripheral electronics envelope logical volume
    double PEenv_width = (lpGBTx_width > opticalconnector_width) ? lpGBTx_width : opticalconnector_width;
    double PEenv_thickness = (lpGBTx_thickness > opticalconnector_thickness) ? lpGBTx_thickness : opticalconnector_thickness;
    // Box PEEnvelopeSolid(PEenv_thickness / 2.0,  PEenv_width/2., support_half_length);
    // Volume PEEnvelopeLogical(name + dd4hep::_toString( layer_id, "_PEEnvelopeLogical_%02d"), PEEnvelopeSolid, air);
    // PEEnvelopeLogical.setVisAttributes(theDetector.visAttributes(lpGBTxVis));

    //create combine(lpGBTx + opticalconnector) envelope logical volume
    Box combineEnvelopeSolid(PEenv_thickness / 2.0,  PEenv_width/2., module_active_length/2.0);
    Volume combineEnvelopeLogical(name + dd4hep::_toString( layer_id, "_combineEnvelopeLogical_%02d"), combineEnvelopeSolid, air);
    combineEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));

    // create lpGBTx envelope logical volume
    Box LpGBTxEnvelopeSolid(lpGBTx_thickness / 2.0, lpGBTx_width/2., lpGBTx_length/2.);
    Volume LpGBTxEnvelopeLogical(name + dd4hep::_toString( layer_id, "_LpGBTxEnvelopeLogical_%02d"), LpGBTxEnvelopeSolid, air);
    LpGBTxEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
    //sit.setVisAttributes(theDetector, lpGBTxVis, LpGBTxEnvelopeLogical);
    std::cout << "lpGBTx Env thickness = " << lpGBTx_width/mm << "mm"  << std::endl;
    std::cout << "lpGBTx Env length = " << lpGBTx_length/mm << "mm"  << std::endl;
    
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
      std::cout << "lpGBTx thickness = " << x_lpGBTx_slice_thickness << std::endl;
      std::cout << "lpGBTx width = " << x_lpGBTx_slice_width << std::endl;
      std::cout << "lpGBTx length = " << x_lpGBTx_slice_length << std::endl;
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
      std::cout << "opticalconnector thickness = " << x_opticalconnector_slice_thickness << std::endl;
      std::cout << "opticalconnector width = " << x_opticalconnector_slice_width << std::endl;
      std::cout << "opticalconnector length = " << x_opticalconnector_slice_length << std::endl;
      // std::cout << "opticalconnector material: " << x_opticalconnector_slice_mat << std::endl;
      opticalconnector_start_height += x_opticalconnector_slice_thickness;
      index++;
    }

    //place the lpGBTx & opticalconnector envelope inside the combine envelope
    pv = combineEnvelopeLogical.placeVolume(LpGBTxEnvelopeLogical, Position(-PEenv_thickness/2.0 + lpGBTx_thickness / 2.0, 0., -module_active_length/2.0+lpGBTx_length / 2.0));
    pv = combineEnvelopeLogical.placeVolume(opticalconnectorEnvelopeLogical, Position(-PEenv_thickness/2.0 + opticalconnector_thickness / 2.0, 0., opticalconnector_length / 2.0));

    // place the combine envelope inside the Stave envelope
    double modulePE_start_height = -stave_thickness / 2.0 + support_thickness + flex_thickness;
    for(int icomb=0; icomb < n_modules_per_stave; ++icomb){
      double comb_total_z = n_modules_per_stave*module_active_length + 2*dead_gap*n_modules_per_stave;
      //double xpos = 0.0;
      //double ypos = 0.0;
      double xpos = modulePE_start_height+PEenv_thickness/2.;
      double ypos = -support_width / 2.0+ PEenv_width/2.0;
      double zpos = -comb_total_z/2.0 + module_active_length/2.0 + dead_gap + icomb*(module_active_length + 2*dead_gap);
      pv = StaveLogical.placeVolume(combineEnvelopeLogical, Position(xpos,ypos,zpos)); 
      //pv = PEEnvelopeLogical.placeVolume(combineEnvelopeLogical, Position(xpos,ypos,zpos)); 
    }

    //place the module envelope inside the stave envelope
    pv = StaveLogical.placeVolume(ModuleEnvelopeLogical, Position(modulePE_start_height+module_thickness/2.0, support_width / 2.0 - moduleenv_width/2.0, 0.));
    //pv = StaveLogical.placeVolume(PEEnvelopeLogical, Position(modulePE_start_height+PEenv_thickness/2.0, -support_width / 2.0 + PEenv_width/2.0, 0.));
    
    //create the stave support envelope
    Box StaveSupportEnvelopeSolid(support_thickness/2.0, support_width/2.0, support_half_length);
    Volume StaveSupportEnvelopeLogical(name + _toString( layer_id,"_SupEnvLogical_%02d"), StaveSupportEnvelopeSolid, air);
    sit.setVisAttributes(theDetector, "seeThrough", StaveSupportEnvelopeLogical);

    //create stave support volume
    Box StaveSupportSolid(support_thickness / 2.0 , support_width / 2.0 , support_half_length);
    Volume StaveSupportLogical(name + _toString( layer_id,"_SupLogical_%02d"), StaveSupportSolid, support_mat);
    StaveSupportLogical.setVisAttributes(theDetector.visAttributes(supportVis));
   
    pv = StaveSupportEnvelopeLogical.placeVolume(StaveSupportLogical);
    pv = StaveLogical.placeVolume(StaveSupportEnvelopeLogical, Position(- stave_thickness / 2.0 + support_thickness / 2.0, 0.0, 0.0));

  for(int i = 0; i < n_staves; i++){
    std::stringstream stave_enum; 
    stave_enum << "sit_stave_" << layer_id << "_" << i;
    DetElement staveDE(layerDE, stave_enum.str(), x_det.id());
    std::cout << "start building " << stave_enum.str() << ":" << endl;

    //====== create the meassurement surface ===================
    dd4hep::rec::Vector3D o(0,0,0);
    dd4hep::rec::Vector3D u( 0., 0., 1.);
    dd4hep::rec::Vector3D v( 0., 1., 0.);
    dd4hep::rec::Vector3D n( 1., 0., 0.);
    double inner_thick = support_thickness/2.0 + flex_thickness + module_thickness/2.0;
    double outer_thick = module_thickness/2.0;
    dd4hep::rec::VolPlane surf( ModuleLogical ,
                                dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
                                inner_thick, outer_thick , u,v,n,o ) ;
    
    for(int imodule=0; imodule < n_modules_per_stave; ++imodule){
      std::stringstream module_str;
      module_str << stave_enum.str() << "_" << imodule;
      // std::cout << "\tstart building " << module_str.str() << ":" << endl;
      DetElement moduleDE(staveDE, module_str.str(), x_det.id());
      moduleDE.setPlacement(Module_pv[imodule]);
      volSurfaceList(moduleDE)->push_back(surf);
      // std::cout << "\t" << module_str.str() << " done." << endl;
    }
    
    // double offset = (support_width - moduleenv_width) / 2.0;
    // double stave_radius = sqrt((sensitive_radius - offset / 2.0 * sin(stave_theta))(sensitive_radius - offset / 2.0 * sin(stave_phi))+ (offset / 2.0 * cos(stave_theta)) * (offset / 2.0 * cos(stave_theta)));
    // double stave_phi0 = asin((offset / 2.0 * cos(stave_theta))/stave_radius);
    Transform3D tr (RotationZYX(stave_dphi*i,0.,0.),Position(stave_radius*cos(stave_phi0+stave_dphi*i), stave_radius*sin(stave_phi0+stave_dphi*i), 0.));
    pv = layer_assembly.placeVolume(StaveLogical,tr);
    pv.addPhysVolID("layer", layer_id).addPhysVolID("stave", i ) ; 
    staveDE.setPlacement(pv);
    std::cout << stave_enum.str() << " done." << endl;
    if(i==0) std::cout << "xy=" << stave_radius*cos(stave_phi0) << " " << stave_radius*sin(stave_phi0) << std::endl;
  }
    
  // package the reconstruction data
  dd4hep::rec::ZPlanarData::LayerLayout Layer;

  //Layer.staveNumber         = n_staves;
  Layer.ladderNumber         = n_staves;
  Layer.phi0                 = 0.;
  //Layer.modulesPerStave     = n_modules_per_stave;
  Layer.sensorsPerLadder     = n_modules_per_stave;
  //Layer.lengthModule         = module_active_length;
  Layer.lengthSensor         = module_active_length;
  Layer.distanceSupport      = sensitive_radius;
  Layer.thicknessSupport     = support_thickness / 2.0;
  Layer.offsetSupport        = -stave_offset;
  Layer.widthSupport         = support_width;
  Layer.zHalfSupport         = support_half_length;
  Layer.distanceSensitive    = sensitive_radius + support_thickness + flex_thickness;
  Layer.thicknessSensitive   = module_thickness;
  Layer.offsetSensitive      = -stave_offset/2.0 + moduleenv_width/2.0;
  Layer.widthSensitive       = module_active_width;
  Layer.zHalfSensitive       = support_half_length;

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
DECLARE_DETELEMENT(SiTracker_itkbarrel_v01,create_element)
