/*********************************************************************
 * Author           : Lan-sx & origin author: F. Gaede, Desy
 * Email            : shexin@ihep.ac.cn
 * Last modified    : 2024-06-02 20:37
 * Filename         : TPC_ModularEndcap_o1_v01.cpp
 * Description      : 
 * ******************************************************************/

#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetType.h"
//#include "./include/LcgeoExceptions.h"
//#include "./include/lcgeo.h"
#include "DDRec/Surface.h"
#include "DDRec/DetectorData.h"
#include "XML/Utilities.h"
//#include "XMLHandlerDB.h"

#include <math.h>

using namespace std;
using namespace dd4hep;
//using namespace lcgeo;

using dd4hep::rec::Vector3D;
using dd4hep::rec::VolCylinder;
using dd4hep::rec::SurfaceType;
using dd4hep::rec::volSurfaceList;
using dd4hep::rec::VolPlane;
using dd4hep::rec::FixedPadSizeTPCData;
using dd4hep::rec::ConicalSupportData;

/** Construction of TPC detector, ported from Mokka driver TPC10.cc
 * Mokka History:
 * - modified version of TPC driver by Ties Behnke
 * - modified version of TPC02 as TPC03 with selectable chamber gas -- Adrian Vogel, 2005-06-09
 * - modified version of TPC03 as TPC04 with limit of step length   -- Adrian Vogel, 2006-02-01
 * - introduced self-scalability, no superdriver needed anymore     -- Adrian Vogel, 2006-03-11
 * - modified version of TPC04 as TPC05 in order to have full MC
 *   information both at entry and exit hits in the TPC ,
 *   more realistic central electrode and endplate             -- Predrag Krstonosic, 2006-07-12
 * - implemented new GEAR interface -- K. Harder, T. Pinto Jayawardena                2007-07-31
 * - TPC10 implemented readout within the Gas volume and layered inner and outer wall -- SJA -- 2010-11-19
 *
 *  @author: F.Gaede, DESY, Nov 2013
 *
 * - Modular Endcap TPC Geo implemention for CEPC TDR TPC
 *  @author: X.She, IHEP, May 2024
 */

static Ref_t create_element(Detector& theDetector, xml_h e, SensitiveDetector sens)  {


    //------------------------------------------
    //  See comments starting with '//**' for
    //     hints on porting issues
    //------------------------------------------
    xml_det_t    x_det = e;
    string       name  = x_det.nameStr();

    DetElement   tpc(  name, x_det.id()  ) ;

    // --- create an envelope volume and position it into the world ---------------------

    Volume envelope = dd4hep::xml::createPlacedEnvelope( theDetector,  e , tpc ) ;

    dd4hep::xml::setDetectorTypeFlag( e, tpc ) ;

    if( theDetector.buildType() == BUILD_ENVELOPE ) return tpc ;

    //-----------------------------------------------------------------------------------

    PlacedVolume pv;  

    sens.setType("tracker");

    std::cout << " **Lan Lan building TPC_ModularEndcap_TDR_o1_v01 construction" << std::endl ;

    //######################################################################################################################################################################
    const double phi1 =   0.0 ;
    const double phi2 =  2*M_PI ;
    const double dzTotal            = theDetector.constant<double>("TPC_half_length") * 2. ; 
    const double rInner             = theDetector.constant<double>("TPC_inner_radius") ;
    const double rOuter             = theDetector.constant<double>("TPC_outer_radius") ;
    const double drInnerWall        = theDetector.constant<double>("TPC_dr_InnerWall");
    const double drOuterWall        = theDetector.constant<double>("TPC_dr_OuterWall");
    const double dz_Cathode         = theDetector.constant<double>("TPC_dz_Cathode");
    const double dz_Endplate        = theDetector.constant<double>("TPC_dz_Endplate");
    const double dz_Readout         = theDetector.constant<double>("TPC_dz_Readout");
    const double tpcpadheight       = theDetector.constant<double>("TPC_pad_height");
    const double tpcpadwidth        = theDetector.constant<double>("TPC_pad_width");
    const int    tpcnumberOfPadRows = theDetector.constant<int>("TPC_numberOfPadrows");


    std::cout<< "============ TPC_HoneyComb_TDR_o1_v01 mother Volume(Tube) (Dz,Ri,Ro) : (" << dzTotal/dd4hep::mm/2. << "\t" 
                                                                    << rInner/dd4hep::mm  << "\t"
                                                                    << rOuter/dd4hep::mm  <<" )"<<std::endl;
    Material materialT2Kgas  = theDetector.material("T2KGas1");
    Material materialAir     = theDetector.material("Air");
    Material materialAlframe = theDetector.material("TPC_Alframe");
    //-------------------------------------------------------------------------------------------------------//
    //-------------------------------- TPC mother volume ----------------------------------------------------//
    //------------ Volume for the whole TPC, Field Cage, Cathode, and Endplate and Sensitive ----------------//

    Tube tpc_motherSolid(rInner ,rOuter ,dzTotal/2.0 , phi1 , phi1+phi2 ); 
    Volume tpc_motherLog(  "TPCLog", tpc_motherSolid, materialT2Kgas);
    pv = envelope.placeVolume( tpc_motherLog ) ;
    tpc.setVisAttributes(theDetector,  "TPCMotherVis1" ,  tpc_motherLog ) ;

    double gasRegion = ((rOuter-drOuterWall)-(rInner+drInnerWall))/dd4hep::mm;
    std::cout << "================================================================"<< std::endl; 
    std::cout << "TPC_HoneyComb_TDR_o1_v01: Total Gas material corresponds to " << ( ( gasRegion ) / (materialT2Kgas->GetMaterial()->GetRadLen() / dd4hep::mm ) * 100.0 ) 
              << "% of a radiation length." << std::endl;
    std::cout << "================================================================"<< std::endl; 
    
    //-------------------------------------------------------------------------------------------------------//
    //Loop all sections
    //-------------------------------------------------------------------------------------------------------//

    for(xml_coll_t si(x_det, Unicode("component"));si;++si)
    {
        xml_comp_t x_section(si);
        std::string types = x_section.attr<std::string>(_Unicode(type));
        const std::string volName = x_section.nameStr();

        //-------------------------------- inner/outer wall construction ----------------------------------------//
        if(types == "TPCinnerWall" || types == "TPCouterWall")
        {
            double r_start = x_section.attr<double>(_Unicode(R_start));
            double r_end =  x_section.attr<double>(_Unicode(R_end));
            double z_fulllength = x_section.attr<double>(_Unicode(Z_fulllength));

            //Create Inner/Outer Wall mother logic volume
            std::string volNameLog = volName + "Log";
            Tube WallSolid(r_start,r_end,z_fulllength/2., phi1 , phi1+phi2);
            Volume WallLog(volNameLog,WallSolid,materialT2Kgas);
            pv=tpc_motherLog.placeVolume(WallLog);
            tpc.setVisAttributes(theDetector,"CyanVis",WallLog);
            
            Vector3D ocyl;
            //SurfaceList data, same as TPC10(CEPCV4)
            double dr_wall = r_end-r_start;
            if(types == "TPCinnerWall")
                ocyl.fill(r_start+0.5*dr_wall,0.,0.);
            else
                ocyl.fill(r_end-0.5*dr_wall,0.,0.);

            //std::cout<<"======>   Vector3D cout : "<<ocyl.x()<<"\t"<<ocyl.y()<<"\t"<<ocyl.z()<<std::endl;
            VolCylinder surfWall(WallLog,SurfaceType( SurfaceType::Helper ),0.5*dr_wall,0.5*dr_wall,ocyl);
            volSurfaceList( tpc )->push_back( surfWall );

            //Loop all layers of inner/outer wall
            int ilayer =0;
            double rCursor = r_start;
            double fracRadLengthWall = 0.;
            for(xml_coll_t li(x_section,_U(layer)); li;++li,++ilayer)
            {
                xml_comp_t x_layer(li);

                double thickness = x_layer.thickness();
                Material layerMaterial = theDetector.material( x_layer.materialStr() );
                char suffix[20];
                sprintf(suffix,"_%d",ilayer);
                std::string layerName = volNameLog + suffix;

                Tube layerSolid(rCursor,rCursor+thickness,z_fulllength/2.,phi1,phi1+phi2);
                Volume layerLog(layerName,layerSolid,layerMaterial);
                //layerLog.setVisAttributes(theDetector,x_layer.visStr());
                pv=WallLog.placeVolume(layerLog);

                rCursor += thickness; 
                double layerRadLength = thickness/(layerMaterial->GetMaterial()->GetRadLen());
                fracRadLengthWall += layerRadLength; 

                std::cout<<"-> "<<volName<<"layer"<<ilayer<<" : "<< thickness/dd4hep::mm << "mm \t Materials: "<<layerMaterial.name()
                         <<" X0= "<<layerMaterial->GetMaterial()->GetRadLen()/dd4hep::mm<<"mm \t"
                         <<layerRadLength<<" X0"<<std::endl;
            }
            
            double drSumThickness = rCursor - r_start;
            if(drSumThickness > (r_end-r_start))
            {
                std::cout<<"Warning!  sum_{i}layerThickness_{i} > drWall !\n"<<std::endl;
                throw "$!!! TPC_ModularEndcap_TDR_o1_v01: Overfull TPC Wall - check your xml file -component <TPCInnerWall/TPCOuterWall>";
            }

            std::cout << "================================================================"<< std::endl; 
            std::cout<<"=====>$ "<<volName<<" material corresponds to "<< int(fracRadLengthWall*1000)/10. << "% of a radiation length."<<std::endl;
            std::cout<<"=====>$ "<<volName<<" effective X0=  "<<std::setw(4)<< (r_end-r_start)/fracRadLengthWall <<" cm "<<std::endl; 
            std::cout<<"=====>$ Sum of layer thickness = "<< drSumThickness/dd4hep::mm <<" mm "<<" \t Wall thickness = "<<(r_end-r_start)/dd4hep::mm <<" mm "<<std::endl;
            std::cout << "================================================================"<< std::endl; 
        }
        //-------------------------------- TPCGrip construction ----------------------------------------//
        if(types == "TPCGrip")
        {
            Material gripMaterial = theDetector.material(x_section.attr<std::string>(_Unicode(material))) ;
            for(xml_coll_t li(x_section,_U(layer));li;++li)
            {
                xml_comp_t x_layer(li);

                //std::string volNameLog = x_layer.nameStr()+"Log"; 
                Tube gripSolid(x_layer.rmin(),x_layer.rmax(),x_layer.z_length()/2., phi1 , phi1+phi2);
                Volume gripLog(x_layer.nameStr()+"Log",gripSolid,gripMaterial);
                pv=tpc_motherLog.placeVolume(gripLog);
                tpc.setVisAttributes(theDetector,x_layer.visStr(),gripLog);
                std::cout << "================================================================"<< std::endl; 
                std::cout<<"=====>$ "<<x_layer.nameStr()<<" Constructed ! "<<std::endl;
                std::cout << "================================================================"<< std::endl; 
            }
        }
        //-------------------------------- TPCCathode construction ----------------------------------------//
        if(types == "TPCCathode")
        {
            for(xml_coll_t li(x_section,_U(layer));li;++li)
            {
                xml_comp_t x_layer(li);
                Material cathodeMaterial = theDetector.material( x_layer.materialStr());
                
                Tube cathodeSolid(x_layer.rmin(),x_layer.rmax(),x_layer.z_length()/2,phi1,phi1+phi2);
                Volume cathodeLog(x_layer.nameStr()+"Log",cathodeSolid,cathodeMaterial);

                for(xml_coll_t pj(x_layer,_U(position));pj;++pj)
                {
                    xml_dim_t x_pos(pj);
                    pv = tpc_motherLog.placeVolume(cathodeLog,Position(x_pos.x(),x_pos.y(),x_pos.z()));
                    tpc.setVisAttributes(theDetector, x_layer.visStr(),cathodeLog);
                    std::cout<<"============>Cathod Z Position: "<<x_pos.z() / dd4hep::mm <<" mm "<<std::endl;
                }
            }
        }
        //-------------------------------- TPC Sensitive Volume construction ----------------------------------------//
        if(types == "TPCSensitiveVol")
        {
            //Material T2KgasMaterial = theDetector.material(x_section.attr<std::string>(_Unicode(material))) ;
            std::cout<<"============>T2K gas RadLen= "<< materialT2Kgas->GetMaterial()->GetRadLen()/dd4hep::mm<<" mm"<<std::endl;

            xml_dim_t dimSD = x_section.dimensions();
            std::cout<<"============>rmin,rmax,dz "<< dimSD.rmin()<<"\t"<<dimSD.rmax()<<"\t"<<dimSD.z_length()<<std::endl; 

            Tube sensitiveGasSolid(dimSD.rmin(),dimSD.rmax(),dimSD.z_length()/2.,phi1,phi1+phi2);
            Volume sensitiveGasLog(volName+"Log",sensitiveGasSolid,materialT2Kgas);

            DetElement sensGasDEfwd(tpc, "tpc_senGas_fwd",x_det.id());
            DetElement sensGasDEbwd(tpc, "tpc_senGas_bwd",x_det.id());

            pv = tpc_motherLog.placeVolume(sensitiveGasLog,Transform3D(RotationY(0.),Position(0,0,+(dz_Cathode/2+dimSD.z_length()/2.))));
            pv.addPhysVolID("side",+1);
            sensGasDEfwd.setPlacement(pv);

            pv = tpc_motherLog.placeVolume(sensitiveGasLog,Transform3D(RotationY(pi),Position(0,0,-(dz_Cathode/2+dimSD.z_length()/2.))));
            pv.addPhysVolID("side",-1);
            sensGasDEbwd.setPlacement(pv);

            tpc.setVisAttributes(theDetector, "gasVis", sensitiveGasLog);
            
            //Pad row doublets construction
            xml_coll_t cc(x_section,_U(layer));
            xml_comp_t x_layer = cc;
            int numberPadRows = x_layer.repeat();
            double padHeight  = x_layer.thickness();
            std::cout<<"##################$$$$$$$$$$$$$$ Number of Pad Rows: > "<<numberPadRows<<"\t padHeight= "<<padHeight/dd4hep::mm<<" mm"<<std::endl;
            
            //Sensitive Volume construction  : readout pad layers
            for(int ilayer=0; ilayer < numberPadRows; ++ilayer)
            {
                // create twice the number of rings as there are pads, producing an lower and upper part of the pad with the boundry between them the pad-ring centre
                const double inner_lowerlayer_radius = dimSD.rmin()+ (ilayer * (padHeight));
                const double outer_lowerlayer_radius = inner_lowerlayer_radius + (padHeight/2.0);

                const double inner_upperlayer_radius = outer_lowerlayer_radius ;
                const double outer_upperlayer_radius = inner_upperlayer_radius + (padHeight/2.0);

                Tube lowerlayerSolid( inner_lowerlayer_radius, outer_lowerlayer_radius, dimSD.z_length() / 2.0, phi1, phi2);
                Tube upperlayerSolid( inner_upperlayer_radius, outer_upperlayer_radius, dimSD.z_length() / 2.0, phi1, phi2);

                Volume lowerlayerLog( _toString( ilayer ,"TPC_lowerlayer_log_%02d") ,lowerlayerSolid, materialT2Kgas );
                Volume upperlayerLog( _toString( ilayer ,"TPC_upperlayer_log_%02d") ,upperlayerSolid, materialT2Kgas );

                tpc.setVisAttributes(theDetector,  "Invisible" ,  lowerlayerLog) ;
                tpc.setVisAttributes(theDetector,  "Invisible" ,  upperlayerLog) ;
                //tpc.setVisAttributes(theDetector,  "RedVis" ,  lowerlayerLog) ;
                //tpc.setVisAttributes(theDetector,  "RedVis" ,  upperlayerLog) ;

                DetElement   layerDEfwd( sensGasDEfwd ,   _toString( ilayer, "tpc_row_fwd_%03d") , x_det.id() );
                DetElement   layerDEbwd( sensGasDEbwd ,   _toString( ilayer, "tpc_row_bwd_%03d") , x_det.id() );

                Vector3D o(  inner_upperlayer_radius + 1e-10  , 0. , 0. ) ;
                // create an unbounded surface (i.e. an infinite cylinder) and assign it to the forward gaseous volume only
                VolCylinder surf( upperlayerLog , SurfaceType(SurfaceType::Sensitive, SurfaceType::Invisible, SurfaceType::Unbounded ) ,  (padHeight/2.0) ,  (padHeight/2.0) ,o ) ;

                volSurfaceList( layerDEfwd )->push_back( surf ) ;
                // volSurfaceList( layerDEbwd )->push_back( surf ) ;

                pv = sensitiveGasLog.placeVolume( lowerlayerLog ) ;
                pv.addPhysVolID("layer", ilayer ).addPhysVolID( "module", 0 ).addPhysVolID("sensor", 1 ) ;

                pv = sensitiveGasLog.placeVolume( upperlayerLog ) ;
                pv.addPhysVolID("layer", ilayer ).addPhysVolID( "module", 0 ).addPhysVolID("sensor", 0 ) ;
                layerDEfwd.setPlacement( pv ) ;
                layerDEbwd.setPlacement( pv ) ;

                lowerlayerLog.setSensitiveDetector(sens);
                upperlayerLog.setSensitiveDetector(sens);
            }

        }
        //-------------------------------- TPC Endplate and readout construction ----------------------------------------//
        if(types == "TPCEndplate")
        {
            xml_dim_t dimEndCap = x_section.dimensions();
            std::cout<<"============>(rmin,rmax,dz): "<< dimEndCap.rmin() / dd4hep::mm<<"mm "
                                                      << dimEndCap.rmax() / dd4hep::mm<<" mm "
                                                      << dimEndCap.z_length() / dd4hep::mm<< " mm" <<std::endl; 
            //Create endcap Log volume
            Tube endcapSolid(dimEndCap.rmin(),dimEndCap.rmax(),dimEndCap.z_length()/2.,phi1,phi1+phi2);
            Volume endcapLog(volName+"Log",endcapSolid,materialAir);

            DetElement endcapDEfwd(tpc, "tpc_endcap_fwd",x_det.id());
            DetElement endcapDEbwd(tpc, "tpc_endcap_bwd",x_det.id());

            //Vectors for endplate plane
            Vector3D u(0.,1.,0.);
            Vector3D v(1.,0.,0.);
            Vector3D n(0.,0.,1.);

            ////need to set the origin of the helper plane to be inside the material (otherwise it would pick up the vacuum at the origin)
            double mid_r = 0.5*(rOuter + rInner);
            Vector3D o(0., mid_r, 0.);

            VolPlane surf( endcapLog, SurfaceType( SurfaceType::Helper ), (dz_Endplate+dz_Readout)/2.,dz_Endplate/2.,u,v,n,o);
            volSurfaceList(endcapDEfwd) -> push_back(surf);
            volSurfaceList(endcapDEbwd) -> push_back(surf);
            
            pv = tpc_motherLog.placeVolume(endcapLog,Transform3D(RotationY(0.),Position(0,0,+(dzTotal/2.-dz_Endplate/2.))));
            endcapDEfwd.setPlacement(pv);

            pv = tpc_motherLog.placeVolume(endcapLog,Transform3D(RotationY(pi),Position(0,0,-(dzTotal/2.-dz_Endplate/2.))));
            endcapDEbwd.setPlacement(pv);
            
            tpc.setVisAttributes(theDetector, "transVis", endcapLog);

            //================================================================================================
            //Modular Endplate construction
            //================================================================================================
            double dz_Endpaltelength     = dimEndCap.z_length();
            double r_start        = dimEndCap.rmin();  
            double ds_reinforce   = x_section.attr<double>(_Unicode(s_frame));
            double dz_Alframe     = x_section.attr<double>(_Unicode(z_frame));
            double rCursor        = r_start;

            //Loop all layers to construct frame-module
            for(xml_coll_t ilayer(x_section,_U(layer)); ilayer; ++ilayer)
            {
                xml_comp_t x_layer(ilayer);

                const std::string layerName = x_layer.nameStr();
                const std::string layerType = x_layer.attr<std::string>(_Unicode(type));
                double layerThickness       = x_layer.thickness();

                double r_end = rCursor + layerThickness;
                std::cout<<"===============>$ "<<layerName<<"\t"<<layerType
                    <<" thickness = "<<layerThickness / dd4hep::mm << "mm "
                    <<" inner radius = "<<rCursor / dd4hep::mm<<" mm"
                    <<" outer radius = "<<r_end /dd4hep::mm<<" mm"
                    <<std::endl;

                //--------------------------------------
                if(layerType == "Frame")
                {
                    double phi_start = 0.;
                    double phi_end   = 2*M_PI;
                    Tube   ringSolid(rCursor, r_end, dz_Alframe/2., phi_start,  phi_end) ;
                    Volume ringLog( layerName+"Log", ringSolid, materialAlframe) ;
                    pv = endcapLog.placeVolume( ringLog, Position(0., 0., -dz_Endpaltelength/2. + dz_Alframe) ) ;
                    tpc.setVisAttributes(theDetector,"GrayVis",ringLog);
                }
                if(layerType == "Module")
                {
                    int numberofModules = x_layer.repeat(); 
                    double phi_start    = x_layer.phi0_offset();
                    double phiCursor    = phi_start;
                    double phiModule    = (2*M_PI*rCursor-numberofModules*ds_reinforce)/numberofModules/rCursor;
                    double phiReinforce = ds_reinforce/rCursor;

                    //Construct each module 
                    for(int k=0; k<numberofModules;++k)
                    {
                        Tube moduleSolid1(rCursor,r_end,dz_Endpaltelength/2.,phiCursor,phiCursor+phiModule);
                        std::string moduleLogName1 = layerName + _toString(k,"Log%00d");
                        Volume moduleLog1(moduleLogName1,moduleSolid1,materialAir);

                        double z_cursor = -dz_Endpaltelength/2.; 
                        int    m_sli = 0;
                        for(xml_coll_t sli(x_layer,_U(slice)); sli; ++sli,++m_sli)
                        {
                            xml_comp_t x_slice = sli; 
                            double dz_modulepiece = sli.attr<double>(_Unicode(dz));
                            std::string moduleSliceName = moduleLogName1 + _toString(m_sli,"slice%d");
                            Material slice_mat = theDetector.material(x_slice.materialStr());

                            Tube moduleSliceSolid(rCursor,r_end,dz_modulepiece/2.,phiCursor,phiCursor+phiModule);
                            Volume moduleSliceLog(moduleSliceName,moduleSliceSolid,slice_mat);

                            pv = moduleLog1.placeVolume(moduleSliceLog,Position(0.,0.,z_cursor+dz_modulepiece/2.));
                            tpc.setVisAttributes(theDetector,  "Invisible" ,  moduleSliceLog) ;
                            z_cursor += dz_modulepiece;

                            if(z_cursor > dz_Endpaltelength/2.)
                            {
                                //std::cout<<" Warning ! TPC_ModularEndcap_TDR_o1_v01: Overfull TPC Module- check your xml file - section <Endpalte>." <<std::endl;
                                throw " $!!! TPC_ModularEndcap_TDR_o1_v01: Overfull TPC Module- check your xml file - component <Endpalte>.";
                            }
                        }

                        pv = endcapLog.placeVolume(moduleLog1);
                        tpc.setVisAttributes(theDetector,"RedVis",moduleLog1);

                        phiCursor = phiCursor+phiModule;

                        //Construct the Al frame between each module 
                        Tube moduleSolid2(rCursor,r_end,dz_Alframe/2.,phiCursor,phiCursor+phiReinforce);
                        std::string moduleLogName2 = layerName + _toString(k,"Log_rein%00d");
                        Volume moduleLog2(moduleLogName2,moduleSolid2,materialAlframe);
                        pv = endcapLog.placeVolume(moduleLog2, Position(0., 0., -dz_Endpaltelength/2.+dz_Alframe/2.));
                        tpc.setVisAttributes(theDetector,"GrayVis",moduleLog2);

                        phiCursor = phiCursor+phiReinforce;
                    }
                }
                rCursor = r_end;
            }

            double RadlenOfAl_Frame = materialAlframe->GetMaterial()->GetRadLen();
            std::cout << "================================================================"<< std::endl; 
            std::cout << "TPC_ModularEndcap_TDR_o1_v01: Endplate Al frame corresponds to " << (2. / RadlenOfAl_Frame*100)<< "% of a radiation length." << std::endl;
            std::cout << "================================================================"<< std::endl; 
        }
        //-------------------------------- TPCreadout construction ----------------------------------------//
        if(types == "TPCreadout")
        {
            xml_dim_t dimReadout = x_section.dimensions();
            double dzReadout = dimReadout.z_length();
            Tube readoutSolid(dimReadout.rmin(),dimReadout.rmax(),dimReadout.z_length()/2.,phi1,phi1+phi2);
            Volume readoutLog(volName+"Log",readoutSolid, materialT2Kgas);

            tpc.setVisAttributes(theDetector,"CyanVis",readoutLog);

            xml_dim_t posReadout = x_section.position();
            pv = tpc_motherLog.placeVolume(readoutLog,Transform3D(RotationY(0.),Position(0,0,posReadout.z())));  
            pv = tpc_motherLog.placeVolume(readoutLog,Transform3D(RotationY(pi),Position(0,0,-posReadout.z())));  

            std::cout<<"=========ReadOut dim: "<< dimReadout.rmin() / dd4hep::mm<<" mm "
                                               << dimReadout.rmax() / dd4hep::mm<<" mm "
                                               << dzReadout / dd4hep::mm <<" mm "<<std::endl;
            std::cout<<"=========ReadOut Z_pos: "<<posReadout.z() / dd4hep::mm << " mm "<<std::endl;

            int pieceCounter = 0;
            double fracRadLengthReadout = 0;
            double zCursor = -dzReadout/ 2;

            for(xml_coll_t li(x_section,_U(layer)); li;++li)
            {
                xml_comp_t  x_layer( li );
            
                const double dzPiece = x_layer.attr<double>(_Unicode(dz)); 
                Material pieceMaterial = theDetector.material( x_layer.materialStr() );
                Tube pieceSolid( dimReadout.rmin(),dimReadout.rmax(), dzPiece / 2, phi1, phi2);
                Volume pieceLog (  _toString( pieceCounter ,"TPCReadoutPieceLog_%02d"), pieceSolid, pieceMaterial ) ;

                pieceLog.setVisAttributes(theDetector,x_layer.visStr());

                pv = readoutLog.placeVolume( pieceLog  , Position(0, 0,  zCursor + dzPiece/2. ) ) ;

                ++pieceCounter;
                fracRadLengthReadout += dzPiece / pieceMaterial->GetMaterial()->GetRadLen();
                zCursor += dzPiece;
                
                std::cout<<"==========> "<<dzPiece/dd4hep::mm<<" mm Material= "<<x_layer.materialStr()<<"\t"
                         <<"X0= "<<pieceMaterial->GetMaterial()->GetRadLen()/dd4hep::mm<<"\t"
                         <<dzPiece / pieceMaterial->GetMaterial()->GetRadLen()  <<" X0"<< std::endl;

                if (zCursor > +dzReadout / 2) 
                {
                    //throw GeometryException(  "TPC11: Overfull TPC readout - check your xml file - section <readout>."   ) ;
                    //std::cout<<" TPC_ModularEndcap_TDR_o1_v01: Overfull TPC readout - check your xml file - component <TPCReadout>." <<std::endl;
                    throw " $!!! TPC_ModularEndcap_TDR_o1_v01: Overfull TPC readout - check your xml file - component <TPCReadout>.";
                }
            }
            std::cout << "================================================================"<< std::endl; 
            std::cout << "TPC_ModularEndcap_TDR_o1_v01: Readout material corresponds to " << int(fracRadLengthReadout * 1000) / 10.0 << "% of a radiation length." << std::endl;
            std::cout << "================================================================"<< std::endl; 
        }

    }
    
    //TPC data
    FixedPadSizeTPCData* tpcData = new FixedPadSizeTPCData();
    tpcData->zHalf = dzTotal/2.;
    tpcData->rMin  = rInner;
    tpcData->rMax  = rOuter;
    tpcData->innerWallThickness = drInnerWall;
    tpcData->outerWallThickness = drOuterWall;
    tpcData->rMinReadout = rInner + drInnerWall;
    tpcData->rMaxReadout = rInner + drInnerWall + tpcnumberOfPadRows*tpcpadheight;
    tpcData->maxRow = tpcnumberOfPadRows;
    tpcData->padHeight = tpcpadheight;
    tpcData->padWidth =  tpcpadwidth;
    tpcData->driftLength = dzTotal/2.- dz_Endplate - dz_Readout - dz_Cathode/2.0; // SJA: cathode has to be added as the sensitive region does not start at 0.00    
    tpcData->zMinReadout = dz_Cathode/2.0; 

    ConicalSupportData* supportData = new ConicalSupportData;

    ConicalSupportData::Section section0;
    section0.rInner = rInner + drInnerWall; 
    section0.rOuter = rOuter - drOuterWall;
    section0.zPos   = dzTotal/2. - dz_Endplate - dz_Readout; 

    ConicalSupportData::Section section1;
    section1.rInner = rInner + drInnerWall; 
    section1.rOuter = rOuter - drOuterWall;
    section1.zPos   = dzTotal/2. - dz_Endplate; 

    ConicalSupportData::Section section2;
    section2.rInner = rInner ; 
    section2.rOuter = rOuter ;
    section2.zPos   = dzTotal/2.; 

    supportData->sections.push_back(section0);
    supportData->sections.push_back(section1);
    supportData->sections.push_back(section2);


    tpc.addExtension< FixedPadSizeTPCData >(tpcData);
    tpc.addExtension< ConicalSupportData >(supportData);

    //tpc.setVisAttributes( theDetector, x_det.visStr(), envelope );
    tpc.setVisAttributes( theDetector, "TPCMotherVis1", envelope );
    //  if( tpc.isValid() ) 
    // tpc.setPlacement(pv);
    std::cout << "================================================================"<< std::endl; 
    std::cout << "TPC_ModularEndcap_TDR_o1_v01 Constructed!"<< std::endl; 
    std::cout << "================================================================"<< std::endl; 

    return tpc;
}
DECLARE_DETELEMENT(TPC_ModularEndcap_o1_v01,create_element)
