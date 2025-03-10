//====================================================================
//  cepcgeo - CEPC silicon detector models in DD4hep 
//--------------------------------------------------------------------
//  Chengdong FU and Tianyuan ZHANG, IHEP
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

#include "DetIdentifier/CEPCDetectorData.h"

using namespace std;

using dd4hep::Box;
using dd4hep::Tube;
using dd4hep::DetElement;
using dd4hep::Material;
using dd4hep::Position;
using dd4hep::RotationY;
using dd4hep::RotationZ;
using dd4hep::RotationZYX;
using dd4hep::Transform3D;
using dd4hep::Rotation3D;
using dd4hep::Volume;
using dd4hep::_toString;
using dd4hep::rec::volSurfaceList;
using dd4hep::rec::ZPlanarData;
using dd4hep::mm;

/** Construction of the VXD detector, ported from Mokka driver SIT_Simple_Pixel.cc
 *
 *  Mokka History:
 *  Feb 7th 2011, Steve Aplin - original version
 *  F.Gaede, DESY, Jan 2014   - dd4hep SIT pixel
 *  Hao Zeng, IHEP, July 2021
 *  Chengdong FU, IHEP, Sep 2024 - composite from SiTrackerStaggeredLadder_v02 and SiTrackerStitching_v01
 *  Tianyuan ZHANG, IHEP, Dec 2024 - add sensor detail based on SiTrackerComposite_v01
 *  Chengdong FU, IHEP, Mar 2025 - change parameter input from SiTrackerComposite_v02
 */

static dd4hep::Ref_t create_element(dd4hep::Detector& theDetector, xml_h e, dd4hep::SensitiveDetector sens) {

  xml_det_t  x_det    = e;
  Material   air      = theDetector.air();
  int        det_id   = x_det.id();
  string     name     = x_det.nameStr();
  DetElement det(name, det_id);

  Volume envelope = dd4hep::xml::createPlacedEnvelope(theDetector, e, det);
  dd4hep::xml::setDetectorTypeFlag(e, det) ;
  if(theDetector.buildType()==dd4hep::BUILD_ENVELOPE) return det;
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

  dd4hep::printout(dd4hep::INFO, "Construct", "** building SiTrackerComposite_v03 ...");

  dd4hep::rec::CompositeData* compositeData = new dd4hep::rec::CompositeData;

  //fetch the display parameters
  xml_comp_t x_display(x_det.child(_Unicode(display)));
  std::string ladderVis      = x_display.attr<string>(_Unicode(ladder));
  std::string supportVis     = x_display.attr<string>(_Unicode(support));
  std::string flexVis        = x_display.attr<string>(_Unicode(flex));
  std::string sensEnvVis     = x_display.attr<string>(_Unicode(sens_env));
  std::string sensVis        = x_display.attr<string>(_Unicode(sens));
  std::string deadsensVis    = x_display.attr<string>(_Unicode(deadsensor));
  std::string deadwireVis    = x_display.attr<string>(_Unicode(deadwire));

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

    compositeData->zHalfShell  = zhalf_shell;
    compositeData->rInnerShell = rmin_shell;
    compositeData->rOuterShell = rmax_shell;
  }

  for(xml_coll_t layer_i(x_det,_U(layer)); layer_i; ++layer_i){
    xml_comp_t x_layer(layer_i);
   
    dd4hep::PlacedVolume pv;
    int layer_id                 = x_layer.id();
    bool isBent = false;
    if (x_layer.hasAttr(_Unicode(isBent))) isBent = x_layer.attr<bool>(_Unicode(isBent));

    dd4hep::printout(dd4hep::INFO, "Construct", "layer_id: %02d --- %s", layer_id, isBent ? "Stitching" : "Planar");

    double z = x_layer.hasAttr(_U(z)) ? x_layer.z() : 0;

    if (isBent) {
      double phi0 = x_layer.phi0();

      dd4hep::rec::CylindricalData::LayerLayout thisLayer;

      int module_id = 0;
      for (xml_coll_t module_i(x_layer, _U(module)); module_i; ++module_i) {
	if (module_id>1) dd4hep::printout(dd4hep::ERROR, "Construct", "Not support more than two modules, possible wrong in reconstruction");

	string name_module = name + dd4hep::_toString(layer_id, "_Layer%02d") + dd4hep::_toString(module_id, "_Stave%02d");
	DetElement moduleDE(det, name_module, x_det.id());

	xml_comp_t x_module(module_i);

	double offset = x_module.offset();
	double phi    = x_module.phi();

	double radius = x_module.radius();
	double backbone = x_module.attr<double>(_Unicode(backbone));
	double switches = x_module.attr<double>(_Unicode(switches));
	double bias = x_module.attr<double>(_Unicode(bias));
	double periphery = x_module.attr<double>(_Unicode(periphery));
	double mechanical_gap = x_module.attr<double>(_Unicode(mechanical_gap));
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
	Volume sensor_volume(name_module + "Sensor", sensor_solid, sensor_mat);
	sensor_volume.setSensitiveDetector(sens);
	if (x_det.hasAttr(_U(limits))) sensor_volume.setLimitSet(theDetector, x_det.limitsStr());
	sensor_volume.setVisAttributes(theDetector.visAttributes(x_sensor.visStr()));

	xml_comp_t x_flex(x_module.child(_Unicode(flex)));
	std::vector<std::pair<double, Material> > flexs;
	double flex_thickness = 0;
	for (xml_coll_t slice_i(x_flex, _U(slice)); slice_i; ++slice_i) {
	  xml_comp_t x_slice(slice_i);
	  double thickness   = x_slice.thickness();
	  Material mat_slice = theDetector.material(x_slice.materialStr());
	  flexs.push_back(std::make_pair(thickness, mat_slice));
	  flex_thickness += thickness;
	  dd4hep::printout(dd4hep::DEBUG, "Construct", "flex %s: %f mm", mat_slice.name(), thickness/dd4hep::mm);
	}
	double flex_width  = (sensor_width+periphery)*ny + bias*ny/2;
	double flex_length = (sensor_length+switches)*nx + backbone*nx/3 + mechanical_gap;
	double flex_radius = radius+sensor_thickness;
	double flex_dphi   = flex_width/radius;
	Tube flex_solid(flex_radius, flex_radius+flex_thickness, flex_length/2, -flex_dphi/2, flex_dphi/2);
	Volume flex_volume(name_module + "Flex", flex_solid, air);
	flex_volume.setVisAttributes(theDetector.visAttributes(x_flex.visStr()));
	double start_radius = flex_radius;
	for (unsigned islice=0; islice<flexs.size(); islice++) {
	  dd4hep::printout(dd4hep::DEBUG, "Construct", "flex start radius: %f mm", start_radius/dd4hep::mm);
	  Tube slice_solid(start_radius, start_radius+flexs[islice].first, flex_length/2, -flex_dphi/2, flex_dphi/2);
	  Volume slice_volume(name_module + dd4hep::_toString(int(islice), "Flex_%d"), slice_solid, flexs[islice].second);
	  flex_volume.placeVolume(slice_volume);
	  start_radius += flexs[islice].first;
	}

	double service_radius = radius + sensor_thickness;

	// in fact driver, the left impact as name of readout
	xml_comp_t x_readout(x_module.child(_Unicode(readout)));
	double readout_thickness = x_readout.thickness();
	double readout_length = x_readout.width();
	double readout_width  = flex_width;
	double readout_dphi  = readout_width/radius;
	Material readout_mat = theDetector.material(x_readout.materialStr());
	Tube readout_solid(service_radius, service_radius+readout_thickness, readout_length/2, -flex_dphi/2, flex_dphi/2);
	Volume readout_volume(name_module + "DriverL", readout_solid, readout_mat);
	readout_volume.setVisAttributes(theDetector.visAttributes(x_readout.visStr()));

	xml_comp_t x_driver(x_module.child(_Unicode(driver)));
        double driver_thickness = x_driver.thickness();
        double driver_length = x_driver.width();
        double driver_width  = flex_width;
        double driver_dphi  = driver_width/radius;
        Material driver_mat = theDetector.material(x_driver.materialStr());
        Tube driver_solid(service_radius, service_radius+driver_thickness, driver_length/2, -flex_dphi/2, flex_dphi/2);
        Volume driver_volume(name_module + "DriverR", driver_solid, driver_mat);
        driver_volume.setVisAttributes(theDetector.visAttributes(x_driver.visStr()));

	double module_length = flex_length + readout_length + driver_length;
	double module_width  = flex_width;
	double module_dphi   = module_width/radius;
	double module_thickness = sensor_thickness + std::max(flex_thickness, std::max(readout_thickness,driver_thickness));
	Tube module_solid(radius, radius+module_thickness, module_length/2, -flex_dphi/2, flex_dphi/2);
	Volume module_volume(name_module, module_solid, air);
	module_volume.setVisAttributes(theDetector.visAttributes("SeeThrough"));

	Tube board_solid(radius, radius+sensor_thickness, module_length/2, -flex_dphi/2, flex_dphi/2);
	Volume board_volume(name_module + "Board", board_solid, mat);
	board_volume.setVisAttributes(theDetector.visAttributes(x_module.visStr()));
	module_volume.placeVolume(board_volume);

	double z_bent = 0.0;
	double delta = 0.0;
	for (int ix = 0; ix < nx; ix++) {
	    if (ix < nx / 2) {
		z_bent = -flex_length/2 + sensor_length/2 + (static_cast<int>(ix/3) + 1)*backbone + ix*(sensor_length + switches);
	    }
	    else {
		z_bent = -flex_length/2 + sensor_length/2 + (static_cast<int>(ix/3) + 1)*backbone + ix*(sensor_length + switches) + mechanical_gap;
	    }
	    for (int iy = 0; iy < ny; iy++) {
	        if (iy % 2 != 0) {
		    delta = -flex_dphi/2 + sensor_dphi/2 + iy*(sensor_dphi + periphery/radius) + ((iy + 1)/2)*bias/radius;
	        }
	        else {
		    delta = -flex_dphi/2 + sensor_dphi/2 + iy*sensor_dphi + (iy + 1)*periphery / radius + iy/2*bias/radius;
	        }
		Transform3D tran(RotationZ(delta), Position(0, 0, z_bent));
		dd4hep::PlacedVolume pv = board_volume.placeVolume(sensor_volume, tran);

		int sensor_id = ix + nx*iy;
		pv.addPhysVolID("sensor", sensor_id);
		dd4hep::rec::Vector3D ocyl(radius + 0.5 * sensor_thickness, 0., 0.);
		dd4hep::rec::VolCylinder surf(sensor_volume, dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
		0.5 * sensor_thickness, module_thickness - 0.5 * sensor_thickness, ocyl);
		DetElement sensorDE(moduleDE, name_module + dd4hep::_toString(sensor_id, "_Sensor%02d"), x_det.id());
		sensorDE.setPlacement(pv);
		volSurfaceList(sensorDE)->push_back(surf);
	  }
        }

	module_volume.placeVolume(flex_volume, Position(0, 0, readout_length/2-driver_length/2));
	module_volume.placeVolume(readout_volume, Position(0, 0, -flex_length/2-driver_length/2));
	module_volume.placeVolume(driver_volume, Position(0, 0,  flex_length/2+readout_length/2));

	if (module_id == 0) {
	  thisLayer.zHalf              = flex_length/2.0;
	  thisLayer.radiusSensitive    = radius;
	  thisLayer.thicknessSensitive = sensor_thickness;
	  thisLayer.radiusSupport      = flex_radius;
	  thisLayer.thicknessSupport   = flex_thickness;
	  thisLayer.width              = flex_width;
	  thisLayer.phi0 = phi0 + phi;
	  thisLayer.rgap = radius;
	}
	else {
	  thisLayer.rgap = radius - thisLayer.rgap;
          thisLayer.dphi = phi0 + phi - thisLayer.phi0;
        }

	Transform3D tran(RotationZ(phi0+phi), Position(offset*cos(phi), offset*sin(phi), z));
	pv = envelope.placeVolume(module_volume, tran);
	pv.addPhysVolID("layer", layer_id).addPhysVolID("module", module_id);

	moduleDE.setPlacement(pv);

	module_id++;
      }

      compositeData->layersBent.push_back(thisLayer);
    }
    else {
      double support_rmin         = x_layer.attr<double>(_Unicode(support_rmin));
      int    n_sensors_per_ladder = x_layer.attr<int>(_Unicode(n_sensors_per_side));
      int    n_ladders            = x_layer.attr<int>(_Unicode(n_ladders)) ;
      double rotate               = x_layer.attr<double>(_Unicode(rotate));
      double phi0                 = x_layer.attr<double>(_Unicode(phi0));

      std::string layerName = dd4hep::_toString(layer_id , "layer_%02d");
      dd4hep::Assembly layer_assembly(layerName);
      pv = envelope.placeVolume(layer_assembly);
      dd4hep::DetElement layerDE(det, layerName, x_det.id());
      layerDE.setPlacement(pv);
    
      const double ladder_dphi = ( dd4hep::twopi / n_ladders ) ;
      dd4hep::printout(dd4hep::DEBUG, "Construct", "ladder_dphi: %f (%f degree)", ladder_dphi, ladder_dphi/dd4hep::degree);
    
      //fetch the ladder parameters
      xml_comp_t x_ladder(x_layer.child(_Unicode(ladder)));
    
      //fetch the ladder support parameters
      xml_comp_t x_ladder_support(x_ladder.child(_Unicode(ladderSupport)));
      double support_length        = x_ladder_support.attr<double>(_Unicode(length));
      double support_thickness     = x_ladder_support.attr<double>(_Unicode(thickness));
      double support_height        = x_ladder_support.attr<double>(_Unicode(height));
      double support_width         = x_ladder_support.attr<double>(_Unicode(width));
      Material support_mat;
      if(x_ladder_support.hasAttr(_Unicode(mat))) {
	support_mat = theDetector.material(x_ladder_support.attr<string>(_Unicode(mat)));
      }
      else {
	support_mat = theDetector.material(x_ladder_support.materialStr());
      }
      dd4hep::printout(dd4hep::INFO, "Construct", "support_length = %f mm, support_thickness = %f mm, support_width = %f mm", support_length/mm, support_thickness/mm, support_width/mm);

      double ladder_offset   = - support_rmin*sin(rotate) - support_width/2.0;
      double ladder_distance = support_rmin*cos(rotate) + support_height/2.0;
      double ladder_radius   = sqrt(ladder_offset*ladder_offset + ladder_distance*ladder_distance);
      double ladder_phi0     = phi0 + atan(ladder_offset/ladder_distance);
      dd4hep::printout(dd4hep::INFO, "Construct", "ladder_radius = %f mm, ladder_distance = %f mm, ladder_offset = %f mm, n_sensors_per_ladder = %d",
		       ladder_radius/mm, ladder_distance/mm, ladder_offset/mm, n_sensors_per_ladder);

      //fetch the flex parameters
      double flex_thickness(0);
      double flex_width(0);
      double flex_length(0);
      xml_comp_t x_flex(x_ladder.child(_Unicode(flex)));
      for(xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i) {
	xml_comp_t x_flex_slice(flex_i);
	double x_flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
	double x_flex_slice_width = x_flex_slice.attr<double>(_Unicode(width));
	double x_flex_slice_length = x_flex_slice.attr<double>(_Unicode(length));
	flex_thickness += x_flex_slice_thickness;
	if (x_flex_slice_width > flex_width) flex_width = x_flex_slice_width;
	if (x_flex_slice_length > flex_length) flex_length = x_flex_slice_length;
	dd4hep::printout(dd4hep::DEBUG, "Construct", "x_flex_slice_thickness: %f mm", x_flex_slice_thickness/mm);
      }
      dd4hep::printout(dd4hep::INFO, "Construct", "flex_length = %f mm, flex_thickness = %f mm, flex_width = %f mm", flex_length/mm, flex_thickness/mm, flex_width/mm);

      //fetch the sensor parameters
      xml_comp_t x_sensor(x_ladder.child(_Unicode(sensor)));
      int n_sensors_per_side                  = x_sensor.attr<int>(_Unicode(n_sensors));
      double dead_gap                         = x_sensor.attr<double>(_Unicode(gap));
      double sensor_thickness                 = x_sensor.attr<double>(_Unicode(thickness));
      double sensor_active_len                = x_sensor.attr<double>(_Unicode(active_length));
      double sensor_active_width              = x_sensor.attr<double>(_Unicode(active_width));
      double sensor_dead_width                = x_sensor.attr<double>(_Unicode(dead_width));
      double sensor_deadwire_length           = x_sensor.attr<double>(_Unicode(deadwire_length));
      double sensor_deadwire_width            = x_sensor.attr<double>(_Unicode(deadwire_width));
      double sensor_deadwire_thickness        = x_sensor.attr<double>(_Unicode(deadwire_thickness));
      Material sensor_mat                     = theDetector.material(x_sensor.attr<string>(_Unicode(sensor_mat)));
      Material sensor_deadwire_mat            = theDetector.material(x_sensor.attr<string>(_Unicode(deadwire_mat)));

      dd4hep::printout(dd4hep::INFO, "Construct", "sensor_active_length = %f mm, sensor_thickness = %f mm, sensor_active_width = %f mm", sensor_active_len/mm, sensor_thickness/mm, sensor_active_width/mm);
      dd4hep::printout(dd4hep::INFO, "Construct", "sensor_dead_width = %f mm, dead_gap = %f mm, n_sensors_per_side = %d", sensor_dead_width/mm, dead_gap/mm, n_sensors_per_side);

      //create ladder logical volume
      Box LadderSolid((support_height+2*sensor_thickness+2*flex_thickness)/2.0, support_width/2.0, support_length/2.0);
      Volume LadderLogical(name + dd4hep::_toString(layer_id, "_LadderLogical_%02d"), LadderSolid, air);
      // create flex envelope logical volume
      Box FlexEnvelopeSolid(flex_thickness/2.0, flex_width/2.0, flex_length/2.0);
      Volume FlexEnvelopeLogical(name + dd4hep::_toString( layer_id, "_FlexEnvelopeLogical_%02d"), FlexEnvelopeSolid, air);
      FlexEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));

      //create the flex layers inside the flex envelope
      double flex_start_height(-flex_thickness/2.); 
      int index = 0;
      for(xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i){
	xml_comp_t x_flex_slice(flex_i);
	double x_flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
	double x_flex_slice_width = x_flex_slice.attr<double>(_Unicode(width));
	double x_flex_slice_length = x_flex_slice.attr<double>(_Unicode(length));
	Material x_flex_slice_mat;
	if(x_flex_slice.hasAttr(_Unicode(mat))) {
	  x_flex_slice_mat = theDetector.material(x_flex_slice.attr<string>(_Unicode(mat)));
	}
	else {
	  x_flex_slice_mat = theDetector.material(x_flex_slice.materialStr());
	}
	// Material x_flex_slice_mat = theDetector.material(x_flex_slice.attr<string>(_Unicode(mat)));
	Box FlexLayerSolid(x_flex_slice_thickness/2.0, x_flex_slice_width/2.0, x_flex_slice_length/2.0);
	Volume FlexLayerLogical(name + dd4hep::_toString( layer_id, "_FlexLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), FlexLayerSolid, x_flex_slice_mat);
	FlexLayerLogical.setVisAttributes(theDetector.visAttributes(flexVis));
	double flex_slice_height = flex_start_height + x_flex_slice_thickness/2.;
	pv = FlexEnvelopeLogical.placeVolume(FlexLayerLogical, Position(flex_slice_height, 0., 0.));
	flex_start_height += x_flex_slice_thickness;
	index++;
      }

      //place the flex envelope inside the ladder envelope
      pv = LadderLogical.placeVolume(FlexEnvelopeLogical, Position((support_height + flex_thickness)/2.0, 0., 0.)); //top side
      //define the transformation3D(only need a combination of translation and rotation)
      Transform3D tran_mirro(RotationZYX(0., dd4hep::twopi/2.0, 0.), Position(-(support_height + flex_thickness)/2.0, 0., 0.));
      pv = LadderLogical.placeVolume(FlexEnvelopeLogical, tran_mirro); //bottom side

      //create sensor envelope logical volume
      Box SensorTopEnvelopeSolid(sensor_thickness/2.0, support_width/2.0, support_length/2.0);
      Volume SensorTopEnvelopeLogical(name + dd4hep::_toString( layer_id, "_SensorEnvelopeLogical_%02d"), SensorTopEnvelopeSolid, air);
      Box SensorBottomEnvelopeSolid(sensor_thickness/2.0, support_width/2.0, support_length/2.0);
      Volume SensorBottomEnvelopeLogical(name + dd4hep::_toString(layer_id, "_SensorEnvelopeLogical_%02d"), SensorBottomEnvelopeSolid, air);
      SensorTopEnvelopeLogical.setVisAttributes(theDetector.visAttributes(sensEnvVis));

      //create sensor logical volume
      Box SensorSolid(sensor_thickness/2.0, sensor_active_width/2.0, sensor_active_len/2.0);
      Volume SensorTopLogical(name + dd4hep::_toString(layer_id+1, "_SensorLogical_%02d"), SensorSolid, sensor_mat);
      Volume SensorBottomLogical(name + dd4hep::_toString(layer_id, "_SensorLogical_%02d"), SensorSolid, sensor_mat);
      SensorTopLogical.setSensitiveDetector(sens);
      SensorBottomLogical.setSensitiveDetector(sens);
      if (x_det.hasAttr(_U(limits))) {
	SensorTopLogical.setLimitSet(theDetector, x_det.limitsStr());
	SensorBottomLogical.setLimitSet(theDetector, x_det.limitsStr());
      }
      SensorTopLogical.setVisAttributes(theDetector.visAttributes(sensVis));
      SensorBottomLogical.setVisAttributes(theDetector.visAttributes(sensVis));

      //create dead sensor logical volume
      Box SensorDeadSolid(sensor_thickness / 2.0, sensor_dead_width / 2.0, sensor_active_len / 2.0);
      Volume SensorDeadLogical(name + dd4hep::_toString( layer_id, "_SensorDeadLogical_%02d"), SensorDeadSolid, sensor_mat);
      SensorDeadLogical.setVisAttributes(theDetector.visAttributes(deadsensVis));

      //create dead wire logical volume
      Box SensorDeadWireSolid(sensor_deadwire_thickness / 2.0, sensor_deadwire_width / 2.0, sensor_deadwire_length / 2.0);
      Volume SensorDeadWireLogical(name + dd4hep::_toString( layer_id, "_SensorDeadWireLogical_%02d"), SensorDeadWireSolid, sensor_deadwire_mat);
      SensorDeadWireLogical.setVisAttributes(theDetector.visAttributes(deadwireVis));

      //place the dead wire in the sensor envelope
      // pv = SensorTopEnvelopeLogical.placeVolume(SensorDeadWireLogical, Position(0.0, (sensor_active_width-support_width/2.0) + sensor_dead_width/2.0 + sensor_deadwire_width/2.0, 0.0));
      // pv = SensorBottomEnvelopeLogical.placeVolume(SensorDeadWireLogical, Position(0.0, (sensor_active_width-support_width/2.0) + sensor_dead_width/2.0 + sensor_deadwire_width/2.0, 0.0));
      pv = SensorTopEnvelopeLogical.placeVolume(SensorDeadWireLogical, Position(0.0, (-support_width/2.0) + (sensor_deadwire_width/2.0), 0.0));
      pv = SensorBottomEnvelopeLogical.placeVolume(SensorDeadWireLogical, Position(0.0, (-support_width/2.0) + (sensor_deadwire_width/2.0), 0.0));

      // place the active sensor and dead sensor inside the sensor envelope
      std::vector<dd4hep::PlacedVolume> TopSensor_pv;
      std::vector<dd4hep::PlacedVolume> BottomSensor_pv;
      for(int isensor=0; isensor < n_sensors_per_side; ++isensor){
	double sensor_total_z = n_sensors_per_side*sensor_active_len + dead_gap*(n_sensors_per_side-1);
	double xpos = 0.0;
	double ypos_active = (support_width/2.0) - (sensor_active_width/2.0);
	double ypos_dead = (-support_width/2.0) + sensor_deadwire_width + (sensor_dead_width/2.0);
	double zpos = -sensor_total_z/2.0 + sensor_active_len/2.0 + isensor*(sensor_active_len + dead_gap);
	pv = SensorTopEnvelopeLogical.placeVolume(SensorTopLogical, Position(xpos,ypos_active,zpos));
	//pv.addPhysVolID("topsensor",  isensor ) ;
	pv.addPhysVolID("layer", layer_id+1).addPhysVolID("sensor", isensor);
	TopSensor_pv.push_back(pv);
	pv = SensorBottomEnvelopeLogical.placeVolume(SensorBottomLogical, Position(xpos,ypos_active,zpos));
	//pv.addPhysVolID("bottomsensor",  isensor ) ;
	pv.addPhysVolID("layer", layer_id  ).addPhysVolID("sensor", isensor);
	BottomSensor_pv.push_back(pv);
	pv = SensorTopEnvelopeLogical.placeVolume(SensorDeadLogical, Position(xpos,ypos_dead,zpos));
	pv = SensorBottomEnvelopeLogical.placeVolume(SensorDeadLogical, Position(xpos,ypos_dead,zpos));
      }
      //place the sensor envelope inside the ladder envelope
      pv = LadderLogical.placeVolume(SensorTopEnvelopeLogical,
				     Position(support_height/2.0 + flex_thickness + sensor_thickness/2.0, 0., 0.));//top-side sensors
      Position pos(-(support_height/2.0 + flex_thickness + sensor_thickness/2.0), 0., 0.);
      pv = LadderLogical.placeVolume(SensorBottomEnvelopeLogical, pos);//bottom-side sensors
    
      //create the ladder support
      Box LadderSupportSolid(support_height/2.0, support_width/2.0, support_length/2.0);
      Volume LadderSupportLogical(name + _toString( layer_id,"_SupLogical_%02d"), LadderSupportSolid, support_mat);
      LadderSupportLogical.setVisAttributes(theDetector.visAttributes(supportVis));

      //create ladder support cavity
      Box LadderSupportCavitySolid(support_height/2.0-support_thickness/2.0, support_width/2.0-support_thickness/2.0, support_length/2.0);
      Volume LadderSupportCavityLogical(name + _toString( layer_id,"_SupCavityLogical_%02d"), LadderSupportCavitySolid, air);
      LadderSupportCavityLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));

      pv = LadderSupportLogical.placeVolume(LadderSupportCavityLogical);
      pv = LadderLogical.placeVolume(LadderSupportLogical);

      for(int i = 0; i < n_ladders; i++){
	std::stringstream ladder_enum; 
	ladder_enum << "vxt_ladder_" << layer_id << "_" << i;
	DetElement ladderDE(layerDE, ladder_enum.str(), x_det.id());
	dd4hep::printout(dd4hep::DEBUG, "Construct", "start building %s", ladder_enum.str().c_str());

	//====== create the meassurement surface ===================
	dd4hep::rec::Vector3D o(0,0,0);
	dd4hep::rec::Vector3D u( 0., 1., 0.);
	dd4hep::rec::Vector3D v( 0., 0., 1.);
	dd4hep::rec::Vector3D n( 1., 0., 0.);
	// fucd: SensorLogical only sensor_thickness, support need another surface, todo
	double inner_thick_top = sensor_thickness/2.0;
	double outer_thick_top = sensor_thickness/2.0;//support_height/2.0 + flex_thickness + sensor_thickness/2.0;
	double inner_thick_bottom = sensor_thickness/2.0;//support_height/2.0 + flex_thickness + sensor_thickness/2.0;
	double outer_thick_bottom = sensor_thickness/2.0;
	dd4hep::rec::VolPlane surfTop(SensorTopLogical,
				      dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
				      inner_thick_top, outer_thick_top, u, v, n, o);
	dd4hep::rec::VolPlane surfBottom(SensorBottomLogical,
					 dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
					 inner_thick_bottom, outer_thick_bottom, u, v, n, o);

	for(int isensor=0; isensor < n_sensors_per_side; ++isensor){
	  std::stringstream topsensor_str;
	  std::stringstream bottomsensor_str;
	  topsensor_str << ladder_enum.str() << "_top_" << isensor;
	  // std::cout << "\tstart building " << topsensor_str.str() << ":" << endl;
	  bottomsensor_str << ladder_enum.str() << "_bottom_" << isensor;
	  // std::cout << "\tstart building " << bottomsensor_str.str() << ":" << endl;
	  DetElement topsensorDE(ladderDE, topsensor_str.str(), x_det.id());
	  DetElement bottomsensorDE(ladderDE, bottomsensor_str.str(), x_det.id());
	  topsensorDE.setPlacement(TopSensor_pv[isensor]);
	  volSurfaceList(topsensorDE)->push_back(surfTop);
	  // std::cout << "\t" << topsensor_str.str() << " done." << endl;
	  bottomsensorDE.setPlacement(BottomSensor_pv[isensor]);
	  // std::cout << "\t" << bottomsensor_str.str() << " done." << endl;
	  volSurfaceList(bottomsensorDE)->push_back(surfBottom);
	}
	Transform3D tr (RotationZYX(rotate+ladder_dphi*i,0.,0.),Position(ladder_radius*cos(ladder_phi0+ladder_dphi*i), ladder_radius*sin(ladder_phi0+ladder_dphi*i), 0.));
	pv = layer_assembly.placeVolume(LadderLogical,tr);
	pv.addPhysVolID("module", i ) ; 
	ladderDE.setPlacement(pv);
	dd4hep::printout(dd4hep::DEBUG, "Construct", "%s done.", ladder_enum.str().c_str());
	if(i==0) dd4hep::printout(dd4hep::DEBUG, "Construct", "(x,y) = (%f, %f)mm", ladder_radius*cos(ladder_phi0)/mm, ladder_radius*sin(ladder_phi0)/mm);
      }

      // package the reconstruction data
      dd4hep::rec::ZPlanarData::LayerLayout topLayer;
      dd4hep::rec::ZPlanarData::LayerLayout bottomLayer;

      topLayer.ladderNumber         = n_ladders;
      topLayer.phi0                 = phi0;
      topLayer.sensorsPerLadder     = n_sensors_per_side;
      topLayer.lengthSensor         = sensor_active_len;
      topLayer.distanceSupport      = ladder_distance + support_height / 2.0 - support_thickness / 2.0;
      topLayer.thicknessSupport     = support_thickness / 2.0 + flex_thickness;
      topLayer.offsetSupport        = ladder_offset;
      topLayer.widthSupport         = support_width;
      topLayer.zHalfSupport         = support_length / 2.0;
      topLayer.distanceSensitive    = ladder_distance + support_height / 2.0 + flex_thickness;
      topLayer.thicknessSensitive   = sensor_thickness;
      topLayer.offsetSensitive      = ladder_offset + (support_width/2.0 - sensor_active_width/2.0 - sensor_dead_width/2.0);
      topLayer.widthSensitive       = sensor_active_width + sensor_dead_width;
      topLayer.zHalfSensitive       = (n_sensors_per_side*(sensor_active_len + dead_gap) - dead_gap) / 2.0;

      bottomLayer.ladderNumber         = n_ladders;
      bottomLayer.phi0                 = phi0;
      bottomLayer.sensorsPerLadder     = n_sensors_per_side;
      bottomLayer.lengthSensor         = sensor_active_len;
      bottomLayer.distanceSupport      = ladder_distance - support_height / 2.0 - flex_thickness;
      bottomLayer.thicknessSupport     = support_thickness / 2.0 + flex_thickness;
      bottomLayer.offsetSupport        = ladder_offset;
      bottomLayer.widthSupport         = support_width;
      bottomLayer.zHalfSupport         = support_length / 2.0;
      bottomLayer.distanceSensitive    = ladder_distance - support_height / 2.0 - sensor_thickness - flex_thickness;
      bottomLayer.thicknessSensitive   = sensor_thickness;
      bottomLayer.offsetSensitive      = ladder_offset + (support_width/2.0 - sensor_active_width/2.0 - sensor_dead_width/2.0);
      bottomLayer.widthSensitive       = sensor_active_width + sensor_dead_width;
      bottomLayer.zHalfSensitive       = (n_sensors_per_side*(sensor_active_len + dead_gap) - dead_gap) / 2.0;

      compositeData->layersPlanar.push_back(bottomLayer);
      compositeData->layersPlanar.push_back(topLayer);
    }
  }
  if (dd4hep::printLevel()<=dd4hep::WARNING) std::cout << (*compositeData) << endl;
  det.addExtension<dd4hep::rec::CompositeData>(compositeData);

  if (x_det.hasAttr(_U(combineHits))) det.setCombineHits(x_det.attr<bool>(_U(combineHits)),sens);

  dd4hep::printout(dd4hep::INFO, "Construct", "SiTrackerComposite_v03 done."); 
  dd4hep::setPrintLevel(oldLevel);

  return det;
}
DECLARE_DETELEMENT(SiTrackerComposite_v03,create_element)
