//====================================================================
//  cepcvxdgeo - CEPC vertex detector models in DD4hep 
//--------------------------------------------------------------------
//  Hao Zeng, IHEP
//  email: zenghao@ihep.ac.cn
//  $Id$
//====================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetType.h"
#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "XML/Utilities.h"
#include <cmath>

#include "Identifier/CEPCDetectorData.h"

using namespace std;

using dd4hep::Box;
using dd4hep::Tube;
using dd4hep::DetElement;
using dd4hep::Material;
using dd4hep::Position;
using dd4hep::RotationZ;
using dd4hep::RotationZYX;
using dd4hep::Transform3D;
using dd4hep::Rotation3D;
using dd4hep::Volume;
using dd4hep::_toString;
using dd4hep::rec::volSurfaceList;
using dd4hep::rec::CylindricalData;
using dd4hep::mm;

/** helper struct */
struct VXD_Module {
  //  int     n_ladders;
  //int     n_sensors_per_ladder;
  //double  sensor_length;
  double  half_z;
  double  sensitive_inner_radius;
  double  support_inner_radius;
  double  support_thickness;
  double  width;
  double  ladder_dphi;
};    

/** Construction of the VXD detector, stitching sensor
 *  @author
 */
static dd4hep::Ref_t create_element(dd4hep::Detector& theDetector, xml_h e, dd4hep::SensitiveDetector sens)  {

  xml_det_t  x_det    = e;
  Material   air      = theDetector.air();
  int        det_id   = x_det.id();
  string     name     = x_det.nameStr();
  DetElement det(name, det_id);

  Volume envelope = dd4hep::xml::createPlacedEnvelope(theDetector, e, det);
  dd4hep::xml::setDetectorTypeFlag(e, det);
  if (theDetector.buildType()==dd4hep::BUILD_ENVELOPE) return det;
  envelope.setVisAttributes(theDetector.visAttributes("SeeThrough"));

  if (x_det.hasChild(_U(sensitive))) {
    xml_dim_t sd_typ = x_det.child(_U(sensitive));
    sens.setType(sd_typ.typeStr());
  }
  else {
    sens.setType("tracker");
  }
  std::cout << " ** building SiTrackerStitchingLadder_v01 ... " << sens.type() << std::endl ;

  dd4hep::rec::CylindricalData*  cylindricalData = new dd4hep::rec::CylindricalData;
  //fetch the shell parameters
  if (x_det.hasChild(_Unicode(shell))) {
    xml_comp_t x_shell(x_det.child(_Unicode(shell)));
    double rmin_shell = x_shell.rmin();
    double rmax_shell = x_shell.rmax();
    double zhalf_shell = x_shell.zhalf();
    Tube shellSolid(rmin_shell, rmax_shell, zhalf_shell);
    Volume shellLogical(name + "_ShellLogical", shellSolid, theDetector.material(x_shell.materialStr()));
    shellLogical.setVisAttributes(theDetector.visAttributes(x_shell.visStr()));
    dd4hep::PlacedVolume pv = envelope.placeVolume(shellLogical);

    DetElement shellDE(det, name + "_Shell", x_det.id());
    shellDE.setPlacement(pv);

    cylindricalData->zHalfShell  = zhalf_shell;
    cylindricalData->rInnerShell = rmin_shell;
    cylindricalData->rOuterShell = rmax_shell;
  }

  std::map<std::string, Volume> module_volumes;
  std::map<std::string, VXD_Module> module_data;

  xml_comp_t x_modules(x_det.child(_U(modules)));
  for (xml_coll_t module_i(x_modules, _U(module)); module_i; ++module_i) {
    xml_comp_t x_module(module_i);
    std::string type = x_module.typeStr();
    double radius = x_module.radius();
    double xdead   = x_module.attr<double>(_Unicode(xdead));
    double ydead   = x_module.attr<double>(_Unicode(ydead));
    int nx = x_module.attr<int>(_Unicode(nx));
    int ny = x_module.attr<int>(_Unicode(ny));
    Material mat = theDetector.material(x_module.materialStr());

    xml_comp_t x_sensor(x_module.child(_U(sensor)));
    double sensor_thickness = x_sensor.thickness();
    double sensor_width     = x_sensor.width();
    double sensor_length    = x_sensor.length();
    double sensor_dphi      = sensor_width/radius;
    Material sensor_mat     = theDetector.material(x_sensor.materialStr());
    
    Tube sensor_solid(radius, radius+sensor_thickness, sensor_length/2, -sensor_width/radius/2, sensor_width/radius/2);
    Volume sensor_volume(name + type + "Sensor", sensor_solid, sensor_mat);
    sensor_volume.setSensitiveDetector(sens);
    if (x_det.hasAttr(_U(limits))) sensor_volume.setLimitSet(theDetector, x_det.limitsStr());
    sensor_volume.setVisAttributes(theDetector.visAttributes(x_sensor.visStr()));
    
    xml_comp_t x_flex(x_module.child(_Unicode(flex)));
    std::vector<std::pair<double, Material> > flexs;
    double flex_thickness = 0;
    for (xml_coll_t slice_i(x_flex, _U(slice)); slice_i; ++slice_i) {
      xml_comp_t x_slice(slice_i);
      double thickness = x_slice.thickness();
      Material mat     = theDetector.material(x_slice.materialStr());
      flexs.push_back(std::make_pair(thickness, mat));
      flex_thickness += thickness;
      std::cout << "flex thickness: " << thickness << std::endl;
    }
    double flex_width  = sensor_width*ny + ydead*(ny-1);
    double flex_length = sensor_length*nx + xdead*(nx-1);
    double flex_radius = radius+sensor_thickness;
    double flex_dphi   = flex_width/radius;
    Tube flex_solid(flex_radius, flex_radius+flex_thickness, flex_length/2, -flex_dphi/2, flex_dphi/2);
    Volume flex_volume(name + type + "Flex", flex_solid, air);
    flex_volume.setVisAttributes(theDetector.visAttributes(x_flex.visStr()));
    double start_radius = flex_radius;
    for (unsigned islice=0; islice<flexs.size(); islice++) {
      //std::cout << "flex start radius " << start_radius << endl;
      Tube slice_solid(start_radius, start_radius+flexs[islice].first, flex_length/2, -flex_dphi/2, flex_dphi/2);
      Volume slice_volume(name + type + dd4hep::_toString(int(islice), "Flex_%d"), slice_solid, flexs[islice].second);
      flex_volume.placeVolume(slice_volume);
      start_radius += flexs[islice].first;
    }

    double service_radius = radius + sensor_thickness;

    xml_comp_t x_electronics(x_module.child(_Unicode(electronics)));
    double electronics_thickness = x_electronics.thickness();
    double electronics_width = x_electronics.width();
    double electronics_dphi  = electronics_width/radius;
    Material electronics_mat = theDetector.material(x_electronics.materialStr());
    Tube electronics_solid(service_radius, service_radius+electronics_thickness, flex_length/2, -electronics_dphi, 0);
    Volume electronics_volume(name + type + "Electronics", electronics_solid, electronics_mat);
    electronics_volume.setVisAttributes(theDetector.visAttributes(x_electronics.visStr()));

    xml_comp_t x_readout(x_module.child(_U(readout)));
    double readout_thickness = x_readout.thickness();
    double readout_width = x_readout.width();
    double readout_dphi  = readout_width/radius;
    Material readout_mat = theDetector.material(x_readout.materialStr());
    Tube readout_solid(service_radius, service_radius+readout_thickness, flex_length/2, 0, readout_dphi);
    Volume readout_volume(name + type + "Electronics", readout_solid, readout_mat);
    readout_volume.setVisAttributes(theDetector.visAttributes(x_readout.visStr()));

    xml_comp_t x_driver(x_module.child(_Unicode(driver)));
    double driver_thickness = x_driver.thickness();
    // width -> length, length->width
    double driver_length = x_driver.width();
    double driver_width  = flex_width + electronics_width + readout_width;
    double driver_dphi  = driver_width/radius;
    Material driver_mat = theDetector.material(x_driver.materialStr());
    Tube driver_solid(service_radius, service_radius+driver_thickness, driver_length/2, -flex_dphi/2-electronics_dphi, flex_dphi/2+readout_dphi);
    Volume driver_volume(name + type + "Driver", driver_solid, driver_mat);
    driver_volume.setVisAttributes(theDetector.visAttributes(x_driver.visStr()));

    double module_length = flex_length + 2*driver_length;
    double module_width  = driver_width;
    double module_dphi   = module_width/radius;
    double module_thickness = sensor_thickness + std::max(std::max(std::max(flex_thickness, driver_thickness), readout_thickness), electronics_thickness);
    Tube module_solid(radius, radius+module_thickness, module_length/2, -flex_dphi/2-electronics_dphi, flex_dphi/2+readout_dphi);
    Volume module_volume(name + type, module_solid, air);
    module_volume.setVisAttributes(theDetector.visAttributes("SeeThrough"));

    Tube board_solid(radius, radius+sensor_thickness, module_length/2, -flex_dphi/2-electronics_dphi, flex_dphi/2+readout_dphi);
    Volume board_volume(name + type + "Board", board_solid, mat);
    board_volume.setVisAttributes(theDetector.visAttributes(x_module.visStr()));
    module_volume.placeVolume(board_volume);

    for (int ix=0; ix<nx; ix++) {
      double z = -flex_length/2+sensor_length/2+ix*(sensor_length+xdead);
      for (int iy=0; iy<ny; iy++) {
	double delta = -flex_dphi/2+sensor_dphi/2+iy*(sensor_dphi+ydead/radius);
	Transform3D tran(RotationZ(delta), Position(0, 0, z));
	dd4hep::PlacedVolume pv = board_volume.placeVolume(sensor_volume, tran);

	int sensor_id = ix + nx*iy;
	pv.addPhysVolID("sensor", sensor_id);
      }
    }

    module_volume.placeVolume(flex_volume);
    module_volume.placeVolume(electronics_volume, RotationZYX(-flex_dphi/2,0,0));
    module_volume.placeVolume(readout_volume, RotationZYX(flex_dphi/2,0,0));
    module_volume.placeVolume(driver_volume, Position(0, 0, -flex_length/2-driver_length/2));
    module_volume.placeVolume(driver_volume, Position(0, 0,  flex_length/2+driver_length/2));
    module_volumes[type] = module_volume;

    VXD_Module thisModule;
    thisModule.half_z                 = flex_length/2;
    thisModule.sensitive_inner_radius = radius;
    thisModule.support_inner_radius   = flex_radius;
    thisModule.support_thickness      = flex_thickness;
    thisModule.width                  = flex_width;
    module_data[type] = thisModule;
  }

  for(xml_coll_t layer_i(x_det,_U(layer)); layer_i; ++layer_i){
    xml_comp_t x_layer(layer_i);

    int layer_id                 = x_layer.id();
    std::cout << "layer_id: " << layer_id << endl;

    dd4hep::rec::CylindricalData::LayerLayout thisLayer;
    
    double phi0 = x_layer.phi0();

    //thisLayer.phi0 = phi0;
    thisLayer.id   = layer_id;

    int module_id = 0;
    for (xml_coll_t module_i(x_layer, _U(module)); module_i; ++module_i) {
      if (module_id>1) std::cerr << "Not support more than two modules, possible wrong in reconstruction" << std::endl;
      xml_comp_t x_module(module_i);
      std::string type = x_module.typeStr();
      std::cout << ">>Module " << type << endl;
      double offset = x_module.offset();
      double phi    = x_module.phi();
      double z      = x_module.z();
      Transform3D tran(RotationZ(phi0+phi), Position(offset*cos(phi), offset*sin(phi), z));
      dd4hep::PlacedVolume pv = envelope.placeVolume(module_volumes[type], tran);
      pv.addPhysVolID("layer", layer_id).addPhysVolID("module", module_id);

      DetElement moduleDE(det, name + dd4hep::_toString(layer_id, "_Layer%02d") + dd4hep::_toString(module_id, "_Stave%02d"), x_det.id());
      moduleDE.setPlacement(pv);

      VXD_Module thisModule = module_data[type];
      if (module_id==0) {
	thisLayer.phi0 = phi0+phi;
	thisLayer.zHalf = thisModule.half_z;
	thisLayer.radiusSensitive = thisModule.sensitive_inner_radius;
	thisLayer.thicknessSensitive = thisModule.support_inner_radius - thisModule.sensitive_inner_radius;
	thisLayer.radiusSupport = thisModule.support_inner_radius;
	thisLayer.thicknessSupport = thisModule.support_thickness;
      }
      else if (module_id==1) {
	thisLayer.rgap = thisModule.sensitive_inner_radius - thisLayer.radiusSensitive;
	thisLayer.dphi = phi0+phi-thisLayer.phi0; 
      }

      module_id++;
    }
    cylindricalData->layers.push_back(thisLayer);
  }

  std::cout << (*cylindricalData) << std::endl;
  det.addExtension<dd4hep::rec::CylindricalData>( cylindricalData );
  
  if ( x_det.hasAttr(_U(combineHits)) ) {
    det.setCombineHits(x_det.attr<bool>(_U(combineHits)),sens);
  }
  std::cout << name << " done." << endl; 
  return det;
}
DECLARE_DETELEMENT(SiTrackerStitching_v01,create_element)
