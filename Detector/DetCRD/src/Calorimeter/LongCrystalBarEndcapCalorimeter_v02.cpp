#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DetType.h"
#include "XML/Layering.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/BitField64.h"
#include "DDSegmentation/Segmentation.h"
#include "DDSegmentation/MultiSegmentation.h"
// #include "LcgeoExceptions.h"

using namespace std;

using dd4hep::BUILD_ENVELOPE;
using dd4hep::BitField64;
using dd4hep::Box;
using dd4hep::Trapezoid;
using dd4hep::Trap;
using dd4hep::EightPointSolid;
using dd4hep::DetElement;
using dd4hep::DetType;
using dd4hep::Detector;
using dd4hep::Layering;
using dd4hep::Material;
using dd4hep::PlacedVolume;
using dd4hep::Position;
using dd4hep::Readout;
using dd4hep::Ref_t;
using dd4hep::RotationX;
using dd4hep::RotationY;
using dd4hep::RotationZ;
using dd4hep::RotationZYX;
using dd4hep::Segmentation;
using dd4hep::SensitiveDetector;
using dd4hep::Transform3D;
using dd4hep::Translation3D;
using dd4hep::Volume;
using dd4hep::_toString;

using dd4hep::rec::LayeredCalorimeterData;

static Ref_t create_detector(Detector& theDetector, xml_h element, SensitiveDetector sens)  {
  xml_det_t   x_det     = element;
  Layering    layering(x_det);
//   xml_dim_t   dim         = x_det.dimensions();
  string      det_name    = x_det.nameStr();

//   Material    air         = theDetector.air();
  Material    stavesMaterial    = theDetector.material(x_det.materialStr());
//   int         numSides    = dim.numsides();

  int           det_id    = x_det.id();

  DetElement   sdet(det_name,det_id);

  PlacedVolume pVol;

  // --- create an envelope volume and position it into the world ---------------------

  Volume envelope = dd4hep::xml::createPlacedEnvelope( theDetector,  element , sdet ) ;
  
//   sdet.setTypeFlag( DetType::CALORIMETER |  DetType::ENDCAP  | DetType::HADRONIC ) ;

//   if( theDetector.buildType() == BUILD_ENVELOPE ) return sdet ;
  //-----------------------------------------------------------------------------------
  
  sens.setType("calorimeter");
  DetElement    stave_det("module0stave0part0",det_id);

  double      EcalEndcap_inner_radius          = theDetector.constant<double>("Ecal_endcap_inner_radius");
  double      EcalEndcap_outer_radius          = theDetector.constant<double>("Ecal_endcap_outer_radius");
  double      EcalEndcap_min_z                 = theDetector.constant<double>("Ecal_endcap_zmin");
  double      EcalEndcap_max_z                 = theDetector.constant<double>("Ecal_endcap_zmax");

  double      EcalEndcap_deadarea_thickness    = theDetector.constant<double>("Ecal_deadarea_thickness");
  

  std::cout << "module_type: " << std::endl;

  int endcapID = 0;

  double dim_x = 0.001;
  double dim_x1 = 0.001;
  double dim_x2 = 0.001;
  double dim_y = 0.001;
  double dim_y1 = 0.001;
  double dim_y2 = 0.001;
  double dim_z = 0.001;
  double box_half_x= 0.001;
  double box_half_y= 0.001;
  double box_half_z= 0.001;
  double trap_half_x1= 0.001;
  double trap_half_x2= 0.001;
  double trap_half_y1= 0.001;
  double trap_half_y2= 0.001;
  double trap_half_z= 0.001;
  
  double pos_x = 0;
  double pos_y = 0;
  double pos_z = 0;
  double EndcapModule_center_pos_x = 0;
  double EndcapModule_center_pos_y = 0;
  double EndcapModule_center_pos_z = 0;

  
  int all_module = 0;
  for(xml_coll_t c(x_det.child(_U(dimensions)),_U(dimensions)); c; ++c) {
    xml_comp_t l(c);
    int N_bar = 0;
    double volume_bar = 0;

    int module_type = l.attr<int>(_Unicode(module_type));
    int module_tag = l.attr<int>(_Unicode(id)) - 1;
    int module_number = l.attr<int>(_Unicode(module_number));
    
    if(module_type == 0){
      dim_x = l.attr<double>(_Unicode(dim_x));
      dim_y = l.attr<double>(_Unicode(dim_y));
      dim_z = l.attr<double>(_Unicode(dim_z));
      pos_x = l.attr<double>(_Unicode(x_offset));
      pos_y = l.attr<double>(_Unicode(y_offset));
      pos_z = l.attr<double>(_Unicode(z_offset));
      box_half_x = dim_x/2.0; 
      box_half_y = dim_y/2.0; 
      box_half_z = dim_z/2.0; 
    }
    else if(module_type == 1 || module_type == 4){
      dim_x1 = l.attr<double>(_Unicode(dim_x1));
      dim_x2 = l.attr<double>(_Unicode(dim_x2));
      dim_y1 = l.attr<double>(_Unicode(dim_y1));
      dim_y2 = l.attr<double>(_Unicode(dim_y2));
      dim_z = l.attr<double>(_Unicode(dim_z));
      pos_x = l.attr<double>(_Unicode(x_offset));
      pos_y = l.attr<double>(_Unicode(y_offset));
      pos_z = l.attr<double>(_Unicode(z_offset));
      trap_half_x1 = dim_x1/2;
      trap_half_x2 = dim_x2/2;
      trap_half_y1 = dim_y1/2;
      trap_half_y2 = dim_y2/2;
      trap_half_z = dim_z/2; 
    }
    else if(module_type == 2 || module_type == 5){
      dim_x1 = l.attr<double>(_Unicode(dim_x1));
      dim_x2 = l.attr<double>(_Unicode(dim_x2));
      dim_x = l.attr<double>(_Unicode(dim_y1));
      dim_y = l.attr<double>(_Unicode(dim_y1));
      dim_y1 = l.attr<double>(_Unicode(dim_y1));
      dim_y2 = l.attr<double>(_Unicode(dim_y2));
      dim_z = l.attr<double>(_Unicode(dim_z));
      pos_x = l.attr<double>(_Unicode(x_offset));
      pos_y = l.attr<double>(_Unicode(y_offset));
      pos_z = l.attr<double>(_Unicode(z_offset));
      trap_half_x1 = dim_x1;
      trap_half_x2 = dim_x2;
      trap_half_y1 = dim_y1;
      trap_half_y2 = dim_y2;
      trap_half_z = dim_z;
    }
    else {
      dim_x = l.attr<double>(_Unicode(dim_x));
      dim_y = l.attr<double>(_Unicode(dim_y));
      dim_z = l.attr<double>(_Unicode(dim_z));
      pos_x = l.attr<double>(_Unicode(x_offset));
      pos_y = l.attr<double>(_Unicode(y_offset));
      pos_z = l.attr<double>(_Unicode(z_offset));
    }


    std::cout << "module_type: " << module_type << std::endl;
    std::cout << "module_number: " << module_number << std::endl;
    std::cout << "dim_x: " << dim_x << std::endl;
    std::cout << "dim_x1: " << dim_x1 << std::endl;
    std::cout << "dim_x2: " << dim_x2 << std::endl;
    std::cout << "dim_y: " << dim_y << std::endl;
    std::cout << "dim_y1: " << dim_y1 << std::endl;
    std::cout << "dim_y2: " << dim_y2 << std::endl;
    std::cout << "dim_z: " << dim_z << std::endl;
    std::cout << "pos_x: " << pos_x << std::endl;
    std::cout << "pos_y: " << pos_y << std::endl;
    std::cout << "pos_z: " << pos_z << std::endl;


    Box EndcapModule(box_half_x,box_half_y,box_half_z);
    Trapezoid EndcapTrapIso(trap_half_x1, trap_half_x2, trap_half_y1, trap_half_y2, trap_half_z);
    Trap EndcapTrapRight(trap_half_y1, trap_half_z, trap_half_x1, trap_half_x2);
    double vertices[] = {-dim_x, -dim_x, -dim_x, dim_y, dim_y, dim_y, dim_y, -dim_x, -dim_x, -dim_x, -dim_x, dim_x, dim_x, dim_x, dim_x, -dim_x};
    EightPointSolid EndcapPrism(dim_z/2,  vertices);

    string envelopeVol_name   = det_name+_toString(endcapID,"_EndcapModule%d");
    string envelopeVolTrapIso_name   = det_name+_toString(endcapID,"_EndcapModule%d");
    string envelopeVolTrapRight_name   = det_name+_toString(endcapID,"_EndcapModule%d");
    string envelopeVolPrism_name   = det_name+_toString(endcapID,"_EndcapModule%d");

    Volume envelopeVol(envelopeVol_name,EndcapModule,stavesMaterial);
    Volume envelopeVolTrapIso(envelopeVolTrapIso_name,EndcapTrapIso,stavesMaterial);
    Volume envelopeVolTrapRight(envelopeVolTrapRight_name,EndcapTrapRight,stavesMaterial);
    Volume envelopeVolPrism(envelopeVolPrism_name,EndcapPrism,stavesMaterial);

    envelopeVol.setAttributes(theDetector,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
    envelopeVolTrapIso.setAttributes(theDetector,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
    envelopeVolTrapRight.setAttributes(theDetector,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
    envelopeVolPrism.setAttributes(theDetector,x_det.regionStr(),x_det.limitsStr(),x_det.visStr());
    
    double layer_pos_z = -dim_z/2;
    int layer_num = 1;
    
    for(xml_coll_t m(x_det,_U(layer)); m; ++m){ 
      xml_comp_t x_layer = m;
	    int repeat = x_layer.repeat();
      double layer_thickness = x_layer.thickness();
      string layer_name = (module_type == 0) ? envelopeVol_name+_toString(0,"_Layer%d") : (module_type == 1 || module_type == 4) ? envelopeVolTrapIso_name+_toString(1,"_Layer%d") : (module_type == 2 || module_type == 5) ? envelopeVolTrapRight_name+_toString(2,"_Layer%d") : envelopeVolPrism_name+_toString(3,"_Layer%d");
      DetElement  layer(stave_det, layer_name, det_id);
      Material layer_material  = theDetector.material(x_layer.materialStr());
      
      if(module_type == 0){ // for cube first place bar then place layer
        double rot_layer = 0;
        double active_layer_dim_x = box_half_x - EcalEndcap_deadarea_thickness;
        double active_layer_dim_y = box_half_y - EcalEndcap_deadarea_thickness;
        double active_layer_dim_z = layer_thickness/2.0;
        
        Volume layer_vol(layer_name, Box(active_layer_dim_x, active_layer_dim_y, active_layer_dim_z), layer_material);

        
        for (int ihardware = 0; ihardware < int(active_layer_dim_y*2/layer_thickness); ihardware++){
          
          string   hardware_name      = layer_name + _toString(ihardware,"_hardware%d");
          Material hardware_material  = layer_material;
          DetElement hardware(layer, _toString(ihardware,"hardware%d"), det_id);
          Volume hardware_vol(hardware_name,Box(active_layer_dim_x, layer_thickness / 2.0, layer_thickness/2.0), hardware_material);
          hardware_vol.setSensitiveDetector(sens);
          hardware_vol.setAttributes(theDetector,x_layer.regionStr(),x_layer.limitsStr(),x_layer.visStr());
          PlacedVolume hardware_phv = layer_vol.placeVolume(hardware_vol,Position(0, -active_layer_dim_y + layer_thickness/2 + layer_thickness*ihardware, 0));
          hardware_phv.addPhysVolID("bar",ihardware);
          hardware.setPlacement(hardware_phv);
          
        }

        layer_vol.setAttributes(theDetector,x_layer.regionStr(),x_layer.limitsStr(),x_layer.visStr());
        
        layer_pos_z += layer_thickness / 2.0;
        
        for (int j = 0; j < repeat; j++){
          
          rot_layer = (j%2==0)? M_PI/2 : 0;

          PlacedVolume layer_phv = envelopeVol.placeVolume(layer_vol, Transform3D(RotationZYX(rot_layer, 0, 0), Position(0,0,layer_pos_z)));
          if(j%2==0) layer_phv.addPhysVolID("slayer", 0).addPhysVolID("dlayer", layer_num);
          else{
            layer_phv.addPhysVolID("slayer", 1).addPhysVolID("dlayer", layer_num);
            ++layer_num;
          }

          layer.setPlacement(layer_phv); 
          layer_pos_z += layer_thickness; 
        }

        N_bar = N_bar + int(active_layer_dim_y*2/layer_thickness)*repeat;
        volume_bar = volume_bar + active_layer_dim_x*2*2*active_layer_dim_y*layer_thickness*repeat;
      }
      else{ //for trap first place layer then place bar
        layer_pos_z += layer_thickness / 2.0;
        double rot_layer = 0;
        double delta_layer = 0;
        for (int j = 0; j < repeat; j++){
          
          delta_layer = (module_type == 1 || module_type == 4) ? layer_thickness*(trap_half_x1 - trap_half_x2)/dim_z : (module_type == 2 || module_type == 5) ? layer_thickness*(dim_x2 - dim_x1)/dim_z : layer_thickness*(dim_x-dim_y)/dim_z;

          double active_layer_dim_x = (module_type == 1  || module_type == 4) ? trap_half_y1 - EcalEndcap_deadarea_thickness : (module_type == 2 || module_type == 5) ? trap_half_x1/2 - EcalEndcap_deadarea_thickness + j*delta_layer/2 : (dim_x+dim_y)/2 - EcalEndcap_deadarea_thickness + j*delta_layer/2; 
          double active_layer_dim_y = (module_type == 1  || module_type == 4) ? trap_half_x1 - EcalEndcap_deadarea_thickness - j*delta_layer : (module_type == 2 || module_type == 5) ? layer_thickness/2.0 : (dim_x+dim_y)/2 - EcalEndcap_deadarea_thickness + j*delta_layer/2; 
          double active_layer_dim_z = (module_type == 1  || module_type == 4) ? layer_thickness/2.0 : (module_type == 2 || module_type == 5) ? trap_half_y1/2 - EcalEndcap_deadarea_thickness : layer_thickness/2.0;
          
          Volume layer_vol(layer_name, Box(active_layer_dim_x, active_layer_dim_y, active_layer_dim_z), layer_material);
          int hardware_num = 0;
          
          if(module_type== 1){
            hardware_num = (j%2==0) ? int(active_layer_dim_x*2/layer_thickness) : int(active_layer_dim_y*2/layer_thickness);
          }
          else if(module_type== 2){
            hardware_num = (j%2==0) ? int(active_layer_dim_z*2/layer_thickness) : int(active_layer_dim_x*2/layer_thickness);
          }
          else if(module_type== 4){
            hardware_num = (j%2==0) ? int(active_layer_dim_y*2/layer_thickness) : int(active_layer_dim_x*2/layer_thickness);
          }
          else if(module_type== 5){
            hardware_num = (j%2==0) ? int(active_layer_dim_x*2/layer_thickness) : int(active_layer_dim_z*2/layer_thickness);
          }
          else {
            hardware_num = (j%2==0) ? int(active_layer_dim_x*2/layer_thickness) : int(active_layer_dim_y*2/layer_thickness);
          }

          double hardware_x = 0;
          double hardware_y = 0;
          double hardware_z = 0;
          double hardware_x_pos = 0;
          double hardware_y_pos = 0;
          double hardware_z_pos = 0;

          for (int ihardware = 0; ihardware < hardware_num; ihardware++){
            string   hardware_name      = layer_name + _toString(ihardware,"_hardware%d");
            Material hardware_material  = layer_material;
            DetElement hardware(layer, _toString(j,"layer%d") + _toString(ihardware,"hardware%d"), det_id);
            if(module_type== 1){
              hardware_x = (j%2==0) ? layer_thickness / 2.0 : active_layer_dim_x;
              hardware_y = (j%2==0) ? active_layer_dim_y : layer_thickness / 2.0;
              hardware_z = layer_thickness/2.0;
            }
            else if(module_type== 2){
              hardware_x = (j%2==0) ? active_layer_dim_x : layer_thickness / 2.0;
              hardware_y = layer_thickness / 2.0;
              hardware_z = (j%2==0) ? layer_thickness / 2.0 : active_layer_dim_z;
            }
            else if(module_type== 3 || module_type== 7){
              hardware_x = (j%2==1) ? layer_thickness / 2.0 : active_layer_dim_x;
              hardware_y = (j%2==1) ? active_layer_dim_y : layer_thickness / 2.0;
              hardware_z = layer_thickness / 2.0;
            }
            else if(module_type== 4){
              hardware_x = (j%2==0) ? active_layer_dim_x : layer_thickness / 2.0;
              hardware_y = (j%2==0) ? layer_thickness / 2.0 : active_layer_dim_y;
              hardware_z = layer_thickness/2.0;
            }
            else if(module_type== 5){
              hardware_x = (j%2==0) ? layer_thickness / 2.0 : active_layer_dim_x;
              hardware_y = layer_thickness / 2.0;
              hardware_z = (j%2==0) ? active_layer_dim_z : layer_thickness / 2.0;
            }
            else if(module_type== 6 || module_type== 8){
              hardware_x = (j%2==1) ? active_layer_dim_x : layer_thickness / 2.0;
              hardware_y = (j%2==1) ? layer_thickness / 2.0 : active_layer_dim_y;
              hardware_z = layer_thickness / 2.0;
            }
            Volume hardware_vol(hardware_name,Box(hardware_x, hardware_y, hardware_z), hardware_material);
            hardware_vol.setSensitiveDetector(sens);
            hardware_vol.setAttributes(theDetector,x_layer.regionStr(),x_layer.limitsStr(),x_layer.visStr());

            volume_bar = volume_bar + hardware_x*2*2*hardware_y*hardware_z*2;

            if(module_type== 1){
              hardware_x_pos = (j%2==0) ? -active_layer_dim_x + layer_thickness/2 + layer_thickness*ihardware : 0;
              hardware_y_pos = (j%2==0) ? 0 : -active_layer_dim_y + layer_thickness/2 + layer_thickness*ihardware;
              hardware_z_pos = 0;
            }
            else if(module_type== 2){
              hardware_x_pos = (j%2==0) ? 0 : -active_layer_dim_x + layer_thickness/2 + layer_thickness*ihardware;
              hardware_y_pos = 0;
              hardware_z_pos = (j%2==0) ? -active_layer_dim_z + layer_thickness/2 + layer_thickness*ihardware : 0;
            }
            else if(module_type== 3 || module_type== 7){
              hardware_x_pos = (j%2==1) ? -active_layer_dim_x + layer_thickness/2 + layer_thickness*ihardware : 0;
              hardware_y_pos = (j%2==1) ? 0 : -active_layer_dim_y + layer_thickness/2 + layer_thickness*ihardware;
              hardware_z_pos = 0;
            }
            else if(module_type== 4){
              hardware_x_pos = (j%2==0) ? 0 : -active_layer_dim_x + layer_thickness/2 + layer_thickness*ihardware;
              hardware_y_pos = (j%2==0) ? -active_layer_dim_y + layer_thickness/2 + layer_thickness*ihardware: 0 ;
              hardware_z_pos = 0;
            }
            else if(module_type== 5){
              hardware_x_pos = (j%2==0) ? -active_layer_dim_x + layer_thickness/2 + layer_thickness*ihardware : 0;
              hardware_y_pos = 0;
              hardware_z_pos = (j%2==0) ? 0 : -active_layer_dim_z + layer_thickness/2 + layer_thickness*ihardware;
            }
            else if(module_type== 6 || module_type== 8){
              hardware_x_pos = (j%2==1) ? 0 : -active_layer_dim_x + layer_thickness/2 + layer_thickness*ihardware;
              hardware_y_pos = (j%2==1) ? -active_layer_dim_y + layer_thickness/2 + layer_thickness*ihardware: 0 ;
              hardware_z_pos = 0;
            }
            PlacedVolume hardware_phv = layer_vol.placeVolume(hardware_vol,Position(hardware_x_pos, hardware_y_pos, hardware_z_pos));
            hardware_phv.addPhysVolID("bar", ihardware);
            hardware.setPlacement(hardware_phv);
          }

          layer_vol.setAttributes(theDetector,x_layer.regionStr(),x_layer.limitsStr(),x_layer.visStr());
          rot_layer = M_PI/2;
          PlacedVolume layer_phv;
          layer_phv = (module_type == 1 || module_type == 4) ? envelopeVolTrapIso.placeVolume(layer_vol, Transform3D(RotationZYX(rot_layer, 0, 0), Position(0,0,layer_pos_z))) : (module_type == 2 || module_type == 5) ? envelopeVolTrapRight.placeVolume(layer_vol, Transform3D(RotationZYX( 0, 0, 0), Position( -(dim_x2 - dim_x1)/2  + j*delta_layer/2 + EcalEndcap_deadarea_thickness, layer_pos_z, 0))) : envelopeVolPrism.placeVolume(layer_vol, Position(-dim_x + active_layer_dim_x + EcalEndcap_deadarea_thickness, -dim_x + active_layer_dim_y + EcalEndcap_deadarea_thickness, layer_pos_z));
          if(j%2==0) layer_phv.addPhysVolID("slayer", 0).addPhysVolID("dlayer", layer_num);
          else{
            layer_phv.addPhysVolID("slayer", 1).addPhysVolID("dlayer", layer_num);
            ++layer_num;
          }
          layer.setPlacement(layer_phv); 
          layer_pos_z += layer_thickness;
        
          N_bar = N_bar + hardware_num;
        }
      }  
    }

    std::cout << "N_bar: " << N_bar << std::endl;
    std::cout << "volume_bar: " << volume_bar << std::endl;

    int sector_sum = 0;
    if(module_type==1  || module_type==4) sector_sum = module_number;
    else if(module_type==3  || module_type==6 || module_type==7  || module_type==8) sector_sum = module_number;
    else sector_sum = module_number*2 + 1;

    for (int sector_num=0;sector_num<sector_sum;sector_num++){
      if(module_type==2 && sector_num<module_number) continue;
      if(module_type==5 && sector_num>module_number) continue;

      double EndcapModule_pos_x = 0;
      double EndcapModule_pos_y = 0;
      double EndcapModule_pos_z = pos_z;
      double rot_EM = 0;
      double rot_ES = 0;
      double rot_EZ = 0;
      double rot_EY = 0;
      double rot_EX = 0;

      if(module_type==0 ){
        if(sector_num<module_number){
          EndcapModule_pos_x = pos_x;
          EndcapModule_pos_y = pos_y + (module_number-sector_num)*dim_y;
        }
        else if(sector_num==module_number){
          EndcapModule_pos_x = pos_x;
          EndcapModule_pos_y = pos_y;
        }
        else{
          EndcapModule_pos_x = pos_x + (sector_num-module_number)*dim_x;
          EndcapModule_pos_y = pos_y;
        }
      }
      else if(module_type==1  || module_type==4){
        EndcapModule_pos_x = pos_x + sector_num*dim_y2;
        EndcapModule_pos_y = pos_y;
      }
      else if(module_type==2 || module_type==5){
        if(sector_num<module_number){
          EndcapModule_pos_x = pos_y;
          EndcapModule_pos_y = pos_x + (module_number-sector_num)*dim_y;
        }
        else if(sector_num==module_number){
          EndcapModule_pos_x = pos_x;
          EndcapModule_pos_y = pos_y;
        }
        else{
          EndcapModule_pos_x = pos_x + (sector_num-module_number)*dim_x;
          EndcapModule_pos_y = pos_y;
        }
      }
      else{
        EndcapModule_pos_x = pos_x;
        EndcapModule_pos_y = pos_y;
      }

      
      for(int stave_num=0;stave_num<4;stave_num++){
        if(module_type==1 && (stave_num==1 || stave_num==3)) continue;
        if(module_type==4 && (stave_num==0 || stave_num==2)) continue;
        if((module_type==3 || module_type==8) && (stave_num==1 || stave_num==3)) continue;
        if((module_type==6 || module_type==7) && (stave_num==0 || stave_num==2)) continue;

        // Set the ID. The ID is composed of the module number, the stave number, the part number.
        int stave_id = 0;
        int part_id = 0;
        if(module_type == 0){
          if(stave_num==0 && sector_num<=module_number) { stave_id = 7 + module_tag;  part_id = sector_num; }
          if(stave_num==0 && sector_num>module_number)  { stave_id = sector_num + 4 + 2*module_tag;  part_id = 3 - module_tag; }
          if(stave_num==1 && sector_num<=module_number) { stave_id = 3 - module_tag;  part_id = sector_num; }
          if(stave_num==1 && sector_num>module_number)  { stave_id = 6 - 2*module_tag - sector_num;  part_id = 3 - module_tag; }
          if(stave_num==2 && sector_num<=module_number) { stave_id = 3 - module_tag;  part_id = 10 - sector_num; }
          if(stave_num==2 && sector_num>module_number)  { stave_id = 6 - 2*module_tag - sector_num;  part_id = 7 + module_tag; }
          if(stave_num==3 && sector_num<=module_number) { stave_id = 7 + module_tag;  part_id = 10 - sector_num; }
          if(stave_num==3 && sector_num>module_number)  { stave_id = sector_num + 4 + 2*module_tag;  part_id = 7 + module_tag; }
        }
        else if(module_type == 1 || module_type == 4) {
          if(stave_num==0) { stave_id = 7 + sector_num;  part_id = 5; }
          if(stave_num==1) { stave_id = 5;  part_id = 3 - sector_num; }
          if(stave_num==2) { stave_id = 3 - sector_num;  part_id = 5; }
          if(stave_num==3) { stave_id = 5;  part_id = 7 + sector_num; }
        }
        else if(module_type == 2){
          if(stave_num==0) { stave_id = 2 + sector_num;  part_id = 4; }
          if(stave_num==1) { stave_id = 8 - sector_num;  part_id = 4; }
          if(stave_num==2) { stave_id = 8 - sector_num;  part_id = 6; }
          if(stave_num==3) { stave_id = 2 + sector_num;  part_id = 6; }
        }
        else if(module_type == 5){
          if(stave_num==0) { stave_id = 6;  part_id = sector_num; }
          if(stave_num==1) { stave_id = 4;  part_id = sector_num; }
          if(stave_num==2) { stave_id = 4;  part_id = 10 - sector_num; }
          if(stave_num==3) { stave_id = 6;  part_id = 10 - sector_num; }
        }
        else {
          if(stave_num==0) { stave_id = 6;  part_id = 4; }
          if(stave_num==1) { stave_id = 4;  part_id = 4; }
          if(stave_num==2) { stave_id = 4;  part_id = 6; }
          if(stave_num==3) { stave_id = 6;  part_id = 6; }
        }

        



        double rot_FZ = 0;
        double rot_FY = 0;
        double rot_FX = 0;

        if(module_type==0){
          EndcapModule_center_pos_x = (stave_num == 0) ? EndcapModule_pos_x : (stave_num == 1) ? -EndcapModule_pos_x : (stave_num == 2) ? -EndcapModule_pos_x : EndcapModule_pos_x;
          EndcapModule_center_pos_y = (stave_num == 0) ? EndcapModule_pos_y : (stave_num == 1) ? EndcapModule_pos_y : (stave_num == 2) ? -EndcapModule_pos_y : -EndcapModule_pos_y;
        }
        else if(module_type==1  || module_type==4){
          EndcapModule_center_pos_x = (stave_num == 0) ? EndcapModule_pos_x : (stave_num == 1) ? EndcapModule_pos_y : (stave_num == 2) ? -EndcapModule_pos_x : EndcapModule_pos_y;
          EndcapModule_center_pos_y = (stave_num == 0) ? EndcapModule_pos_y : (stave_num == 1) ? EndcapModule_pos_x : (stave_num == 2) ? -EndcapModule_pos_y : -EndcapModule_pos_x;
        }
        else{
          EndcapModule_center_pos_x = (stave_num == 0) ? EndcapModule_pos_x : (stave_num == 1) ? -EndcapModule_pos_x : (stave_num == 2) ? -EndcapModule_pos_x : EndcapModule_pos_x;
          EndcapModule_center_pos_y = (stave_num == 0) ? EndcapModule_pos_y : (stave_num == 1) ? EndcapModule_pos_y : (stave_num == 2) ? -EndcapModule_pos_y : -EndcapModule_pos_y;
        }

        rot_ES = (stave_num == 0) ? M_PI/2 : (stave_num == 1) ? 0 : (stave_num == 2) ? M_PI/2 : 0;
        if(sector_num<=module_number) rot_EX = (stave_num == 0) ? M_PI : (stave_num == 1) ? 0 : (stave_num == 2) ? 0 : M_PI;
        else rot_EX = (stave_num == 0) ? M_PI+M_PI/2 : (stave_num == 1) ? 0-M_PI/2 : (stave_num == 2) ? 0+M_PI/2 : M_PI-M_PI/2;
        rot_EZ = (stave_num == 0) ? 0 : (stave_num == 1) ? 0 : (stave_num == 2) ? 0 : 0;
        

        for(int module_num=0;module_num<2;module_num++) {
          if(module_num==0 && (module_type==7 || module_type==8)) continue;
          if(module_num==1 && (module_type==3 || module_type==6)) continue;

          int module_id = (module_num==0)? 0:1;
          
          if(module_type==2 || module_type==5){
            if(sector_num<=module_number) rot_EM = (module_id==0)?M_PI*3/2: M_PI/2;
            else rot_EM = (module_id==0)?M_PI*3/2: M_PI/2;
            if(sector_num>module_number) rot_EY = (module_id==0)? rot_EX+M_PI : rot_EX;
            else rot_EY = rot_EX;
          } 
          else rot_EM = (module_id==0)?M_PI:0;
          
          EndcapModule_center_pos_z = (module_id==0)? -EndcapModule_pos_z:EndcapModule_pos_z;
          
          PlacedVolume env_phv;
          if(module_type==0){
            env_phv = envelope.placeVolume(envelopeVol,
                        Transform3D(RotationX(rot_EM),
                        Translation3D(EndcapModule_center_pos_x,
                          EndcapModule_center_pos_y,
                          EndcapModule_center_pos_z)));
            env_phv.addPhysVolID("module", module_id).addPhysVolID("type", module_tag).addPhysVolID("part", part_id).addPhysVolID("stave", stave_id);
            all_module++;
          }
          else if(module_type==1  || module_type==4){
            env_phv = envelope.placeVolume(envelopeVolTrapIso,
                        Transform3D(RotationZYX(rot_ES, 0, rot_EM),
                        Position(EndcapModule_center_pos_x,
                          EndcapModule_center_pos_y,
                          EndcapModule_center_pos_z)));
            env_phv.addPhysVolID("module", module_id).addPhysVolID("type", module_tag).addPhysVolID("part", part_id).addPhysVolID("stave", stave_id);
            all_module++;
          }
          else if(module_type==2 || module_type==5){
            if(sector_num==module_number){
              continue;
            }
            else{
              env_phv = envelope.placeVolume(envelopeVolTrapRight,
                        Transform3D(RotationZYX(rot_EZ, rot_EY, rot_EM),
                        Position(EndcapModule_center_pos_x,
                          EndcapModule_center_pos_y,
                          EndcapModule_center_pos_z)));
              env_phv.addPhysVolID("module", module_id).addPhysVolID("type", module_tag).addPhysVolID("part", part_id).addPhysVolID("stave", stave_id);
              all_module++;
            }
          }
          else{
            if(module_id==0){
              rot_FZ = (stave_num == 0) ? M_PI/2 : (stave_num == 1) ? 0: (stave_num == 2) ? -M_PI/2 : M_PI;  
            } 
            else{
              rot_FZ = (stave_num == 0) ? M_PI : (stave_num == 1) ? -M_PI/2 : (stave_num == 2) ? 0 : M_PI/2;
            }
            rot_FY = (stave_num == 0) ? 0 : (stave_num == 1) ? 0 : (stave_num == 2) ? 0 : 0;
            rot_FX = (module_id==0)?M_PI:0;
            
            env_phv = envelope.placeVolume(envelopeVolPrism,
                        Transform3D(RotationZYX(rot_FZ, rot_FY, rot_FX),
                        Position(EndcapModule_center_pos_x,
                          EndcapModule_center_pos_y,
                          EndcapModule_center_pos_z)));
            env_phv.addPhysVolID("module", module_id).addPhysVolID("type", module_tag).addPhysVolID("part", part_id).addPhysVolID("stave", stave_id);
            all_module++;
          }
          
          DetElement sd = (module_id==0&&part_id==0&&stave_id==0) ? stave_det : stave_det.clone(_toString(module_id,"module%d")+_toString(stave_id,"stave%d")+_toString(part_id,"part%d"));	  
	        sd.setPlacement(env_phv);
        }
      }
    }
    endcapID++;
  }
  cout<<"EndcapModule: "<<all_module<<endl;
  return sdet;
}

DECLARE_DETELEMENT(LongCrystalBarEndcapCalorimeter_v02, create_detector)