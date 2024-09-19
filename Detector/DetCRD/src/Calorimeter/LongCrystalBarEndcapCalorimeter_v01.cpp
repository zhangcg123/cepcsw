//==========================================================================
// LongCrystalBarEndcapCalorimeter_v01 implementation 
//--------------------------------------------------------------------------
// Author: Song Weizheng, IHEP
//--------------------------------------------------------------------------
// Data: 2024.6
//==========================================================================

#include "DD4hep/DetFactoryHelper.h" 
#include "XML/Layering.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/Segmentation.h"

#define MYDEBUG(x) std::cout << __FILE__ << ":" << __LINE__ << ": " << x << std::endl;
#define MYDEBUGVAL(x) std::cout << __FILE__ << ":" << __LINE__ << ": " << #x << ": " << x << std::endl;

using dd4hep::rec::LayeredCalorimeterData;
using namespace dd4hep;
using namespace std;

static dd4hep::Ref_t create_detector(dd4hep::Detector& theDetector,
                                     xml_h e,
                                     dd4hep::SensitiveDetector sens) {

    xml_det_t x_det = e;
    std::string det_name = x_det.nameStr();
    std::string det_type = x_det.typeStr();
    MYDEBUGVAL(det_name);
    MYDEBUGVAL(det_type);
    int detid = x_det.id();

    // ######################################
    // ### detector description parameter ###
    // ######################################

    double radius_inner = theDetector.constant<double>("ecalendcap_inner_radius");
    double radius_outer = theDetector.constant<double>("ecalendcap_outer_radius");
    double z_min = theDetector.constant<double>("ecalendcap_zmin");
    double z_depth = theDetector.constant<double>("ecalendcap_depth");

    double x_width = theDetector.constant<double>("ecalendcap_x_width");
    double y_width = theDetector.constant<double>("ecalendcap_y_width");
    int Nlayers = theDetector.constant<double>("ecalendcap_layer");

    double width_crystal = theDetector.constant<double>("ecalendcap_width_crystal");
    double crystal_wrapping = theDetector.constant<double>("ecalendcap_crystal_wrapping");
    double photoelectronic = theDetector.constant<double>("ecalendcap_length_photoelectronic");
    double photoelectronic_width = theDetector.constant<double>("ecalendcap_width_photoelectronic");

    double carbon = theDetector.constant<double>("ecalendcap_length_carbon");
    double cable = theDetector.constant<double>("ecalendcap_length_cable");
    double pcb = theDetector.constant<double>("ecalendcap_length_pcb");
    double asic = theDetector.constant<double>("ecalendcap_length_asic");
    double cooling = theDetector.constant<double>("ecalendcap_length_cooling");
    double back_plate = theDetector.constant<double>("ecalendcap_length_back");
    
    // ###########################
    // ### general measurement ###
    // ###########################

    double length_crystal = x_width - 2*carbon - 2*cable - 2*pcb - 2*asic - 2*cooling; // length of crystal wrap sipm
    int Nbar = floor(length_crystal/width_crystal); // number of crystal

    std::cout << "length_crystal" << " : " << length_crystal << std::endl;
    std::cout << "Nbar" << " : " << Nbar << std::endl;

    // // ####################
    // // ### World Volume ###
    // // ####################
    dd4hep::Material air(theDetector.material("Air"));
    dd4hep::Material vacuum(theDetector.material("Vacuum"));
    dd4hep::Material mat_BGO(theDetector.material("G4_BGO")); 
    dd4hep::Material mat_CF(theDetector.material("CarbonFiber"));
    dd4hep::Material mat_Cu(theDetector.material("G4_Cu"));
    dd4hep::Material mat_ESR(theDetector.material("G4_ESR"));
    dd4hep::Material mat_Si(theDetector.material("G4_Si"));
    dd4hep::Material mat_PCB(theDetector.material("PCB"));
    
    dd4hep::DetElement ECAL(det_name, detid);
    dd4hep::Volume motherVol = theDetector.pickMotherVolume(ECAL);

    dd4hep::Tube tube_shape(0, radius_outer, z_depth/2);
    dd4hep::Box box_shape(x_width, y_width, z_depth);
    dd4hep::SubtractionSolid booleanZplus(tube_shape, box_shape, Position(0.0, 0.0, 0.0));
    dd4hep::SubtractionSolid booleanZminus(tube_shape, box_shape, Position(0.0, 0.0, 0.0));
    dd4hep::UnionSolid unionean(booleanZplus, booleanZminus, Position(0.0, 0.0, 2*z_min+z_depth));
    
    // dd4hep::Volume envelopeVolZplus("envelopeVolZplus", boolean, vacuum); 
    // dd4hep::Volume envelopeVolZminus("envelopeVolZminus", boolean, vacuum); 

    
    dd4hep::Volume envelopeVol("envelopeVol", unionean, vacuum); 
    dd4hep::Transform3D transform(dd4hep::RotationZ(90*degree), dd4hep::Position(0.0, 0.0, -z_min-z_depth/2)); 
    dd4hep::PlacedVolume envelopePlv = motherVol.placeVolume(envelopeVol, transform);

    
    // dd4hep::Transform3D transform2(dd4hep::RotationZ(90*degree),  dd4hep::Position(0,0,-z_min-z_depth/2));
    // dd4hep::PlacedVolume	envelopePlvZplus = motherVol.placeVolume(envelopeVolZplus, transform1);
    // dd4hep::PlacedVolume	envelopePlvZminus = motherVol.placeVolume(envelopeVolZminus, transform2);
    // envelopePlvZplus.addPhysVolID("system",x_det.id()).addPhysVolID("module", 0);
    envelopePlv.addPhysVolID("system",x_det.id());
    // envelopeVolZplus.setVisAttributes(theDetector, "BlueVis");
    envelopeVol.setVisAttributes(theDetector, "BlueVis");
    ECAL.setPlacement(envelopePlv);
    // moduleEle.setPlacement(envelopePlvZminus);

    dd4hep::DetElement moduleEle(ECAL, "moduleEle", detid);
    // dd4hep::DetElement towerEle(moduleEle, "towerEle", detid);
    dd4hep::DetElement stavedet(moduleEle, "staveEle", detid);
    
    
    // // ####################
    // // ### Print Volume ###
    // // ####################


    // // ###############################
    // // ### module inside placement ###
    // // ###############################
    
    dd4hep::Box module_box(x_width/2, y_width/2, z_depth/2);
    dd4hep::Volume module_volume("module_volume", module_box, air);
    module_volume.setVisAttributes(theDetector, "GreenVis");

    //Carbon fiber supporting

    dd4hep::Volume CarbonFiber0("CarbonFiber0", dd4hep::Box(x_width/2, carbon/2, z_depth/2), mat_CF); 
    CarbonFiber0.setVisAttributes(theDetector, "GrayVis");

    dd4hep::Volume CarbonFiber1("CarbonFiber1", dd4hep::Box(carbon/2, (y_width-2*carbon)/2, z_depth/2), mat_CF); 
    CarbonFiber1.setVisAttributes(theDetector, "GrayVis");

    dd4hep::PlacedVolume plv_cf0 = module_volume.placeVolume(CarbonFiber0, Position(0, y_width/2-carbon/2, 0));
    std::string cfname0 = "carbonfiber_s0";	
    dd4hep::DetElement cfdet0(stavedet, cfname0, detid);
    cfdet0.setPlacement(plv_cf0);

    dd4hep::PlacedVolume plv_cf1 = module_volume.placeVolume(CarbonFiber0, Position(0, -y_width/2+carbon/2, 0));
    std::string cfname1 = "carbonfiber_s1";	
    dd4hep::DetElement cfdet1(stavedet, cfname1, detid);
    cfdet1.setPlacement(plv_cf1);

    dd4hep::PlacedVolume plv_bar2 = module_volume.placeVolume(CarbonFiber1, Position(x_width/2-carbon/2, 0, 0));
    std::string barname2 = "carbonfiber_s2";
    dd4hep::DetElement bardet2(stavedet, barname2, detid);
    bardet2.setPlacement(plv_bar2);

    dd4hep::PlacedVolume plv_bar3= module_volume.placeVolume(CarbonFiber1, Position(-x_width/2+carbon/2, 0, 0));
    std::string barname3 = "carbonfiber_s3";
    dd4hep::DetElement bardet3(stavedet, barname3, detid);
    bardet3.setPlacement(plv_bar3);

    //Cooling copper

    dd4hep::Volume copper0("copper0", dd4hep::Box((x_width-2*carbon)/2, cooling/2, z_depth/2), mat_Cu); 
    copper0.setVisAttributes(theDetector, "RedVis");

    dd4hep::Volume copper1("copper1", dd4hep::Box(cooling/2, (x_width-2*carbon-2*cooling)/2, z_depth/2), mat_Cu); 
    copper1.setVisAttributes(theDetector, "RedVis");

    dd4hep::PlacedVolume plv_copper0 = module_volume.placeVolume(copper0, Position(0, y_width/2-carbon-cooling/2, 0));
    std::string coppername0 = "copper_s0";
    dd4hep::DetElement copperdet0(stavedet, coppername0, detid);
    copperdet0.setPlacement(plv_copper0);

    dd4hep::PlacedVolume plv_copper1 = module_volume.placeVolume(copper0, Position(0, -y_width/2+carbon+cooling/2, 0));
    std::string coppername1 = "copper_s1";
    dd4hep::DetElement copperdet1(stavedet, coppername1, detid);
    copperdet1.setPlacement(plv_copper1);

    dd4hep::PlacedVolume plv_copper2 = module_volume.placeVolume(copper1, Position(x_width/2-carbon-cooling/2, 0, 0));
    std::string coppername2 = "copper_s2";
    dd4hep::DetElement copperdet2(stavedet, coppername2, detid);
    copperdet2.setPlacement(plv_copper2);

    dd4hep::PlacedVolume plv_copper3 = module_volume.placeVolume(copper1, Position(-x_width/2+carbon+cooling/2, 0, 0));
    std::string coppername3 = "copper_s3";
    dd4hep::DetElement copperdet3(stavedet, coppername3, detid);
    copperdet3.setPlacement(plv_copper3);

    // Electronics

    double electronics = cable + pcb + asic;

    dd4hep::Volume electronics0("electronics0", dd4hep::Box((x_width-2*carbon-2*cooling)/2, electronics/2, z_depth/2), mat_PCB);
    electronics0.setVisAttributes(theDetector, "YellowVis");

    dd4hep::Volume electronics1("electronics1", dd4hep::Box(electronics/2, (y_width-2*carbon-2*cooling-2*electronics)/2, z_depth/2), mat_PCB);
    electronics1.setVisAttributes(theDetector, "YellowVis");

    dd4hep::PlacedVolume plv_elec0 = module_volume.placeVolume(electronics0, Position(0, y_width/2-carbon-cooling-electronics/2, 0));
    std::string elecname0 = "electronics_s0";	
    dd4hep::DetElement elecdet0(stavedet, elecname0, detid);
    elecdet0.setPlacement(plv_elec0);

    dd4hep::PlacedVolume plv_elec1 = module_volume.placeVolume(electronics0, Position(0, -y_width/2+carbon+cooling+electronics/2, 0));
    std::string elecname1 = "electronics_s1";
    dd4hep::DetElement elecdet1(stavedet, elecname1, detid);
    elecdet1.setPlacement(plv_elec1);

    dd4hep::PlacedVolume plv_elec2 = module_volume.placeVolume(electronics1, Position(x_width/2-carbon-cooling-electronics/2, 0, 0));
    std::string elecname2 = "electronics_s2";
    dd4hep::DetElement elecdet2(stavedet, elecname2, detid);
    elecdet2.setPlacement(plv_elec2);

    dd4hep::PlacedVolume plv_elec3 = module_volume.placeVolume(electronics1, Position(-x_width/2+carbon+cooling+electronics/2, 0, 0));
    std::string elecname3 = "electronics_s3";
    dd4hep::DetElement elecdet3(stavedet, elecname3, detid);
    elecdet3.setPlacement(plv_elec3);

    //Back Plate

    // dd4hep::Volume backPlate("back_plate", dd4hep::Box(back_plate*15, back_plate*15, back_plate/2), mat_PCB);
    // backPlate.setVisAttributes(theDetector, "GrayVis");
    // std::string blockname = "back_plate_positive";
    // dd4hep::PlacedVolume plv = subtrap_positive_vol.placeVolume(backPlate, Position(0, 0, dim_z_p-back_plate/2));
    // sd.setPlacement(plv);

    // single layer
    dd4hep::Volume block("block", dd4hep::Box((x_width-2*carbon-2*cooling-2*electronics)/2, (y_width-2*carbon-2*cooling-2*electronics)/2, width_crystal/2), air);
    block.setVisAttributes(theDetector, "SeeThrough");
    std::string blockname = "Block";
    dd4hep::DetElement sd(stavedet, blockname, detid);

    dd4hep::Volume sipm_s0("sipm_s0", dd4hep::Box(photoelectronic/2, photoelectronic_width/2, photoelectronic_width/2), mat_Si); 
    sipm_s0.setVisAttributes(theDetector, "BlueVis");

    dd4hep::Volume sipm_s1("sipm_s1", dd4hep::Box(crystal_wrapping/2, photoelectronic_width/2, photoelectronic_width/2), mat_Si); 
    sipm_s1.setVisAttributes(theDetector, "BlueVis");

    double last_bar = length_crystal - (Nbar-1)*width_crystal;

    dd4hep::Volume bar_s0("bar_s0", dd4hep::Box(length_crystal/2-photoelectronic-crystal_wrapping, width_crystal/2-crystal_wrapping, width_crystal/2-crystal_wrapping), mat_BGO); 
    bar_s0.setVisAttributes(theDetector, "EcalBarrelVis");
    bar_s0.setSensitiveDetector(sens);

    dd4hep::Volume bar_s1("bar_s1", dd4hep::Box(length_crystal/2-photoelectronic-crystal_wrapping, last_bar/2-crystal_wrapping, width_crystal/2-crystal_wrapping), mat_BGO); 
    bar_s1.setVisAttributes(theDetector, "EcalBarrelVis");
    bar_s1.setSensitiveDetector(sens);

    for(int ibar0=0;ibar0<Nbar;ibar0++){
        if(ibar0 == Nbar-1){
            dd4hep::Volume hardware_s1("hardware_s1", dd4hep::Box((x_width-2*carbon-2*cooling-2*electronics)/2, last_bar/2, width_crystal/2), air); 
            hardware_s1.setVisAttributes(theDetector, "SeeThrough");

            dd4hep::Volume crystal_s1("crystal_s1", dd4hep::Box((x_width-2*carbon-2*cooling-2*electronics-2*photoelectronic)/2, last_bar/2, width_crystal/2), mat_ESR); 
            crystal_s1.setVisAttributes(theDetector, "EcalBarrelVis");

            dd4hep::PlacedVolume plv_bar0 = crystal_s1.placeVolume(bar_s1, Position(0, 0, 0));
            std::string barname0 = "CrystalBar_positive_s0_"+std::to_string(ibar0);	
            dd4hep::DetElement bardet0(sd, barname0, detid);
            bardet0.setPlacement(plv_bar0);

            dd4hep::PlacedVolume plv_sipm4 = crystal_s1.placeVolume(sipm_s1, Position(-(x_width-2*carbon-2*cooling-2*electronics)/2+photoelectronic+crystal_wrapping/2, 0, -photoelectronic_width/2));
            std::string sipmname4 = "SiPM_positive_s8_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
            sipmdet4.setPlacement(plv_sipm4);

            dd4hep::PlacedVolume plv_sipm5 = crystal_s1.placeVolume(sipm_s1, Position((x_width-2*carbon-2*cooling-2*electronics)/2-photoelectronic-crystal_wrapping/2, 0, photoelectronic_width/2));
            std::string sipmname5 = "SiPM_positive_s9_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
            sipmdet5.setPlacement(plv_sipm5);

            dd4hep::PlacedVolume plv_sipm6 = crystal_s1.placeVolume(sipm_s1, Position(-(x_width-2*carbon-2*cooling-2*electronics)/2+photoelectronic+crystal_wrapping/2, 0, photoelectronic_width/2));
            std::string sipmname6 = "SiPM_positive_s10_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
            sipmdet6.setPlacement(plv_sipm6);

            dd4hep::PlacedVolume plv_sipm7 = crystal_s1.placeVolume(sipm_s1, Position((x_width-2*carbon-2*cooling-2*electronics)/2-photoelectronic-crystal_wrapping/2, 0, -photoelectronic_width/2));
            std::string sipmname7 = "SiPM_positive_s11_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
            sipmdet7.setPlacement(plv_sipm7);

            dd4hep::PlacedVolume plv_sipm0 = hardware_s1.placeVolume(sipm_s0, Position(-(x_width-2*carbon-2*cooling-2*electronics)/2+photoelectronic/2, 0, -photoelectronic_width/2));
            std::string sipmname0 = "SiPM_positive_s0_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
            sipmdet0.setPlacement(plv_sipm0);

            dd4hep::PlacedVolume plv_sipm1 = hardware_s1.placeVolume(sipm_s0, Position((x_width-2*carbon-2*cooling-2*electronics)/2-photoelectronic/2, 0, photoelectronic_width/2));
            std::string sipmname1 = "SiPM_positive_s1_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
            sipmdet1.setPlacement(plv_sipm1);

            dd4hep::PlacedVolume plv_sipm2 = hardware_s1.placeVolume(sipm_s0, Position(-(x_width-2*carbon-2*cooling-2*electronics)/2+photoelectronic/2, 0, photoelectronic_width/2));
            std::string sipmname2 = "SiPM_positive_s2_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
            sipmdet2.setPlacement(plv_sipm2);

            dd4hep::PlacedVolume plv_sipm3 = hardware_s1.placeVolume(sipm_s0, Position((x_width-2*carbon-2*cooling-2*electronics)/2-photoelectronic/2, 0, -photoelectronic_width/2));
            std::string sipmname3 = "SiPM_positive_s3_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
            sipmdet3.setPlacement(plv_sipm3);

            dd4hep::PlacedVolume plv_cry0 = hardware_s1.placeVolume(crystal_s1, Position(0, 0, 0));
            std::string cryname0 = "Crystal_positive_s0_"+std::to_string(ibar0);
            dd4hep::DetElement crydet0(sd, cryname0, detid);
            crydet0.setPlacement(plv_cry0);

            dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s1, Position(0, -length_crystal/2+last_bar/2, 0));
            plv_hard0.addPhysVolID("bar",ibar0);
            std::string hardname0 = "Hardware_positive_s0_"+std::to_string(ibar0);
            dd4hep::DetElement harddet0(sd, hardname0, detid);
            harddet0.setPlacement(plv_hard0);
        }
        else{

            dd4hep::Volume hardware_s0("hardware_s0", dd4hep::Box((x_width-2*carbon-2*cooling-2*electronics)/2, width_crystal/2, width_crystal/2), air); 
            hardware_s0.setVisAttributes(theDetector, "SeeThrough");

            dd4hep::Volume crystal_s0("crystal_s0", dd4hep::Box((x_width-2*carbon-2*cooling-2*electronics-2*photoelectronic)/2, width_crystal/2, width_crystal/2), mat_ESR); 
            crystal_s0.setVisAttributes(theDetector, "EcalBarrelVis");

            dd4hep::PlacedVolume plv_bar0 = crystal_s0.placeVolume(bar_s0, Position(0, 0, 0));
            std::string barname0 = "CrystalBar_s0_"+std::to_string(ibar0);	
            dd4hep::DetElement bardet0(sd, barname0, detid);
            bardet0.setPlacement(plv_bar0);

            dd4hep::PlacedVolume plv_sipm4 = crystal_s0.placeVolume(sipm_s1, Position(-(x_width-2*carbon-2*cooling-2*electronics-2*photoelectronic)/2+crystal_wrapping/2, 0, -photoelectronic_width/2));
            std::string sipmname4 = "SiPM_positive_s8_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
            sipmdet4.setPlacement(plv_sipm4);

            dd4hep::PlacedVolume plv_sipm5 = crystal_s0.placeVolume(sipm_s1, Position((x_width-2*carbon-2*cooling-2*electronics-2*photoelectronic)/2-crystal_wrapping/2, 0, photoelectronic_width/2));
            std::string sipmname5 = "SiPM_positive_s9_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
            sipmdet5.setPlacement(plv_sipm5);

            dd4hep::PlacedVolume plv_sipm6 = crystal_s0.placeVolume(sipm_s1, Position(-(x_width-2*carbon-2*cooling-2*electronics-2*photoelectronic)/2+crystal_wrapping/2, 0, photoelectronic_width/2));
            std::string sipmname6 = "SiPM_positive_s10_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
            sipmdet6.setPlacement(plv_sipm6);

            dd4hep::PlacedVolume plv_sipm7 = crystal_s0.placeVolume(sipm_s1, Position((x_width-2*carbon-2*cooling-2*electronics-2*photoelectronic)/2-crystal_wrapping/2, 0, -photoelectronic_width/2));
            std::string sipmname7 = "SiPM_positive_s11_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
            sipmdet7.setPlacement(plv_sipm7);

            dd4hep::PlacedVolume plv_sipm0 = hardware_s0.placeVolume(sipm_s0, Position(-(x_width-2*carbon-2*cooling-2*electronics)/2+photoelectronic/2, 0, -photoelectronic_width/2));
            std::string sipmname0 = "SiPM_positive_s0_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
            sipmdet0.setPlacement(plv_sipm0);

            dd4hep::PlacedVolume plv_sipm1 = hardware_s0.placeVolume(sipm_s0, Position((x_width-2*carbon-2*cooling-2*electronics)/2-photoelectronic/2, 0, photoelectronic_width/2));
            std::string sipmname1 = "SiPM_positive_s1_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
            sipmdet1.setPlacement(plv_sipm1);

            dd4hep::PlacedVolume plv_sipm2 = hardware_s0.placeVolume(sipm_s0, Position(-(x_width-2*carbon-2*cooling-2*electronics)/2+photoelectronic/2, 0, photoelectronic_width/2));
            std::string sipmname2 = "SiPM_positive_s2_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
            sipmdet2.setPlacement(plv_sipm2);

            dd4hep::PlacedVolume plv_sipm3 = hardware_s0.placeVolume(sipm_s0, Position((x_width-2*carbon-2*cooling-2*electronics)/2-photoelectronic/2, 0, -photoelectronic_width/2));
            std::string sipmname3 = "SiPM_positive_s3_"+std::to_string(ibar0);
            dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
            sipmdet3.setPlacement(plv_sipm3);

            dd4hep::PlacedVolume plv_cry0 = hardware_s0.placeVolume(crystal_s0, Position(0, 0, 0));
            std::string cryname0 = "Crystal_positive_s0_"+std::to_string(ibar0);
            dd4hep::DetElement crydet0(sd, cryname0, detid);
            crydet0.setPlacement(plv_cry0);

            dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s0, Position(0, length_crystal/2-(2*ibar0+1)*width_crystal/2, 0));
            std::string hardname0 = "Hardware_positive_s0_"+std::to_string(ibar0);
            dd4hep::DetElement harddet0(sd, hardname0, detid);
            plv_hard0.addPhysVolID("bar",ibar0);
            harddet0.setPlacement(plv_hard0);
        }
    }

    // // #########################
    // // ### modules placement ###
    // // #########################

    for(int ilayer=0; ilayer<Nlayers; ilayer=ilayer+1){
        if(ilayer%2==0){
            dd4hep::PlacedVolume plv = module_volume.placeVolume(block, Position(0, 0, 0.5*width_crystal+ilayer*width_crystal-z_depth/2));
            plv.addPhysVolID("slayer", 0).addPhysVolID("dlayer", floor(ilayer/2+1));
            stavedet.setPlacement(plv); 
        }
        else{
            dd4hep::Transform3D transform(dd4hep::RotationZ(90*degree),  dd4hep::Position(0, 0, 0.5*width_crystal+ilayer*width_crystal-z_depth/2)); 
            dd4hep::PlacedVolume plv = module_volume.placeVolume(block, transform);
            plv.addPhysVolID("slayer", 1).addPhysVolID("dlayer", floor(ilayer/2+1));
            stavedet.setPlacement(plv); 
            
        }             
    }

    // several smaller module
    // dd4hep::Box module_box1(x_width/2, 260/2, z_depth/2);
    // dd4hep::Volume module_volume1("module_volume1", module_box1, air);
    // module_volume1.setVisAttributes(theDetector, "GreenVis");

    // dd4hep::Volume block1("block1", dd4hep::Box(x_width/2, 260/2, width_crystal/2), air);
    // block1.setVisAttributes(theDetector, "SeeThrough");
    // std::string blockname = "Block1";
    // dd4hep::DetElement sd1(stavedet, blockname1, detid);

    // dd4hep::Volume bar_s0("bar_s0", dd4hep::Box(length_crystal/2, width_crystal/2-crystal_wrapping, width_crystal/2-crystal_wrapping), mat_BGO); 
    // bar_s0.setVisAttributes(theDetector, "EcalBarrelVis");
    // bar_s0.setSensitiveDetector(sens);

    // // #########################
    // // ### modules placement ###
    // // #########################

    int number = 0;
    for(int i=-6; i<7; i=i+1){
        for(int j=-6; j<7; j=j+1){
            // if((i==2 && j==2) || (i==2 && j==3) || (i==3 && j==2) || (i==3 && j==3)){
                if(i==0 || j==0) continue; 
                if(sqrt(abs(i)*abs(i) + abs(j)*abs(j))<2 || sqrt(abs(i)*abs(i) + abs(j)*abs(j))>6.1) continue;
                else{
                    dd4hep::PlacedVolume plvPlus, plvMinus;
                    // cout<<"i: "<<i<<" j: "<<j<<endl;
                    if(i>0 && j>0){
                        plvPlus = envelopeVol.placeVolume(module_volume, Position(x_width*i - x_width/2, y_width*j - y_width/2, 2*z_min+z_depth));
                        dd4hep::Transform3D transform(dd4hep::RotationY(-180*degree),  Position(x_width*i - x_width/2, y_width*j - y_width/2, 0)); 
                        plvMinus = envelopeVol.placeVolume(module_volume, transform);

                        
                    } 
                    else if (i>0 && j<0){
                        plvPlus = envelopeVol.placeVolume(module_volume, Position(x_width*i - x_width/2, y_width*j + y_width/2, 2*z_min+z_depth));
                        dd4hep::Transform3D transform(dd4hep::RotationY(-180*degree),  Position(x_width*i - x_width/2, y_width*j + y_width/2, 0));
                        plvMinus = envelopeVol.placeVolume(module_volume, transform);
                    }  

                    else if (i<0 && j>0) {
                        plvPlus = envelopeVol.placeVolume(module_volume, Position(x_width*i + x_width/2, y_width*j - y_width/2, 2*z_min+z_depth));
                        dd4hep::Transform3D transform(dd4hep::RotationY(-180*degree),  Position(x_width*i + x_width/2, y_width*j - y_width/2, 0));
                        plvMinus = envelopeVol.placeVolume(module_volume, transform);
                    } 
                    else  {
                        plvPlus = envelopeVol.placeVolume(module_volume, Position(x_width*i + x_width/2, y_width*j + y_width/2, 2*z_min+z_depth));
                        dd4hep::Transform3D transform(dd4hep::RotationY(-180*degree),  Position(x_width*i + x_width/2, y_width*j + y_width/2, 0));
                        plvMinus = envelopeVol.placeVolume(module_volume, transform);
                    }

                    plvPlus.addPhysVolID("module", 0).addPhysVolID("stave", 100*(i+6)+(j+6));
                    plvMinus.addPhysVolID("module", 1).addPhysVolID("stave", 100*(i+6)+(j+6));
                    // DetElement sd1(towerEle, "stave1_"+std::to_string(100*(i+6)+(j+6)), detid);
                    // DetElement sd2(towerEle, "stave2_"+std::to_string(100*(i+6)+(j+6)), detid);
                    moduleEle.setPlacement(plvPlus);
                    moduleEle.setPlacement(plvMinus);
                    number++; 
                }
            // }
        }   
    }

    cout<<"number: "<<number<<endl;

  
     

    sens.setType("calorimeter");
    MYDEBUG("create_detector DONE. ");
    return ECAL;
} 

DECLARE_DETELEMENT(LongCrystalBarEndcapCalorimeter_v01, create_detector)
