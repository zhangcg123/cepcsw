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


static dd4hep::Ref_t create_element(dd4hep::Detector& theDetector, xml_h e, dd4hep::SensitiveDetector sens)  {

    xml_det_t  x_det    = e;
    Material   air      = theDetector.air();
    int        det_id   = x_det.id();
    string     name     = x_det.nameStr();
    DetElement otkendcap(name, det_id);

    Volume envelope = dd4hep::xml::createPlacedEnvelope(theDetector, e, otkendcap);
    dd4hep::xml::setDetectorTypeFlag(e, otkendcap) ;
    if(theDetector.buildType()==dd4hep::BUILD_ENVELOPE) return otkendcap;
    envelope.setVisAttributes(theDetector.visAttributes("SeeThrough"));

    if (x_det.hasAttr(_U(sensitive))) {
        xml_dim_t sd_typ = x_det.child(_U(sensitive));
        sens.setType(sd_typ.typeStr());
    }
    else {
        sens.setType("tracker");
    }
    std::cout << " ** building SiTracker_otkendcap ..." << std::endl ;

    dd4hep::rec::ZPlanarData* zPlanarData = new dd4hep::rec::ZPlanarData;
    //*****************************************************************//

    // Reading parameters from the .xml file

    //*****************************************************************//

    //fetch the geometry parameters
    const double inner_radius   = theDetector.constant<double>("OTKEndCapBarrel_inner_radius");
    const double outer_radius   = theDetector.constant<double>("OTKEndCap_outer_radius");
    const double total_length   = theDetector.constant<double>("OTKEndCap_half_length")*2.;
    const int total_sections    = theDetector.constant<int>("OTKEndCap_total_sections");
    const double deg            = theDetector.constant<double>("OTKEndCap_piece_deg")*dd4hep::degree;
    const double piece_number   = theDetector.constant<double>("OTKEndCap_piece_num");
    const double deg_interval   = 360  / piece_number * dd4hep::degree;
    const double r0             = theDetector.constant<double>("OTKEndCap_r0");
    const double r1             = theDetector.constant<double>("OTKEndCap_r1");
    const double r2             = theDetector.constant<double>("OTKEndCap_r2");
    const double r3             = theDetector.constant<double>("OTKEndCap_r3");
    const double r4             = theDetector.constant<double>("OTKEndCap_r4");
    const double r5             = theDetector.constant<double>("OTKEndCap_r5");
    const double r6             = theDetector.constant<double>("OTKEndCap_r6");
    const double r7             = theDetector.constant<double>("OTKEndCap_r7");
    const double r8             = theDetector.constant<double>("OTKEndCap_r8");
    const double r9             = theDetector.constant<double>("OTKEndCap_r9");
    const double r10            = theDetector.constant<double>("OTKEndCap_r10");
    const int asic_num_0        = theDetector.constant<int>("OTKEndCap_asic_num_0");
    const int asic_num_1        = theDetector.constant<int>("OTKEndCap_asic_num_1");
    const int asic_num_2        = theDetector.constant<int>("OTKEndCap_asic_num_2");
    const int asic_num_3        = theDetector.constant<int>("OTKEndCap_asic_num_3");
    const int asic_num_4        = theDetector.constant<int>("OTKEndCap_asic_num_4");
    const int asic_num_5        = theDetector.constant<int>("OTKEndCap_asic_num_5");
    const int asic_num_6        = theDetector.constant<int>("OTKEndCap_asic_num_6");
    const int asic_num_7        = theDetector.constant<int>("OTKEndCap_asic_num_7");
    const int asic_num_8        = theDetector.constant<int>("OTKEndCap_asic_num_8");
    const int asic_num_9        = theDetector.constant<int>("OTKEndCap_asic_num_9");
    const int module_num_0      = theDetector.constant<int>("OTKEndCap_module_num_0");
    const int module_num_1      = theDetector.constant<int>("OTKEndCap_module_num_1");
    const int module_num_2      = theDetector.constant<int>("OTKEndCap_module_num_2");
    const int module_num_3      = theDetector.constant<int>("OTKEndCap_module_num_3");
    const int module_num_4      = theDetector.constant<int>("OTKEndCap_module_num_4");
    const int module_num_5      = theDetector.constant<int>("OTKEndCap_module_num_5");
    const int module_num_6      = theDetector.constant<int>("OTKEndCap_module_num_6");
    const int module_num_7      = theDetector.constant<int>("OTKEndCap_module_num_7");
    const int module_num_8      = theDetector.constant<int>("OTKEndCap_module_num_8");
    const int module_num_9      = theDetector.constant<int>("OTKEndCap_module_num_9");

    std::cout << "inner_radius: "   << inner_radius/mm      << "mm"     << std::endl;
    std::cout << "outer_radius: "   << outer_radius/mm      << "mm"     << std::endl;
    std::cout << "total_length: "   << total_length/mm      << "mm"     << std::endl;
    std::cout << "total_sections: " << total_sections                   << std::endl;
    std::cout << "deg: "            << deg/dd4hep::degree   << "deg"    << std::endl;
    std::cout << "piece_number: "   << piece_number                     << std::endl;
    std::cout << "deg interval: "   << deg_interval/dd4hep::degree  << "deg"    << std::endl;
    std::cout << "r0: "             << r0/mm                << "mm"     << std::endl;
    std::cout << "r1: "             << r1/mm                << "mm"     << std::endl;
    std::cout << "r2: "             << r2/mm                << "mm"     << std::endl;
    std::cout << "r3: "             << r3/mm                << "mm"     << std::endl;
    std::cout << "r4: "             << r4/mm                << "mm"     << std::endl;
    std::cout << "r5: "             << r5/mm                << "mm"     << std::endl;
    std::cout << "r6: "             << r6/mm                << "mm"     << std::endl;
    std::cout << "r7: "             << r7/mm                << "mm"     << std::endl;
    std::cout << "r8: "             << r8/mm                << "mm"     << std::endl;
    std::cout << "r9: "             << r9/mm                << "mm"     << std::endl;
    std::cout << "r10: "            << r10/mm               << "mm"     << std::endl;
    std::cout << "asic_num_0: "     << asic_num_0                       << std::endl;
    std::cout << "asic_num_1: "     << asic_num_1                       << std::endl;
    std::cout << "asic_num_2: "     << asic_num_2                       << std::endl;
    std::cout << "asic_num_3: "     << asic_num_3                       << std::endl;
    std::cout << "asic_num_4: "     << asic_num_4                       << std::endl;
    std::cout << "asic_num_5: "     << asic_num_5                       << std::endl;
    std::cout << "asic_num_6: "     << asic_num_6                       << std::endl;
    std::cout << "asic_num_7: "     << asic_num_7                       << std::endl;
    std::cout << "asic_num_8: "     << asic_num_8                       << std::endl;
    std::cout << "asic_num_9: "     << asic_num_9                       << std::endl;
    std::cout << "module_num_0: "   << module_num_0                     << std::endl;
    std::cout << "module_num_1: "   << module_num_1                     << std::endl;
    std::cout << "module_num_2: "   << module_num_2                     << std::endl;
    std::cout << "module_num_3: "   << module_num_3                     << std::endl;
    std::cout << "module_num_4: "   << module_num_4                     << std::endl;
    std::cout << "module_num_5: "   << module_num_5                     << std::endl;
    std::cout << "module_num_6: "   << module_num_6                     << std::endl;
    std::cout << "module_num_7: "   << module_num_7                     << std::endl;
    std::cout << "module_num_8: "   << module_num_8                     << std::endl;
    std::cout << "module_num_9: "   << module_num_9                     << std::endl;

    double r[11]        = {r0,
                           r1,
                           r2,
                           r3,
                           r4,
                           r5,
                           r6,
                           r7,
                           r8,
                           r9,
                           r10};
    int asic_num[10]    = {asic_num_0,
                           asic_num_1,
                           asic_num_2,
                           asic_num_3,
                           asic_num_4,
                           asic_num_5,
                           asic_num_6,
                           asic_num_7,
                           asic_num_8,
                           asic_num_9};
    int module_num[10]  = {module_num_0,
                           module_num_1,
                           module_num_2,
                           module_num_3,
                           module_num_4,
                           module_num_5,
                           module_num_6,
                           module_num_7,
                           module_num_8,
                           module_num_9};

    //fetch the display parameters
    xml_comp_t x_display(x_det.child(_Unicode(display)));
    std::string supportVis          = x_display.attr<string>(_Unicode(support));
    std::string sensEnvVis          = x_display.attr<string>(_Unicode(sens_env));
    std::string sensVis             = x_display.attr<string>(_Unicode(sens));
    std::string deadsensVis         = x_display.attr<string>(_Unicode(deadsensor));
    std::string pcbVis              = x_display.attr<string>(_Unicode(pcb));
    std::string asicVis             = x_display.attr<string>(_Unicode(asic));

    //fetch the support parameters
    xml_comp_t x_support(x_det.child(_Unicode(support)));
    double support_thickness        = x_support.attr<double>(_Unicode(thickness));
    double support_inner_radius     = x_support.attr<double>(_Unicode(inner_radius));
    double support_outer_radius     = x_support.attr<double>(_Unicode(outer_radius));
    Material support_mat            = theDetector.material(x_support.attr<string>(_Unicode(mat)));
    std::cout << "support_thickness: "      << support_thickness/mm     << " mm" << std::endl;
    std::cout << "support_inner_radius: "   << support_inner_radius/mm  << " mm" << std::endl;
    std::cout << "support_outer_radius: "   << support_outer_radius/mm  << " mm" << std::endl;

    for(xml_coll_t layer_i(x_det,_U(layer));layer_i;++layer_i){
        //fetch the overall parameters of this layer
        xml_comp_t x_layer(layer_i);
        dd4hep::PlacedVolume pv;
        int layer_id                = x_layer.attr<int>(_Unicode(id));
        double layer_thickness      = x_layer.attr<double>(_Unicode(thickness));
        double layer_zpos           = x_layer.attr<double>(_Unicode(zpos));
        std::cout << "layer_id: "           << layer_id                         << std::endl;
        std::cout << "layer_thickness: "    << layer_thickness/mm       << "mm" << std::endl;
        std::cout << "layer_zpos: "         << layer_zpos/mm            << "mm" << std::endl;

        //fetch the sensor parameters
        xml_comp_t x_sensor(x_layer.child(_Unicode(sensor)));
        double sensor_dead_gap      = x_sensor.attr<double>(_Unicode(gap));
        double sensor_thickness     = x_sensor.attr<double>(_Unicode(thickness));
        double sensor_dead_width    = x_sensor.attr<double>(_Unicode(dead_width));
        Material sensor_mat         = theDetector.material(x_sensor.attr<string>(_Unicode(mat)));

        //fetch parameters for the pcb and asic parts
        xml_comp_t x_pcb(x_layer.child(_Unicode(pcb)));
        double pcb_thickness        = x_pcb.attr<double>(_Unicode(thickness));
        double pcb_rlength          = x_pcb.attr<double>(_Unicode(rlength));
        double pcb_rgap             = x_pcb.attr<double>(_Unicode(rgap));
        Material pcb_mat            = theDetector.material(x_pcb.attr<string>(_Unicode(mat)));

        xml_comp_t x_asic(x_layer.child(_Unicode(asic)));
        double asic_thickness       = x_asic.attr<double>(_Unicode(thickness));
        double asic_width           = x_asic.attr<double>(_Unicode(width));
        double asic_rlength         = x_asic.attr<double>(_Unicode(rlength));
        double asic_rgap            = x_asic.attr<double>(_Unicode(rgap));
        Material asic_mat           = theDetector.material(x_asic.attr<string>(_Unicode(mat)));

        std::cout << "sensor_dead_gap: "    << sensor_dead_gap/mm       << " mm" << std::endl;
        std::cout << "sensor_thickness: "   << sensor_thickness/mm      << " mm" << std::endl;
        std::cout << "sensor_dead_width: "  << sensor_dead_width/mm     << " mm" << std::endl;
        std::cout << "pcb_thickness: "      << pcb_thickness/mm         << " mm" << std::endl;
        std::cout << "pcb_rlength: "        << pcb_rlength/mm           << " mm" << std::endl;
        std::cout << "pcb_rgap: "           << pcb_rgap/mm              << " mm" << std::endl;
        std::cout << "asic_thickness: "     << asic_thickness/mm        << " mm" << std::endl;
        std::cout << "asic_width: "         << asic_width/mm            << " mm" << std::endl;
        std::cout << "asic_rlength: "       << asic_rlength/mm          << " mm" << std::endl;
        std::cout << "asic_rgap: "          << asic_rgap/mm             << " mm" << std::endl;

        //*****************************************************************//

        // Creating objects

        //*****************************************************************//

        //layer definition
        std::string layer_name = dd4hep::_toString(layer_id, "layer_%d");
        dd4hep::Assembly layer_assembly(layer_name);
        pv = envelope.placeVolume(layer_assembly);
        dd4hep::DetElement layerDE(otkendcap, layer_name, x_det.id());
        layerDE.setPlacement(pv);

        //create piece envelope logical volume
        dd4hep::Tube PieceEnvSolid(         inner_radius,
                                            outer_radius,
                                            layer_thickness / 2.0,
                                            0.,
                                            deg);
        Volume PieceEnvLogical(             name + dd4hep::_toString(layer_id, "_PieceEnvLogical_%02d"),
                                            PieceEnvSolid,
                                            air);

        //create and place support logical volume
        dd4hep::Tube SupportSolid(          support_inner_radius,
                                            support_outer_radius,
                                            support_thickness / 2.0,
                                            0.,
                                            deg);
        Volume SupportLogical(              name + dd4hep::_toString(layer_id, "_SupportLogical_%02d"),
                                            SupportSolid,
                                            support_mat);

        SupportLogical.setVisAttributes(theDetector.visAttributes(supportVis));
        pv = PieceEnvLogical.placeVolume(SupportLogical, Position(0, 0, (-layer_thickness + support_thickness) / 2.0));

        //create sensor envelope logical volume
        dd4hep::Tube SensorEnvSolid(        inner_radius,
                                            outer_radius,
                                            (sensor_thickness + pcb_thickness + asic_thickness) / 2.0,
                                            0.,
                                            deg);
        Volume SensorEnvLogical(            name + dd4hep::_toString(layer_id, "SensorEnvLogical_%02d"),
                                            SensorEnvSolid,
                                            air);
        SensorEnvLogical.setVisAttributes(theDetector.visAttributes(sensEnvVis));

        //create and place utilities in each ring
        std::vector<dd4hep::PlacedVolume> sensor_pv;
        std::vector<dd4hep::rec::VolPlane> sensor_surf;
        dd4hep::rec::Vector3D o(0., 0., 0.);
        dd4hep::rec::Vector3D u(0., 1., 0.);
        dd4hep::rec::Vector3D v(1., 0., 0.);
        dd4hep::rec::Vector3D n(0., 0., 1.);
        double inner_thick = sensor_thickness/2.0;
        double outer_thick = (support_thickness + sensor_thickness)/2.0;
        double ring_inner_radius;
        double ring_outer_radius        = r[0] - sensor_dead_gap;
        int    ring_asic_number;
        int    ring_module_number;
        double ring_dead_angle;
        double ring_active_angle;
        double ring_active_module_angle;
        double ring_asic_angle;
        double ring_asic_interval;
        double asic_mid_angle;
        for(int i=0;i<10;i++){
            ring_inner_radius               = ring_outer_radius + sensor_dead_gap * 2;
            ring_outer_radius               = r[i+1] - sensor_dead_gap;
            ring_asic_number                = asic_num[i];
            ring_module_number              = module_num[i];
            ring_dead_angle                 = sensor_dead_width / ring_inner_radius * 360 * dd4hep::degree;
            ring_active_angle               = deg - ring_dead_angle * 2 * ring_module_number;
            ring_active_module_angle        = ring_active_angle / ring_module_number;
            ring_asic_angle                 = asic_width / ring_outer_radius;
            ring_asic_interval              = deg / ring_asic_number;
            //create and place sensor logical volume
            dd4hep::Tube SensorSolid(       ring_inner_radius,
                                            ring_outer_radius,
                                            sensor_thickness/2.0,
                                            ring_dead_angle,
                                            ring_dead_angle + ring_active_module_angle);
            Volume SensorLogical(           name + dd4hep::_toString(layer_id, "_SensorLogical_%02d_") + std::to_string(i),
                                            SensorSolid,
                                            sensor_mat);
            dd4hep::Tube DeadSensorSolidA(  ring_inner_radius,
                                            ring_outer_radius,
                                            sensor_thickness/2.0,
                                            0.,
                                            ring_dead_angle);
            Volume DeadSensorLogicalA(      name + dd4hep::_toString(layer_id, "_DeadSensorLogicalA_%02d_") + std::to_string(i),
                                            DeadSensorSolidA,
                                            sensor_mat);
            dd4hep::Tube DeadSensorSolidB(  ring_inner_radius,
                                            ring_outer_radius,
                                            sensor_thickness/2.0,
                                            ring_dead_angle + ring_active_module_angle,
                                            ring_dead_angle*2 + ring_active_module_angle);
            Volume DeadSensorLogicalB(      name + dd4hep::_toString(layer_id, "_DeadSensorLogicalB_%02d_") + std::to_string(i),
                                            DeadSensorSolidB,
                                            sensor_mat);

            SensorLogical.setSensitiveDetector(sens);
            SensorLogical.setVisAttributes(theDetector.visAttributes(sensVis));
            if (x_det.hasAttr(_U(limits))) SensorLogical.setLimitSet(theDetector, x_det.limitsStr());
            dd4hep::rec::VolPlane surfsens( SensorLogical,
                                            dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
                                            inner_thick,
                                            outer_thick,
                                            u,v,n,o);
            sensor_surf.push_back(surfsens);
            DeadSensorLogicalA.setVisAttributes(theDetector.visAttributes(deadsensVis));
            DeadSensorLogicalB.setVisAttributes(theDetector.visAttributes(deadsensVis));
            pv = SensorEnvLogical.placeVolume(SensorLogical,        Position(0, 0, -(pcb_thickness + asic_thickness) / 2.0));
            pv.addPhysVolID("layer", layer_id).addPhysVolID("active", 0).addPhysVolID("sensor", i);
            sensor_pv.push_back(pv);
            pv = SensorEnvLogical.placeVolume(DeadSensorLogicalA,   Position(0, 0, -(pcb_thickness + asic_thickness) / 2.0));
            pv = SensorEnvLogical.placeVolume(DeadSensorLogicalB,   Position(0, 0, -(pcb_thickness + asic_thickness) / 2.0));

            if(ring_module_number==2){
                dd4hep::Tube SensorSolid(   ring_inner_radius,
                                            ring_outer_radius,
                                            sensor_thickness/2.0,
                                            ring_dead_angle*3 + ring_active_module_angle,
                                            ring_dead_angle*3 + ring_active_module_angle*2);
                Volume SensorLogical(       name + dd4hep::_toString(layer_id, "_SensorLogical_%02d_2_") + std::to_string(i),
                                            SensorSolid,
                                            sensor_mat);
                dd4hep::Tube DeadSensorSolidA(ring_inner_radius,
                                              ring_outer_radius,
                                              sensor_thickness/2.0,
                                              ring_dead_angle*2 + ring_active_module_angle,
                                              ring_dead_angle*3 + ring_active_module_angle);
                Volume DeadSensorLogicalA(  name + dd4hep::_toString(layer_id, "_DeadSensorLogicalA_%02d_2_") + std::to_string(i),
                                            DeadSensorSolidA,
                                            sensor_mat);
                dd4hep::Tube DeadSensorSolidB(ring_inner_radius,
                                              ring_outer_radius,
                                              sensor_thickness/2.0,
                                              ring_dead_angle*3 + ring_active_module_angle*2,
                                              ring_dead_angle*4 + ring_active_module_angle*2);
                Volume DeadSensorLogicalB(  name + dd4hep::_toString(layer_id, "_DeadSensorLogicalB_%02d_2_") + std::to_string(i),
                                            DeadSensorSolidB,
                                            sensor_mat);

                SensorLogical.setSensitiveDetector(sens);
                SensorLogical.setVisAttributes(theDetector.visAttributes(sensVis));
                if (x_det.hasAttr(_U(limits))) SensorLogical.setLimitSet(theDetector, x_det.limitsStr());
                dd4hep::rec::VolPlane surfsens( SensorLogical ,
                                                dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
                                                inner_thick,
                                                outer_thick,
                                                u,v,n,o);
                sensor_surf.push_back(surfsens);
                DeadSensorLogicalA.setVisAttributes(theDetector.visAttributes(sensVis));
                DeadSensorLogicalB.setVisAttributes(theDetector.visAttributes(sensVis));
                pv = SensorEnvLogical.placeVolume(SensorLogical,        Position(0, 0, -(pcb_thickness + asic_thickness) / 2.0));
                pv.addPhysVolID("layer", layer_id).addPhysVolID("active", 0).addPhysVolID("sensor", i + 10);
                sensor_pv.push_back(pv);
                pv = SensorEnvLogical.placeVolume(DeadSensorLogicalA,   Position(0, 0, -(pcb_thickness + asic_thickness) / 2.0));
                pv = SensorEnvLogical.placeVolume(DeadSensorLogicalB,   Position(0, 0, -(pcb_thickness + asic_thickness) / 2.0));
            }

            //create and place pcb logical volume
            dd4hep::Tube PcbSolidA(         ring_inner_radius + pcb_rgap,
                                            ring_inner_radius + pcb_rgap + pcb_rlength,
                                            pcb_thickness / 2.0,
                                            0.,
                                            deg);
            Volume PcbLogicalA(             name + dd4hep::_toString(layer_id, "_PcbLogicalA_%02d_") + std::to_string(i),
                                            PcbSolidA,
                                            pcb_mat);
            dd4hep::Tube PcbSolidB(         ring_outer_radius - pcb_rgap - pcb_rlength,
                                            ring_outer_radius - pcb_rgap,
                                            pcb_thickness / 2.0,
                                            0.,
                                            deg);
            Volume PcbLogicalB(             name + dd4hep::_toString(layer_id, "_PcbLogicalB_%02d_") + std::to_string(i),
                                            PcbSolidB,
                                            pcb_mat);

            PcbLogicalA.setVisAttributes(theDetector.visAttributes(pcbVis));
            PcbLogicalB.setVisAttributes(theDetector.visAttributes(pcbVis));
            pv = SensorEnvLogical.placeVolume(PcbLogicalA, Position(0, 0, (sensor_thickness - asic_thickness) / 2.0));
            pv = SensorEnvLogical.placeVolume(PcbLogicalB, Position(0, 0, (sensor_thickness - asic_thickness) / 2.0));

            //create and place asic logical volume
            for(int j=0;j<ring_asic_number;j++){
                asic_mid_angle              = ring_asic_interval * (j + 0.5);
                dd4hep::Tube AsicSolidA(    ring_inner_radius + asic_rgap,
                                            ring_inner_radius + asic_rgap + asic_rlength,
                                            asic_thickness / 2.0,
                                            asic_mid_angle - ring_asic_angle / 2.0,
                                            asic_mid_angle + ring_asic_angle / 2.0);
                Volume AsicLogicalA(        name + dd4hep::_toString(layer_id, "_AsicLogicalA_%02d_") + std::to_string(i) + std::to_string(j),
                                            AsicSolidA,
                                            asic_mat);
                dd4hep::Tube AsicSolidB(    ring_outer_radius - asic_rgap - asic_rlength,
                                            ring_outer_radius - asic_rgap,
                                            asic_thickness / 2.0,
                                            asic_mid_angle - ring_asic_angle / 2.0,
                                            asic_mid_angle + ring_asic_angle / 2.0);
                Volume AsicLogicalB(        name + dd4hep::_toString(layer_id, "_AsicLogicalB_%02d_") + std::to_string(i) + std::to_string(j),
                                            AsicSolidB,
                                            asic_mat);

                AsicLogicalA.setVisAttributes(theDetector.visAttributes(asicVis));
                AsicLogicalB.setVisAttributes(theDetector.visAttributes(asicVis));
                pv = SensorEnvLogical.placeVolume(AsicLogicalA, Position(0, 0, (sensor_thickness + pcb_thickness) / 2.0));
                pv = SensorEnvLogical.placeVolume(AsicLogicalB, Position(0, 0, (sensor_thickness + pcb_thickness) / 2.0));
            }

        }
        double zpos = (-layer_thickness + support_thickness*2 + sensor_thickness + pcb_thickness + asic_thickness) / 2.0;
        pv = PieceEnvLogical.placeVolume(SensorEnvLogical, Position(0, 0, zpos));

        //*****************************************************************//

        // Assembling

        //*****************************************************************//

        for(int i=0;i<piece_number; i++){
            float rot = layer_id*0.5;
            std::stringstream piece_enum;
            piece_enum << "otkendcap_piece_" << layer_id << "_" << i;
            DetElement pieceDE(layerDE, piece_enum.str(), x_det.id());

            //create the meassurement surface
            // int sensor_num = std::accumulate(module_num_v.begin(), module_num_v.end(), 0);
            int sensor_num = 15;
            for(int isensor=0;isensor<sensor_num;++isensor){
                std::stringstream sensor_str;
                sensor_str << piece_enum.str() << "_" << isensor;
                DetElement sensorDE(pieceDE, sensor_str.str(), x_det.id());
                sensorDE.setPlacement(sensor_pv[isensor]);
                volSurfaceList(sensorDE)->push_back(sensor_surf[isensor]);
            }
            Transform3D trA (               RotationZYX(deg_interval*(i+rot),
                                                        180*dd4hep::degree,
                                                        0.),
                                            Position(0.,
                                                     0.,
                                                     layer_zpos));
            Transform3D trB (               RotationZYX(deg_interval*(i+rot),
                                                        0.,
                                                        0.),
                                            Position(0.,
                                                     0.,
                                                     -layer_zpos));
            pv = layer_assembly.placeVolume(PieceEnvLogical,trA);
            pv.addPhysVolID("module", i*2);
            pieceDE.setPlacement(pv);
            pv = layer_assembly.placeVolume(PieceEnvLogical,trB);
            pv.addPhysVolID("module", i*2+1);
            pieceDE.setPlacement(pv);
            std::cout << piece_enum.str() << " done." << std::endl;
        }

        // package the reconstruction data
        dd4hep::rec::ZPlanarData::LayerLayout otkendcapLayer;

        otkendcapLayer.ladderNumber         = piece_number;
        otkendcapLayer.phi0                 = 0.;
        otkendcapLayer.sensorsPerLadder     = 15;
        otkendcapLayer.lengthSensor         = r1-r0-sensor_dead_gap*2;
        otkendcapLayer.distanceSupport      = support_thickness/2.0;
        otkendcapLayer.thicknessSupport     = support_thickness/2.0;
        otkendcapLayer.offsetSupport        = 0.;
        otkendcapLayer.widthSupport         = support_inner_radius;
        otkendcapLayer.zHalfSupport         = support_outer_radius;
        otkendcapLayer.distanceSensitive    = support_thickness;
        otkendcapLayer.thicknessSensitive   = sensor_thickness;
        otkendcapLayer.offsetSensitive      = 0.;
        otkendcapLayer.widthSensitive       = 0.;
        otkendcapLayer.zHalfSensitive       = 0.;

        zPlanarData->layers.push_back(otkendcapLayer);
    }
    std::cout << (*zPlanarData) << endl;
    otkendcap.addExtension< ZPlanarData >(zPlanarData);
    if ( x_det.hasAttr(_U(combineHits)) ) {
        otkendcap.setCombineHits(x_det.attr<bool>(_U(combineHits)),sens);
    }
    std::cout << "otkendcap done." << std::endl;
    return otkendcap;
}
DECLARE_DETELEMENT(SiTracker_otkendcap_v01,create_element)

