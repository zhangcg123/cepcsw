#include <DD4hep/Detector.h>
#include "DD4hep/DetFactoryHelper.h"
#include "XML/Utilities.h"
#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "Math/AxisAngle.h"
#include <map>

using namespace dd4hep;

static Ref_t create_detector(Detector& description, xml_h e, SensitiveDetector sens)
{
  xml_det_t x_det = e;
  Material air = description.air();
  int det_id = x_det.id();
  std::string name = x_det.nameStr();
  bool reflect = x_det.reflect(false);
  DetElement tracker(name, det_id);

  Volume envelope = createPlacedEnvelope(description, e, tracker);
  setDetectorTypeFlag(e, tracker);
  if (description.buildType() == BUILD_ENVELOPE)
    return tracker;
  envelope = envelope.setVisAttributes(description.visAttributes("SeeThrough"));

  if (x_det.hasChild(_U(sensitive)))
  {
    xml_dim_t sd_typ = x_det.child(_U(sensitive));
    sens.setType(sd_typ.typeStr());
  }
  else
  {
    sens.setType("tracker");
  }
  std::cout << " ** building ITK_EndCap_v01 ... " << sens.type() << std::endl;

  const xml::Component layers_xml = x_det.child("layers");
  for (xml_coll_t layer(layers_xml, "layer"); layer; ++layer)
  {
    //id="0" z="505*mm" dz="SiliconThickness*2 + SupportThickness" inner_r="81.5*mm" outer_r="242.5*mm" vis="SeeThrough"
    const auto layer_id = layer.attr<std::string>("id");
    const auto dz = layer.attr<double>("dz");
    const auto inner_r = layer.attr<double>("inner_r");
    const auto outer_r = layer.attr<double>("outer_r");
    const auto layer_vis = layer.attr<std::string>("vis");
    const auto z = layer.attr<double>("z");
    std::string layer_name = name + "_layer_" + layer_id;

    const auto SiliconThickness = description.constant<double>("SiliconThickness");
    const auto SupportThickness = description.constant<double>("SupportThickness");

    Tube layer_tube(inner_r, outer_r, dz / 2.0, 0.0, 2 * M_PI);
    Volume layer_vol(layer_name, layer_tube, air);
    DetElement layer_det(layer_name, det_id);
    layer_vol = layer_vol.setVisAttributes(description.visAttributes(layer_vis));

    // populate support
    double support_z = -SupportThickness / 2.0;
    const xml::Component supports = x_det.child("support");
    for (xml_coll_t support(supports, "slice"); support; ++support)
    {
      const auto support_name = support.attr<std::string>("name");
      const auto support_material = support.attr<std::string>("material");
      const auto support_thickness = support.attr<double>("thickness");
      const auto support_vis = support.attr<std::string>("vis");
      std::string support_vol_name = layer_name + "_" + support_name;
      Tube support_tube(inner_r, outer_r, support_thickness / 2.0, 0.0, 2 * M_PI);
      Volume support_vol(support_vol_name, support_tube, description.material(support_material));
      DetElement support_det(support_vol_name, det_id);
      support_vol = support_vol.setVisAttributes(description.visAttributes(support_vis));
      support_det.setPlacement(layer_vol.placeVolume(support_vol, Position(0, 0, support_z + support_thickness / 2.0)));
      support_z += support_thickness;
      layer_det.add(support_det);
      rec::Vector3D u(1., 0., 0.);
      rec::Vector3D v(0., 0., 1.);
      rec::Vector3D n(0., 1., 0.);
      rec::VolPlane surf(support_vol, rec::SurfaceType::Plane, support_thickness / 2.0, support_thickness / 2.0, u, v,
                         n);
      rec::volSurfaceList(layer_det)->push_back(surf);
    }


    // populate sensor
    for (xml_coll_t ring(layer, "ring"); ring; ++ring)
    {
      const auto ring_id = ring.attr<std::string>("id");
      const auto ring_inner_r = ring.attr<double>("inner_r");
      const auto ring_outer_r = ring.attr<double>("outer_r");
      const auto module_dr = ring.attr<double>("module_dr");
      const auto module_dphi = ring.attr<double>("module_dphi");
      const auto nmodule = ring.attr<double>("nmodule");
      const auto vis = ring.attr<std::string>("vis");

      std::string ring_vol_name = layer_name + "_ring_" + ring_id;
      Tube ring_tube(ring_inner_r, ring_outer_r, SiliconThickness / 2.0, 0.0, 2 * M_PI);
      Volume ring_vol(ring_vol_name, ring_tube, air);
      DetElement ring_det(ring_vol_name, det_id);
      ring_vol = ring_vol.setVisAttributes(description.visAttributes(vis));

      // populate sensor
      Box sensor_box(module_dr / 2.0, module_dphi / 2.0, SiliconThickness / 2.0);
      std::string sensor_name = ring_vol_name + "_sensor";
      Volume sensor_vol(sensor_name, sensor_box, air);
      DetElement sensor_det(sensor_name, det_id);
      sensor_vol = sensor_vol.setVisAttributes(description.visAttributes("SeeThrough"));
      const xml::Component sensor = x_det.child("sensor");
      double sensor_z = -SiliconThickness / 2.0;
      for (xml_coll_t sensor_layer(sensor, "slice"); sensor_layer; ++sensor_layer)
      {
        const auto sensor_layer_name = sensor_layer.attr<std::string>("name");
        const auto sensor_layer_material = sensor_layer.attr<std::string>("material");
        const auto sensor_layer_thickness = sensor_layer.attr<double>("thickness");
        const auto sensor_layer_vis = sensor_layer.attr<std::string>("vis");
        auto is_sensitive = sensor_layer.hasAttr(_U(sensitive));
        std::string sensor_layer_vol_name = sensor_name + "_" + sensor_layer_name;
        Box sensor_layer_box(module_dr / 2.0, module_dphi / 2.0, sensor_layer_thickness / 2.0);
        Volume sensor_layer_vol(sensor_layer_vol_name, sensor_layer_box, description.material(sensor_layer_material));
        DetElement sensor_layer_det(sensor_layer_vol_name, det_id);
        sensor_layer_vol = sensor_layer_vol.setVisAttributes(description.visAttributes(sensor_layer_vis));
        rec::SurfaceType surf_type;
        if (is_sensitive)
        {
          sensor_layer_vol = sensor_layer_vol.setSensitiveDetector(sens);
          surf_type = rec::SurfaceType(rec::SurfaceType::Sensitive,rec::SurfaceType::Plane);
        }
        else
        {
          surf_type = rec::SurfaceType(rec::SurfaceType::Helper,rec::SurfaceType::Plane);
        }
        sensor_layer_det.setPlacement(
          sensor_vol.placeVolume(sensor_layer_vol, Position(0, 0, sensor_z + sensor_layer_thickness / 2.0)));
        sensor_z += sensor_layer_thickness;

        sensor_det.add(sensor_layer_det);
        rec::Vector3D u(1., 0., 0.);
        rec::Vector3D v(0., 1., 0.);
        rec::Vector3D n(0., 0., 1.);
        rec::VolPlane surf(sensor_layer_vol, surf_type, sensor_layer_thickness / 2.0,
                           sensor_layer_thickness / 2.0, u, v,
                           n);
        rec::volSurfaceList(sensor_det)->push_back(surf);
      }

      // place all the sensors into ring
      for (int i = 0; i < nmodule; ++i)
      {
        double angle = i * 2 * M_PI / nmodule;
        double r_offset = ring_inner_r + module_dr / 2.0;
        double rotated_x = r_offset * cos(angle);
        double rotated_y = r_offset * sin(angle);
        auto transform = Transform3D(
          RotationZ(angle),
          Position(rotated_x, rotated_y, 0));

        DetElement rotated_sensor_det = sensor_det.clone(sensor_name + "_rotated_" + std::to_string(i));
        rotated_sensor_det.setPlacement(
          ring_vol.placeVolume(sensor_vol, transform).addPhysVolID("module", i));
        ring_det.add(rotated_sensor_det);
      }

      // place ring into layer
      ring_det.setPlacement(
        layer_vol.placeVolume(ring_vol, Position(0, 0, (SupportThickness + SiliconThickness) / 2.0)).addPhysVolID("sensor", 2*std::stoi(ring_id)));
      layer_det.add(ring_det);
      // rotate and reflect ring
      std::string reflect_ring_name = ring_vol_name + "_reflect";
      auto reflect_ring_det = ring_det.clone(reflect_ring_name);
      reflect_ring_det.setPlacement(layer_vol.placeVolume(
        ring_vol, Transform3D(RotationZ(M_PI / nmodule) * Rotation3D(1, 0, 0, 0, 1, 0, 0, 0, -1) ,
                              Position(0, 0, -(SupportThickness + SiliconThickness) / 2.0))).addPhysVolID("sensor", 2*std::stoi(ring_id)+1));
      layer_det.add(reflect_ring_det);
    }


    // put layer into envelope
    layer_det.setPlacement(envelope.placeVolume(layer_vol, Position(0, 0, z)).
                                    addPhysVolID("side", 1).addPhysVolID("layer", std::stoi(layer_id)));
    tracker.add(layer_det);
    // copy layer_vol and put reflect
    if (reflect)
    {
      std::string reflect_layer_name = layer_name + "_reflect";
      auto reflect_layer_det = layer_det.clone(reflect_layer_name);
      reflect_layer_det.setPlacement(
        envelope.placeVolume(layer_vol, Transform3D(Rotation3D(1, 0, 0, 0, 1, 0, 0, 0, -1), Position(0, 0, -z))).addPhysVolID("side", -1).
                 addPhysVolID("layer", std::stoi(layer_id)));
      tracker.add(reflect_layer_det);
    }
  }

  if (x_det.hasAttr(_U(combineHits)))
  {
    tracker.setCombineHits(x_det.attr<bool>(_U(combineHits)), sens);
  }

  return tracker;
}

DECLARE_DETELEMENT(ITK_EndCap_v01, create_detector)
