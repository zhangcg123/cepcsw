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
    DetElement otkbarrel(name, det_id);

    Volume envelope = dd4hep::xml::createPlacedEnvelope(theDetector, e, otkbarrel);
    dd4hep::xml::setDetectorTypeFlag(e, otkbarrel) ;
    if(theDetector.buildType()==dd4hep::BUILD_ENVELOPE) return otkbarrel;
    envelope.setVisAttributes(theDetector.visAttributes("SeeThrough"));

    if (x_det.hasAttr(_U(sensitive))) {
        xml_dim_t sd_typ = x_det.child(_U(sensitive));
        sens.setType(sd_typ.typeStr());
    }
    else {
        sens.setType("tracker");
    }
    std::cout << " ** building SiTracker_otkbarrel ..." << std::endl ;

    dd4hep::rec::ZPlanarData* zPlanarData = new dd4hep::rec::ZPlanarData;
    //*****************************************************************//

    // Reading parameters from the .xml file

    //*****************************************************************//

    //fetch the display parameters
    xml_comp_t x_display(x_det.child(_Unicode(display)));
    std::string ladderVis      = x_display.attr<string>(_Unicode(ladder));
    std::string supportVis     = x_display.attr<string>(_Unicode(support));
    std::string flexVis        = x_display.attr<string>(_Unicode(flex));
    std::string sensEnvVis     = x_display.attr<string>(_Unicode(sens_env));
    std::string sensVis        = x_display.attr<string>(_Unicode(sens));
    std::string deadsensVis    = x_display.attr<string>(_Unicode(deadsensor));
    std::string pcbVis         = x_display.attr<string>(_Unicode(pcb));
    std::string asicVis        = x_display.attr<string>(_Unicode(asic));


    for(xml_coll_t layer_i(x_det,_U(layer)); layer_i; ++layer_i){
        //fetch the ladder overall parameters of this layer
        xml_comp_t x_layer(layer_i);

        dd4hep::PlacedVolume pv;
        int layer_id                 = x_layer.attr<int>(_Unicode(layer_id));

        std::cout << "layer_id: " << layer_id << " ......." << endl;

        double sensitive_radius      = x_layer.attr<double>(_Unicode(ladder_radius));
        int n_ladders                = x_layer.attr<int>(_Unicode(n_ladders)) ;
        double ladder_offset         = x_layer.attr<double>(_Unicode(ladder_offset));
        double ladder_radius         = sqrt(ladder_offset*ladder_offset + sensitive_radius*sensitive_radius);
        double ladder_phi0           = -atan(ladder_offset/sensitive_radius);
        double ladder_total_thickness         = x_layer.attr<double>(_Unicode(ladder_thickness));
        double ladder_total_width         = x_layer.attr<double>(_Unicode(ladder_width));
        double ladder_total_length         = x_layer.attr<double>(_Unicode(ladder_length));
        std::cout << "ladder_radius: " << ladder_radius/mm <<" mm" << endl;
        std::cout << "sensitive_radius: " << sensitive_radius/mm << " mm" << endl;
        std::cout << "ladder_thickness: " << ladder_total_thickness/mm << " mm" << endl;
        std::cout << "ladder_width: " << ladder_total_width/mm << " mm" << endl;
        std::cout << "ladder_length: " << ladder_total_length/mm << " mm" << endl;

        std::string layerName = dd4hep::_toString( layer_id , "layer_%d"  );
        dd4hep::Assembly layer_assembly( layerName ) ;
        pv = envelope.placeVolume( layer_assembly ) ;
        dd4hep::DetElement layerDE( otkbarrel , layerName  , x_det.id() );
        layerDE.setPlacement( pv ) ;

        const double ladder_dphi = ( dd4hep::twopi / n_ladders ) ;
        std::cout << "ladder_dphi: " << ladder_dphi << endl;


        //fetch the ladder parameters
        xml_comp_t x_ladder(x_layer.child(_Unicode(ladder)));

        //fetch the ladder support parameters
        xml_comp_t x_ladder_support(x_ladder.child(_Unicode(ladderSupport)));
        double support_height        = x_ladder_support.attr<double>(_Unicode(height));
        double support_length        = x_ladder_support.attr<double>(_Unicode(length));
        double support_thickness     = x_ladder_support.attr<double>(_Unicode(thickness));
        double support_width         = x_ladder_support.attr<double>(_Unicode(width));
        Material support_mat;
        if(x_ladder_support.hasAttr(_Unicode(mat)))
        {
            support_mat = theDetector.material(x_ladder_support.attr<string>(_Unicode(mat)));
        }
        else
        {
            support_mat = theDetector.material(x_ladder_support.materialStr());
        }
        std::cout << "support_length: " << support_length/mm << " mm" << endl;
        std::cout << "support_thickness: " << support_thickness/mm << " mm" << endl;
        std::cout << "support_width: " << support_width/mm << " mm" << endl;


        //fetch the flex parameters
        double flex_thickness(0);
        double flex_width(0);
        double flex_length(0);
        xml_comp_t x_flex(x_ladder.child(_Unicode(flex)));
        for(xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i){
            xml_comp_t x_flex_slice(flex_i);
            double x_flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
            double x_flex_slice_width = x_flex_slice.attr<double>(_Unicode(width));
            double x_flex_slice_length = x_flex_slice.attr<double>(_Unicode(length));
            flex_thickness += x_flex_slice_thickness;
            if (x_flex_slice_width > flex_width) flex_width = x_flex_slice_width;
            if (x_flex_slice_length > flex_length) flex_length = x_flex_slice_length;
            std::cout << "x_flex_slice_thickness: " << x_flex_slice_thickness/mm << " mm" << endl;
        }
        std::cout << "flex_thickness: " << flex_thickness/mm << " mm" << endl;
        std::cout << "flex_width: " << flex_width/mm << " mm" << endl;
        std::cout << "flex_length: " << flex_length/mm << " mm" << endl;


        //fetch the sensor parameters
        xml_comp_t x_sensor(x_ladder.child(_Unicode(sensor)));
        int n_sensors_per_side                  = x_sensor.attr<int>(_Unicode(n_sensors));
        double dead_gap                         = x_sensor.attr<double>(_Unicode(gap));
        double sensor_thickness                 = x_sensor.attr<double>(_Unicode(thickness));
        double sensor_length                    = x_sensor.attr<double>(_Unicode(length));
        double sensor_active_width              = x_sensor.attr<double>(_Unicode(active_width));
        double sensor_dead_width                = x_sensor.attr<double>(_Unicode(dead_width));
        double sensor_total_width               = sensor_active_width + sensor_dead_width;
        Material sensor_mat                     = theDetector.material(x_sensor.attr<string>(_Unicode(sensor_mat)));

        std::cout << "n_sensors_per_side: " << n_sensors_per_side << endl;
        std::cout << "dead_gap: " << dead_gap/mm << " mm" << endl;
        std::cout << "sensor_thickness: " << sensor_thickness/mm << " mm" << endl;
        std::cout << "sensor_length: " << sensor_length/mm << " mm" << endl;
        std::cout << "sensor_active_width: " << sensor_active_width/mm << " mm" << endl;
        std::cout << "sensor_dead_width: " << sensor_dead_width/mm << " mm" << endl;
        std::cout << "sensor_total_width: " << sensor_total_width/mm << " mm" << endl;

        //fetch parameters for the pcb and asic parts
        xml_comp_t x_other(x_ladder.child(_Unicode(other)));
        double pcb_thickness               = x_other.attr<double>(_Unicode(pcb_thickness));
        double pcb_width                   = x_other.attr<double>(_Unicode(pcb_width));
        double pcb_length                  = x_other.attr<double>(_Unicode(pcb_length));
        double pcb_zgap                    = x_other.attr<double>(_Unicode(pcb_zgap));
        double port_pcb_width              = x_other.attr<double>(_Unicode(port_pcb_width));
        double asic_thickness              = x_other.attr<double>(_Unicode(asic_thickness));
        double asic_width                  = x_other.attr<double>(_Unicode(asic_width));
        double asic_length                 = x_other.attr<double>(_Unicode(asic_length));
        double asic_zgap                   = x_other.attr<double>(_Unicode(asic_zgap));
        Material pcb_mat                   = theDetector.material(x_other.attr<string>(_Unicode(pcb_mat)));
        Material asic_mat                  = theDetector.material(x_other.attr<string>(_Unicode(asic_mat)));

        std::cout << "pcb_thickness: " << pcb_thickness/mm << " mm" << endl;
        std::cout << "pcb_width: " << pcb_width/mm << " mm" << endl;
        std::cout << "pcb_length: " << pcb_length/mm << " mm" << endl;
        std::cout << "pcb_zgap: " << pcb_zgap/mm << " mm" << endl;
        std::cout << "port_pcb_width: " << port_pcb_width/mm << " mm" << endl;
        std::cout << "asic_thickness: " << asic_thickness/mm << " mm" << endl;    std::cout << "asic_width: " << asic_width/mm << " mm" << endl;
        std::cout << "asic_length: " << asic_length/mm << " mm" << endl;
        std::cout << "asic_zgap: " << asic_zgap/mm << " mm" << endl;

        //*****************************************************************//

        // Creating objects

        //*****************************************************************//


        //create ladder logical volume
        Box LadderSolid(ladder_total_thickness / 2.0,
                        ladder_total_width / 2.0,
                        ladder_total_length / 2.0);
        Volume LadderLogical(name + dd4hep::_toString( layer_id, "_LadderLogical_%02d"),
                             LadderSolid, air);
        // create flex envelope logical volume
        Box FlexEnvelopeSolid(flex_thickness / 2.0, flex_width / 2.0, flex_length / 2.0);
        Volume FlexEnvelopeLogical(name + dd4hep::_toString( layer_id, "_FlexEnvelopeLogical_%02d"), FlexEnvelopeSolid, air);
        FlexEnvelopeLogical.setVisAttributes(theDetector.visAttributes("SeeThrough"));
        //otkbarrel.setVisAttributes(theDetector, flexVis, FlexEnvelopeLogical);

        //create the flex layers inside the flex envelope
        double flex_start_height(-flex_thickness/2.);
        int index = 0;
        for(xml_coll_t flex_i(x_flex,_U(slice)); flex_i; ++flex_i){
            xml_comp_t x_flex_slice(flex_i);
            double x_flex_slice_thickness = x_flex_slice.attr<double>(_Unicode(thickness));
            double x_flex_slice_width = x_flex_slice.attr<double>(_Unicode(width));
            double x_flex_slice_length = x_flex_slice.attr<double>(_Unicode(length));
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
            Box FlexLayerSolid(x_flex_slice_thickness/2.0, x_flex_slice_width/2.0, x_flex_slice_length/2.0);
            Volume FlexLayerLogical(name + dd4hep::_toString( layer_id, "_FlexLayerLogical_%02d") + dd4hep::_toString( index, "index_%02d"), FlexLayerSolid, x_flex_slice_mat);
            FlexLayerLogical.setVisAttributes(theDetector.visAttributes(flexVis));
            double flex_slice_height = flex_start_height + x_flex_slice_thickness/2.;
            pv = FlexEnvelopeLogical.placeVolume(FlexLayerLogical, Position(flex_slice_height, 0., 0.));
            std::cout << "flex thickness = " << x_flex_slice_thickness << std::endl;
            std::cout << "flex width = " << x_flex_slice_width << std::endl;
            std::cout << "flex length = " << x_flex_slice_length << std::endl;
            // std::cout << "flex material: " << x_flex_slice_mat << std::endl;
            flex_start_height += x_flex_slice_thickness;
            index++;
        }

        //place the flex envelope inside the ladder envelope
        pv = LadderLogical.placeVolume(FlexEnvelopeLogical, Position((support_height+flex_thickness)/2.0+sensor_thickness+pcb_thickness+asic_thickness, (-support_width+flex_width)/2.0, 0.));

        //create sensor envelope logical volume
        Box SensorEnvelopeSolid((sensor_thickness+pcb_thickness+asic_thickness) / 2.0, sensor_total_width / 2.0, support_length / 2.0);
        Volume SensorEnvelopeLogical(name + dd4hep::_toString( layer_id, "_SensorEnvelopeLogical_%02d"), SensorEnvelopeSolid, air);
        SensorEnvelopeLogical.setVisAttributes(theDetector.visAttributes(sensEnvVis));

        //create sensor logical volume
        Box SensorSolid(sensor_thickness / 2.0, sensor_active_width / 2.0, sensor_length / 2.0);
        Volume SensorLogical(name + dd4hep::_toString( layer_id, "_SensorLogical_%02d"), SensorSolid, sensor_mat);
        SensorLogical.setSensitiveDetector(sens);
        if (x_det.hasAttr(_U(limits))) SensorLogical.setLimitSet(theDetector, x_det.limitsStr());
        //otkbarrel.setVisAttributes(theDetector, deadsensVis, SensorDeadLogical);
        SensorLogical.setVisAttributes(theDetector.visAttributes(sensVis));

        //create dead sensor logical volume
        Box SensorDeadSolid(sensor_thickness / 2.0, sensor_dead_width / 2.0, sensor_length / 2.0);
        Volume SensorDeadLogical(name + dd4hep::_toString( layer_id, "_SensorDeadLogical_%02d"), SensorDeadSolid, sensor_mat);
        SensorDeadLogical.setVisAttributes(theDetector.visAttributes(deadsensVis));

        //create pcb sensor logical volume
        Box PcbSolid(pcb_thickness / 2.0, pcb_width / 2.0, pcb_length / 2.0);
        Volume PcbLogical(name + dd4hep::_toString( layer_id, "_PcbLogical_%02d"), PcbSolid, pcb_mat);
        PcbLogical.setVisAttributes(theDetector.visAttributes(pcbVis));

        Box PortPcbSolid(pcb_thickness / 2.0, port_pcb_width / 2.0, sensor_length / 2.0);
        Volume PortPcbLogical(name + dd4hep::_toString( layer_id, "_PortPcbLogical_%02d"), PortPcbSolid, pcb_mat);
        PortPcbLogical.setVisAttributes(theDetector.visAttributes(pcbVis));

        //create asic sensor logical volume
        Box AsicSolid(asic_thickness / 2.0, asic_width / 2.0, asic_length / 2.0);
        Volume AsicLogical(name + dd4hep::_toString( layer_id, "_AsicLogical_%02d"), AsicSolid, asic_mat);
        AsicLogical.setVisAttributes(theDetector.visAttributes(asicVis));

        // place the active sensor, dead sensor, pcb and asic inside the sensor envelope
        std::vector<dd4hep::PlacedVolume> Sensor_pv;
        for(int isensor=0;isensor<n_sensors_per_side;++isensor){
            double sensor_total_z = n_sensors_per_side*sensor_length + dead_gap*n_sensors_per_side;
            double sensor_xpos = -(pcb_thickness+asic_thickness)/2.0;
            double sensor_ypos_active = (sensor_total_width-sensor_active_width)/2.0;
            double sensor_ypos_dead = (sensor_total_width-sensor_active_width*2-sensor_dead_width)/ 2.0;
            double sensor_zpos = -sensor_total_z/2.0 + (sensor_length + dead_gap)/2.0 + isensor*(sensor_length + dead_gap);
            double pcb_xpos = (sensor_thickness-asic_thickness)/2.0;
            double pcb_ypos =  -(sensor_total_width-port_pcb_width*2-pcb_width)/2.0;
            double pcb_zpos_0 = sensor_zpos-(sensor_length/2.0-pcb_zgap-pcb_length/2.0);
            double pcb_zpos_1 = sensor_zpos+(sensor_length/2.0-pcb_zgap-pcb_length/2.0);
            double port_pcb_ypos = -(sensor_total_width-port_pcb_width)/2.0;
            double port_pcb_zpos = sensor_zpos;
            double asic_xpos = (sensor_thickness+pcb_thickness)/2.0;
            double asic_ypos = pcb_ypos;
            double asic_zpos_0 = sensor_zpos-(sensor_length/2.0-asic_zgap-asic_length/2.0);
            double asic_zpos_1 = sensor_zpos+(sensor_length/2.0-asic_zgap-asic_length/2.0);
            pv = SensorEnvelopeLogical.placeVolume(SensorLogical, Position(sensor_xpos,sensor_ypos_active,sensor_zpos));
            pv.addPhysVolID("layer", layer_id).addPhysVolID("active", 0).addPhysVolID("sensor", isensor) ;
            Sensor_pv.push_back(pv);
            pv = SensorEnvelopeLogical.placeVolume(SensorDeadLogical, Position(sensor_xpos,sensor_ypos_dead,sensor_zpos));
            pv = SensorEnvelopeLogical.placeVolume(PcbLogical, Position(pcb_xpos,pcb_ypos,pcb_zpos_0));
            pv = SensorEnvelopeLogical.placeVolume(PcbLogical, Position(pcb_xpos,pcb_ypos,pcb_zpos_1));
            pv = SensorEnvelopeLogical.placeVolume(PortPcbLogical, Position(pcb_xpos,port_pcb_ypos,port_pcb_zpos));
            pv = SensorEnvelopeLogical.placeVolume(AsicLogical, Position(asic_xpos,asic_ypos,asic_zpos_0));
            pv = SensorEnvelopeLogical.placeVolume(AsicLogical, Position(asic_xpos,asic_ypos,asic_zpos_1));
        }

        //place the sensor envelope inside the ladder envelope
        pv = LadderLogical.placeVolume(SensorEnvelopeLogical,
                                       Position(support_height/2.0+(sensor_thickness+pcb_thickness+asic_thickness)/2.0,0.,0.));

        //create the ladder support envelope
        Box LadderSupportEnvelopeSolid(support_height/2.0, support_width/2.0, support_length/2.0);
        Volume LadderSupportEnvelopeLogical(name + _toString( layer_id,"_SupEnvLogical_%02d"), LadderSupportEnvelopeSolid, air);
        otkbarrel.setVisAttributes(theDetector, "seeThrough", LadderSupportEnvelopeLogical);

        //create ladder support volume
        Box LadderSupportSolid(support_thickness / 2.0 , support_width / 2.0 , support_length / 2.0);
        Volume LadderSupportLogical(name + _toString( layer_id,"_SupLogical_%02d"), LadderSupportSolid, support_mat);
        LadderSupportLogical.setVisAttributes(theDetector.visAttributes(supportVis));

        pv = LadderSupportEnvelopeLogical.placeVolume(LadderSupportLogical);
        pv = LadderLogical.placeVolume(LadderSupportEnvelopeLogical);

	float rot = layer_id*0.5;
        for(int i = 0; i < n_ladders; i++){
            std::stringstream ladder_enum;
            ladder_enum << "otkbarrel_ladder_" << layer_id << "_" << i;
            DetElement ladderDE(layerDE, ladder_enum.str(), x_det.id());
            // std::cout << "start building " << ladder_enum.str() << ":" << endl;



            //====== create the meassurement surface ===================
            dd4hep::rec::Vector3D o(0,0,0);
            dd4hep::rec::Vector3D u( 0., 0., 1.);
            dd4hep::rec::Vector3D v( 0., 1., 0.);
            dd4hep::rec::Vector3D n( 1., 0., 0.);
            double inner_thick = sensor_thickness/2.0;
            double outer_thick = (support_height + sensor_thickness)/2.0;
            dd4hep::rec::VolPlane surfSens( SensorLogical ,
                                            dd4hep::rec::SurfaceType(dd4hep::rec::SurfaceType::Sensitive),
                                            inner_thick, outer_thick , u,v,n,o ) ;


            for(int isensor=0; isensor < n_sensors_per_side; ++isensor){
                std::stringstream sensor_str;
                sensor_str << ladder_enum.str() << "_" << isensor;
                // std::cout << "\tstart building " << sensor_str.str() << ":" << endl;
                DetElement sensorDE(ladderDE, sensor_str.str(), x_det.id());
                sensorDE.setPlacement(Sensor_pv[isensor]);
                volSurfaceList(sensorDE)->push_back(surfSens);
                // std::cout << "\t" << sensor_str.str() << " done." << endl;
            }
            Transform3D tr (RotationZYX(ladder_dphi*(i+rot)+dd4hep::twopi/2,0.,0.),Position(ladder_radius*cos(ladder_phi0+ladder_dphi*(i+rot)), ladder_radius*sin(ladder_phi0+ladder_dphi*(i+rot)), 0.));
            pv = layer_assembly.placeVolume(LadderLogical,tr);
            pv.addPhysVolID("module", i ) ;
            ladderDE.setPlacement(pv);
            //std::cout << ladder_enum.str() << " done." << endl;
            if(i==0) std::cout << "xy=" << ladder_radius*cos(ladder_phi0) << " " << ladder_radius*sin(ladder_phi0) << std::endl;
        }

        // package the reconstruction data
        dd4hep::rec::ZPlanarData::LayerLayout otkbarrelLayer;

        otkbarrelLayer.ladderNumber         = n_ladders;
        otkbarrelLayer.phi0                 = ladder_phi0 + ladder_dphi*rot;
        otkbarrelLayer.sensorsPerLadder     = n_sensors_per_side;
        otkbarrelLayer.lengthSensor         = sensor_length + dead_gap; // dead region also has silicon material
        otkbarrelLayer.distanceSupport      = ladder_radius*cos(ladder_phi0) - support_thickness/2.0; //sensitive_radius;
        otkbarrelLayer.thicknessSupport     = support_thickness / 2.0;
        otkbarrelLayer.offsetSupport        = -ladder_offset;
        otkbarrelLayer.widthSupport         = support_width;
        otkbarrelLayer.zHalfSupport         = support_length / 2.0;
        otkbarrelLayer.distanceSensitive    = ladder_radius*cos(ladder_phi0) - support_thickness/2.0
                                            - sensor_thickness; //sensitive_radius + support_height / 2.0 + flex_thickness;
        otkbarrelLayer.thicknessSensitive   = sensor_thickness;
        otkbarrelLayer.offsetSensitive      = -ladder_offset + (support_width/2.0 - sensor_total_width/2.0);
        otkbarrelLayer.widthSensitive       = sensor_total_width;
        //otkbarrelLayer.zHalfSensitive       = (n_sensors_per_side*(sensor_length + dead_gap) - dead_gap) / 2.0;
	otkbarrelLayer.zHalfSensitive       = n_sensors_per_side*(sensor_length + dead_gap) / 2.0; // add dead_gap to same sensor, little effect?

        zPlanarData->layers.push_back(otkbarrelLayer);
    }
    std::cout << (*zPlanarData) << endl;
    otkbarrel.addExtension< ZPlanarData >(zPlanarData);
    if ( x_det.hasAttr(_U(combineHits)) ) {
        otkbarrel.setCombineHits(x_det.attr<bool>(_U(combineHits)),sens);
    }
    std::cout << "otkbarrel done." << endl;
    return otkbarrel;
}
DECLARE_DETELEMENT(SiTracker_otkbarrel_v01,create_element)

