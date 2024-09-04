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

    double env_rmax = theDetector.constant<double>("Muon_standard_scale") * std::sqrt( ( 1.5 * 1.5 + ( 3 + 1.5 * std::sqrt(3) ) * ( 3 + 1.5 * std::sqrt(3) ) ) );

    double r1 = theDetector.constant<double>("Muon_endcap_length_cut_1");
    double r2 = theDetector.constant<double>("Muon_endcap_length_cut_2");
    double r_gap = theDetector.constant<double>("Muon_endcap_length_cut_gap");
    double gap = theDetector.constant<double>("Muon_endcap_gap");

    dd4hep::Material det_mat(theDetector.material(x_det.materialStr()));

    dd4hep::Assembly assembly(det_name);

    dd4hep::DetElement sdet("envelope", 1);

    dd4hep::Volume envelope( "Muon_endcap_envelope", dd4hep::Tube( dim.rmin(), env_rmax, dim.dz() ), det_mat);
    envelope.setVisAttributes(theDetector.visAttributes(x_det.visStr()));

    sens.setType("muonendcap");

//    dd4hep::Readout readout = sens.readout();
//    dd4hep::Segmentation seg = readout.segmentation();
    for (int i = 0; i < theDetector.constant<int>("Muon_endcap_superlayer_num"); i++)
    {
      std::string superlayer_name = "superlayer_1" + dd4hep::_toString(i,"_%d");
      dd4hep::Assembly superlayer_vol(superlayer_name);
      dd4hep::Transform3D superlayer_transform(dd4hep::Rotation3D(),dd4hep::Position(0,0,theDetector.constant<double>("Muon_endcap_iron_init") + theDetector.constant<double>("Muon_endcap_iron_gap") * ( i + 1 ) + theDetector.constant<double>("Muon_strip_y") * ( i + 0.5 )));
      for (int i0 = 0; i0 < theDetector.constant<int>("Muon_endcap_part_num"); i0++)
      {
        std::string env_name = "part_1" + dd4hep::_toString(i0,"_%d");
        int test1 = (i0+1) / 3;
        int test2 = (i0+1) / 2;
        int test3 = 1 - 2 * test1;
        int test4 = 2 * ( test2 - 1 ) * ( test2 - 1 ) - 1;
        //dd4hep::Tube env_solid( dim.rmin(), theDetector.constant<double>( "Muon_endcap_magnification" ) * env_rmax, dim.dz(), 0, 90 * dd4hep::degree);
        //dd4hep::Volume env_vol( env_name, env_solid, det_mat);
        //env_vol.setVisAttributes(theDetector.visAttributes(x_det.visStr()));
        dd4hep::Assembly env_vol( env_name );
        dd4hep::Transform3D env_transform(dd4hep::Rotation3D(dd4hep::RotationZ(i0 * 90 * dd4hep::degree)),dd4hep::Position(test4 * gap,test3 * gap,0));
        for (int i1 = 0; i1 < theDetector.constant<int>("Muon_endcap_layer_num"); i1++ )
        {
          for ( int i2 = 0; i2 < theDetector.constant<int>("Muon_endcap_strip_num_1"); i2++)
          {
            double strip_posX, strip_posY, strip_posZ;
            double strip_sizeZ, surface_halfZ, BC420_halfZ, fiber_halfZ, cut_halfZ;
            dd4hep::Rotation3D strip_rot;
            if ( i1 == 1 )
            {
              strip_rot = dd4hep::Rotation3D( dd4hep::RotationX( 90 * dd4hep::degree ) );
              strip_posX = theDetector.constant<double>("Muon_strip_x") * ( i2 + 0.5 );
              strip_posZ = 0.5 * ( 1 - 2 * ( i0 % 2 ) ) * theDetector.constant<double>("Muon_strip_y");
              if ( i2 < theDetector.constant<int>("Muon_endcap_strip_num_cut_1") )
              {
                strip_sizeZ = std::sqrt( r1 * r1 - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") ) - std::sqrt( dim.rmin() * dim.rmin() - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") );
                strip_posY = std::sqrt( r1 * r1 - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") ) - 0.5 * strip_sizeZ; 
              }
              if ( i2 >= theDetector.constant<int>("Muon_endcap_strip_num_cut_1") )
              {
                strip_sizeZ = std::sqrt( r1 * r1 - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") );
                strip_posY = 0.5 * strip_sizeZ;
              }
            }
            if ( i1 == 0 )
            {
              strip_rot = dd4hep::Rotation3D( dd4hep::RotationX( 90 * dd4hep::degree ) * dd4hep::RotationY( 90 * dd4hep::degree ) );
              strip_posY = theDetector.constant<double>("Muon_strip_x") * ( i2 + 0.5 );
              strip_posZ = 0.5 * ( 2 * ( i0 % 2 ) - 1 ) * theDetector.constant<double>("Muon_strip_y");
              if ( i2 < theDetector.constant<int>("Muon_endcap_strip_num_cut_1") )
              {
                strip_sizeZ = std::sqrt( r1 * r1 - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") ) - std::sqrt( dim.rmin() * dim.rmin() - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") );
                strip_posX = std::sqrt( r1 * r1 - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") ) - 0.5 * strip_sizeZ;
              }
              if ( i2 >= theDetector.constant<int>("Muon_endcap_strip_num_cut_1") )
              {
                strip_sizeZ = std::sqrt( r1 * r1 - i2 * theDetector.constant<double>("Muon_strip_x") * i2 * theDetector.constant<double>("Muon_strip_x") );
                strip_posX = 0.5 * strip_sizeZ;
              }
            }
            double strip_halfZ = 0.5 * strip_sizeZ;
            double SiPM_posZ = strip_halfZ - 0.5 * theDetector.constant<double>("Muon_strip_surf");
            surface_halfZ = BC420_halfZ = fiber_halfZ = cut_halfZ = strip_halfZ - theDetector.constant<double>("Muon_strip_surf"); 
            xml_coll_t dcStrip(x_det,Unicode("stripe"));
            xml_comp_t x_strip = dcStrip;
            std::string strip_name = x_strip.nameStr() + dd4hep::_toString(1,"_%d") + dd4hep::_toString(i,"_%d") + dd4hep::_toString(i0,"_%d") + dd4hep::_toString(i1,"_%d") + dd4hep::_toString(i2,"_%d");
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
            for(xml_coll_t dcModule(x_strip,Unicode("component")); dcModule; dcModule++)
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
/*
                xml_coll_t dcCut1(x_module,Unicode("cut"));
                xml_comp_t x_cut1 = dcCut1;
                cut1_name = x_cut1.nameStr();
                cut1_vis = x_cut1.visStr();
                cut1_mat = theDetector.material(x_cut1.materialStr());
                xml_dim_t c1_pos(x_cut1.child(_U(position)));
                xml_dim_t c1_dim(x_cut1.child(_U(dimensions)));
                cut1_pos = c1_pos;
                cut1_dim = c1_dim;*/
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

                xml_coll_t dcCut3(x_module,Unicode("cut"));
                xml_comp_t x_cut3 = dcCut3;
                cut3_name = x_cut3.nameStr();
                cut3_vis = x_cut3.visStr();
                cut3_mat = theDetector.material(x_cut3.materialStr());
                xml_dim_t c3_pos(x_cut3.child(_U(position)));
                xml_dim_t c3_dim(x_cut3.child(_U(dimensions)));
                cut3_pos = c3_pos;
                cut3_dim = c3_dim;
/*
                xml_coll_t dcCut2(x_cut3,Unicode("comb"));
                xml_comp_t x_cut2 = dcCut2;
                cut2_name = x_cut2.nameStr();
                cut2_vis = x_cut2.visStr();
                cut2_mat = theDetector.material(x_cut2.materialStr());
                xml_dim_t c2_pos(x_cut2.child(_U(position)));
                xml_dim_t c2_dim(x_cut2.child(_U(dimensions)));
                cut2_pos = c2_pos;
                cut2_dim = c2_dim;*/
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
            //dd4hep::Box cut1_solid(cut1_dim.dx(),cut1_dim.dy(),cut_halfZ);
            dd4hep::Box BC420_solid(BC420_dim.dx(),BC420_dim.dy(),BC420_halfZ);
            dd4hep::Tube cut3_solid(cut3_dim.rmin(),cut3_dim.rmax(),cut_halfZ);
            //dd4hep::Box cut2_solid(cut2_dim.dx(),cut2_dim.dy(),cut_halfZ);
            dd4hep::Box SiPM_solid(SiPM_dim.dx(),SiPM_dim.dy(),SiPM_dim.dz());

            dd4hep::Transform3D surface_transform(dd4hep::Rotation3D(),dd4hep::Position(surface_pos.x(),surface_pos.y(),surface_pos.z()));
            //dd4hep::Transform3D cut1_transform(dd4hep::Rotation3D(),dd4hep::Position(cut1_pos.x(),cut1_pos.y(),cut1_pos.z()));
            dd4hep::Transform3D BC420_transform(dd4hep::Rotation3D(),dd4hep::Position(BC420_pos.x(),BC420_pos.y(),BC420_pos.z()));
            //dd4hep::Transform3D cut2_transform(dd4hep::Rotation3D(),dd4hep::Position(cut2_pos.x(),cut2_pos.y(),cut2_pos.z()));
            dd4hep::Transform3D cut3_transform(dd4hep::Rotation3D(),dd4hep::Position(cut3_pos.x(),cut3_pos.y(),cut3_pos.z()));
            dd4hep::Transform3D SiPM_transform1(dd4hep::Rotation3D(),dd4hep::Position(SiPM_pos.x(),SiPM_pos.y(),SiPM_posZ));
            dd4hep::Transform3D SiPM_transform2(dd4hep::Rotation3D(),dd4hep::Position(SiPM_pos.x(),SiPM_pos.y(), -1 * SiPM_posZ));

            dd4hep::Volume surface_vol(surface_name, surface_solid, surface_mat);
            surface_vol.setVisAttributes(theDetector.visAttributes(surface_vis));
            //surface_vol.setSensitiveDetector(sens);
/*
            dd4hep::Volume cut1_vol(cut1_name, cut1_solid, cut1_mat);
            cut1_vol.setVisAttributes(theDetector.visAttributes(cut1_vis));

            dd4hep::Volume cut2_vol(cut2_name, cut2_solid, cut2_mat);
            cut2_vol.setVisAttributes(theDetector.visAttributes(cut2_vis));
*/
            dd4hep::Volume cut3_vol(cut3_name, cut3_solid, cut3_mat);
            cut3_vol.setVisAttributes(theDetector.visAttributes(cut3_vis));

            dd4hep::Volume BC420_vol(BC420_name,BC420_solid,BC420_mat);
            BC420_vol.setVisAttributes(theDetector.visAttributes(BC420_vis));
            BC420_vol.setSensitiveDetector(sens);

            dd4hep::Volume SiPM_vol(SiPM_name, SiPM_solid, SiPM_mat);
            SiPM_vol.setVisAttributes(theDetector.visAttributes(SiPM_vis));
            //SiPM_vol.setSensitiveDetector(sens);

            //BC420_vol.placeVolume(cut2_vol,cut2_transform);
            BC420_vol.placeVolume(cut3_vol,cut3_transform);
            dd4hep::PlacedVolume cladding_place, core_place;
            for(xml_coll_t dcSection(x_strip,Unicode("fiber")); dcSection; dcSection++)
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
                cladding_place = cut3_vol.placeVolume(section_vol,section_transform);
                //cladding_place = BC420_vol.placeVolume(section_vol,section_transform);
              }
              if (x_section.id() == 1)
              {
                core_place = cut3_vol.placeVolume(section_vol,section_transform);
                //core_place = BC420_vol.placeVolume(section_vol,section_transform);
              }
            }
            //surface_vol.placeVolume(cut1_vol,cut1_transform);
            dd4hep::PlacedVolume BC420_place = surface_vol.placeVolume(BC420_vol,BC420_transform);
            //BC420_place.addPhysVolID("Stripe",i2+1);
            dd4hep::PlacedVolume surf_place = strip_vol.placeVolume(surface_vol,surface_transform);
            int lnum = ( ( 2 * ( i0 % 2 ) - 1 ) * ( 2 * i1 - 1 ) + 1 ) / 2;
            //surf_place.addPhysVolID("Layer", lnum+1);
            //surf_place.addPhysVolID("SiPM",0);
            dd4hep::PlacedVolume SiPM_place1 = strip_vol.placeVolume(SiPM_vol,SiPM_transform1);
            SiPM_place1.addPhysVolID("SiPM",1);
            dd4hep::PlacedVolume SiPM_place2 = strip_vol.placeVolume(SiPM_vol,SiPM_transform2);
            SiPM_place2.addPhysVolID("SiPM",2);
	    dd4hep::PlacedVolume strip_place = env_vol.placeVolume(strip_vol,strip_transform);
	    strip_place.addPhysVolID("Stripe", i2+1).addPhysVolID("Layer", lnum+1);
            dd4hep::OpticalSurfaceManager surfMgr = theDetector.surfaceManager();
            std::string optical_surf = strip_name + "_surf";
            std::string optical_fiber = strip_name + "_fiber";
            dd4hep::OpticalSurface Surf_stripe = surfMgr.opticalSurface("Muon_surf_stripe");
            dd4hep::OpticalSurface Surf_fiber = surfMgr.opticalSurface("Muon_surf_fiber");
            dd4hep::BorderSurface surface_stripe( theDetector, sdet, optical_surf, Surf_stripe, BC420_place, surf_place);
            dd4hep::BorderSurface surface_fiber( theDetector, sdet, optical_fiber, Surf_fiber, core_place, cladding_place);
          }
          for ( int i3 = 0; i3 < theDetector.constant<int>("Muon_endcap_strip_num_2"); i3++ )
          {
            double strip_posX, strip_posY, strip_posZ;
            double strip_sizeZ, surface_halfZ, BC420_halfZ, fiber_halfZ, cut_halfZ;
            dd4hep::Rotation3D strip_rot;
            if ( i1 == 1 )
            {
              strip_rot = dd4hep::Rotation3D( dd4hep::RotationX( 90 * dd4hep::degree ) );
              strip_posX = theDetector.constant<double>("Muon_strip_x") * ( i3 + 0.5 );
              strip_posZ = 0.5 * ( 1 - 2 * ( i0 % 2 ) ) * theDetector.constant<double>("Muon_strip_y");
              if ( i3 < theDetector.constant<int>("Muon_endcap_strip_num_cut_2") )
              {
              strip_sizeZ = std::sqrt( r2 * r2 - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") ) - std::sqrt( r_gap * r_gap - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") );
              strip_posY = std::sqrt( r2 * r2 - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") ) - 0.5 * strip_sizeZ; 
              }
              if ( i3 >= theDetector.constant<int>("Muon_endcap_strip_num_cut_2") )
              {
                strip_sizeZ = std::sqrt( r2 * r2 - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") );
                strip_posY = 0.5 * strip_sizeZ;
              }

            }
            if ( i1 == 0 )
            {
              strip_rot = dd4hep::Rotation3D( dd4hep::RotationX( 90 * dd4hep::degree ) * dd4hep::RotationY( 90 * dd4hep::degree ) );
              strip_posY = theDetector.constant<double>("Muon_strip_x") * ( i3 + 0.5 );
              strip_posZ = 0.5 * ( 2 * ( i0 % 2 ) - 1 ) * theDetector.constant<double>("Muon_strip_y");
              if ( i3 < theDetector.constant<int>("Muon_endcap_strip_num_cut_2") )
              {
                strip_sizeZ = std::sqrt( r2 * r2 - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") ) - std::sqrt( r_gap * r_gap - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") );
                strip_posX = std::sqrt( r2 * r2 - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") ) - 0.5 * strip_sizeZ;
              }
              if ( i3 >= theDetector.constant<int>("Muon_endcap_strip_num_cut_2") )
              {
                strip_sizeZ = std::sqrt( r2 * r2 - i3 * theDetector.constant<double>("Muon_strip_x") * i3 * theDetector.constant<double>("Muon_strip_x") );
                strip_posX = 0.5 * strip_sizeZ;
              }

            }
            double strip_halfZ = 0.5 * strip_sizeZ;
            double SiPM_posZ = strip_halfZ - 0.5 * theDetector.constant<double>("Muon_strip_surf");
            surface_halfZ = BC420_halfZ = fiber_halfZ = cut_halfZ = strip_halfZ - theDetector.constant<double>("Muon_strip_surf"); 
            xml_coll_t dcStrip(x_det,Unicode("stripe"));
            xml_comp_t x_strip = dcStrip;
            std::string strip_name = x_strip.nameStr() + dd4hep::_toString(2,"_%d") + dd4hep::_toString(i,"_%d") + dd4hep::_toString(i0,"_%d") + dd4hep::_toString(i1,"_%d") + dd4hep::_toString(i3,"_%d");
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
            for(xml_coll_t dcModule(x_strip,Unicode("component")); dcModule; dcModule++)
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

                xml_coll_t dcCut3(x_module,Unicode("cut"));
                xml_comp_t x_cut3 = dcCut3;
                cut3_name = x_cut3.nameStr();
                cut3_vis = x_cut3.visStr();
                cut3_mat = theDetector.material(x_cut3.materialStr());
                xml_dim_t c3_pos(x_cut3.child(_U(position)));
                xml_dim_t c3_dim(x_cut3.child(_U(dimensions)));
                cut3_pos = c3_pos;
                cut3_dim = c3_dim;

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
            dd4hep::Box BC420_solid(BC420_dim.dx(),BC420_dim.dy(),BC420_halfZ);
            dd4hep::Tube cut3_solid(cut3_dim.rmin(),cut3_dim.rmax(),cut_halfZ);
            dd4hep::Box SiPM_solid(SiPM_dim.dx(),SiPM_dim.dy(),SiPM_dim.dz());

            dd4hep::Transform3D surface_transform(dd4hep::Rotation3D(),dd4hep::Position(surface_pos.x(),surface_pos.y(),surface_pos.z()));
            dd4hep::Transform3D BC420_transform(dd4hep::Rotation3D(),dd4hep::Position(BC420_pos.x(),BC420_pos.y(),BC420_pos.z()));
            dd4hep::Transform3D cut3_transform(dd4hep::Rotation3D(),dd4hep::Position(cut3_pos.x(),cut3_pos.y(),cut3_pos.z()));
            dd4hep::Transform3D SiPM_transform1(dd4hep::Rotation3D(),dd4hep::Position(SiPM_pos.x(),SiPM_pos.y(),SiPM_posZ));
            dd4hep::Transform3D SiPM_transform2(dd4hep::Rotation3D(),dd4hep::Position(SiPM_pos.x(),SiPM_pos.y(), -1 * SiPM_posZ));

            dd4hep::Volume surface_vol(surface_name, surface_solid, surface_mat);
            surface_vol.setVisAttributes(theDetector.visAttributes(surface_vis));

            dd4hep::Volume cut3_vol(cut3_name, cut3_solid, cut3_mat);
            cut3_vol.setVisAttributes(theDetector.visAttributes(cut3_vis));

            dd4hep::Volume BC420_vol(BC420_name,BC420_solid,BC420_mat);
            BC420_vol.setVisAttributes(theDetector.visAttributes(BC420_vis));
            BC420_vol.setSensitiveDetector(sens);

            dd4hep::Volume SiPM_vol(SiPM_name, SiPM_solid, SiPM_mat);
            SiPM_vol.setVisAttributes(theDetector.visAttributes(SiPM_vis));
            //SiPM_vol.setSensitiveDetector(sens);

            BC420_vol.placeVolume(cut3_vol,cut3_transform);
            dd4hep::PlacedVolume cladding_place, core_place;
            for(xml_coll_t dcSection(x_strip,Unicode("fiber")); dcSection; dcSection++)
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
                cladding_place = cut3_vol.placeVolume(section_vol,section_transform);
                
              }
              if (x_section.id() == 1)
              {
                core_place = cut3_vol.placeVolume(section_vol,section_transform);
                
              }
            }

            dd4hep::PlacedVolume BC420_place = surface_vol.placeVolume(BC420_vol,BC420_transform);

            dd4hep::PlacedVolume surf_place = strip_vol.placeVolume(surface_vol,surface_transform);
            int lnum = ( ( 2 * ( i0 % 2 ) - 1 ) * ( 2 * i1 - 1 ) + 1 ) / 2;

            dd4hep::PlacedVolume SiPM_place1 = strip_vol.placeVolume(SiPM_vol,SiPM_transform1);
           // SiPM_place1.addPhysVolID("SiPM",1);
            dd4hep::PlacedVolume SiPM_place2 = strip_vol.placeVolume(SiPM_vol,SiPM_transform2);
           // SiPM_place2.addPhysVolID("SiPM",2);
	    dd4hep::PlacedVolume strip_place = env_vol.placeVolume(strip_vol,strip_transform);
	    strip_place.addPhysVolID("Stripe", theDetector.constant<int>("Muon_endcap_strip_num_1")+i3+1).addPhysVolID("Layer", lnum+1);
            dd4hep::OpticalSurfaceManager surfMgr = theDetector.surfaceManager();
            std::string optical_surf = strip_name + "_surf";
            std::string optical_fiber = strip_name + "_fiber";
            dd4hep::OpticalSurface Surf_stripe = surfMgr.opticalSurface("Muon_surf_stripe");
            dd4hep::OpticalSurface Surf_fiber = surfMgr.opticalSurface("Muon_surf_fiber");
            dd4hep::BorderSurface surface_stripe( theDetector, sdet, optical_surf, Surf_stripe, BC420_place, surf_place);
            dd4hep::BorderSurface surface_fiber( theDetector, sdet, optical_fiber, Surf_fiber, core_place, cladding_place);
          }
        }
        dd4hep::PlacedVolume env_place = superlayer_vol.placeVolume(env_vol,env_transform);
        env_place.addPhysVolID("Env",i0+1);
      }
      dd4hep::PlacedVolume superlayer_place = envelope.placeVolume(superlayer_vol,superlayer_transform);
      superlayer_place.addPhysVolID("Superlayer",i+1);
    }
    dd4hep::PlacedVolume pv;
    dd4hep::DetElement both_endcap(det_name,2000);
    dd4hep::Volume motherVol = theDetector.pickMotherVolume(both_endcap);
    dd4hep::DetElement sdetA = sdet;
    dd4hep::Ref_t(sdetA)->SetName((det_name+"_A").c_str());
    dd4hep::DetElement sdetB = sdet.clone(det_name+"_B",2002);
    pv = assembly.placeVolume(envelope,dd4hep::Transform3D(dd4hep::Rotation3D(dd4hep::RotationZ(90*dd4hep::degree)),dd4hep::Position(0,0,pos.y())));
    pv.addPhysVolID("Endcap",1);

    sdetA.setPlacement(pv);

    pv = assembly.placeVolume(envelope,dd4hep::Transform3D(dd4hep::Rotation3D(dd4hep::RotationZ(90*dd4hep::degree) * dd4hep::RotationY(180*dd4hep::degree)),dd4hep::Position(0,0,-1 * pos.y())));
    pv.addPhysVolID("Endcap",2);

    sdetB.setPlacement(pv);

    pv = motherVol.placeVolume(assembly);
    pv.addPhysVolID("system",x_det.id());
    both_endcap.setPlacement(pv);
    both_endcap.add(sdetA);
    both_endcap.add(sdetB);

    MYDEBUG("create_detector DONE. ");
    return both_endcap;
}

DECLARE_DETELEMENT(Muon_Endcap_v01_02, create_detector)
