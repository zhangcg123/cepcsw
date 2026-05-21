//==========================================================================
// LongCrystalBarBarrelCalorimeter32Polygon_v02 implementation 
//--------------------------------------------------------------------------
// Author: Song Weizheng, IHEP
//--------------------------------------------------------------------------
// Data: 2024.7.1
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

    double radius_inner = theDetector.constant<double>("ecalbarrel_inner_radius");
    double radius_outer = theDetector.constant<double>("ecalbarrel_outer_radius");
    double length_z = theDetector.constant<double>("ecalbarrel_zlength");

    double X024 = theDetector.constant<double>("ecalbarrel_24X0");

    int Nlayers = theDetector.constant<double>("ecalbarrel_layer");
    int n_module = theDetector.constant<double>("ecalbarrel_phimodule_number");
    int Nblock_z = theDetector.constant<double>("ecalbarrel_Zmodule_number");
    int n_module_display = theDetector.constant<double>("ecalbarrel_phimodule_number_display");
    int Nblock_z_display = theDetector.constant<double>("ecalbarrel_Zmodule_number_display");
    double rotation_angle = theDetector.constant<double>("ecalbarrel_module_ratation"); 

    double width_crystal = theDetector.constant<double>("ecalbarrel_width_crystal");
    double width_crystal_n = theDetector.constant<double>("ecalbarrel_width_crystal_n");
    double size_crystal1 = theDetector.constant<double>("ecalbarrel_size_crystal");
    double crystal_wrapping = theDetector.constant<double>("ecalbarrel_crystal_wrapping");
    double crystal_supportting = theDetector.constant<double>("ecalbarrel_crystal_supportting");
    double photoelectronic = theDetector.constant<double>("ecalbarrel_length_photoelectronic");
    double photoelectronic_width = theDetector.constant<double>("ecalbarrel_width_photoelectronic");

    double dead_material = theDetector.constant<double>("ecalbarrel_length_carbon");
    double dead_material_zz = theDetector.constant<double>("ecalbarrel_length_carbon_z");
    double cable = theDetector.constant<double>("ecalbarrel_length_cable");
    double pcb = theDetector.constant<double>("ecalbarrel_length_pcb");
    double asic = theDetector.constant<double>("ecalbarrel_length_asic");
    double cooling = theDetector.constant<double>("ecalbarrel_length_cooling");
    double back_plate = theDetector.constant<double>("ecalbarrel_length_back");
    
    // ###########################
    // ### general measurement ###
    // ###########################

    // Z direction
    double pZ = length_z/Nblock_z; // length of module in z
    double length_crystal_z = pZ - 2*dead_material_zz - 2*cable - 2*pcb - 2*asic - 2*cooling; // length of crystal wrap sipm in z
    

    // R - phi direction
    double angle = 360./n_module*degree;

    double trap_depth = radius_outer*cos(angle/2.) - radius_inner;
    double trap_line = trap_depth/cos(angle/2.);
    double alice_angle = 0.5*angle + rotation_angle; // angle of "|/" in inner<outer trapozoid
    double alice_1 = 0.5*trap_depth/tan(90*degree-alice_angle);
    double alice_2 = 0.5*trap_line*sin(angle/2.);
    double deviation_positive = alice_1 - alice_2; 
    double bob_angle = 90*degree - angle; 
    double deviation_bob = tan(bob_angle)/(tan(alice_angle)+tan(bob_angle))*deviation_positive;
    double deviation_negative = deviation_bob/cos(angle); 
    double height_layer1 = deviation_bob*tan(angle); 
    double deviation_layer1 = height_layer1*tan(alice_angle); 
    double copper_angle = 180*degree - alice_angle - bob_angle; // angle of "/_" in inner>outer trapozoid
    
    //Half-length of the trapezoid
    double dim_x1 = radius_inner*tan(angle/2.)-deviation_positive+deviation_layer1;
    double dim_x3 = radius_inner*tan(angle/2.)+deviation_negative;

    // Thickness of the trapezoid
    double depth_longitudinal_p = Nlayers*width_crystal + back_plate;
    // double trap_line_p = depth_longitudinal_p/cos(alice_angle);
    // double depth_longitudinal_n = trap_line_p*sin(copper_angle);
    double depth_longitudinal_n = Nlayers*width_crystal_n + back_plate;

    double plus = depth_longitudinal_p*tan(alice_angle);
    double minus = depth_longitudinal_n/tan(copper_angle);
    double dim_x2 = dim_x1+plus;
    double dim_x4 = dim_x3-minus;

    double dim_z_p = depth_longitudinal_p/2.;	
    double dim_z_n = depth_longitudinal_n/2.;	

    double size_crystal = (depth_longitudinal_n-back_plate)/Nlayers; 
    double size_posi_crystal = (depth_longitudinal_p-back_plate)/Nlayers; //Crystal thickness in positive trapezoid

    width_crystal = size_crystal1; // width of crystal in phi
    width_crystal_n = size_crystal1; // width of crystal in phi
    
    int Nbar_phi = floor(length_crystal_z/width_crystal); // number of crystal in phi
    // ####################
    // ### World Volume ###
    // ####################

    dd4hep::DetElement ECAL(det_name, detid);
    dd4hep::Volume motherVol = theDetector.pickMotherVolume(ECAL);

    dd4hep::PolyhedraRegular envelope(n_module, angle/2., radius_inner, radius_outer, length_z);
    dd4hep::Material	air(theDetector.material("Air"));
    dd4hep::Material	vacuum(theDetector.material("Vacuum"));
    dd4hep::Volume	   envelopeVol(det_name, envelope, air);
    dd4hep::PlacedVolume	envelopePlv = motherVol.placeVolume(envelopeVol, Position(0,0,0));
    envelopePlv.addPhysVolID("system",x_det.id());
    envelopeVol.setVisAttributes(theDetector, "SeeThrough" );
    ECAL.setPlacement(envelopePlv);   
    
    dd4hep::Material mat_BGO(theDetector.material("G4_BGO")); 
    dd4hep::Material mat_CF(theDetector.material("CarbonFiber"));
    dd4hep::Material mat_Cu(theDetector.material("G4_Cu"));
    dd4hep::Material mat_ESR(theDetector.material("G4_ESR"));
    dd4hep::Material mat_Si(theDetector.material("G4_Si"));
    dd4hep::Material mat_PCB(theDetector.material("PCB"));

    dd4hep::Trapezoid trap_positive(dim_x1, dim_x2, length_z/2, length_z/2, dim_z_p);
    dd4hep::Volume trap_positive_vol("trap_positive_vol", trap_positive, air);
    trap_positive_vol.setVisAttributes(theDetector, "BlueVis");

    dd4hep::Trapezoid trap_negative(dim_x3, dim_x4, length_z/2, length_z/2, dim_z_n);
    dd4hep::Volume trap_negative_vol("trap_negative_vol", trap_negative, air);
    trap_negative_vol.setVisAttributes(theDetector, "BlueVis");

    dd4hep::Trapezoid subtrap_positive(dim_x1, dim_x2, length_z/2/Nblock_z, length_z/2/Nblock_z, dim_z_p);
    dd4hep::Volume subtrap_positive_vol("subtrap_positive_vol", subtrap_positive, air);
    subtrap_positive_vol.setVisAttributes(theDetector, "SeeThrough"); 

    dd4hep::Trapezoid subtrap_negative(dim_x3, dim_x4, length_z/2/Nblock_z, length_z/2/Nblock_z, dim_z_n);
    dd4hep::Volume subtrap_negative_vol("subtrap_negative_vol", subtrap_negative, air);
    subtrap_negative_vol.setVisAttributes(theDetector, "SeeThrough");

    dd4hep::DetElement stavedet(ECAL, "trap",detid);
    
    // ##################################
    // ### SiPM PCB ASIC Cable Carbon ###
    // ##################################

    double electronics = cable + pcb + asic + cooling; // + photoelectronic 

    double dead_material_p = dead_material/cos(alice_angle);
    double dead_material_n = dead_material/sin(copper_angle);
    
    double dead_material_l = dead_material_p + electronics;
    double dead_material_r = dead_material_n + electronics;
    double dead_material_z = dead_material_zz + electronics;

    dd4hep::Trap left_trap(pZ, size_posi_crystal, dead_material_p + size_posi_crystal*tan(alice_angle), (dead_material_p));
    dd4hep::Volume left("left_vol", left_trap, mat_CF);
    left.setVisAttributes(theDetector, "GrayVis");

    dd4hep::Trap right_trap(pZ, size_crystal, (dead_material_n + size_crystal/tan(copper_angle)), dead_material_n); //Dead material in each layer in positive trapezoid
    dd4hep::Volume right("right_vol", right_trap, mat_CF);
    right.setVisAttributes(theDetector, "GrayVis");

    // ####################
    // ### Print Volume ###
    // ####################

    //std::cout << "dim_x1" << " : " << dim_x1*2 << std::endl;
    //std::cout << "dim_x2" << " : " << dim_x2*2 << std::endl;
    //std::cout << "dim_x3" << " : " << dim_x3*2 << std::endl;
    //std::cout << "dim_x4" << " : " << dim_x4*2 << std::endl;
    //std::cout << "depth_longitudinal_p" << " : " << depth_longitudinal_p << std::endl;
    //std::cout << "depth_longitudinal_n" << " : " << depth_longitudinal_n << std::endl;

    //std::cout << "radius outer" << " : " << sqrt((radius_inner+depth_longitudinal_n)*(radius_inner+depth_longitudinal_n)+dim_x4*dim_x4) << std::endl;
    //std::cout << "radius outer" << " : " << sqrt((radius_inner+height_layer1+depth_longitudinal_p)*(radius_inner+height_layer1+depth_longitudinal_p)+dim_x2*dim_x2) << std::endl;

    //std::cout << "size_crystal" << " : " << size_crystal << std::endl;
    //std::cout << "size_posi_crystal" << " : " << size_posi_crystal << std::endl;

    //std::cout << "pZ" << " : " << pZ << std::endl;
    //std::cout << "Nbar_phi" << " : " << Nbar_phi << std::endl;

    //std::cout << "area_1" << " : " << dead_material_p*2*size_posi_crystal << std::endl;
    //std::cout << "area_2" << " : " << (dead_material_p+dead_material_p+size_posi_crystal*tan(alice_angle))*size_posi_crystal << std::endl;
    //std::cout << "area_3" << " : " << (dead_material_p+dead_material_p+2*size_posi_crystal*tan(alice_angle))*size_posi_crystal << std::endl;

    //std::cout << "area_4" << " : " << dead_material_n*2*size_crystal << std::endl;
    //std::cout << "area_5" << " : " << (dead_material_n+dead_material_n+size_crystal/tan(copper_angle))*size_crystal << std::endl;
    //std::cout << "area_6" << " : " << (dead_material_n+dead_material_n+2*size_crystal/tan(copper_angle))*size_crystal << std::endl;
    //
    //std::cout << "total volume" << " : " << 
    //( dim_x1*2 + (dim_x1+Nlayers*size_crystal*tan(alice_angle))*2 ) * Nlayers*size_crystal /2 * pZ + 
    //( dim_x3*2 + (dim_x3-Nlayers*size_posi_crystal/tan(copper_angle))*2 ) * Nlayers*size_posi_crystal /2 * pZ
    //<< std::endl;

    // ####################################
    // ### placement of all the volumes ###
    // ####################################

    double length_posi; // bottom lenght
    double length_nega; // 
    
    int Nbar_z;
    double last_bar; // 
    double last_bar_z;

    double positive_volume = 0;
    double negative_volume = 0;
    double volume_bar_s0 = 0;
    double volume_bar_s1 = 0;

    // ##########################
    // ### positive trapezoid ###
    // ##########################

    dd4hep::Volume backPlate("back_plate", dd4hep::Box(back_plate*13, back_plate*13, back_plate/2), mat_PCB);
    backPlate.setVisAttributes(theDetector, "GrayVis");
    std::string blockname = "back_plate_positive";
    dd4hep::DetElement sd(stavedet, blockname, detid);
    dd4hep::PlacedVolume plv = subtrap_positive_vol.placeVolume(backPlate, Position(0, 0, dim_z_p-back_plate/2));
    sd.setPlacement(plv);
    // maybe posi and nega bakplate height is different!!!

    for(int ilayer=0; ilayer<Nlayers; ilayer=ilayer+1){

        length_posi = dim_x1  - dead_material_l + ilayer*size_posi_crystal*tan(alice_angle);
        //cout<< "layer: "<< ilayer+1 << endl;
        //cout<< "   length_posi: "<< length_posi*2 << endl;
        
        //Trapezoid in layer
        dd4hep::Volume block("block", dd4hep::Trapezoid( dim_x1+ilayer*size_posi_crystal*tan(alice_angle), dim_x1+(ilayer+1)*size_posi_crystal*tan(alice_angle), pZ/2, pZ/2, size_posi_crystal/2), air);
        block.setVisAttributes(theDetector, "SeeThrough");
        std::string blockname = "Block_positive_"+std::to_string(ilayer+1);
        dd4hep::DetElement sd(stavedet, blockname, detid);

        // Carbon fiber supporting
        dd4hep::Volume CarbonFiber0("CarbonFiber0", dd4hep::Box(length_posi+electronics, dead_material_zz/2, size_posi_crystal/2), mat_CF); //phi
        CarbonFiber0.setVisAttributes(theDetector, "GrayVis");

        dd4hep::PlacedVolume plv_cf0 = block.placeVolume(CarbonFiber0, Position(0, pZ/2-dead_material_zz/2, 0));
        std::string cfname0 = "Deadzone_s4_"+std::to_string(ilayer+1);	
        dd4hep::DetElement cfdet0(sd, cfname0, detid);
        cfdet0.setPlacement(plv_cf0);

        dd4hep::PlacedVolume plv_cf1 = block.placeVolume(CarbonFiber0, Position(0, -pZ/2+dead_material_zz/2, 0));
        std::string cfname1 = "Deadzone_s5_"+std::to_string(ilayer+1);	
        dd4hep::DetElement cfdet1(sd, cfname1, detid);
        cfdet1.setPlacement(plv_cf1);

        dd4hep::Transform3D transform(dd4hep::RotationZ(180*degree)*dd4hep::RotationX(-90*degree),  dd4hep::Position(-length_posi-electronics-(dead_material_p)/2-(((dead_material_p) + size_posi_crystal*tan(alice_angle))/2-(dead_material_p)/2)/2, 0., 0.)); 
        dd4hep::PlacedVolume plv_bar2 = block.placeVolume(left, transform);
        std::string barname2 = "Deadzone_s2_"+std::to_string(ilayer+1);
        dd4hep::DetElement bardet2(sd, barname2, detid);
        bardet2.setPlacement(plv_bar2);

        dd4hep::Transform3D transform2(dd4hep::RotationX(-90*degree),  dd4hep::Position(length_posi+electronics+(dead_material_p)/2+(((dead_material_p) + size_posi_crystal*tan(alice_angle))/2-(dead_material_p)/2)/2, 0., 0.)); 
        dd4hep::PlacedVolume plv_bar3= block.placeVolume(left, transform2);
        std::string barname3 = "Deadzone_s3_"+std::to_string(ilayer+1);
        dd4hep::DetElement bardet3(sd, barname3, detid);
        bardet3.setPlacement(plv_bar3);

        // Electronics

        dd4hep::Volume electronics_phi0("electronics_phi0", dd4hep::Box(length_posi+electronics, (electronics-cooling)/2, size_posi_crystal/2), mat_PCB); //phi
        electronics_phi0.setVisAttributes(theDetector, "YellowVis");

        dd4hep::PlacedVolume plv_elec0 = block.placeVolume(electronics_phi0, Position(0, -length_crystal_z/2-(electronics-cooling)/2, 0));
        std::string elecname0 = "electronics_positive_phi_u_"+std::to_string(ilayer+1);	
        dd4hep::DetElement elecdet0(sd, elecname0, detid);
        elecdet0.setPlacement(plv_elec0);

        dd4hep::PlacedVolume plv_elec1 = block.placeVolume(electronics_phi0, Position(0, length_crystal_z/2+(electronics-cooling)/2, 0));
        std::string elecname1 = "electronics_positive_phi_d_"+std::to_string(ilayer+1);
        dd4hep::DetElement elecdet1(sd, elecname1, detid);
        elecdet1.setPlacement(plv_elec1);

        dd4hep::Volume electronics_phi_l("electronics_phi_l", dd4hep::Box((electronics-cooling)/2, length_crystal_z/2, size_posi_crystal/2), mat_PCB); //phi
        electronics_phi_l.setVisAttributes(theDetector, "YellowVis");

        dd4hep::PlacedVolume plv_elec2 = block.placeVolume(electronics_phi_l, Position(-length_posi-(electronics-cooling)/2, 0, 0));
        std::string elecname2 = "electronics_positive_phi_l_"+std::to_string(ilayer+1);
        dd4hep::DetElement elecdet2(sd, elecname2, detid);
        elecdet2.setPlacement(plv_elec2);

        dd4hep::PlacedVolume plv_elec3 = block.placeVolume(electronics_phi_l, Position(length_posi+(electronics-cooling)/2, 0, 0));
        std::string elecname3 = "electronics_positive_phi_r_"+std::to_string(ilayer+1);
        dd4hep::DetElement elecdet3(sd, elecname3, detid);
        elecdet3.setPlacement(plv_elec3);

        //Cooling copper

        dd4hep::Volume copper_phi0("copper_phi0", dd4hep::Box(length_posi+electronics, cooling/2, size_posi_crystal/2), mat_Cu); //phi
        copper_phi0.setVisAttributes(theDetector, "RedVis");

        dd4hep::PlacedVolume plv_copper0 = block.placeVolume(copper_phi0, Position(0, -length_crystal_z/2-electronics+cooling/2, 0));
        std::string coppername0 = "copper_positive_phi_u_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet0(sd, coppername0, detid);
        copperdet0.setPlacement(plv_copper0);

        dd4hep::PlacedVolume plv_copper1 = block.placeVolume(copper_phi0, Position(0, length_crystal_z/2+electronics-cooling/2, 0));
        std::string coppername1 = "copper_positive_phi_d_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet1(sd, coppername1, detid);
        copperdet1.setPlacement(plv_copper1);

        dd4hep::Volume copper_phi_l("copper_phi_l", dd4hep::Box(cooling/2, length_crystal_z/2, size_posi_crystal/2), mat_Cu); //phi
        copper_phi_l.setVisAttributes(theDetector, "RedVis");

        dd4hep::PlacedVolume plv_copper2 = block.placeVolume(copper_phi_l, Position(-length_posi-electronics+cooling/2, 0, 0));
        std::string coppername2 = "copper_positive_phi_l_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet2(sd, coppername2, detid);
        copperdet2.setPlacement(plv_copper2);

        dd4hep::PlacedVolume plv_copper3 = block.placeVolume(copper_phi_l, Position(length_posi+electronics-cooling/2, 0, 0));
        std::string coppername3 = "copper_positive_phi_r_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet3(sd, coppername3, detid);
        copperdet3.setPlacement(plv_copper3);

        // ###################################
        // ####### Phi direction place #######
        // ###################################

        if(ilayer%2==0){
            //cout<< "   Nbar_phi: " << Nbar_phi << endl;

            dd4hep::Volume sipm_s0("sipm_s0", dd4hep::Box(photoelectronic/2, photoelectronic_width/2, photoelectronic_width/2), mat_Si); 
            sipm_s0.setVisAttributes(theDetector, "BlueVis");

            dd4hep::Volume sipm_s1("sipm_s1", dd4hep::Box(crystal_wrapping/2, photoelectronic_width/2, photoelectronic_width/2), mat_Si); 
            sipm_s1.setVisAttributes(theDetector, "BlueVis");

            last_bar_z = length_crystal_z - (Nbar_phi-1)*width_crystal;

            dd4hep::Volume bar_s0("bar_s0", dd4hep::Box(length_posi-photoelectronic-crystal_wrapping-crystal_supportting, width_crystal/2-crystal_wrapping-crystal_supportting, size_posi_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO); 
            bar_s0.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s0.setSensitiveDetector(sens);
            
            volume_bar_s0 = (length_posi-photoelectronic-crystal_wrapping-crystal_supportting)*(width_crystal/2-crystal_wrapping-crystal_supportting)*(size_posi_crystal/2-crystal_wrapping-crystal_supportting)*8;

            dd4hep::Volume bar_s1("bar_s1", dd4hep::Box(length_posi-photoelectronic-crystal_wrapping-crystal_supportting, last_bar_z/2-crystal_wrapping-crystal_supportting, size_posi_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO); 
            bar_s1.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s1.setSensitiveDetector(sens);
            
            volume_bar_s1 = (length_posi-photoelectronic-crystal_wrapping-crystal_supportting)*(last_bar_z/2-crystal_wrapping-crystal_supportting)*(size_posi_crystal/2-crystal_wrapping-crystal_supportting)*8;

            positive_volume = positive_volume + volume_bar_s0*(Nbar_phi-1) + volume_bar_s1;

            for(int ibar0=0;ibar0<Nbar_phi;ibar0++){
                if(ibar0 == Nbar_phi-1){

                    dd4hep::Volume hardware_s1("hardware_s1", dd4hep::Box(length_posi, last_bar_z/2, size_posi_crystal/2), air); 
                    hardware_s1.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s1("carbonbox_s1", dd4hep::Box(length_posi-photoelectronic, last_bar_z/2, size_posi_crystal/2), mat_CF); 
                    carbonbox_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s1("crystal_s1", dd4hep::Box(length_posi-photoelectronic-crystal_supportting, last_bar_z/2-crystal_supportting, size_posi_crystal/2-crystal_supportting), mat_ESR); 
                    crystal_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s1.placeVolume(bar_s1, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_positive_s0_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s1.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_positive_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s1.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_positive_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s1.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_positive_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s1.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_positive_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s1.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_positive_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s1.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_positive_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s1.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_positive_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s1.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_positive_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s1.placeVolume(sipm_s0, Position(-length_posi+photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s1.placeVolume(sipm_s0, Position(length_posi-photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_positive_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s1.placeVolume(sipm_s0, Position(-length_posi+photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_positive_s2_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s1.placeVolume(sipm_s0, Position(length_posi-photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_positive_s3_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s1.placeVolume(crystal_s1, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s1.placeVolume(carbonbox_s1, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0);

                    dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s1, Position(0, -length_crystal_z/2+last_bar_z/2, 0));
                    plv_hard0.addPhysVolID("slayer",0).addPhysVolID("bar",ibar0);
                    std::string hardname0 = "Hardware_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    harddet0.setPlacement(plv_hard0);
                }
                else{

                    dd4hep::Volume hardware_s0("hardware_s0", dd4hep::Box(length_posi, width_crystal/2, size_posi_crystal/2), air); 
                    hardware_s0.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s0("carbonbox_s0", dd4hep::Box(length_posi-photoelectronic, width_crystal/2, size_posi_crystal/2), mat_CF); 
                    carbonbox_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s0("crystal_s0", dd4hep::Box(length_posi-photoelectronic-crystal_supportting, width_crystal/2-crystal_supportting, size_posi_crystal/2-crystal_supportting), mat_ESR); 
                    crystal_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s0.placeVolume(bar_s0, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_positive_s0_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s0.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_positive_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s0.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_positive_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s0.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_positive_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s0.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_positive_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s0.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_positive_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s0.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_positive_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s0.placeVolume(sipm_s1, Position(-length_posi+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_positive_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s0.placeVolume(sipm_s1, Position(length_posi-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_positive_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s0.placeVolume(sipm_s0, Position(-length_posi+photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s0.placeVolume(sipm_s0, Position(length_posi-photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_positive_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s0.placeVolume(sipm_s0, Position(-length_posi+photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_positive_s2_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s0.placeVolume(sipm_s0, Position(length_posi-photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_positive_s3_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s0.placeVolume(crystal_s0, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s0.placeVolume(carbonbox_s0, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0);

                    dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s0, Position(0, length_crystal_z/2-(2*ibar0+1)*width_crystal/2, 0));
                    std::string hardname0 = "Hardware_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    plv_hard0.addPhysVolID("slayer",0).addPhysVolID("bar",ibar0);
                    harddet0.setPlacement(plv_hard0);
                }
            }
        }

        // #################################
        // ####### Z direction place #######
        // #################################

        else{
            
            Nbar_z = int((length_posi-0.5*width_crystal)/width_crystal);
            last_bar = length_posi-0.5*width_crystal - (Nbar_z-1)*width_crystal;

            //cout<< "   Nbar_z: " << 2*Nbar_z+1 << endl;

            dd4hep::Volume sipm_s0("sipm_s0", dd4hep::Box(photoelectronic_width/2, photoelectronic/2, photoelectronic_width/2), mat_Si); 
            sipm_s0.setVisAttributes(theDetector, "BlueVis");

            dd4hep::Volume sipm_s1("sipm_s1", dd4hep::Box(photoelectronic_width/2, crystal_wrapping/2, photoelectronic_width/2), mat_Si);
            sipm_s1.setVisAttributes(theDetector, "BlueVis");

            dd4hep::Volume bar_s0("bar_s0", dd4hep::Box(width_crystal/2-crystal_wrapping-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting, size_posi_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO); 
            bar_s0.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s0.setSensitiveDetector(sens);

            volume_bar_s0 = (width_crystal/2-crystal_wrapping-crystal_supportting)*(length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting)*(size_posi_crystal/2-crystal_wrapping-crystal_supportting)*8;

            dd4hep::Volume bar_s1("bar_s1", dd4hep::Box(last_bar/2-crystal_wrapping-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting, size_posi_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO); 
            bar_s1.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s1.setSensitiveDetector(sens);

            volume_bar_s1 = (last_bar/2-crystal_wrapping-crystal_supportting)*(length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting)*(size_posi_crystal/2-crystal_wrapping-crystal_supportting)*8;
            
            positive_volume = positive_volume + volume_bar_s0*(2*Nbar_z-1) + volume_bar_s1*2;

            for(int ibar0=0;ibar0<2*Nbar_z+1;ibar0++){
                if( (ibar0 == 0) || (ibar0 == 2*Nbar_z)){
                    dd4hep::Volume hardware_s1("hardware_s1", dd4hep::Box(last_bar/2, length_crystal_z/2, size_posi_crystal/2), air); 
                    hardware_s1.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s1("carbonbox_s1", dd4hep::Box(last_bar/2, length_crystal_z/2-photoelectronic, size_posi_crystal/2), mat_CF);
                    carbonbox_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s1("crystal_s1", dd4hep::Box(last_bar/2-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_supportting, size_posi_crystal/2-crystal_supportting), mat_ESR); 
                    crystal_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s1.placeVolume(bar_s1, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_positive_s1_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_positive_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_positive_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_positive_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_positive_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_positive_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_positive_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_positive_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_positive_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s1.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s1.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_positive_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s1.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_positive_s2_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s1.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_positive_s3_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s1.placeVolume(crystal_s1, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_positive_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s1.placeVolume(carbonbox_s1, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0);

                    dd4hep::PlacedVolume plv_hard0;
                    if(ibar0==0){
                        plv_hard0 = block.placeVolume(hardware_s1, Position(0.5*width_crystal + (Nbar_z-1)*width_crystal + 0.5*last_bar, 0, 0));
                    } 
                    else{
                        plv_hard0 = block.placeVolume(hardware_s1, Position(-0.5*width_crystal - (Nbar_z-1)*width_crystal - 0.5*last_bar, 0, 0));
                    }
                    plv_hard0.addPhysVolID("slayer",1).addPhysVolID("bar",ibar0);
                    std::string hardname0 = "Hardware_positive_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    harddet0.setPlacement(plv_hard0);
                }
                else{
                    dd4hep::Volume hardware_s0("hardware_s0", dd4hep::Box(width_crystal/2, length_crystal_z/2, size_posi_crystal/2), air); 
                    hardware_s0.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s0("carbonbox_s0", dd4hep::Box(width_crystal/2, length_crystal_z/2-photoelectronic, size_posi_crystal/2), mat_CF);
                    carbonbox_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s0("crystal_s0", dd4hep::Box(width_crystal/2-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_supportting, size_posi_crystal/2-crystal_supportting), mat_ESR);   
                    crystal_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s0.placeVolume(bar_s0, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_positive_s1_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_positive_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_positive_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_positive_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_positive_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_positive_s12_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_positive_s13_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_positive_s14_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_positive_s15_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s0.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_positive_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s0.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_positive_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s0.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_positive_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s0.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_positive_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s0.placeVolume(crystal_s0, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_positive_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s0.placeVolume(carbonbox_s0, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_positive_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0);

                    dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s0, Position(length_posi - last_bar - (ibar0-1)*width_crystal - width_crystal/2, 0, 0));
                    std::string hardname0 = "Hardware_positive_s1_"+std::to_string(ibar0);
                    plv_hard0.addPhysVolID("slayer",1).addPhysVolID("bar",ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    harddet0.setPlacement(plv_hard0);
                }
            }
        }

        dd4hep::PlacedVolume plv = subtrap_positive_vol.placeVolume(block, Position(0, 0, 0.5*size_posi_crystal+ilayer*size_posi_crystal-dim_z_p));
        plv.addPhysVolID("dlayer", floor(ilayer/2+1));
        // cout<<"*****layer: "<<floor(ilayer/2+1)<<endl;
        sd.setPlacement(plv);              
    }

    // ##########################
    // ### negative trapezoid ###
    // ##########################

    dd4hep::Volume backPlate1("back_plate_negative", dd4hep::Box(back_plate*13, back_plate*13, back_plate/2), mat_PCB);
    backPlate1.setVisAttributes(theDetector, "GrayVis");
    std::string blockname1 = "back_plate_negative";
    dd4hep::DetElement sd1(stavedet, blockname1, detid);
    dd4hep::PlacedVolume plv1 = subtrap_negative_vol.placeVolume(backPlate1, Position(0, 0, dim_z_n-back_plate/2));
    sd1.setPlacement(plv1);


    for(int ilayer=0; ilayer<Nlayers; ilayer=ilayer+1){
        
        length_nega = dim_x3 - dead_material_r - (ilayer+1)*size_crystal/tan(copper_angle);
        
        //cout << "layer: " << ilayer+1 << endl;
        //cout << "   length_nega: " << length_nega*2 << endl;


        //Trapezoid in layer
        dd4hep::Volume block("block", dd4hep::Trapezoid( dim_x3-ilayer*size_crystal/tan(copper_angle), dim_x3-(ilayer+1)*size_crystal/tan(copper_angle), pZ/2, pZ/2, size_crystal/2), air);
        block.setVisAttributes(theDetector, "SeeThrough");
        std::string blockname = "Block_nagative_"+std::to_string(ilayer+1);
        dd4hep::DetElement sd(stavedet, blockname, detid);

        //Carbon fiber supporting
        dd4hep::Volume CarbonFiber0("CarbonFiber0", dd4hep::Box(length_nega+electronics, dead_material_zz/2, size_crystal/2), mat_CF); //phi
        CarbonFiber0.setVisAttributes(theDetector, "GrayVis");

        dd4hep::PlacedVolume plv_cf0 = block.placeVolume(CarbonFiber0, Position(0, pZ/2-dead_material_zz/2, 0));
        std::string cfname0 = "Deadzone_s4_"+std::to_string(ilayer+1);	
        dd4hep::DetElement cfdet0(sd, cfname0, detid);
        cfdet0.setPlacement(plv_cf0);

        dd4hep::PlacedVolume plv_cf1 = block.placeVolume(CarbonFiber0, Position(0, -pZ/2+dead_material_zz/2, 0));
        std::string cfname1 = "Deadzone_s5_"+std::to_string(ilayer+1);	
        dd4hep::DetElement cfdet1(sd, cfname1, detid);
        cfdet1.setPlacement(plv_cf1);

        dd4hep::Transform3D transform(dd4hep::RotationX(-90*degree)*dd4hep::RotationZ(180*degree), dd4hep::Position(-length_nega-electronics-dead_material_n/2-((dead_material_n + size_crystal/tan(copper_angle))/2-dead_material_n/2)/2, 0., 0.)  ); 
        dd4hep::PlacedVolume plv_bar2 = block.placeVolume(right, transform);
        std::string barname2 = "Deadzone_s2_"+std::to_string(ilayer+1);
        dd4hep::DetElement bardet2(sd, barname2, detid);
        bardet2.setPlacement(plv_bar2);

        dd4hep::Transform3D transform2(dd4hep::RotationX(90*degree),  dd4hep::Position(length_nega+electronics+dead_material_n/2+((dead_material_n + size_crystal/tan(copper_angle))/2-dead_material_n/2)/2, 0., 0.)  ); 
        dd4hep::PlacedVolume plv_bar3= block.placeVolume(right, transform2);
        std::string barname3 = "Deadzone_s3_"+std::to_string(ilayer+1);
        dd4hep::DetElement bardet3(sd, barname3, detid);
        bardet3.setPlacement(plv_bar3);
     
        //Electronics
        dd4hep::Volume electronics_phi0("electronics_phi0", dd4hep::Box(length_nega+electronics, (electronics-cooling)/2, size_crystal/2), mat_PCB); //phi
        electronics_phi0.setVisAttributes(theDetector, "YellowVis");

        dd4hep::PlacedVolume plv_elec0 = block.placeVolume(electronics_phi0, Position(0, -length_crystal_z/2-(electronics-cooling)/2, 0));
        std::string elecname0 = "electronics_nega_phi_u_"+std::to_string(ilayer+1);	
        dd4hep::DetElement elecdet0(sd, elecname0, detid);
        elecdet0.setPlacement(plv_elec0);

        dd4hep::PlacedVolume plv_elec1 = block.placeVolume(electronics_phi0, Position(0, length_crystal_z/2+(electronics-cooling)/2, 0));
        std::string elecname1 = "electronics_nega_phi_d_"+std::to_string(ilayer+1);
        dd4hep::DetElement elecdet1(sd, elecname1, detid);
        elecdet1.setPlacement(plv_elec1);

        dd4hep::Volume electronics_phi_l("electronics_phi_l", dd4hep::Box((electronics-cooling)/2, length_crystal_z/2, size_crystal/2), mat_PCB); //phi
        electronics_phi_l.setVisAttributes(theDetector, "YellowVis");

        dd4hep::PlacedVolume plv_elec2 = block.placeVolume(electronics_phi_l, Position(-length_nega-(electronics-cooling)/2, 0, 0));
        std::string elecname2 = "electronics_nega_phi_l_"+std::to_string(ilayer+1);
        dd4hep::DetElement elecdet2(sd, elecname2, detid);
        elecdet2.setPlacement(plv_elec2);

        dd4hep::PlacedVolume plv_elec3 = block.placeVolume(electronics_phi_l, Position(length_nega+(electronics-cooling)/2, 0, 0));
        std::string elecname3 = "electronics_nega_phi_r_"+std::to_string(ilayer+1);
        dd4hep::DetElement elecdet3(sd, elecname3, detid);
        elecdet3.setPlacement(plv_elec3);

        //Cooling copper

        dd4hep::Volume copper_phi0("copper_phi0", dd4hep::Box(length_nega+electronics, cooling/2, size_crystal/2), mat_Cu); //phi
        copper_phi0.setVisAttributes(theDetector, "RedVis");

        dd4hep::PlacedVolume plv_copper0 = block.placeVolume(copper_phi0, Position(0, -length_crystal_z/2-electronics+cooling/2, 0));
        std::string coppername0 = "cooling_nega_phi_u_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet0(sd, coppername0, detid);
        copperdet0.setPlacement(plv_copper0);

        dd4hep::PlacedVolume plv_copper1 = block.placeVolume(copper_phi0, Position(0, length_crystal_z/2+electronics-cooling/2, 0));
        std::string coppername1 = "cooling_nega_phi_d_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet1(sd, coppername1, detid);
        copperdet1.setPlacement(plv_copper1);

        dd4hep::Volume copper_phi_l("copper_phi_l", dd4hep::Box(cooling/2, length_crystal_z/2, size_crystal/2), mat_Cu); //phi
        copper_phi_l.setVisAttributes(theDetector, "RedVis");

        dd4hep::PlacedVolume plv_copper2 = block.placeVolume(copper_phi_l, Position(-length_nega-electronics+cooling/2, 0, 0));
        std::string coppername2 = "cooling_nega_phi_l_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet2(sd, coppername2, detid);
        copperdet2.setPlacement(plv_copper2);

        dd4hep::PlacedVolume plv_copper3 = block.placeVolume(copper_phi_l, Position(length_nega+electronics-cooling/2, 0, 0));
        std::string coppername3 = "cooling_nega_phi_r_"+std::to_string(ilayer+1);
        dd4hep::DetElement copperdet3(sd, coppername3, detid);
        copperdet3.setPlacement(plv_copper3);
        
        // ###################################
        // ####### Phi direction place #######
        // ###################################

        if(ilayer%2==0){

            //cout<< "   Nbar_phi: " << Nbar_phi << endl;

            dd4hep::Volume sipm_s0("sipm_s0", dd4hep::Box(photoelectronic/2, photoelectronic_width/2, photoelectronic_width/2), mat_Si); 
            sipm_s0.setVisAttributes(theDetector, "BlueVis");

            dd4hep::Volume sipm_s1("sipm_s1", dd4hep::Box(crystal_wrapping/2, photoelectronic_width/2, photoelectronic_width/2), mat_Si);
            sipm_s1.setVisAttributes(theDetector, "BlueVis");

            last_bar_z = length_crystal_z - (Nbar_phi-1)*width_crystal;

            dd4hep::Volume bar_s0("bar_s0", dd4hep::Box(length_nega-photoelectronic-crystal_wrapping-crystal_supportting, width_crystal/2-crystal_wrapping-crystal_supportting, size_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO); 
            bar_s0.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s0.setSensitiveDetector(sens);

            volume_bar_s0 = (length_nega-photoelectronic-crystal_wrapping-crystal_supportting)*(width_crystal/2-crystal_wrapping-crystal_supportting)*(size_crystal/2-crystal_wrapping-crystal_supportting)*8;

            dd4hep::Volume bar_s1("bar_s1", dd4hep::Box(length_nega-photoelectronic-crystal_wrapping-crystal_supportting, last_bar_z/2-crystal_wrapping-crystal_supportting, size_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO); 
            bar_s1.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s1.setSensitiveDetector(sens);

            volume_bar_s1 = (length_nega-photoelectronic-crystal_wrapping-crystal_supportting)*(last_bar_z/2-crystal_wrapping-crystal_supportting)*(size_crystal/2-crystal_wrapping-crystal_supportting)*8;
            
            negative_volume = negative_volume + volume_bar_s0*(Nbar_phi-1) + volume_bar_s1;

            for(int ibar0=0;ibar0<Nbar_phi;ibar0++){
                if(ibar0 == (Nbar_phi-1)){
                    
                    dd4hep::Volume hardware_s1("hardware_s1", dd4hep::Box(length_nega, last_bar_z/2, size_crystal/2), air); 
                    hardware_s1.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s1("carbonbox_s1", dd4hep::Box(length_nega-photoelectronic, last_bar_z/2, size_crystal/2), mat_CF);
                    carbonbox_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s1("crystal_s1", dd4hep::Box(length_nega-photoelectronic-crystal_supportting, last_bar_z/2-crystal_supportting, size_crystal/2-crystal_supportting), mat_ESR); 
                    crystal_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s1.placeVolume(bar_s1, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_negative_s0_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s1.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_negative_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s1.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_negative_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s1.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_negative_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s1.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_negative_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s1.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_negative_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s1.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_negative_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s1.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_negative_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s1.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_negative_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s1.placeVolume(sipm_s0, Position(-length_nega+photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s1.placeVolume(sipm_s0, Position(length_nega-photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s1.placeVolume(sipm_s0, Position(-length_nega+photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_negative_s2_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s1.placeVolume(sipm_s0, Position(length_nega-photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_negative_s3_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s1.placeVolume(crystal_s1, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s1.placeVolume(carbonbox_s1, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0);

                    dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s1, Position(0, -length_crystal_z/2+last_bar_z/2, 0));
                    plv_hard0.addPhysVolID("slayer",0).addPhysVolID("bar",ibar0);
                    std::string hardname0 = "Hardware_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    harddet0.setPlacement(plv_hard0);
                }
                else{
                    dd4hep::Volume hardware_s0("hardware_s0", dd4hep::Box(length_nega, width_crystal/2, size_crystal/2), air); 
                    hardware_s0.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s0("carbonbox_s0", dd4hep::Box(length_nega-photoelectronic, width_crystal/2, size_crystal/2), mat_CF);
                    carbonbox_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s0("crystal_s0", dd4hep::Box(length_nega-photoelectronic-crystal_supportting, width_crystal/2-crystal_supportting, size_crystal/2-crystal_supportting), mat_ESR); 
                    crystal_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s0.placeVolume(bar_s0, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_negative_s0_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s0.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_negative_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s0.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_negative_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s0.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+photoelectronic/2, 0, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_negative_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s0.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-photoelectronic/2, 0, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_negative_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s0.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_negative_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s0.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_negative_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s0.placeVolume(sipm_s1, Position(-length_nega+photoelectronic+crystal_supportting+crystal_wrapping/2, 0, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_negative_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s0.placeVolume(sipm_s1, Position(length_nega-photoelectronic-crystal_supportting-crystal_wrapping/2, 0, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_negative_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s0.placeVolume(sipm_s0, Position(-length_nega+photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s0.placeVolume(sipm_s0, Position(length_nega-photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s0.placeVolume(sipm_s0, Position(-length_nega+photoelectronic/2, 0, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_negative_s2_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s0.placeVolume(sipm_s0, Position(length_nega-photoelectronic/2, 0, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_negative_s3_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s0.placeVolume(crystal_s0, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s0.placeVolume(carbonbox_s0, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0); 

                    dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s0, Position(0, length_crystal_z/2-(2*ibar0+1)*width_crystal/2, 0));
                    plv_hard0.addPhysVolID("slayer",0).addPhysVolID("bar",ibar0);
                    std::string hardname0 = "Hardware_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    harddet0.setPlacement(plv_hard0);
                }
            }
        }

        // #################################
        // ####### Z direction place #######
        // #################################

        else{
        
            Nbar_z = int((length_nega-0.5*width_crystal)/width_crystal);
            last_bar = length_nega-0.5*width_crystal - (Nbar_z-1)*width_crystal;

            //cout << "   Nbar_z: " << 2*Nbar_z+1 << endl;

            dd4hep::Volume sipm_s0("sipm_s0", dd4hep::Box(photoelectronic_width/2, photoelectronic/2, photoelectronic_width/2), mat_Si); 
            sipm_s0.setVisAttributes(theDetector, "BlueVis");

            dd4hep::Volume sipm_s1("sipm_s1", dd4hep::Box(photoelectronic_width/2, crystal_wrapping/2, photoelectronic_width/2), mat_Si);
            sipm_s1.setVisAttributes(theDetector, "BlueVis");

            dd4hep::Volume bar_s0("bar_s0", dd4hep::Box(width_crystal/2-crystal_wrapping-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting, size_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO); 
            bar_s0.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s0.setSensitiveDetector(sens);

            volume_bar_s0 = (width_crystal/2-crystal_wrapping-crystal_supportting)*(length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting)*(size_crystal/2-crystal_wrapping-crystal_supportting)*8;

            dd4hep::Volume bar_s1("bar_s1", dd4hep::Box(last_bar/2-crystal_wrapping-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting, size_crystal/2-crystal_wrapping-crystal_supportting), mat_BGO);
            bar_s1.setVisAttributes(theDetector, "EcalBarrelVis");
            bar_s1.setSensitiveDetector(sens);

            volume_bar_s1 = (last_bar/2-crystal_wrapping-crystal_supportting)*(length_crystal_z/2-photoelectronic-crystal_wrapping-crystal_supportting)*(size_crystal/2-crystal_wrapping-crystal_supportting)*8;

            negative_volume = negative_volume + volume_bar_s0*(2*Nbar_z-1) + volume_bar_s1*2;

            for(int ibar0=0;ibar0<2*Nbar_z+1;ibar0++){
                if( (ibar0 == 0) || (ibar0 == 2*Nbar_z)){
                
                    dd4hep::Volume hardware_s1("hardware_s1", dd4hep::Box(last_bar/2, length_crystal_z/2, size_crystal/2), air); 
                    hardware_s1.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s1("carbonbox_s1", dd4hep::Box(last_bar/2, length_crystal_z/2-photoelectronic, size_crystal/2), mat_CF);
                    carbonbox_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s1("crystal_s1", dd4hep::Box(last_bar/2-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_supportting, size_crystal/2-crystal_supportting), mat_ESR); 
                    crystal_s1.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s1.placeVolume(bar_s1, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_negative_s1_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_negative_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_negative_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_negative_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_negative_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_negative_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_negative_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s1.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_negative_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s1.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_negative_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s1.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s1.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s1.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_negative_s2_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s1.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_negative_s3_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s1.placeVolume(crystal_s1, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s1.placeVolume(carbonbox_s1, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0); 

                    dd4hep::PlacedVolume plv_hard0;
                    if(ibar0==0){
                        plv_hard0 = block.placeVolume(hardware_s1, Position(length_nega - 0.5*last_bar, 0, 0));
                    } 
                    else{
                        plv_hard0 = block.placeVolume(hardware_s1, Position(-length_nega + 0.5*last_bar, 0, 0));
                    }
                    plv_hard0.addPhysVolID("slayer",1).addPhysVolID("bar",ibar0);
                    std::string hardname0 = "Hardware_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    harddet0.setPlacement(plv_hard0);
                }
                else{
                    
                    dd4hep::Volume hardware_s0("hardware_s0", dd4hep::Box(width_crystal/2, length_crystal_z/2, size_crystal/2), air); 
                    hardware_s0.setVisAttributes(theDetector, "SeeThrough");

                    dd4hep::Volume carbonbox_s0("carbonbox_s0", dd4hep::Box(width_crystal/2, length_crystal_z/2-photoelectronic, size_crystal/2), mat_CF);
                    carbonbox_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::Volume crystal_s0("crystal_s0", dd4hep::Box(width_crystal/2-crystal_supportting, length_crystal_z/2-photoelectronic-crystal_supportting, size_crystal/2-crystal_supportting), mat_ESR); 
                    crystal_s0.setVisAttributes(theDetector, "CyanVis");

                    dd4hep::PlacedVolume plv_bar0 = crystal_s0.placeVolume(bar_s0, Position(0, 0, 0));
                    std::string barname0 = "CrystalBar_negative_s1_"+std::to_string(ibar0);	
                    dd4hep::DetElement bardet0(sd, barname0, detid);
                    bardet0.setPlacement(plv_bar0);

                    // dd4hep::PlacedVolume plv_sipm8 = carbonbox_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname8 = "SiPM_negative_s8_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet8(sd, sipmname8, detid);
                    // sipmdet8.setPlacement(plv_sipm8);

                    // dd4hep::PlacedVolume plv_sipm9 = carbonbox_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname9 = "SiPM_negative_s9_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet9(sd, sipmname9, detid);
                    // sipmdet9.setPlacement(plv_sipm9);

                    // dd4hep::PlacedVolume plv_sipm10 = carbonbox_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+photoelectronic/2, photoelectronic_width/2));
                    // std::string sipmname10 = "SiPM_negative_s10_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet10(sd, sipmname10, detid);
                    // sipmdet10.setPlacement(plv_sipm10);

                    // dd4hep::PlacedVolume plv_sipm11 = carbonbox_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-photoelectronic/2, -photoelectronic_width/2));
                    // std::string sipmname11 = "SiPM_negative_s11_"+std::to_string(ibar0);
                    // dd4hep::DetElement sipmdet11(sd, sipmname11, detid);
                    // sipmdet11.setPlacement(plv_sipm11);

                    dd4hep::PlacedVolume plv_sipm4 = crystal_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname4 = "SiPM_negative_s4_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet4(sd, sipmname4, detid);
                    sipmdet4.setPlacement(plv_sipm4);

                    dd4hep::PlacedVolume plv_sipm5 = crystal_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname5 = "SiPM_negative_s5_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet5(sd, sipmname5, detid);
                    sipmdet5.setPlacement(plv_sipm5);

                    dd4hep::PlacedVolume plv_sipm6 = crystal_s0.placeVolume(sipm_s1, Position(0, -length_crystal_z/2+photoelectronic+crystal_supportting+crystal_wrapping/2, photoelectronic_width/2));
                    std::string sipmname6 = "SiPM_negative_s6_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet6(sd, sipmname6, detid);
                    sipmdet6.setPlacement(plv_sipm6);

                    dd4hep::PlacedVolume plv_sipm7 = crystal_s0.placeVolume(sipm_s1, Position(0, length_crystal_z/2-photoelectronic-crystal_supportting-crystal_wrapping/2, -photoelectronic_width/2));
                    std::string sipmname7 = "SiPM_negative_s7_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet7(sd, sipmname7, detid);
                    sipmdet7.setPlacement(plv_sipm7);

                    dd4hep::PlacedVolume plv_sipm0 = hardware_s0.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname0 = "SiPM_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet0(sd, sipmname0, detid);
                    sipmdet0.setPlacement(plv_sipm0);

                    dd4hep::PlacedVolume plv_sipm1 = hardware_s0.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname1 = "SiPM_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet1(sd, sipmname1, detid);
                    sipmdet1.setPlacement(plv_sipm1);

                    dd4hep::PlacedVolume plv_sipm2 = hardware_s0.placeVolume(sipm_s0, Position(0, -length_crystal_z/2+photoelectronic/2, photoelectronic_width/2));
                    std::string sipmname2 = "SiPM_negative_s2_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet2(sd, sipmname2, detid);
                    sipmdet2.setPlacement(plv_sipm2);

                    dd4hep::PlacedVolume plv_sipm3 = hardware_s0.placeVolume(sipm_s0, Position(0, length_crystal_z/2-photoelectronic/2, -photoelectronic_width/2));
                    std::string sipmname3 = "SiPM_negative_s3_"+std::to_string(ibar0);
                    dd4hep::DetElement sipmdet3(sd, sipmname3, detid);
                    sipmdet3.setPlacement(plv_sipm3);

                    dd4hep::PlacedVolume plv_cry0 = carbonbox_s0.placeVolume(crystal_s0, Position(0, 0, 0));
                    std::string cryname0 = "Crystal_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement crydet0(sd, cryname0, detid);
                    crydet0.setPlacement(plv_cry0);

                    dd4hep::PlacedVolume plv_cfb0 = hardware_s0.placeVolume(carbonbox_s0, Position(0, 0, 0));
                    std::string cfbname0 = "cfbstal_negative_s0_"+std::to_string(ibar0);
                    dd4hep::DetElement cfbdet0(sd, cfbname0, detid);
                    cfbdet0.setPlacement(plv_cfb0);


                    dd4hep::PlacedVolume plv_hard0 = block.placeVolume(hardware_s0, Position(length_nega - last_bar - (ibar0-1)*width_crystal - width_crystal/2, 0, 0));
                    plv_hard0.addPhysVolID("slayer",1).addPhysVolID("bar",ibar0);
                    std::string hardname0 = "Hardware_negative_s1_"+std::to_string(ibar0);
                    dd4hep::DetElement harddet0(sd, hardname0, detid);
                    harddet0.setPlacement(plv_hard0);
                }
            }
        }

        dd4hep::PlacedVolume plv = subtrap_negative_vol.placeVolume(block, Position(0, 0, 0.5*size_crystal+ilayer*size_crystal-dim_z_n));
        plv.addPhysVolID("dlayer", floor(ilayer/2+1));
        // cout<<"*****layer: "<<floor(ilayer/2+1)<<endl;
        sd.setPlacement(plv); 
    }

    //cout<<"positive volume "<< positive_volume<<endl;
    //cout<<"negative volume "<< negative_volume<<endl;

    // ###########################
    // ### Z modules placement ###
    // ###########################

    for(int iz=0; iz<Nblock_z_display; iz=iz+1){  //1 Nblock_z
        dd4hep::PlacedVolume plv = trap_positive_vol.placeVolume(subtrap_positive_vol, Position(0, 0.5*length_z-(2*iz+1)*pZ/2,0) );
        plv.addPhysVolID("stave", iz);
        DetElement sd(stavedet, _toString(iz,"stave_posi_%3d"), detid);
        sd.setPlacement(plv);    
    }

    for(int iz=0; iz<Nblock_z_display; iz=iz+1){
        dd4hep::PlacedVolume plv = trap_negative_vol.placeVolume(subtrap_negative_vol, Position(0, 0.5*length_z-(2*iz+1)*pZ/2,0) );
        plv.addPhysVolID("stave", iz);
        DetElement sd(stavedet, _toString(iz,"stave_nega_%3d"), detid);
        sd.setPlacement(plv);    
    }  

    // #############################
    // ### phi modules placement ###
    // #############################

    double r0 = radius_inner + height_layer1 + depth_longitudinal_p/2;  //Phi-module rotation
    double r1 = radius_inner + depth_longitudinal_n/2; 

    for(int i=0;i<n_module_display;i=i+2){ //n_module
        double m_rot = angle*i;
        double posx = -r0*sin(m_rot);
        double posy = r0*cos(m_rot);
        dd4hep::Transform3D transform(dd4hep::RotationZ(m_rot)*dd4hep::RotationX(-90*degree),  dd4hep::Position(posx, posy, 0.)); 
        dd4hep::PlacedVolume plv = envelopeVol.placeVolume(trap_positive_vol, transform);
        plv.addPhysVolID("module", i);
        DetElement sd(ECAL, _toString(i,"trap_posi_%3d"), detid);
        sd.setPlacement(plv);
    }

    for(int i=1;i<n_module_display;i=i+2){
        double m_rot = angle*i;
        double posx = -r1*sin(m_rot);
        double posy = r1*cos(m_rot);
        dd4hep::Transform3D transform(dd4hep::RotationZ(m_rot)*dd4hep::RotationX(-90*degree),  dd4hep::Position(posx, posy, 0.)); 
        dd4hep::PlacedVolume plv = envelopeVol.placeVolume(trap_negative_vol, transform);
        plv.addPhysVolID("module", i);
        DetElement sd(ECAL, _toString(i,"trap_nega_%3d"), detid);
        sd.setPlacement(plv);
    }

    sens.setType("calorimeter");

    MYDEBUG("create_detector DONE. ");
    return ECAL;
} 

DECLARE_DETELEMENT(LongCrystalBarBarrelCalorimeter32Polygon_v02, create_detector)
