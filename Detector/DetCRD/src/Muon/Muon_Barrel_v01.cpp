//====================================================================
//  Detector description implementation for Chunxiu Liu's EcalMatrix
//--------------------------------------------------------------------
//
//  Author     : Tao Lin
//               Examples from lcgeo
//                   lcgeo/detector/calorimeter/
//
//====================================================================
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
    xml_dim_t   dim    (x_det.child(_U(dimensions)));

    dd4hep::Material det_mat(theDetector.material(x_det.materialStr()));

    dd4hep::DetElement sdet(det_name, x_det.id());

    dd4hep::Volume envelope(det_name,dd4hep::Box(dim.dx(),dim.dy(),dim.dz()),det_mat);

    dd4hep::xml::setDetectorTypeFlag( e, sdet ) ;

    dd4hep::Volume motherVol = theDetector.pickMotherVolume(sdet);

    sens.setType("muonbarrel");

//    dd4hep::Readout readout = sens.readout();
//    dd4hep::Segmentation seg = readout.segmentation();
    xml_coll_t dcEnv(x_det,_U(module));
    xml_comp_t x_env = dcEnv;
    for(int i0 = 0; i0 < theDetector.constant<int>("env_num"); i0++)
    {
      for(int i1 = 0; i1 < x_env.id(); i1++ )
      {
        std::string env_name = x_env.nameStr() + dd4hep::_toString(i1,"_%d");
        dd4hep::Material env_mat(theDetector.material(x_env.materialStr()));
        xml_dim_t env_pos(x_env.child(_U(position)));
        xml_dim_t env_dim(x_env.child(_U(dimensions)));

        xml_coll_t dcFe(x_env,_U(module));
        xml_comp_t x_Fe = dcFe;
        std::string Fe_name = x_Fe.nameStr() + dd4hep::_toString(i1,"_%d");
        dd4hep::Material Fe_mat(theDetector.material(x_Fe.materialStr()));
        xml_dim_t Fe_pos(x_Fe.child(_U(position)));
        xml_dim_t Fe_dim(x_Fe.child(_U(dimensions)));

        double Fe_halfX2 = theDetector.constant<double>("Fe_x1") * ( 0.5 + std::sqrt(3));
        double Fe_posZ = -1 * theDetector.constant<double>("Fe_x1") * ( 2.5 + 1.5 * std::sqrt(3) );
        double env_halfX = 2 * theDetector.constant<double>("Fe_x1") * ( 1 + std::sqrt(3) );
        double env_halfZ = 2 * theDetector.constant<double>("Fe_x1") * ( 3 + 1.5 * std::sqrt(3) );
        dd4hep::Box env_solid(env_halfX,env_dim.dy(),env_halfZ);
        dd4hep::Volume env_vol(env_name, env_solid, env_mat);
        env_vol.setVisAttributes(theDetector.visAttributes(x_env.visStr()));
        double env_rot = i1 * 360 * dd4hep::degree / x_env.id();
        dd4hep::Transform3D env_transform(dd4hep::Rotation3D(dd4hep::RotationY(env_rot)),dd4hep::Position(env_pos.x(), ( 2 * i0 - 1 ) * env_pos.y(),env_pos.z()));

        dd4hep::Trd2 Fe_solid(Fe_dim.x1(),Fe_halfX2,Fe_dim.y1(),Fe_dim.y2(),Fe_dim.dz());
        dd4hep::Volume Fe_vol(Fe_name, Fe_solid, Fe_mat);
        Fe_vol.setVisAttributes(theDetector.visAttributes(x_Fe.visStr()));
        dd4hep::Transform3D Fe_transform(dd4hep::Rotation3D(),dd4hep::Position(Fe_pos.x(),Fe_pos.y(),Fe_posZ));

        xml_coll_t dcSuperlayer(x_Fe,_U(module));
        xml_comp_t x_superlayer = dcSuperlayer;
        for(int i2 = 0; i2 < x_superlayer.id(); i2++)
        {
          std::string superlayer_name = x_superlayer.nameStr() + dd4hep::_toString(i2,"_%d");
          std::string num_name = "strip_num" + dd4hep::_toString(i2,"_%d");

          int strip_num = theDetector.constant<int>(num_name);
          double superlayer_halfX = 0.5 * strip_num * theDetector.constant<double>("strip_x") + theDetector.constant<double>("superlayer_air");
          double superlayer_halfY = 0.5 * theDetector.constant<double>("superlayer_y");
          double superlayer_halfZ = 0.5 * theDetector.constant<double>("superlayer_z"); 

          double superlayer_posZ = i2 * theDetector.constant<double>("layer_gap") + theDetector.constant<double>("layer_init");

          dd4hep::Material superlayer_mat(theDetector.material(x_superlayer.materialStr()));
          dd4hep::Box superlayer_solid(superlayer_halfX, superlayer_halfY, superlayer_halfZ); 
          dd4hep::Volume superlayer_vol(superlayer_name, superlayer_solid, superlayer_mat);
          superlayer_vol.setVisAttributes(theDetector.visAttributes(x_superlayer.visStr()));
          dd4hep::Transform3D superlayer_transform(dd4hep::Rotation3D(dd4hep::RotationX(90*dd4hep::degree)),dd4hep::Position(0, 0, superlayer_posZ));

          xml_coll_t dcAl(x_superlayer,_U(module));
          xml_comp_t x_Al = dcAl;
          std::string Al_name = x_Al.nameStr();
          dd4hep::Material Al_mat(theDetector.material(x_Al.materialStr()));
          xml_dim_t Al_pos(x_Al.child(_U(position)));

          double Al_halfX = superlayer_halfX - theDetector.constant<double>("superlayer_Al");
          double Al_halfY = superlayer_halfY - theDetector.constant<double>("superlayer_Al");
          double Al_halfZ = superlayer_halfZ - theDetector.constant<double>("superlayer_Al");
          dd4hep::Box Al_solid(Al_halfX, Al_halfY, Al_halfZ);
          dd4hep::Volume Al_vol(Al_name, Al_solid, Al_mat);
          Al_vol.setVisAttributes(theDetector.visAttributes(x_Al.visStr()));
          dd4hep::Transform3D Al_transform(dd4hep::Rotation3D(),dd4hep::Position(Al_pos.x(),Al_pos.y(),Al_pos.z()));
          for (int i3 = 0; i3 < 2; i3++ )
          {
            int num;
            if ( i3 == 0 )
            {
              num = strip_num;
            }
            if ( i3 == 1 )
            {
              num = theDetector.constant<int>("strip_num");
            }
            for ( int i4 = 0; i4 < num; i4++ )
            {
              double strip_halfZ, strip_posX, strip_posY, strip_posZ;
              dd4hep::Rotation3D strip_rot;
              if ( i3 == 0 )
              {
                strip_halfZ = 0.5 * theDetector.constant<double>("strip_z") + theDetector.constant<double>("surf");
                strip_posX = theDetector.constant<double>("strip_x") * ( i4 + 0.5 * (1 - strip_num));
                strip_posY = 0.5 * theDetector.constant<double>("strip_y");
                strip_posZ = 0;
              }
              if ( i3 == 1 )
              {
                strip_rot = dd4hep::Rotation3D(dd4hep::RotationY( 90 * dd4hep::degree ));
                strip_halfZ = 0.5 * strip_num * theDetector.constant<double>("strip_x") + theDetector.constant<double>("surf");
                strip_posX = 0;
                strip_posY = -0.5 * theDetector.constant<double>("strip_y");
                strip_posZ = theDetector.constant<double>("strip_x") * ( i4 + 0.5 ) - 0.5 * theDetector.constant<double>("strip_z");
              }
              double surface_halfZ, BC420_halfZ, fiber_halfZ, cut_halfZ;
              double SiPM_posZ = strip_halfZ + theDetector.constant<double>("surf");
              surface_halfZ = BC420_halfZ = fiber_halfZ = cut_halfZ = strip_halfZ - theDetector.constant<double>("surf"); 
              xml_coll_t dcStrip(x_Al,_U(module));
              xml_comp_t x_strip = dcStrip;
              std::string strip_name = x_strip.nameStr() + dd4hep::_toString(i0,"_%d") + dd4hep::_toString(i1,"_%d") + dd4hep::_toString(i2,"_%d") + dd4hep::_toString(i3,"_%d") + dd4hep::_toString(i4,"_%d");
              dd4hep::Material strip_mat(theDetector.material(x_strip.materialStr()));
              xml_dim_t strip_dim(x_strip.child(_U(dimensions)));   

              dd4hep::Box strip_solid(strip_dim.dx(),strip_dim.dy(),strip_halfZ);
              dd4hep::Volume strip_vol(strip_name,strip_solid,strip_mat);
              strip_vol.setVisAttributes(theDetector.visAttributes(x_strip.visStr()));
              dd4hep::Transform3D strip_transform(strip_rot,dd4hep::Position(strip_posX,strip_posY,strip_posZ));

              std::string surface_name, BC420_name, cut1_name, cut2_name, cut3_name, SiPM_name;
              std::string surface_vis, BC420_vis, cut1_vis, cut2_vis, cut3_vis, SiPM_vis;
              dd4hep::Material surface_mat, BC420_mat, cut1_mat, cut2_mat, cut3_mat, SiPM_mat; 
              xml_dim_t surface_pos, cut1_pos, BC420_pos, cut3_pos, cut2_pos, SiPM_pos;
              xml_dim_t surface_dim, cut1_dim, BC420_dim, cut3_dim, cut2_dim, SiPM_dim;
              for(xml_coll_t dcModule(x_strip,_U(module)); dcModule; dcModule++)
              {
                xml_comp_t x_module = dcModule;

                xml_dim_t module_pos(x_module.child(_U(position)));
                xml_dim_t module_dim(x_module.child(_U(dimensions)));

                dd4hep::Box module_solid(module_dim.dx(),module_dim.dy(),module_dim.dz());
                if(x_module.id()==0)
                {
                  surface_name = x_module.nameStr();
                  surface_vis = x_module.visStr();
                  surface_mat = theDetector.material(x_module.materialStr());
                  xml_dim_t s_pos(x_module.child(_U(position)));
                  xml_dim_t s_dim(x_module.child(_U(dimensions)));
                  surface_pos = s_pos;
                  surface_dim = s_dim;

                  xml_coll_t dcCut1(x_module,_U(section));
                  xml_comp_t x_cut1 = dcCut1;
                  cut1_name = x_cut1.nameStr();
                  cut1_vis = x_cut1.visStr();
                  cut1_mat = theDetector.material(x_cut1.materialStr());
                  xml_dim_t c1_pos(x_cut1.child(_U(position)));
                  xml_dim_t c1_dim(x_cut1.child(_U(dimensions)));
                  cut1_pos = c1_pos;
                  cut1_dim = c1_dim;
                }
                if(x_module.id()==1)
                {
                  BC420_name = x_module.nameStr();
                  BC420_vis = x_module.visStr();
                  BC420_mat = theDetector.material(x_module.materialStr());
                  xml_dim_t b_pos(x_module.child(_U(position)));
                  xml_dim_t b_dim(x_module.child(_U(dimensions)));
                  BC420_pos = b_pos;
                  BC420_dim = b_dim;

                  xml_coll_t dcCut3(x_module,_U(section));
                  xml_comp_t x_cut3 = dcCut3;
                  cut3_name = x_cut3.nameStr();
                  cut3_vis = x_cut3.visStr();
                  cut3_mat = theDetector.material(x_cut3.materialStr());
                  xml_dim_t c3_pos(x_cut3.child(_U(position)));
                  xml_dim_t c3_dim(x_cut3.child(_U(dimensions)));
                  cut3_pos = c3_pos;
                  cut3_dim = c3_dim;

                  xml_coll_t dcCut2(x_cut3,_U(section));
                  xml_comp_t x_cut2 = dcCut2;
                  cut2_name = x_cut2.nameStr();
                  cut2_vis = x_cut2.visStr();
                  cut2_mat = theDetector.material(x_cut2.materialStr());
                  xml_dim_t c2_pos(x_cut2.child(_U(position)));
                  xml_dim_t c2_dim(x_cut2.child(_U(dimensions)));
                  cut2_pos = c2_pos;
                  cut2_dim = c2_dim; 
                }
                if(x_module.id()==2)
                {
                  SiPM_name = x_module.nameStr();
                  SiPM_vis = x_module.visStr();
                  SiPM_mat = theDetector.material(x_module.materialStr());
                  xml_dim_t S_pos(x_module.child(_U(position)));
                  xml_dim_t S_dim(x_module.child(_U(dimensions)));
                  SiPM_pos = S_pos;
                  SiPM_dim = S_dim;
                }
              }
              dd4hep::Box surface_solid(surface_dim.dx(),surface_dim.dy(),surface_halfZ);
              dd4hep::Box cut1_solid(cut1_dim.dx(),cut1_dim.dy(),cut_halfZ);
              dd4hep::Box BC420_solid(BC420_dim.dx(),BC420_dim.dy(),BC420_halfZ);
              dd4hep::Tube cut3_solid(cut3_dim.rmin(),cut3_dim.rmax(),cut_halfZ);
              dd4hep::Box cut2_solid(cut2_dim.dx(),cut2_dim.dy(),cut_halfZ);
              dd4hep::Box SiPM_solid(SiPM_dim.dx(),SiPM_dim.dy(),SiPM_dim.dz());

              dd4hep::Transform3D surface_transform(dd4hep::Rotation3D(),dd4hep::Position(surface_pos.x(),surface_pos.y(),surface_pos.z()));
              dd4hep::Transform3D cut1_transform(dd4hep::Rotation3D(),dd4hep::Position(cut1_pos.x(),cut1_pos.y(),cut1_pos.z()));
              dd4hep::Transform3D BC420_transform(dd4hep::Rotation3D(),dd4hep::Position(BC420_pos.x(),BC420_pos.y(),BC420_pos.z()));
              dd4hep::Transform3D cut2_transform(dd4hep::Rotation3D(),dd4hep::Position(cut2_pos.x(),cut2_pos.y(),cut2_pos.z()));
              dd4hep::Transform3D cut3_transform(dd4hep::Rotation3D(),dd4hep::Position(cut3_pos.x(),cut3_pos.y(),cut3_pos.z()));
              dd4hep::Transform3D SiPM_transform0(dd4hep::Rotation3D(),dd4hep::Position(SiPM_pos.x(),SiPM_pos.y(),SiPM_posZ));
              dd4hep::Transform3D SiPM_transform1(dd4hep::Rotation3D(),dd4hep::Position(SiPM_pos.x(),SiPM_pos.y(), -1 * SiPM_posZ));

              dd4hep::Volume surface_vol(surface_name, surface_solid, surface_mat);
              surface_vol.setVisAttributes(theDetector.visAttributes(surface_vis));
              surface_vol.setSensitiveDetector(sens);

              dd4hep::Volume cut1_vol(cut1_name, cut1_solid, cut1_mat);
              cut1_vol.setVisAttributes(theDetector.visAttributes(cut1_vis));

              dd4hep::Volume cut2_vol(cut2_name, cut2_solid, cut2_mat);
              cut2_vol.setVisAttributes(theDetector.visAttributes(cut2_vis));

              dd4hep::Volume cut3_vol(cut3_name, cut3_solid, cut3_mat);
              cut3_vol.setVisAttributes(theDetector.visAttributes(cut3_vis));

              dd4hep::Volume BC420_vol(BC420_name,BC420_solid,BC420_mat);
              BC420_vol.setVisAttributes(theDetector.visAttributes(BC420_vis));

              dd4hep::Volume SiPM_vol(SiPM_name, SiPM_solid, SiPM_mat);
              SiPM_vol.setVisAttributes(theDetector.visAttributes(SiPM_vis));
              SiPM_vol.setSensitiveDetector(sens);

              BC420_vol.placeVolume(cut2_vol,cut2_transform);
              BC420_vol.placeVolume(cut3_vol,cut3_transform);
              dd4hep::PlacedVolume cladding_place, core_place;
              for(xml_coll_t dcSection(x_strip,_U(section)); dcSection; dcSection++)
              {
                xml_comp_t x_section = dcSection;
                std::string section_name = x_section.nameStr();
                dd4hep::Material section_mat = theDetector.material(x_section.materialStr());

                xml_dim_t section_pos(x_section.child(_U(position)));
                xml_dim_t section_dim(x_section.child(_U(dimensions)));

                dd4hep::Tube section_solid(section_dim.rmin(),section_dim.rmax(),fiber_halfZ);
                dd4hep::Volume section_vol(section_name,section_solid,section_mat);
                section_vol.setVisAttributes(theDetector.visAttributes(x_section.visStr()));

                dd4hep::Transform3D section_transform(dd4hep::Rotation3D(),dd4hep::Position(section_pos.x(),section_pos.y(),section_pos.z()));
                if (x_section.id() == 0)
                {
                  cladding_place = BC420_vol.placeVolume(section_vol,section_transform);
                }
                if (x_section.id() == 1)
                {
                  core_place = BC420_vol.placeVolume(section_vol,section_transform);
                }
              }
              surface_vol.placeVolume(cut1_vol,cut1_transform);
              dd4hep::PlacedVolume BC420_place = surface_vol.placeVolume(BC420_vol,BC420_transform);
              BC420_place.addPhysVolID("Stripe",i4+1);
              dd4hep::PlacedVolume surf_place = strip_vol.placeVolume(surface_vol,surface_transform);
              surf_place.addPhysVolID("Layer",i3+1);
              surf_place.addPhysVolID("SiPM",0);
              dd4hep::PlacedVolume SiPM_place0 = strip_vol.placeVolume(SiPM_vol,SiPM_transform0);
              SiPM_place0.addPhysVolID("SiPM",1);

              dd4hep::PlacedVolume SiPM_place1 = strip_vol.placeVolume(SiPM_vol,SiPM_transform1);
              SiPM_place1.addPhysVolID("SiPM",2);
              Al_vol.placeVolume(strip_vol,strip_transform);
              dd4hep::OpticalSurfaceManager surfMgr = theDetector.surfaceManager();
              std::string optical_surf = strip_name + "_surf";
              std::string optical_fiber = strip_name + "_fiber";
              dd4hep::OpticalSurface Surf_stripe = surfMgr.opticalSurface("Surf__stripe");
              dd4hep::OpticalSurface Surf_fiber = surfMgr.opticalSurface("Surf__fiber");
              dd4hep::BorderSurface surface_stripe( theDetector, sdet, optical_surf, Surf_stripe, BC420_place, surf_place);
              dd4hep::BorderSurface surface_fiber( theDetector, sdet, optical_fiber, Surf_fiber, core_place, cladding_place);
            }
          }
          dd4hep::PlacedVolume Al_place = superlayer_vol.placeVolume(Al_vol,Al_transform);
          Al_place.addPhysVolID("Superlayer",i2+1);
          Fe_vol.placeVolume(superlayer_vol,superlayer_transform);
        }
        dd4hep::PlacedVolume Fe_place = env_vol.placeVolume(Fe_vol,Fe_transform);
        Fe_place.addPhysVolID("Fe",i1+1);
        dd4hep::PlacedVolume env_place = envelope.placeVolume(env_vol,env_transform);
        env_place.addPhysVolID("Env",i0+1);
      }
    }
    dd4hep::Transform3D pv(dd4hep::Rotation3D(),dd4hep::Position(0,0,0));
    dd4hep::PlacedVolume phv = motherVol.placeVolume(envelope,pv);
    sdet.setPlacement(phv);

    MYDEBUG("create_detector DONE. ");
    return sdet;
}

DECLARE_DETELEMENT(Muon_Barrel_v01, create_detector)
