//====================================================================
//  cepcgeo - CEPC silicon tracker detector models in DD4hep 
//--------------------------------------------------------------------
//  FU Chengdong, IHEP
//  email: fucd@ihep.ac.cn
//  $Id$
//====================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetType.h"
#include "DD4hep/Printout.h"
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

/** Construction of the silicon detector
 *
 *  ladder staggered to a cirle, like v01 and v02
 *  three supper layer: support, sensitive and flex/power/readout
 *  simple to uniform in each supper layer
 *                     \
 *         ___________  \
 *                       \      x-y
 *                        \
 *                         \
 *  sensor: s--side, d--dead
 *     ___________________
 *    |  _______s_______  |
 *    | |               | |
 *    | |_______d_______| |
 *    | |               | |
 *    |s|               |s|   ----------> z
 *    | |     active    | |
 *    | |               | |
 *    | |_______________| |
 *    |_________s_ _______|
 *  @author FU Chengdong, IHEP, Jan 2025
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

  dd4hep::PrintLevel printLevel = dd4hep::ERROR;
  if (x_det.hasAttr(_Unicode(printLevel))) {
    printLevel = dd4hep::printLevel(x_det.attr<string>(_Unicode(printLevel)));
  }
  dd4hep::PrintLevel oldLevel = dd4hep::setPrintLevel(printLevel);

  if (x_det.hasChild(_U(sensitive))) {
    xml_dim_t sd_typ = x_det.child(_U(sensitive));
    sens.setType(sd_typ.typeStr());
  }
  else {
    sens.setType("tracker");
  }
  dd4hep::printout(dd4hep::INFO, "Construct", "** building SiTrackerStaggeredLadder_v03 ...");

  dd4hep::rec::ZPlanarData* zPlanarData = new dd4hep::rec::ZPlanarData;

  //fetch the shell parameters
  if (x_det.hasChild(_Unicode(shell))) {
    xml_comp_t x_shell(x_det.child(_Unicode(shell)));
    double rmin_shell = x_shell.rmin();
    double rmax_shell = x_shell.rmax();
    double zhalf_shell = x_shell.zhalf();
    Tube shellSolid(rmin_shell, rmax_shell, zhalf_shell);
    Volume shellLogical(name + "_ShellLogical", shellSolid, theDetector.material(x_shell.materialStr()));
    shellLogical.setVisAttributes(theDetector.visAttributes(x_shell.visStr()));
    envelope.placeVolume(shellLogical);

    zPlanarData->zHalfShell  = zhalf_shell;
    zPlanarData->rInnerShell = rmin_shell;
    zPlanarData->rOuterShell = rmax_shell;
  }

  for (xml_coll_t layer_i(x_det,_U(layer)); layer_i; ++layer_i) {
    xml_comp_t x_layer(layer_i);

    int layer_id                 = x_layer.attr<int>(_Unicode(layer_id));
    dd4hep::printout(dd4hep::INFO, "Construct", "layer_id: %02d", layer_id);

    dd4hep::PlacedVolume pv;

    bool dmode = false;
    if (x_layer.hasAttr(_Unicode(distance))) {
      dmode = true;
    }
    else if (x_layer.hasAttr(_Unicode(radius))) {
      dmode = false;
    }
    else {
      dd4hep::printout(dd4hep::ERROR, "Construct", "not found parameter distance and radius, at least one");
      throw runtime_error("dd4hep: lack parameter");
    }

    std::string layer_name = name + dd4hep::_toString(layer_id, "_layer%02d");

    DetElement ladderDE(layer_name + "_ladder", det_id);
    DetElement supportDE(layer_name + "_supp", det_id);
    DetElement sensitiveDE(layer_name + "_sens", det_id);
    DetElement flexDE(layer_name + "_flex", det_id);
    DetElement moduleDE(layer_name + "_module", det_id);
    DetElement sensorDE(layer_name + "_sensor", det_id);
    
    dd4hep::rec::Vector3D o(0., 0., 0.);
    dd4hep::rec::Vector3D u(0., 1., 0.);
    dd4hep::rec::Vector3D v(0., 0., 1.);
    dd4hep::rec::Vector3D n(1., 0., 0.);

    xml_comp_t x_support(x_layer.child(_Unicode(support)));

    double support_thickness = 0;
    double support_length    = x_support.attr<double>(_Unicode(length));
    double support_width     = x_support.attr<double>(_Unicode(width));

    for (xml_coll_t support_i(x_support,_U(slice)); support_i; ++support_i) {
      xml_comp_t x_support_slice(support_i);
      double support_slice_thickness = x_support_slice.attr<double>(_Unicode(thickness));
      support_thickness += support_slice_thickness;
    }
    dd4hep::printout(dd4hep::INFO, "Construct", "support: thickness = %f mm, width = %f mm, length = %f mm",
		     support_thickness/mm, support_width/mm, support_length/mm);

    Material supportMat = theDetector.material(x_support.materialStr());
    Box supportSolid(support_thickness/2.0, support_width/2.0, support_length/2.0);
    Volume supportLogical(layer_name + "_SupportLog", supportSolid, supportMat);
    supportLogical.setVisAttributes(theDetector.visAttributes(x_support.visStr()));

    dd4hep::rec::VolPlane supportSurf(supportLogical, dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Helper),
			       support_thickness/2.0, support_thickness/2.0, u, v, n, o);
    dd4hep::rec::volSurfaceList(supportDE)->push_back(supportSurf);
    
    double support_xpos = -support_thickness/2.0;
    for (xml_coll_t support_i(x_support,_U(slice)); support_i; ++support_i) {
      xml_comp_t x_support_slice(support_i);
      double support_slice_thickness = x_support_slice.attr<double>(_Unicode(thickness));
      double support_slice_width     = support_width;
      double support_slice_length    = support_length;
      double y_offset = 0;
      double z_offset = 0;

      if (x_support_slice.hasAttr(_Unicode(width)))  support_slice_width  = x_support_slice.width();//attr<double>(_Unicode(width));
      if (x_support_slice.hasAttr(_Unicode(length))) support_slice_length = x_support_slice.length();//attr<double>(_Unicode(length));
      if (x_support_slice.hasAttr(_Unicode(y0)))     y_offset = x_support_slice.y0();
      if (x_support_slice.hasAttr(_Unicode(z0)))     z_offset = x_support_slice.z0();

      Material sliceMat = theDetector.material(x_support_slice.materialStr());
      Box sliceSolid(support_slice_thickness/2.0, support_slice_width/2.0, support_slice_length/2.0);
      Volume sliceLogical(layer_name + "_SupportSliceLog", sliceSolid, sliceMat);
      sliceLogical.setVisAttributes(theDetector.visAttributes(x_support_slice.visStr()));

      support_xpos += support_slice_thickness/2.0;
      pv = supportLogical.placeVolume(sliceLogical, Position(support_xpos, y_offset, z_offset));
      support_xpos += support_slice_thickness/2.0;
    }

    xml_comp_t x_sensitive(x_layer.child(_Unicode(sensitive)));

    double sensitive_thickness = x_sensitive.thickness();
    double sensitive_length    = x_sensitive.length();
    double sensitive_width     = x_sensitive.width();
    double sensitive_center    = x_sensitive.hasAttr(_Unicode(gap)) ? x_sensitive.gap() : 0;
    dd4hep::printout(dd4hep::INFO, "Construct", "sensitive: thickness = %f mm, width = %f mm, length = %f mm",
		     sensitive_thickness/mm, sensitive_width/mm, sensitive_length/mm);

    Material sensitiveMat = theDetector.material(x_sensitive.materialStr());
    Box      sensitiveSolid(sensitive_thickness/2.0, sensitive_width/2.0, sensitive_length/2.0);
    Volume   sensitiveLogical(layer_name + "_SensitiveLog", sensitiveSolid, sensitiveMat);
    sensitiveLogical.setVisAttributes(theDetector.visAttributes(x_sensitive.visStr()));

    dd4hep::rec::VolPlane sensitiveSurf(sensitiveLogical, dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Helper),
					sensitive_thickness/2.0, sensitive_thickness/2.0, u, v, n, o);
    dd4hep::rec::volSurfaceList(sensitiveDE)->push_back(sensitiveSurf);
    
    xml_comp_t x_module(x_sensitive.child(_Unicode(module)));
    double module_thickness = sensitive_thickness;
    double module_length    = x_module.length();
    double module_width     = x_module.width();
    double module_y0        = x_module.hasAttr(_Unicode(y0)) ? x_module.y0() : 0;
    double module_z0        = x_module.hasAttr(_Unicode(z0)) ? x_module.z0() : 0;
    dd4hep::printout(dd4hep::INFO, "Construct", "module: thickness = %f mm, width = %f mm, length = %f mm, y0 = %f mm, z0 = %f mm",
		     module_thickness/mm, module_width/mm, module_length/mm, module_y0/mm, module_z0/mm);

    Material moduleMat = theDetector.material(x_module.materialStr());
    Box      moduleSolid(module_thickness/2.0, module_width/2.0, module_length/2.0);
    Volume   moduleLogical(layer_name + "_ModuleLog", moduleSolid, moduleMat);
    moduleLogical.setVisAttributes(theDetector.visAttributes(x_module.visStr()));

    xml_comp_t x_row(x_module.child(_Unicode(row)));
    int    row_n         = x_row.repeat();
    double row_gap       = x_row.gap();
    double row_side   = x_row.attr<double>(_Unicode(up_side));
    //double row_down_side = x_row.attr<double>(_Unicode(down_side));

    xml_comp_t x_column(x_module.child(_Unicode(column)));
    int    column_n          = x_column.repeat();
    double column_gap        = x_column.gap();
    //double column_side  = x_column.attr<double>(_Unicode(left_side));
    //double column_right_side = x_column.attr<double>(_Unicode(right_side));

    xml_comp_t x_sensor(x_module.child(_Unicode(sensor)));
    double sensor_thickness = sensitive_thickness;
    double sensor_length    = x_sensor.length();
    double sensor_width     = x_sensor.width();
    int    sensor_n         = x_sensor.repeat();
    double sensor_gap       = x_sensor.gap();
    double sensor_side      = x_sensor.attr<double>(_Unicode(side));
    double sensor_dead      = x_sensor.attr<double>(_Unicode(dead));
    int sensor_row_n        = std::floor((module_width-sensor_dead)/sensor_width);
    int sensor_column_n     = sensor_n/sensor_row_n;
    double sensor_total_length = (sensor_length + sensor_side*2 + sensor_gap) * sensor_column_n - sensor_gap;
    dd4hep::printout(dd4hep::INFO, "Construct", "sensor: thickness = %f mm, width = %f mm, length = %f mm, total length = %f mm",
		     sensor_thickness/mm, sensor_width/mm, sensor_length/mm, sensor_total_length/mm);
    dd4hep::printout(dd4hep::INFO, "Construct", "        nrow = %d, ncolumn = %d, gap = %f mm, side = %f mm, dead = %f mm",
		     sensor_row_n, sensor_column_n, sensor_gap/mm, sensor_side/mm, sensor_dead/mm);

    Material sensorMat = theDetector.material(x_sensor.materialStr());
    Box      sensorSolid(sensor_thickness/2.0, sensor_width/2.0, sensor_length/2.0);
    Volume   sensorLogical(layer_name + "_SensorLog", sensorSolid, sensorMat);
    sensorLogical.setVisAttributes(theDetector.visAttributes(x_sensor.visStr()));
    sensorLogical.setSensitiveDetector(sens);
    if (x_det.hasAttr(_U(limits))) sensorLogical.setLimitSet(theDetector, x_det.limitsStr());

    dd4hep::rec::VolPlane surf(sensorLogical, dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
			       sensor_thickness/2.0, sensor_thickness/2.0, u, v, n, o);
    
    for (int sensor_id = 0; sensor_id < sensor_n; sensor_id++) {
      int irow            = std::floor(sensor_id/sensor_column_n);
      int icolumn         = sensor_id%sensor_column_n;
      double ypos = module_y0 - module_width/2.0 + sensor_dead + sensor_side + irow*sensor_gap + (irow + 0.5)*sensor_width;
      double zpos = module_z0 - sensor_total_length/2.0 + sensor_side + icolumn*(sensor_gap + sensor_side*2) + (icolumn + 0.5)*sensor_length;
      pv = moduleLogical.placeVolume(sensorLogical, Position(0, ypos, zpos));
      pv.addPhysVolID("sensor", sensor_id);

      DetElement currentDE = sensorDE.clone(layer_name + dd4hep::_toString(sensor_id, "_sensor%03d"), det_id);
      currentDE.setPlacement(pv);
      moduleDE.add(currentDE);
      dd4hep::rec::volSurfaceList(currentDE)->push_back(surf);
    }

    double sensitive_active_length = module_length*column_n + sensitive_center + column_gap*column_n;
    for (int module_id = 0; module_id < row_n*column_n; module_id++) {
      int irow    = std::floor(module_id/column_n);
      int icolumn = module_id%column_n;
      double ypos = -sensitive_width/2.0 + row_side + irow*row_gap + (irow + 0.5)*module_width;
      double zpos = -sensitive_active_length/2.0 + (icolumn + 0.5)*(module_length + column_gap);
      if (zpos>0) zpos += sensitive_center;
      pv = sensitiveLogical.placeVolume(moduleLogical, Position(0, ypos, zpos));
      pv.addPhysVolID("module", module_id);

      DetElement currentDE = moduleDE.clone(layer_name + dd4hep::_toString(module_id, "_module%02d"), det_id);
      currentDE.setPlacement(pv);
      sensitiveDE.add(currentDE);
    }

    xml_comp_t x_flex(x_layer.child(_Unicode(flex)));

    double flex_thickness = 0;
    double flex_length    = x_flex.attr<double>(_Unicode(length));
    double flex_width     = x_flex.attr<double>(_Unicode(width));

    for (xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i) {
      xml_comp_t x_flex_slice(flex_i);
      double flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
      flex_thickness += flex_slice_thickness;
    }
    dd4hep::printout(dd4hep::INFO, "Construct", "flex: thickness = %f mm, width = %f mm, length = %f mm",
		     flex_thickness/mm, flex_width/mm, flex_length/mm);

    Material flexMat = theDetector.material(x_flex.materialStr());
    Box      flexSolid(flex_thickness/2.0, flex_width/2.0, flex_length/2.0);
    Volume   flexLogical(layer_name + "_FlexLog", flexSolid, flexMat);
    flexLogical.setVisAttributes(theDetector.visAttributes(x_flex.visStr()));

    dd4hep::rec::VolPlane flexSurf(flexLogical, dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Helper),
					flex_thickness/2.0, flex_thickness/2.0, u, v, n, o);
    dd4hep::rec::volSurfaceList(flexDE)->push_back(flexSurf);
    
    double flex_xpos = -flex_thickness/2.0;
    for (xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i) {
      xml_comp_t x_flex_slice(flex_i);
      double flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
      double flex_slice_width     = flex_width;
      double flex_slice_length    = flex_length;
      double y_offset = 0;
      double z_offset = 0;

      if (x_flex_slice.hasAttr(_Unicode(width)))  flex_slice_width  = x_flex_slice.width();//attr<double>(_Unicode(width));
      if (x_flex_slice.hasAttr(_Unicode(length))) flex_slice_length = x_flex_slice.length();//attr<double>(_Unicode(length));
      if (x_flex_slice.hasAttr(_Unicode(y0)))     y_offset = x_flex_slice.y0();
      if (x_flex_slice.hasAttr(_Unicode(z0)))     z_offset = x_flex_slice.z0();

      Material sliceMat = theDetector.material(x_flex_slice.materialStr());
      Box sliceSolid(flex_slice_thickness/2.0, flex_slice_width/2.0, flex_slice_length/2.0);
      Volume sliceLogical(layer_name + "_FlexSliceLog", sliceSolid, sliceMat);
      sliceLogical.setVisAttributes(theDetector.visAttributes(x_flex_slice.visStr()));

      flex_xpos += flex_slice_thickness/2.0;
      pv = flexLogical.placeVolume(sliceLogical, Position(flex_xpos, y_offset, z_offset));
      flex_xpos += flex_slice_thickness/2.0;
    }

    double ladder_thickness = support_thickness + sensitive_thickness + flex_thickness;
    double ladder_width     = std::max(support_width, std::max(sensitive_width, flex_width));
    double ladder_length    = std::max(support_length, std::max(sensitive_length, flex_length));
    Box    ladderSolid(ladder_thickness/2.0, ladder_width/2.0, ladder_length/2.0);
    Volume ladderLogical(layer_name + "_LadderLog", ladderSolid, air);
    ladderLogical.setVisAttributes(theDetector.visAttributes(x_det.visStr()));

    pv = ladderLogical.placeVolume(supportLogical,   Position(-(sensitive_thickness + flex_thickness)/2.0, 0, 0));
    supportDE.setPlacement(pv);
    ladderDE.add(supportDE);

    pv = ladderLogical.placeVolume(sensitiveLogical, Position( (support_thickness - flex_thickness)/2.0, 0, 0));
    pv.addPhysVolID("layer", layer_id);
    sensitiveDE.setPlacement(pv);
    ladderDE.add(sensitiveDE);
    
    pv = ladderLogical.placeVolume(flexLogical,      Position( (support_thickness + sensitive_thickness)/2.0, 0, 0));
    flexDE.setPlacement(pv);
    ladderDE.add(flexDE);
    
    double ladder_radius   = 0;
    double ladder_distance = 0;
    double ladder_offset   = 0;
    double ladder_rotate   = 0;
    if (dmode) {
      ladder_distance = x_layer.attr<double>(_Unicode(distance));
      ladder_offset   = x_layer.attr<double>(_Unicode(offset));
      ladder_radius   = sqrt(ladder_offset*ladder_offset + ladder_distance*ladder_distance);
      ladder_rotate   = -atan(ladder_offset/ladder_distance);
    }
    else {
      double radius   = x_layer.attr<double>(_Unicode(radius));
      //ladder_radius   = x_layer.attr<double>(_Unicode(radius));
      ladder_rotate   = x_layer.attr<double>(_Unicode(rotate));
      //double tanr     = fabs(tan(ladder_rotate));
      //ladder_distance = (tanr*ladder_width/2+sqrt(radius*radius*(tanr*tanr+1)-ladder_width*ladder_width/4))/(tanr*tanr+1);
      //ladder_distance = ladder_radius*cos(ladder_rotate);
      ladder_distance = radius*cos(ladder_rotate) - (support_thickness - flex_thickness)/2.0;
      ladder_radius   = ladder_distance/cos(ladder_rotate);
      //ladder_offset   = -ladder_radius*sin(ladder_rotate);
      ladder_offset   = -radius*sin(ladder_rotate);
    }
    //int n_sensors_per_ladder = x_layer.attr<int>(_Unicode(n_sensors_per_side));
    const double ladder_phi0 = x_layer.hasAttr(_Unicode(phi0)) ? x_layer.attr<double>(_Unicode(phi0)) : 0;
    const int    ladder_n    = x_layer.attr<int>(_Unicode(n_ladders));
    const double ladder_dphi = dd4hep::twopi / ladder_n;
    dd4hep::printout(dd4hep::INFO, "Construct", "ladder: distance = %f mm, offset = %f mm", ladder_distance/mm, ladder_offset/mm);
    dd4hep::printout(dd4hep::INFO, "Construct", "ladder: center radius = %f mm, phi0 = %f mm", ladder_radius/mm, ladder_phi0);
    dd4hep::printout(dd4hep::INFO, "Construct", "ladder: n = %d, %d sensors per ladder", ladder_n, row_n*column_n*sensor_n);

    for (int ladder_id = 0; ladder_id < ladder_n; ladder_id++) {
      DetElement currentDE = ladderDE.clone(layer_name + dd4hep::_toString(ladder_id, "_ladder%03d"), det_id);

      double phi = ladder_phi0 + ladder_dphi*ladder_id;
      Position pos(ladder_radius*cos(-ladder_rotate+phi), ladder_radius*sin(-ladder_rotate+phi), 0.);
      Transform3D tr (RotationZYX(phi,0.,0.), pos);
      pv = envelope.placeVolume(ladderLogical,tr);
      pv.addPhysVolID("stave", ladder_id);
      currentDE.setPlacement(pv);
      det.add(currentDE);
   }
   
   // package the reconstruction data
   dd4hep::rec::ZPlanarData::LayerLayout layer;
   
   layer.ladderNumber         = ladder_n;
   layer.phi0                 = ladder_phi0;
   layer.sensorsPerLadder     = row_n*column_n;
   layer.lengthSensor         = module_length + column_gap;
   layer.distanceSupport      = ladder_distance - ladder_thickness/2.0;
   layer.thicknessSupport     = support_thickness;
   layer.offsetSupport        = ladder_offset;
   layer.widthSupport         = support_width;
   layer.zHalfSupport         = support_length/2.0;
   layer.distanceSensitive    = ladder_distance - ladder_thickness/2.0 + support_thickness;
   layer.thicknessSensitive   = sensitive_thickness;
   layer.offsetSensitive      = ladder_offset;
   //FIXME: treat widthSensitive as flex_thickness 
   //layer.widthSensitive       = module_width;
   layer.widthSensitive       = flex_thickness;
   layer.zHalfSensitive       = sensitive_active_length/2.0;

   zPlanarData->layers.push_back(layer);
  }

  if (dd4hep::printLevel()<=dd4hep::WARNING) std::cout << (*zPlanarData) << endl;
  det.addExtension< ZPlanarData >(zPlanarData);

  if (x_det.hasAttr(_U(combineHits))) det.setCombineHits(x_det.attr<bool>(_U(combineHits)),sens);

  dd4hep::printout(dd4hep::INFO, "Construct", "SiTrackerStaggeredLadder_v03 done.");
  dd4hep::setPrintLevel(oldLevel);

  return det;
}
DECLARE_DETELEMENT(SiTrackerStaggeredLadder_v03,create_element)
