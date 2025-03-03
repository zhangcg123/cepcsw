#include <DD4hep/Detector.h>
#include "DD4hep/DetFactoryHelper.h"
#include "XML/Utilities.h"
#include "DDRec/Surface.h"
#include "DetIdentifier/CEPCDetectorData.h"
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

  dd4hep::rec::MultiRingsZDiskData* diskData = new dd4hep::rec::MultiRingsZDiskData;

  const xml::Component supports = x_det.child("support");
  const xml::Component sensor   = x_det.child("sensor");

  const xml::Component layers_xml = x_det.child("layers");
  for (xml_coll_t layer(layers_xml, "layer"); layer; ++layer)
  {
    dd4hep::rec::MultiRingsZDiskData::LayerLayout disk;
    //id="0" z="505*mm" dz="SiliconThickness*2 + SupportThickness" inner_r="81.5*mm" outer_r="242.5*mm" vis="SeeThrough"
    const auto layer_id = layer.attr<std::string>("id");
    //const auto dz = layer.attr<double>("dz");
    const auto inner_r = layer.attr<double>("inner_r");
    const auto outer_r = layer.attr<double>("outer_r");
    const auto phi0 = layer.attr<double>("phi0");
    const auto layer_vis = layer.attr<std::string>("vis");
    const auto z = layer.attr<double>("z");
    std::string layer_name = "layer" + layer_id;

    double SiliconThickness = 0;
    double SupportThickness = 0;
    for (xml_coll_t sensor_layer(sensor, "slice"); sensor_layer; ++sensor_layer)
    {
      const auto slice_thickness = sensor_layer.attr<double>("thickness");
      SiliconThickness += slice_thickness;
    }
    for (xml_coll_t support(supports, "slice"); support; ++support)
    {
      const auto slice_thickness = support.attr<double>("thickness");
      SupportThickness += slice_thickness;
    }
    const auto dz = 2*SiliconThickness + SupportThickness;

    Tube layer_tube(inner_r, outer_r, dz / 2.0, 0.0, 2 * M_PI);
    Volume layer_vol(layer_name + "P", layer_tube, air);
    Volume layerNeg_vol(layer_name + "N", layer_tube, air);
    DetElement layer_det(layer_name + "P", det_id);
    DetElement layerNeg_det(layer_name + "N", det_id);
    layer_vol.setVisAttributes(description.visAttributes(layer_vis));
    layerNeg_vol.setVisAttributes(description.visAttributes(layer_vis));

    // populate support
    double support_z = -SupportThickness / 2.0;
    //const xml::Component supports = x_det.child("support");
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
      DetElement supportNeg_det(support_vol_name + "N", det_id);
      support_vol = support_vol.setVisAttributes(description.visAttributes(support_vis));
      support_det.setPlacement(layer_vol.placeVolume(support_vol, Position(0, 0, support_z + support_thickness / 2.0)));
      supportNeg_det.setPlacement(layerNeg_vol.placeVolume(support_vol, Position(0, 0, - support_z - support_thickness / 2.0)));
      support_z += support_thickness;
      layer_det.add(support_det);
      layerNeg_det.add(supportNeg_det);
      rec::Vector3D u(1., 0., 0.);
      rec::Vector3D v(0., 1., 0.);
      rec::Vector3D n(0., 0., 1.);
      rec::VolPlane surf(support_vol, rec::SurfaceType::Helper, support_thickness / 2.0, support_thickness / 2.0, u, v,
                         n);
      rec::volSurfaceList(support_det)->push_back(surf);
      rec::volSurfaceList(supportNeg_det)->push_back(surf);
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

      std::string ring_vol_name = layer_name + "_ring" + ring_id;
      Tube ring_tube(ring_inner_r, ring_outer_r, SiliconThickness / 2.0, 0.0, 2 * M_PI);
      Volume ringPosF_vol(ring_vol_name + "PF", ring_tube, air);
      Volume ringPosR_vol(ring_vol_name + "PR", ring_tube, air);
      Volume ringNegF_vol(ring_vol_name + "NF", ring_tube, air);
      Volume ringNegR_vol(ring_vol_name + "NR", ring_tube, air);
      DetElement ringPosF_det(ring_vol_name + "PF", det_id);
      DetElement ringPosR_det(ring_vol_name + "PR", det_id);
      DetElement ringNegF_det(ring_vol_name + "NF", det_id);
      DetElement ringNegR_det(ring_vol_name + "NR", det_id);
      ringPosF_vol.setVisAttributes(description.visAttributes(vis));
      ringPosR_vol.setVisAttributes(description.visAttributes(vis));
      ringNegF_vol.setVisAttributes(description.visAttributes(vis));
      ringNegR_vol.setVisAttributes(description.visAttributes(vis));

      // populate sensor
      Box sensor_box(module_dr / 2.0, module_dphi / 2.0, SiliconThickness / 2.0);
      std::string sensor_name = ring_vol_name + "_petal";
      Volume sensor_vol(sensor_name, sensor_box, air);
      DetElement sensor_det(sensor_name, det_id);
      sensor_vol = sensor_vol.setVisAttributes(description.visAttributes("SeeThrough"));

      double sensitive_thickness = 0;
      double glue_thickness      = 0;
      double service_thickness   = 0;
      //const xml::Component sensor = x_det.child("sensor");
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
	  sensitive_thickness += sensor_layer_thickness;
        }
        else
        {
          surf_type = rec::SurfaceType(rec::SurfaceType::Helper,rec::SurfaceType::Plane);
	  if (sensitive_thickness==0) glue_thickness    += sensor_layer_thickness;
          else                        service_thickness += sensor_layer_thickness;
        }
        sensor_layer_det.setPlacement(
          sensor_vol.placeVolume(sensor_layer_vol, Position(0, 0, sensor_z + sensor_layer_thickness / 2.0)));
        sensor_z += sensor_layer_thickness;

        sensor_det.add(sensor_layer_det);
        rec::Vector3D u(0.,-1., 0.);
        rec::Vector3D v(1., 0., 0.);
        rec::Vector3D n(0., 0., 1.);
        rec::VolPlane surf(sensor_layer_vol, surf_type, sensor_layer_thickness / 2.0,
                           sensor_layer_thickness / 2.0, u, v,
                           n);
        rec::volSurfaceList(sensor_layer_det)->push_back(surf);
      }

      PlacedVolume pv;
      // place all the sensors into ring
      double r_offset = ring_inner_r + module_dr / 2.0;
      for (int i = 0; i < nmodule; ++i)
      {
        double angle = phi0 - i * 2 * M_PI / nmodule;
	double rotated_x = r_offset * cos(angle);
	double rotated_y = r_offset * sin(angle);
        auto transform = Transform3D(RotationZ(angle), Position(rotated_x, rotated_y, 0));

        DetElement negative_even_sensor_det = i > 0 ? sensor_det.clone(ring_vol_name + "_petalN" + std::to_string(2 * i)) : sensor_det;
	pv = ringNegF_vol.placeVolume(sensor_vol, transform).addPhysVolID("module", 2 * i);
        negative_even_sensor_det.setPlacement(pv);
        ringNegF_det.add(negative_even_sensor_det);

        transform = Transform3D(RotationZYX(-angle, 0, M_PI), Position(rotated_x, rotated_y, 0));
	DetElement positive_even_sensor_det = sensor_det.clone(ring_vol_name + "_petalP" + std::to_string(2 * i));
        pv = ringPosF_vol.placeVolume(sensor_vol, transform).addPhysVolID("module", 2 * i);
        positive_even_sensor_det.setPlacement(pv);
        ringPosF_det.add(positive_even_sensor_det);

	angle -= M_PI / nmodule;
        rotated_x = r_offset * cos(angle);
        rotated_y = r_offset * sin(angle);
	transform = Transform3D(RotationZ(angle), Position(rotated_x, rotated_y, 0));

	DetElement positive_odd_sensor_det = sensor_det.clone(ring_vol_name + "_petalP" + std::to_string(2 * i));
        pv = ringPosR_vol.placeVolume(sensor_vol, transform).addPhysVolID("module", 2 * i + 1);
        positive_odd_sensor_det.setPlacement(pv);
        ringPosR_det.add(positive_odd_sensor_det);

	transform = Transform3D(RotationZYX(-angle, 0, M_PI), Position(rotated_x, rotated_y, 0));
	DetElement negative_odd_sensor_det = sensor_det.clone(ring_vol_name + "_petalN" + std::to_string(2 * i));
	pv = ringNegR_vol.placeVolume(sensor_vol, transform).addPhysVolID("module", 2 * i + 1);
        negative_odd_sensor_det.setPlacement(pv);
        ringNegR_det.add(negative_odd_sensor_det);
      }

      // place ring into layer
      pv = layer_vol.placeVolume(ringPosR_vol, Position(0, 0, +(SupportThickness + SiliconThickness) / 2.0));
      pv = pv.addPhysVolID("sensor", std::stoi(ring_id));
      ringPosR_det.setPlacement(pv);
      layer_det.add(ringPosR_det);

      pv = layer_vol.placeVolume(ringPosF_vol, Position(0, 0, -(SupportThickness + SiliconThickness) / 2.0));
      pv = pv.addPhysVolID("sensor", std::stoi(ring_id));
      ringPosF_det.setPlacement(pv);
      layer_det.add(ringPosF_det);

      pv = layerNeg_vol.placeVolume(ringNegR_vol, Position(0, 0, +(SupportThickness + SiliconThickness) / 2.0));
      pv = pv.addPhysVolID("sensor", std::stoi(ring_id));
      ringNegR_det.setPlacement(pv);
      layerNeg_det.add(ringNegR_det);

      pv = layerNeg_vol.placeVolume(ringNegF_vol, Position(0, 0, -(SupportThickness + SiliconThickness) / 2.0));
      pv = pv.addPhysVolID("sensor", std::stoi(ring_id));
      ringNegF_det.setPlacement(pv);
      layerNeg_det.add(ringNegF_det);
      // rotate and reflect ring
      // fucd: error while use Rotation3D(1, 0, 0, 0, 1, 0, 0, 0, -1)
      // Geant4VolumeManager INFO  +++   Bad volume Geant4 Path
      /*
      std::string reflect_ring_name = ring_vol_name + "_reflect";
      auto reflect_ring_det = ring_det.clone(reflect_ring_name, det_id);
      reflect_ring_det.setPlacement(layer_vol.placeVolume(
        ring_vol, Transform3D(RotationZ(M_PI / nmodule) * Rotation3D(1, 0, 0, 0, 1, 0, 0, 0, -1) ,
                              Position(0, 0, -(SupportThickness + SiliconThickness) / 2.0)))
				    .addPhysVolID("sensor", std::stoi(ring_id)).addPhysVolID("face", 1));
      layer_det.add(reflect_ring_det);
      */
      // fucd: also error
      /*
      std::pair<DetElement,Volume> reflected_ring = ring_det.reflect(ring_vol_name + "_reflect");
      auto pv = layer_vol.placeVolume(reflected_ring.second, Position(0, 0, -(SupportThickness + SiliconThickness) / 2.0))
	                 .addPhysVolID("sensor", std::stoi(ring_id))
	                 .addPhysVolID("face", 1);
      reflected_ring.first.setPlacement(pv);
      layer_det.add(reflected_ring.first);
      */
      dd4hep::rec::MultiRingsZDiskData::Ring ringData;
      ringData.petalNumber        = 2 * nmodule;
      ringData.sensorsPerPetal    = 1;
      ringData.phi0               = phi0;
      ringData.phiOffsetOdd       = 2 * M_PI / nmodule;
      ringData.distance           = ring_inner_r;
      ringData.widthInner         = module_dphi;
      ringData.widthOuter         = module_dphi;
      ringData.length             = module_dr;
      ringData.thicknessSensitive = SiliconThickness - glue_thickness - service_thickness;
      ringData.thicknessGlue      = glue_thickness;
      ringData.thicknessService   = service_thickness;
      disk.rings.push_back(ringData);
    }

    // put layer into envelope
    layer_det.setPlacement(envelope.placeVolume(layer_vol, Position(0, 0, z))
			   .addPhysVolID("layer", std::stoi(layer_id)).addPhysVolID("side", 1));
    tracker.add(layer_det);
    // copy layer_vol and put reflect
    if (reflect)
    {
      /*
      std::string reflect_layer_name = layer_name + "_reflect";
      auto reflect_layer_det = layer_det.clone(reflect_layer_name, det_id);
      reflect_layer_det.setPlacement(
        envelope.placeVolume(layer_vol, Transform3D(Rotation3D(1, 0, 0, 0, 1, 0, 0, 0, -1), Position(0, 0, -z))).addPhysVolID("side", -1).
                 addPhysVolID("layer", std::stoi(layer_id)));
      tracker.add(reflect_layer_det);
      */
      /*
      std::pair<DetElement,Volume> reflected_layer = layer_det.reflect(layer_name + "_reflect");
      auto pv = envelope.placeVolume(reflected_layer.second, Position(0, 0, -z))
	                .addPhysVolID("side", -1).addPhysVolID("layer", std::stoi(layer_id));
      reflected_layer.first.setPlacement(pv);
      tracker.add(reflected_layer.first);
      */
      layerNeg_det.setPlacement(envelope.placeVolume(layerNeg_vol, Position(0, 0, -z))
				.addPhysVolID("layer", std::stoi(layer_id)).addPhysVolID("side", -1));
      tracker.add(layerNeg_det);
    }

    disk.typeFlags[dd4hep::rec::MultiRingsZDiskData::SensorType::DoubleSided] = 1;
    disk.typeFlags[dd4hep::rec::MultiRingsZDiskData::SensorType::Pixel]       = 1;
    disk.alphaPetal       = 0;
    disk.zPosition        = z;
    disk.zOffsetSupport   = 0;
    disk.rminSupport      = inner_r;
    disk.rmaxSupport      = outer_r;
    disk.thicknessSupport = SupportThickness;
    diskData->layers.push_back(disk);
  }

  std::cout << (*diskData) << std::endl;
  tracker.addExtension<dd4hep::rec::MultiRingsZDiskData>(diskData);

  if (x_det.hasAttr(_U(combineHits)))
  {
    tracker.setCombineHits(x_det.attr<bool>(_U(combineHits)), sens);
  }

  return tracker;
}

DECLARE_DETELEMENT(ITK_EndCap_v01, create_detector)
