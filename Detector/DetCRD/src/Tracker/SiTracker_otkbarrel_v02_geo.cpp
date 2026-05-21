#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DetType.h"
#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "XML/Utilities.h"
#include <cmath>

using namespace dd4hep;

void check_det_element(DetElement det);

static Ref_t create_element(Detector& theDetector, xml_h e, SensitiveDetector sens)
{
  xml_det_t x_det = e;
  Material air = theDetector.air();
  int det_id = x_det.id();
  const std::string name = x_det.nameStr();
  DetElement otk_barrel(name, det_id);

  const xml::Component envelop_xml_element = x_det.child("envelope");
  Volume envelope = createPlacedEnvelope(theDetector, e, otk_barrel);
  envelope = envelope.setVisAttributes(theDetector, envelop_xml_element.attr<std::string>("vis"));
  setDetectorTypeFlag(e, otk_barrel);
  if (theDetector.buildType() == BUILD_ENVELOPE)
    return otk_barrel;

  if (x_det.hasAttr(_U(sensitive)))
  {
    const xml_dim_t sd_typ = x_det.child(_U(sensitive));
    sens.setType(sd_typ.typeStr());
  }
  else
  {
    sens.setType("tracker");
  }

  const auto detector_name = x_det.attr<std::string>(_U(name));
  const auto detector_type = x_det.attr<std::string>(_U(type));
  std::cout << "Processing Detector: " << detector_name
    << ", Type: " << detector_type << std::endl;

  /* ================ read parameter from xml file ===================== */

  // read parameter of stave
  const xml::Component stave = x_det.child("stave");
  auto stave_name = stave.attr<std::string>("name");
  int stave_repeat = stave.attr<int>("repeat");
  double angle_step = 2 * M_PI / stave_repeat;
  auto stave_length = stave.attr<double>("length");
  auto stave_thickness = stave.attr<double>("thickness");
  auto stave_width = stave.attr<double>("width");

  DetElement stave_det(stave_name, det_id);

  // read parameter of ladders in stave
  const xml::Component ladder = stave.child(_U(ladder));
  auto ladder_name = ladder.attr<std::string>("name");
  int ladder_repeat_inner = ladder.attr<int>("repeat_inner");
  int ladder_repeat_outer = ladder.attr<int>("repeat_outer");
  double ladder_width = (ladder.attr<double>("width"));
  double ladder_thickness = (ladder.attr<double>("thickness"));
  double ladder_length_inner = (ladder.attr<double>("length_inner"));
  double ladder_length_outer = (ladder.attr<double>("length_outer"));
  DetElement inner_ladder_det("inner_" + ladder_name, det_id);
  DetElement outer_ladder_det("outer_" + ladder_name, det_id);


  Box inner_ladder_shape(ladder_thickness / 2.0, ladder_width / 2.0, ladder_length_inner / 2.0);
  Volume inner_ladder_vol("inner_" + ladder_name, inner_ladder_shape, air);
  inner_ladder_vol = inner_ladder_vol.setVisAttributes(theDetector, ladder.attr<std::string>("vis_inner"));
  Box outer_ladder_shape(ladder_thickness / 2.0, ladder_width / 2.0, ladder_length_outer / 2.0);
  Volume outer_ladder_vol("outer_" + ladder_name, outer_ladder_shape, air);
  outer_ladder_vol = outer_ladder_vol.setVisAttributes(theDetector, ladder.attr<std::string>("vis_outer"));

  // read parameter of second data-aggregation in module
  const xml::Component second_data_aggregation = ladder.child("data_aggregation");
  auto second_data_aggregation_name = second_data_aggregation.attr<std::string>("name");
  auto second_data_aggregation_thickness = second_data_aggregation.attr<double>("thickness");
  auto second_data_aggregation_width = second_data_aggregation.attr<double>("width");
  auto second_data_aggregation_length = second_data_aggregation.attr<double>("length");

  Box second_data_aggregation_shape(second_data_aggregation_thickness / 2.0, second_data_aggregation_width / 2.0,
                                    second_data_aggregation_length / 2.0);
  Volume second_data_aggregation_vol(second_data_aggregation_name, second_data_aggregation_shape,
                                     air);
  second_data_aggregation_vol = second_data_aggregation_vol.setVisAttributes(
    theDetector, second_data_aggregation.attr<std::string>("vis"));

  // read parameter of module in ladder
  const xml::Component module = ladder.child("module");
  auto module_name = module.attr<std::string>("name");
  double module_thickness = (module.attr<double>("thickness"));
  int module_repeat = module.attr<int>("repeat");
  double module_width = (module.attr<double>("width"));
  double module_length_inner = (module.attr<double>("length_inner"));
  double module_length_outer = (module.attr<double>("length_outer"));
  DetElement inner_module_det("inner_" + module_name, det_id);
  DetElement outer_module_det("outer_" + module_name, det_id);

  // read parameter of first data-aggregation in module
  xml::Component first_data_aggregation = module.child("data_aggregation");
  auto first_data_aggregation_name = first_data_aggregation.attr<std::string>("name");
  auto first_data_aggregation_thickness = first_data_aggregation.attr<double>("thickness");
  double first_data_aggregation_width = (
    first_data_aggregation.attr<double>("width"));
  auto first_data_aggregation_length = first_data_aggregation.attr<double>("length");
  Box first_data_aggregation_shape(first_data_aggregation_thickness / 2.0, first_data_aggregation_width / 2.0,
                                   first_data_aggregation_length / 2.0);
  Volume first_data_aggregation_vol(first_data_aggregation_name, first_data_aggregation_shape,
                                    air);
  first_data_aggregation_vol = first_data_aggregation_vol.setVisAttributes(
    theDetector, first_data_aggregation.attr<std::string>("vis"));

  // FIXME: Overlap: first_data_aggregation extruded by: first_data_aggregation/PCB_0 ovlp=0.02
  // {
  //   xml::Component pcb = first_data_aggregation.child("layer");
  //   auto pcb_name = pcb.attr<std::string>("name");
  //   auto pcb_thickness = pcb.attr<double>("thickness");
  //   auto pcb_width = pcb.attr<double>("width");
  //   auto pcb_length = pcb.attr<double>("length");
  //   Box pcb_shape(pcb_thickness / 2.0, pcb_width / 2.0, pcb_length / 2.0);
  //   Volume pcb_vol(pcb_name, pcb_shape, air);
  //   pcb_vol = pcb_vol.setVisAttributes(theDetector, pcb.attr<std::string>("vis"));
  //   first_data_aggregation_vol.placeVolume(
  //     pcb_vol, Position((-first_data_aggregation_thickness + pcb_thickness) / 2.0, 0, 0));
  //
  //   xml::Component dc = first_data_aggregation.child("DC");
  //   auto dc_name = dc.attr<std::string>("name");
  //   auto dc_thickness = dc.attr<double>("thickness");
  //   auto dc_width = dc.attr<double>("width");
  //   auto dc_length = dc.attr<double>("length");
  //   Box dc_shape(dc_thickness / 2.0, dc_width / 2.0, dc_length / 2.0);
  //   Volume dc_vol(dc_name, dc_shape, air);
  //   dc_vol = dc_vol.setVisAttributes(theDetector, dc.attr<std::string>("vis"));
  //   first_data_aggregation_vol.placeVolume(
  //     dc_vol, Position((first_data_aggregation_thickness - dc_thickness) / 2.0, 0, 0));
  // }


  Box stave_shape(stave_thickness / 2.0, stave_width / 2.0, stave_length / 2.0);
  double stave_sub_shape_width = (stave_width - first_data_aggregation_width) / 2.0;
  Box stave_sub_shape(first_data_aggregation_thickness / 2.0, stave_sub_shape_width / 2.0, stave_length / 2.0);
  SubtractionSolid stave_subtracted_shape(stave_shape, stave_sub_shape, Position(
                                            (stave_thickness - first_data_aggregation_thickness) / 2.0,
                                            -stave_width / 2.0 + stave_sub_shape_width / 2.0, 0));
  Volume stave_vol(stave_name, stave_subtracted_shape, air);
  stave_vol = stave_vol.setVisAttributes(theDetector, stave.attr<std::string>("vis"));

  double module_envelope_shape_base_thickness = module_thickness - first_data_aggregation_thickness;
  Position translation(module_thickness / 2.0, 0, 0);

  Box inner_module_envelope_shape_base(module_envelope_shape_base_thickness / 2.0, module_width / 2.0,
                                       module_length_inner / 2.0);
  UnionSolid inner_module_envelope_shape(inner_module_envelope_shape_base, first_data_aggregation_shape, translation);
  Volume inner_module_envelope_vol(module_name + "_inner",
                                   inner_module_envelope_shape, air);
  inner_module_envelope_vol = inner_module_envelope_vol.
    setVisAttributes(theDetector, module.attr<std::string>("vis"));

  Box outer_module_envelope_shape_base(module_envelope_shape_base_thickness / 2.0, module_width / 2.0,
                                       module_length_outer / 2.0);
  UnionSolid outer_module_envelope_shape(outer_module_envelope_shape_base, first_data_aggregation_shape, translation);
  Volume outer_module_envelope_vol(module_name + "_outer",
                                   outer_module_envelope_shape, air);
  outer_module_envelope_vol = outer_module_envelope_vol.
    setVisAttributes(theDetector, module.attr<std::string>("vis"));


  // Process layers in module
  double x_offset = -module_envelope_shape_base_thickness / 2.0;
  double thickness_sensitive = 0 * mm;
  double thickness_support = 0 * mm;
  for (xml_coll_t layers(module, "layer"); layers; ++layers)
  {
    xml::Component layer = xml::Handle_t(layers);
    auto layer_name = layer.attr<std::string>("name");
    auto layer_thickness = layer.attr<double>("thickness");
    auto layer_width = layer.attr<double>("width");
    DetElement inner_layer_det(layer_name + "_inner", det_id);
    DetElement outer_layer_det(layer_name + "_outer", det_id);

    const Material material = theDetector.material(layer.attr<std::string>("material"));
    auto vis = layer.attr<std::string>("vis");
    auto is_sensitive = layer.hasAttr(_U(sensitive));

    const Box inner_layer_shape(layer_thickness / 2.0, layer_width / 2.0, module_length_inner / 2.0);
    Volume inner_layer_vol(module_name + layer_name + "_inner", inner_layer_shape, material);
    inner_layer_vol = inner_layer_vol.setVisAttributes(theDetector, vis);

    const Box outer_layer_box(layer_thickness / 2.0, layer_width / 2.0, module_length_outer / 2.0);
    Volume outer_layer_vol(module_name + layer_name + "_outer", outer_layer_box, material);
    outer_layer_vol = outer_layer_vol.setVisAttributes(theDetector, vis);

    rec::SurfaceType surf_type;

    if (is_sensitive)
    {
      inner_layer_vol = inner_layer_vol.setSensitiveDetector(sens);
      outer_layer_vol = outer_layer_vol.setSensitiveDetector(sens);
      thickness_sensitive += layer_thickness;
      surf_type = rec::SurfaceType(rec::SurfaceType::Sensitive, rec::SurfaceType::Plane, rec::SurfaceType::ParallelToZ);
    }
    else
    {
      if (thickness_sensitive == 0) thickness_support += layer_thickness;
      surf_type = rec::SurfaceType(rec::SurfaceType::Helper, rec::SurfaceType::Plane, rec::SurfaceType::ParallelToZ);
    }


    //TODO: add tube
    // if (layer.hasChild(_U(tube)))
    // {
    //     xml::Component tube = layer.child(_U(tube));
    //     auto tube_name = tube.attr<std::string>("name");
    //     double inner_diameter = tube.attr<double>("inner_diameter");
    //     double outer_diameter = tube.attr<double>("outer_diameter");
    // }


    rec::Vector3D u(0., 1., 0.);
    rec::Vector3D v(0., 0., 1.);
    rec::Vector3D n(1., 0., 0.);
    rec::VolPlane inner_surf(inner_layer_vol, surf_type, layer_thickness / 2.0, layer_thickness / 2.0, u, v, n);
    rec::VolPlane outer_surf(outer_layer_vol, surf_type, layer_thickness / 2.0, layer_thickness / 2.0, u, v, n);
    rec::volSurfaceList(inner_layer_det)->push_back(inner_surf);
    rec::volSurfaceList(outer_layer_det)->push_back(outer_surf);

    // construct module
    Position layer_pos(x_offset + layer_thickness / 2.0, 0, 0);
    inner_layer_det.setPlacement(
      inner_module_envelope_vol.placeVolume(inner_layer_vol, layer_pos));
    outer_layer_det.setPlacement(
      outer_module_envelope_vol.placeVolume(outer_layer_vol, layer_pos));
    inner_module_det.add(inner_layer_det);
    outer_module_det.add(outer_layer_det);
    x_offset += layer_thickness;

  }

  // construct module
  DetElement first_data_aggregation_det_inner(first_data_aggregation_name + "_inner", det_id);
  DetElement first_data_aggregation_det_outer(first_data_aggregation_name + "_outer", det_id);
  Position first_data_aggregation_pos(x_offset + first_data_aggregation_thickness / 2.0, 0, 0);
  first_data_aggregation_det_inner.setPlacement(
    inner_module_envelope_vol.placeVolume(first_data_aggregation_vol, first_data_aggregation_pos));
  first_data_aggregation_det_outer.setPlacement(
    outer_module_envelope_vol.placeVolume(first_data_aggregation_vol, first_data_aggregation_pos));
  inner_module_det.add(first_data_aggregation_det_inner);
  outer_module_det.add(first_data_aggregation_det_outer);

  // construct ladder
  for (int i = 0; i < module_repeat; ++i)
  {
    double z_position = -(module_repeat / 2.0) * module_length_inner + module_length_inner / 2.0 + i *
      module_length_inner;
    Position pos((module_envelope_shape_base_thickness - ladder_thickness) / 2.0, 0, z_position);
    auto cloned_inner_module_det = inner_module_det.clone("inner_" + module_name + std::to_string(i));
    cloned_inner_module_det.setPlacement(
      inner_ladder_vol.placeVolume(inner_module_envelope_vol, pos).addPhysVolID("mmodule", i + 1));
    inner_ladder_det.add(cloned_inner_module_det);

    double z_position_outer = -(module_repeat / 2.0) * module_length_outer + module_length_outer / 2.0 + i *
      module_length_outer;
    Position pos_outer((module_envelope_shape_base_thickness - ladder_thickness) / 2.0, 0, z_position_outer);
    auto cloned_outer_module_det = outer_module_det.clone("outer_" + module_name + std::to_string(i));
    cloned_outer_module_det.setPlacement(
      outer_ladder_vol.placeVolume(outer_module_envelope_vol, pos_outer).addPhysVolID("mmodule", -(i + 1)));
    outer_ladder_det.add(cloned_outer_module_det);
  }

  DetElement second_data_aggregation_det_inner(second_data_aggregation_name + "_inner", det_id);
  DetElement second_data_aggregation_det_outer(second_data_aggregation_name + "_outer", det_id);
  Position second_data_aggregation_pos((ladder_thickness - second_data_aggregation_thickness) / 2.0,
                                       (module_width - second_data_aggregation_width) / 2.0, 0);
  second_data_aggregation_det_inner.setPlacement(
    inner_ladder_vol.placeVolume(second_data_aggregation_vol, second_data_aggregation_pos));
  second_data_aggregation_det_outer.setPlacement(
    outer_ladder_vol.placeVolume(second_data_aggregation_vol, second_data_aggregation_pos));
  inner_ladder_det.add(second_data_aggregation_det_inner);
  outer_ladder_det.add(second_data_aggregation_det_outer);

  // inner ladder
  for (int i = 0; i < ladder_repeat_inner; ++i)
  {
    double z_position = -(ladder_repeat_inner / 2.0) * ladder_length_inner + (i + 0.5) * ladder_length_inner;
    Position pos(0, 0, z_position);
    auto cloned_inner_ladder_det = inner_ladder_det.clone("inner_" + ladder_name + std::to_string(i));
    cloned_inner_ladder_det.setPlacement(
      stave_vol.placeVolume(inner_ladder_vol, pos).addPhysVolID("iladder", i));
    stave_det.add(cloned_inner_ladder_det);
  }

  // outer ladder (positive x)
  for (int i = 0; i < ladder_repeat_outer / 2; ++i)
  {
    double z_position = ladder_length_inner * (ladder_repeat_inner / 2.0) + (i + 0.5) * ladder_length_outer;
    Position pos(0, 0, z_position);
    auto cloned_outer_ladder_det = outer_ladder_det.clone("outer_" + ladder_name + std::to_string(i + 1));
    cloned_outer_ladder_det.setPlacement(
      stave_vol.placeVolume(outer_ladder_vol, pos).addPhysVolID("oladder", i + 1));
    stave_det.add(cloned_outer_ladder_det);
  }

  // outer ladder (neg x)
  for (int i = 0; i < ladder_repeat_outer / 2; ++i)
  {
    double z_position = -ladder_length_inner * (ladder_repeat_inner / 2.0) - (i + 0.5) * ladder_length_outer;
    Position pos(0, 0, z_position);
    auto cloned_outer_ladder_det = outer_ladder_det.clone("outer_" + ladder_name + std::to_string(-i - 1));
    cloned_outer_ladder_det.setPlacement(stave_vol.placeVolume(outer_ladder_vol, pos).addPhysVolID("oladder", -i - 1));
    stave_det.add(cloned_outer_ladder_det);
  }

  // place rotated stave into envelope
  double otk_inner_radius = theDetector.constant<double>("OTKBarrel_inner_radius");
  double stave_x_offset = otk_inner_radius + stave_thickness / 2.0;
  double stave_y_offset = stave_width / 2.0;
  auto z_planar_data = new rec::ZPlanarData;
  for (int i = 0; i < stave_repeat; ++i)
  {
    double angle = i * angle_step;
    // calculate rotated pos
    double rotated_x = stave_x_offset * cos(angle) - stave_y_offset * sin(angle);
    double rotated_y = stave_x_offset * sin(angle) + stave_y_offset * cos(angle);
    auto cloned_stave_det = stave_det.clone("stave_" + std::to_string(i));
    cloned_stave_det.setPlacement(
      envelope.placeVolume(stave_vol, Transform3D(RotationZ(angle), Position(rotated_x, rotated_y, 0.0))).
               addPhysVolID("module", i));
    otk_barrel.add(cloned_stave_det);
  }
  rec::ZPlanarData::LayerLayout otk_barrel_layer;
  otk_barrel_layer.phi0 = 0;
  otk_barrel_layer.ladderNumber = stave_repeat;
  otk_barrel_layer.thicknessSensitive = thickness_sensitive;
  otk_barrel_layer.thicknessSupport = thickness_support;
  otk_barrel_layer.distanceSensitive = otk_inner_radius + thickness_support;
  otk_barrel_layer.distanceSupport = otk_inner_radius;
  otk_barrel_layer.offsetSensitive = stave_y_offset;
  otk_barrel_layer.offsetSupport = stave_y_offset;
  otk_barrel_layer.widthSensitive = module_width;
  otk_barrel_layer.widthSupport = module_width;
  otk_barrel_layer.sensorsPerLadder = module_repeat * 4;
  double z_half = ladder_length_inner * ladder_repeat_inner / 2.0 + ladder_length_outer * ladder_repeat_outer / 2.0;
  otk_barrel_layer.zHalfSensitive = z_half;
  otk_barrel_layer.zHalfSupport = z_half;
  z_planar_data->layers.push_back(otk_barrel_layer);
  // check_det_element(otk_barrel);

  std::cout << (*z_planar_data) << std::endl;
  otk_barrel.addExtension<rec::ZPlanarData>(z_planar_data);
  if (x_det.hasAttr(_U(combineHits)))
  {
    otk_barrel.setCombineHits(x_det.attr<bool>(_U(combineHits)), sens);
  }
  return otk_barrel;
}

DECLARE_DETELEMENT(SiTracker_otkbarrel_v02, create_element)
