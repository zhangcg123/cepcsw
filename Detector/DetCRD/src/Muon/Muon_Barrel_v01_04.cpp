#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/OpticalSurfaces.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/Segmentation.h"
#include <cmath>

#define MYDEBUG(x) std::cout << __FILE__ << ":" << __LINE__ << ": " << x << std::endl;
#define MYDEBUGVAL(x) std::cout << __FILE__ << ":" << __LINE__ << ": " << #x << ": " << x << std::endl;

using dd4hep::rec::LayeredCalorimeterData;
static dd4hep::Ref_t create_detector(dd4hep::Detector& theDetector,
                                     xml_h e,
                                     dd4hep::SensitiveDetector sens) {

    xml_det_t x_det = e;

    std::string det_name = x_det.nameStr();
    std::string det_type = x_det.typeStr();
    MYDEBUGVAL(det_name);
    MYDEBUGVAL(det_type);
    xml_dim_t   pos    (x_det.child(_U(position)));

    dd4hep::DetElement sdet(det_name, x_det.id());

    dd4hep::Assembly envelope(det_name);

    dd4hep::xml::setDetectorTypeFlag( e, sdet ) ;

    dd4hep::Volume motherVol = theDetector.pickMotherVolume(sdet);

    sens.setType("muonbarrel");

//    dd4hep::Readout readout = sens.readout();
//    dd4hep::Segmentation seg = readout.segmentation();
    xml_coll_t dcEnv(x_det,Unicode("barrel"));
    xml_comp_t x_env = dcEnv;
    xml_coll_t dcFe(x_env,Unicode("iron"));
    xml_comp_t x_Fe = dcFe;

    std::string Fe_name = x_Fe.nameStr();
    dd4hep::Material Fe_mat(theDetector.material(x_Fe.materialStr()));
    xml_dim_t Fe_pos(x_Fe.child(_U(position)));
    xml_dim_t Fe_dim(x_Fe.child(_U(dimensions)));
    //double Fe_halfX1 = 0.5 * theDetector.constant<double>("Muon_standard_scale");
   // double Fe_halfX2 = Fe_halfX1 + theDetector.constant<double>("Muon_barrel_iron_z") * std::sqrt(3);

   // dd4hep::Trd2 Fe_solid(Fe_halfX1,Fe_halfX2,Fe_dim.y1(),Fe_dim.y2(),Fe_dim.dz());
   // dd4hep::Volume Fe_vol(Fe_name, Fe_solid, Fe_mat);
   // Fe_vol.setVisAttributes(theDetector.visAttributes(x_Fe.visStr()));

    xml_coll_t dcSuperlayer(x_Fe,Unicode("superlayer"));
    xml_comp_t x_superlayer = dcSuperlayer;

    int strip_type_num_total = 8;
    dd4hep::Box strip_solid[strip_type_num_total], strip_solid_fixed[strip_type_num_total];//, surface_solid[strip_type_num_total], BC420_solid[strip_type_num_total];
    //dd4hep::Tube cut3_solid[strip_type_num_total];
    dd4hep::Volume strip_vol[strip_type_num_total], strip_vol_fixed[strip_type_num_total];//, surface_vol[strip_type_num_total], BC420_vol[strip_type_num_total], cut3_vol[strip_type_num_total];

    xml_coll_t dcStrip(x_superlayer,Unicode("stripe"));
    xml_comp_t x_strip = dcStrip;
    std::string strip_name = x_strip.nameStr();
    dd4hep::Material strip_mat(theDetector.material(x_strip.materialStr()));
    xml_dim_t strip_dim(x_strip.child(_U(dimensions)));
    double fixed_width = theDetector.constant<double>("Muon_barrel_strip_width_fixed");

    for (int i = 0; i < strip_type_num_total; i++ )
    {
      int type_num = i;
      double strip_halfZ;
      if ( i < 6 )
      {
        std::string num_name = "Muon_barrel_strip_num" + dd4hep::_toString(i,"_%d");
        int strip_num = theDetector.constant<int>(num_name);
        strip_halfZ = 0.5 * strip_num * theDetector.constant<double>("Muon_strip_x") + theDetector.constant<double>("Muon_strip_surf");
      }

      if ( i >= 6)
      {
        int fixed_num = i - 6;
        std::string num_name = "Muon_barrel_strip_num_fixed" + dd4hep::_toString( fixed_num, "_%d");
        int strip_num = theDetector.constant<int>(num_name);
        strip_halfZ = 0.5 * fixed_width + 0.5 * strip_num * theDetector.constant<double>("Muon_strip_x") + theDetector.constant<double>("Muon_strip_surf");
      }
      //double surface_halfZ, BC420_halfZ, fiber_halfZ, cut_halfZ;
      
      //surface_halfZ = BC420_halfZ = fiber_halfZ = cut_halfZ = strip_halfZ - theDetector.constant<double>("Muon_strip_surf");   

      std::string strip_name = x_strip.nameStr() + dd4hep::_toString( i, "_%d");
      std::string strip_name_fixed = x_strip.nameStr() + dd4hep::_toString( i, "_%d");

      strip_solid[type_num] = dd4hep::Box(strip_dim.dx(),strip_dim.dy(),strip_halfZ);
      strip_solid_fixed[type_num] = dd4hep::Box(fixed_width, strip_dim.dy(), strip_halfZ);
      strip_vol[type_num] = dd4hep::Volume(strip_name,strip_solid[type_num],strip_mat);
      strip_vol_fixed[type_num] = dd4hep::Volume(strip_name_fixed,strip_solid_fixed[type_num],strip_mat);
      strip_vol[type_num].setVisAttributes(theDetector.visAttributes(x_strip.visStr()));
      //strip_vol[type_num].setSensitiveDetector(sens);
      /*surface_solid[type_num] = dd4hep::Box(surface_dim.dx(),surface_dim.dy(),surface_halfZ);
      BC420_solid[type_num] = dd4hep::Box(BC420_dim.dx(),BC420_dim.dy(),BC420_halfZ);
      cut3_solid[type_num] = dd4hep::Tube(cut3_dim.rmin(),cut3_dim.rmax(),cut_halfZ);
      surface_vol[type_num] = dd4hep::Volume(surface_name, surface_solid[type_num], surface_mat);
      surface_vol[type_num].setVisAttributes(theDetector.visAttributes(surface_vis));

      cut3_vol[type_num] = dd4hep::Volume(cut3_name, cut3_solid[type_num], cut3_mat);
      cut3_vol[type_num].setVisAttributes(theDetector.visAttributes(cut3_vis));

      BC420_vol[type_num] = dd4hep::Volume(BC420_name,BC420_solid[type_num],BC420_mat);
      BC420_vol[type_num].setVisAttributes(theDetector.visAttributes(BC420_vis));
      BC420_vol[type_num].setSensitiveDetector(sens); */
    }

    for(int i0 = 0; i0 < x_env.id(); i0++)
    {
      std::string env_name = x_env.nameStr() + dd4hep::_toString(i0,"_%d");
      xml_dim_t env_pos(x_env.child(_U(position)));

      /*std::string Fe_name = x_Fe.nameStr() + dd4hep::_toString(i0,"_%d");
      dd4hep::Material Fe_mat(theDetector.material(x_Fe.materialStr()));
      xml_dim_t Fe_pos(x_Fe.child(_U(position)));
      xml_dim_t Fe_dim(x_Fe.child(_U(dimensions)));*/
      double Fe_halfX1 = 0.5 * theDetector.constant<double>("Muon_standard_scale");
      double Fe_halfX2 = Fe_halfX1 + theDetector.constant<double>("Muon_barrel_iron_z") * std::sqrt(3);
      double Fe_posZ = -1 * theDetector.constant<double>("Muon_standard_scale") * ( 2.5 + 1.5 * std::sqrt(3) );
      dd4hep::Assembly env_vol(env_name);
      double env_rot = i0 * 360 * dd4hep::degree / x_env.id();
      dd4hep::Transform3D env_transform(dd4hep::Rotation3D(dd4hep::RotationY(env_rot)),dd4hep::Position(env_pos.x(), 0,env_pos.z()));

      dd4hep::Trd2 Fe_solid(Fe_halfX1,Fe_halfX2,Fe_dim.y1(),Fe_dim.y2(),Fe_dim.dz());
      dd4hep::Volume Fe_vol(Fe_name, Fe_solid, Fe_mat);
      Fe_vol.setVisAttributes(theDetector.visAttributes(x_Fe.visStr()));
      dd4hep::Transform3D Fe_transform(dd4hep::Rotation3D(),dd4hep::Position(Fe_pos.x(),Fe_pos.y(),Fe_posZ));
      for(int i1 = 0; i1 < theDetector.constant<int>("Muon_barrel_barrel_num"); i1++ )
      {
        xml_coll_t dcSuperlayer(x_Fe,Unicode("superlayer"));
        xml_comp_t x_superlayer = dcSuperlayer;
        for(int i2 = 0; i2 < x_superlayer.id(); i2++)  // FIXME:: 8 layers start from 0
        {
          std::string superlayer_name = x_superlayer.nameStr() + dd4hep::_toString(i1,"_%d") + dd4hep::_toString(i2,"_%d");
          std::string num_name = "Muon_barrel_strip_num" + dd4hep::_toString(i2,"_%d");

          int strip_num = theDetector.constant<int>(num_name);
          int superlayer_posy_num = ( (2 * i1 - 1 ) * (2 * ( i2 % 2 ) - 1 ) + 1 ) / 2;
          std::string superlayer_posy_name = "Muon_barrel_strip_num_fixed" + dd4hep::_toString( superlayer_posy_num, "_%d");
          std::string superlayer_length_name = "Muon_barrel_superlayer_length" + dd4hep::_toString( superlayer_posy_num, "_%d");
          double superlayer_length = theDetector.constant<double>(superlayer_length_name);

          double superlayer_halfX = 0.5 * strip_num * theDetector.constant<double>("Muon_strip_x") + theDetector.constant<double>("Muon_barrel_superlayer_air_gap");
          double superlayer_halfY = 0.5 * theDetector.constant<double>("Muon_barrel_superlayer_y");
          double superlayer_halfZ = 0.5 * superlayer_length;
          
          double superlayer_posy = ( 2 * i1 - 1 ) * ( 0.5 * theDetector.constant<double>("Muon_total_length") - theDetector.constant<double>("Muon_barrel_superlayer_endcap_gap") - superlayer_halfZ);
          double superlayer_posZ = i2 * theDetector.constant<double>("Muon_barrel_superlayer_gap") + theDetector.constant<double>("Muon_barrel_superlayer_init");
          dd4hep::Assembly superlayer_vol(superlayer_name);
          dd4hep::Transform3D superlayer_transform(dd4hep::Rotation3D(dd4hep::RotationX(90*dd4hep::degree)),dd4hep::Position(0, superlayer_posy, superlayer_posZ));

          for (int i3 = 0; i3 < 2; i3++ )
          {
            int num;
            int type;
            if ( i3 == 0 )
            {
              num = strip_num;
            }
            if ( i3 == 1 )
            {
              num = theDetector.constant<int>(superlayer_posy_name);
              strip_vol_fixed[i2].setSensitiveDetector(sens);
              dd4hep::Transform3D strip_transform_fixed(dd4hep::Rotation3D(dd4hep::RotationY(90*dd4hep::degree)), dd4hep::Position(0, -0.5 * theDetector.constant<double>("Muon_strip_y"), theDetector.constant<double>("Muon_strip_x") * ( num - 0.5 ) - 0.5 * theDetector.constant<int>(superlayer_posy_name) * theDetector.constant<double>("Muon_strip_x")));
              dd4hep::PlacedVolume strip_place_fixed = superlayer_vol.placeVolume(strip_vol_fixed[i2],strip_transform_fixed);
              strip_place_fixed.addPhysVolID("Stripe", num+1).addPhysVolID("Layer",i3+1).addPhysVolID("Superlayer",i2+1).addPhysVolID("Env",i1+1);
            }
            for ( int i4 = 0; i4 < num; i4++ )
            {
              double strip_posX, strip_posY, strip_posZ;
              dd4hep::Rotation3D strip_rot;
              if ( i3 == 0 )
              {
                type = superlayer_posy_num + 6;
                strip_posX = theDetector.constant<double>("Muon_strip_x") * ( i4 + 0.5 * (1 - strip_num));
                strip_posY = 0.5 * theDetector.constant<double>("Muon_strip_y");
                strip_posZ = 0;
              }
              if ( i3 == 1 )
              {
                type = i2;
                strip_rot = dd4hep::Rotation3D(dd4hep::RotationY( 90 * dd4hep::degree ));
                strip_posX = 0;
                strip_posY = -0.5 * theDetector.constant<double>("Muon_strip_y");
                strip_posZ = theDetector.constant<double>("Muon_strip_x") * ( i4 + 0.5 ) - 0.5 * theDetector.constant<int>(superlayer_posy_name) * theDetector.constant<double>("Muon_strip_x") - 0.5 * fixed_width;
              }
              strip_vol[type].setSensitiveDetector(sens);

              dd4hep::Transform3D strip_transform(strip_rot,dd4hep::Position(strip_posX,strip_posY,strip_posZ));
	      dd4hep::PlacedVolume strip_place = superlayer_vol.placeVolume(strip_vol[type],strip_transform);
              strip_place.addPhysVolID("Stripe",i4+1).addPhysVolID("Layer",i3+1).addPhysVolID("Superlayer",i2+1).addPhysVolID("Env",i1+1);
            }
          }

          dd4hep::PlacedVolume Superlayer_place = Fe_vol.placeVolume(superlayer_vol,superlayer_transform);
        }
      }
      dd4hep::PlacedVolume Fe_place = env_vol.placeVolume(Fe_vol,Fe_transform);
      Fe_place.addPhysVolID("Fe",i0+1);
      envelope.placeVolume(env_vol,env_transform);
    }
    dd4hep::Transform3D pv(dd4hep::Rotation3D(dd4hep::RotationX(90*dd4hep::degree)),dd4hep::Position(0,0,0));
    dd4hep::PlacedVolume phv = motherVol.placeVolume(envelope,pv);
    phv.addPhysVolID("system",x_det.id());
    sdet.setPlacement(phv);

    MYDEBUG("create_detector DONE. ");
    return sdet;
}

DECLARE_DETELEMENT(Muon_Barrel_v01_04, create_detector)
