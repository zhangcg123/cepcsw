#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DetType.h"
#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "XML/Utilities.h"
#include <cmath>

using namespace dd4hep;

inline double calculate_polygon_side_length(const double radius, const int sides)
{
    return 2 * radius * tan(M_PI / sides);
}

// #define DET_ELEMENT_DEBUG
#ifdef DET_ELEMENT_DEBUG
void check_det_element(const DetElement det)
{
    std::cout << "Checking DetElement: " << det.name() << std::endl;
    if (det.children().size() == 0)
    {
        std::cout << "name: " << det.name() << std::endl;
        std::cout << "path: " << det.path() << std::endl;
        std::cout << "No Children: " << det.name() << std::endl;
        const auto list = det.extension<rec::VolSurfaceList>(false);
        if (!list) return;
        for (auto surf : *list)
        {
            std::cout << "!!!Surface: " << surf.volume().name() << std::endl;
        }
        return;
    }
    for (auto [fst, snd] : det.children())
    {
        check_det_element(snd);
    }
}
#endif


static Ref_t create_element(Detector& theDetector, xml_h e, SensitiveDetector sens)
{
    xml_det_t x_det = e;
    Material air = theDetector.air();
    int det_id = x_det.id();
    const std::string name = x_det.nameStr();
    DetElement otk_endcaps(name, det_id);

    const xml::Component envelop_xml_element = x_det.child("envelope");
    Volume envelope = createPlacedEnvelope(theDetector, e, otk_endcaps);
    envelope = envelope.setVisAttributes(theDetector, envelop_xml_element.attr<std::string>("vis"));
    setDetectorTypeFlag(e, otk_endcaps);
    if (theDetector.buildType() == BUILD_ENVELOPE) return otk_endcaps;

    if (x_det.hasAttr(_U(sensitive)))
    {
        const xml_dim_t sd_typ = x_det.child(_U(sensitive));
        sens.setType(sd_typ.typeStr());
    }
    else
    {
        sens.setType("tracker");
    }

    const std::string detector_name = x_det.attr<std::string>(_U(name));
    const std::string detector_type = x_det.attr<std::string>(_U(type));
    std::cout << "Processing Detector: " << detector_name
        << ", Type: " << detector_type << std::endl;

    // start construction
    const xml::Component rings_xml = x_det.child("rings");
    const xml::Component layers_xml = x_det.child("layers");

    double zmax = theDetector.constant<double>(rings_xml.attr<std::string>("zmax"));

    const xml::Component first_data_aggregation = layers_xml.child("first_data_aggregation");
    const double first_data_aggregation_thickness = first_data_aggregation.attr<double>("thickness");

    int ring_num = 0;
    for (xml_coll_t rings(rings_xml, "ring"); rings; ++rings, ++ring_num)
    {
        xml::Component ring = xml::Handle_t(rings);
        const auto ring_name = ring.attr<std::string>("name");
        const double ring_inner_radius = ring.attr<double>("inner_radius");
        const double ring_outer_radius = ring.attr<double>("outer_radius");
        const int repeat = ring.attr<int>("repeat");
        double angle_step = 2 * M_PI / repeat;
        DetElement ring_piece_det(ring_name, det_id);

        const double trapezoid_height = ring_outer_radius - ring_inner_radius;
        const double trapezoid_inner_length = calculate_polygon_side_length(ring_inner_radius, repeat);
        const double trapezoid_outer_length = calculate_polygon_side_length(ring_outer_radius, repeat);
        const double layers_thickness = layers_xml.attr<double>("thickness");
        const double layers_base_thickness = layers_thickness - first_data_aggregation_thickness;

        Trapezoid layer_base_shape(trapezoid_inner_length / 2.0, trapezoid_outer_length / 2.0,
                                   layers_base_thickness / 2.0, layers_base_thickness / 2.0, trapezoid_height / 2.0);
        Volume layer_base_vol(ring_name, layer_base_shape, air);
        layer_base_vol = layer_base_vol.setVisAttributes(theDetector, ring.attr<std::string>("vis"));

        double y_offset = -layers_base_thickness / 2.0;
        {
        }
        for (xml_coll_t layers(layers_xml, "layer"); layers; ++layers)
        {
            const xml::Component layer = xml::Handle_t(layers);
            const auto layer_name = layer.attr<std::string>("name");
            const double layer_thickness = layer.attr<double>("thickness");
            const Material material = theDetector.material(layer.attr<std::string>("material"));
            const auto vis = layer.attr<std::string>("vis");
            auto is_sensitive = layer.hasAttr(_U(sensitive));
            DetElement layer_base_det(layer_name, det_id);
            rec::SurfaceType surf_type;

            Trapezoid layer_shape(trapezoid_inner_length / 2.0, trapezoid_outer_length / 2.0, layer_thickness / 2.0,
                                  layer_thickness / 2.0, trapezoid_height / 2.0);
            Volume layer_vol(layer_name, layer_shape, material);
            layer_vol = layer_vol.setVisAttributes(theDetector, vis);
            if (is_sensitive)
            {
                layer_vol = layer_vol.setSensitiveDetector(sens);
                surf_type = rec::SurfaceType::Sensitive;
            }
            else
            {
                surf_type = rec::SurfaceType::Plane;
            }
            rec::Vector3D u(0., 0., 1.);
            rec::Vector3D v(0., 1., 0.);
            rec::Vector3D n(1., 0., 0.);
            rec::VolPlane surf(layer_vol, surf_type, layer_thickness / 2.0, layer_thickness / 2.0, u, v, n);
            rec::volSurfaceList(layer_base_det)->push_back(surf);

            layer_base_det.setPlacement(
                layer_base_vol.placeVolume(layer_vol, Position(0, y_offset + layer_thickness / 2.0, 0)));
            ring_piece_det.add(layer_base_det);
            y_offset += layer_thickness;
        }

        auto rotationX = RotationX(90.0 * deg);
        auto rotationZ = RotationZ(90.0 * deg);
        auto neg_rotationX = RotationX(-90.0 * deg);
        auto neg_rotationZ = RotationZ(-90.0 * deg);

        for (int i = 0; i < repeat; ++i)
        {
            double angle = i * angle_step;
            double r_offset = ring_inner_radius + trapezoid_height / 2.0;
            double rotated_x = r_offset * cos(angle);
            double rotated_y = r_offset * sin(angle);
            auto transform = Transform3D(
                RotationZ(angle) * rotationZ * rotationX,
                Position(rotated_x, rotated_y,
                         zmax - layers_base_thickness / 2.0));
            auto neg_transform = Transform3D(
                RotationZ(angle) * neg_rotationZ * neg_rotationX,
                Position(rotated_x, rotated_y,
                         -(zmax - layers_base_thickness / 2.0)));
            auto cloned_ring_piece_det = ring_piece_det.clone(ring_name + std::to_string(i + 1));
            auto cloned_neg_ring_piece_det = ring_piece_det.clone(ring_name + std::to_string(-(i + 1)));
            auto pv = envelope.placeVolume(layer_base_vol, transform).addPhysVolID("side", -1).addPhysVolID("module", i)
                              .addPhysVolID("layer", ring_num);
            auto neg_pv = envelope.placeVolume(layer_base_vol, neg_transform).addPhysVolID("side", 1).
                                   addPhysVolID("module", i).addPhysVolID("layer", ring_num);
            cloned_ring_piece_det.setPlacement(pv);
            cloned_neg_ring_piece_det.setPlacement(neg_pv);
            otk_endcaps.add(cloned_ring_piece_det);
            otk_endcaps.add(cloned_neg_ring_piece_det);
        }
    }
#ifdef DET_ELEMENT_DEBUG
    check_det_element(otk_endcaps);
#endif

    return otk_endcaps;
}

DECLARE_DETELEMENT(SiTracker_otkendcap_v02, create_element)

