//==========================================================================
//Lumical Detector Construction
//--------------------------------------------------------------------------
// 
// Author: Sun Xingyang , NJU
//==========================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "DDRec/DetectorData.h"
#include "XML/Utilities.h"
#include "cmath"
#include "DDSegmentation/BitField64.h"
#include "DDSegmentation/TiledLayerGridXY.h"
#include "DDSegmentation/Segmentation.h"
#include "DDSegmentation/MultiSegmentation.h"
#include <vector>
#include <iostream>
#include "XML/Layering.h"
#include "DD4hep/Printout.h"

using namespace std;
using namespace dd4hep;
using namespace dd4hep::detail;
using dd4hep::Readout;
using dd4hep::Position;
using dd4hep::BUILD_ENVELOPE;
using dd4hep::Box;
using dd4hep::DetElement;
using dd4hep::Detector;
using dd4hep::IntersectionSolid;
using dd4hep::Material;
using dd4hep::PlacedVolume;
using dd4hep::Ref_t;
using dd4hep::Rotation3D;
using dd4hep::RotationZ;
using dd4hep::RotationZYX;
using dd4hep::SensitiveDetector;
using dd4hep::Transform3D;
using dd4hep::Trapezoid;
using dd4hep::Tube;
using dd4hep::Volume;
using dd4hep::_toString;

using dd4hep::rec::LayeredCalorimeterData;


static Ref_t create_detector(Detector& description, xml_h e, SensitiveDetector sens) {
    std::cout << "This is the Lumical_v01:"  << std::endl;
//get the define of LYSO cyrstals in xml
    xml_det_t x_det = e;
    string det_name = x_det.nameStr();
    int loop = 0;
    xml_coll_t LYSO_layer(x_det,_U(layer));
    xml_coll_t second_LYSO_layer(x_det,_U(layer));
    for(xml_coll_t c(x_det,_U(layer));c;c++){
        if(loop == 3){LYSO_layer = c;}
        else if(loop == 4){second_LYSO_layer=c;
        break;
        }
        loop++;
    }
//get the define information of the first LYSO crystal
    xml_coll_t k_0(LYSO_layer,_U(slice));
    xml_comp_t component = k_0;
    double dx = component.dx();
    double dy = component.dy();
    double dz = component.dz();
    Material mat = description.material(component.materialStr());
    int crystal_id,crystal_num = 0;
    xml_dim_t pos = component.position();
    double x  = pos.x();
    double y  = pos.y();
    double z  = pos.z();

    xml_dim_t rot = component.rotation();
    double rotx = rot.x();
    double roty = rot.y();
    double rotz = rot.z();
  
    DetElement cal(det_name, x_det.id());

// --- create an envelope volume and position it into the world ---------------------
    
    Volume envelope = dd4hep::xml::createPlacedEnvelope( description, e, cal );
    dd4hep::xml::setDetectorTypeFlag( e, cal ) ;
    if( description.buildType() == BUILD_ENVELOPE ) return cal;
    envelope.setVisAttributes(description, x_det.visStr());
  


    xml_dim_t sd_typ = x_det.child(_U(sensitive));
    sens.setType(sd_typ.typeStr());
    Readout readout = sens.readout();

    dd4hep::Segmentation seg = readout.segmentation();
  
    dd4hep::DDSegmentation::BitField64 encoder = seg.decoder();
    encoder.setValue(0) ;

    dd4hep::DDSegmentation::MultiSegmentation* multiSeg = 
    dynamic_cast< dd4hep::DDSegmentation::MultiSegmentation*>( seg.segmentation() ) ;
  
    dd4hep::DDSegmentation::TiledLayerGridXY* tileSeg = 0 ;
    int sensitive_slice_number = -1 ;
    try{
        if( multiSeg ){
            try{ 
    // check if we have an entry for the subsegmentation to be used 
                xml_comp_t segxml = x_det.child( _Unicode( subsegmentation ) ) ;

                std::string keyStr = segxml.attr<std::string>( _Unicode(key) ) ;
                int keyVal = segxml.attr<int>( _Unicode(value) )  ;

                encoder[ keyStr ] =  keyVal ;
                std::cout<<"keyStr:"<<keyStr<<"_keyVal:"<<keyVal<<"\n";
    // if we have a multisegmentation that uses the slice as key, we need to know for the
    // computation of the layer parameters in LayeredCalorimeterData::Layer below
            if( keyStr == "layer"){sensitive_slice_number = keyVal;}
            }
            catch(const std::runtime_error &) {std::cerr << "Error: "  << std::endl;}
    
    // check if we have a TiledLayerGridXY segmentation :s
            const dd4hep::DDSegmentation::TiledLayerGridXY* ts0 =
            dynamic_cast<const dd4hep::DDSegmentation::TiledLayerGridXY*>(  &multiSeg->subsegmentation( encoder.getValue() ) ) ;
    
            tileSeg = const_cast<dd4hep::DDSegmentation::TiledLayerGridXY*>( ts0 ) ;
    
            if( ! tileSeg ){ // if the current segmentation is not a tileSeg, we see if there is another one
      
            for( auto s : multiSeg->subSegmentations() ){
	            const dd4hep::DDSegmentation::TiledLayerGridXY* ts =
	            dynamic_cast<const dd4hep::DDSegmentation::TiledLayerGridXY*>( s.segmentation ) ;
	
	            if( ts ) {
	                tileSeg = const_cast<dd4hep::DDSegmentation::TiledLayerGridXY*>( ts ) ;
	                break ;
	            }
            }
            }
    
        } else {
            tileSeg = dynamic_cast< dd4hep::DDSegmentation::TiledLayerGridXY*>( seg.segmentation() ) ;
        }
    }   
    catch (const std::exception& ex) {
        std::cerr << "Exception caught: " << std::endl;
        // Handle the exception gracefully, possibly by logging the error or providing fallback behavior
    }

    LayeredCalorimeterData* caloData = new LayeredCalorimeterData ;
    for(int il=0;il<2; il++){
    //used for reconstruction, so write a 1*1*2 layer cell size. No absorber or dead-meaterial.
        dd4hep::rec::LayeredCalorimeterData::Layer _caloLayer;
        _caloLayer.distance                        = 560+il*80*mm;
        _caloLayer.phi0                               = 0;
        _caloLayer.absorberThickness            = 0;
        _caloLayer.inner_nRadiationLengths   = 0.01;
        _caloLayer.inner_nInteractionLengths = 0.01;
        _caloLayer.outer_nRadiationLengths   = 0.01;
        _caloLayer.outer_nInteractionLengths = 0.01;
        _caloLayer.inner_thickness              = 3*mm;    //1cm
        _caloLayer.outer_thickness              = 3*mm;    //1cm
        _caloLayer.sensitive_thickness       = 2*3*mm; //2cm
        _caloLayer.cellSize0                         = 3*mm;    //1cm
        _caloLayer.cellSize1                         = 3*mm;    //1cm
        caloData->layers.push_back(_caloLayer);
    }

    caloData->layoutType = LayeredCalorimeterData::BarrelLayout ;
    caloData->inner_symmetry = 8  ;
    caloData->outer_symmetry = 8  ;
    caloData->phi0 = 0 ; // hardcoded

  // extent of the calorimeter in the r-z-plane [ rmin, rmax, zmin, zmax ] in mm.
    caloData->extent[0] = 10*mm ;
    caloData->extent[1] = 50*mm;
    caloData->extent[2] = 0. ;
    caloData->extent[3] = 700*mm ;


//loop in order to build symmetry geo
    for(int kz=1;kz>=-1;kz-=2){
/////////////////////////////////////////////////////////////////
//build Disk and flange
    std::vector<double> cellSizeVector = seg.cellDimensions( encoder.getValue() ); //Assume uniform cell sizes, provide dummy cellID
    int layer_num=1;// count the number of layers in order to define the ID of objects
    for(int ky = 1;ky>=-1;ky-=2){
            int logical_disk_id = 1; //count the number of disks
            int l=0;//stand for the ID number of module
            for(xml_coll_t c(x_det,_U(layer));c;c++){
                string module_name = _toString(l+1,"_module%d")+_toString(0,"_stave%d");
                int slice_number = 0;//used to count the number of slices we have placed
                xml_comp_t   x_layer = c;
                string layer_name      = det_name+module_name+_toString(layer_num,"_layer%d");
            
                encoder["layer"] = logical_disk_id ;
                cellSizeVector = seg.segmentation()->cellDimensions( encoder.getValue() ); 
                int slice_num = 0;//used to distinguish which slice we are building
                if(l>=2){
                    //build flange
                    if(ky==-1)
                    {
                        break;
                    }
                    slice_number=1;
                    for(xml_coll_t k(x_layer,_U(slice)); k; k++){
                        xml_comp_t x_slice = k;
                        string slice_name      = layer_name+_toString(slice_num,"_slice%d");
                        DetElement slice(slice_name,_toString(slice_num,"_slice%d"),x_det.id());
                        Material slice_material  = description.material(x_slice.materialStr());
                        Volume slice_vol(slice_name,Tube(x_slice.rmin(),x_slice.rmax(),x_slice.z()/2,x_slice.phi1(),x_slice.phi2()),slice_material);
                        slice_vol.setVisAttributes(description,x_slice.visStr());
                        PlacedVolume slice_phv = envelope.placeVolume(slice_vol,Position(0*mm,0*mm,kz*x_slice.position().z()));
                        slice_phv.addPhysVolID("side",kz).addPhysVolID("module",4).addPhysVolID("layer",0  ).addPhysVolID("slice",layer_num);
                        if(ky==1 && kz==1){
                            dd4hep::PrintLevel printLevel = dd4hep::ERROR;
                            if (x_det.hasAttr(_Unicode(printLevel))) {
                                printLevel = dd4hep::printLevel(x_det.attr<std::string>(_Unicode(printLevel)));
                            }
                            dd4hep::PrintLevel oldLevel = dd4hep::setPrintLevel(printLevel);
                            dd4hep::printout(dd4hep::INFO, "Construct", "Flange ->layer: r0 = %f r1 = %f mat = %s", x_slice.rmin()/dd4hep::mm, x_slice.rmax()/dd4hep::mm,"stainless_steel");
                            dd4hep::setPrintLevel(oldLevel);
                        }
                        slice.setPlacement(slice_phv);
                    
                        logical_disk_id++;
                        slice_number++;
                    }
                    break;
                }
            for(xml_coll_t k(x_layer,_U(slice)); k; k++)  {
                
                xml_comp_t x_slice = k;
	            string   slice_name      = layer_name + _toString(slice_number,"_slice%d");
	            Material slice_material  = description.material(x_slice.materialStr());
	            DetElement slice(layer_name,_toString(slice_number,"slice%d"),x_det.id());
                if(slice_num%2==0){
                    Volume slice_vol(slice_name,Box(x_slice.dx()/2,x_slice.dy()/2,x_slice.dz()/2),slice_material);
                    slice_vol.setSensitiveDetector(sens);
                    slice_vol.setVisAttributes(description,x_slice.visStr());
                    PlacedVolume slice_phv = envelope.placeVolume(slice_vol,Position(x_slice.position().x(),ky*x_slice.position().y(),kz*x_slice.position().z()));
	                slice_phv.addPhysVolID("side",kz).addPhysVolID("stave",ky).addPhysVolID("module",l+1).addPhysVolID("layer",layer_num  ).addPhysVolID("slice",slice_number);
                    std::cout<<"l + 1 = "<<l+1<<"\n";
                    if(ky==1 && kz==1){
                            dd4hep::PrintLevel printLevel = dd4hep::ERROR;
                            if (x_det.hasAttr(_Unicode(printLevel))) {
                                printLevel = dd4hep::printLevel(x_det.attr<std::string>(_Unicode(printLevel)));
                            }
                            dd4hep::PrintLevel oldLevel = dd4hep::setPrintLevel(printLevel);
                            dd4hep::printout(dd4hep::INFO, "Construct", "Disk ->layer: y0 = %f y1 = %f mat = %s", (x_slice.position().y()-x_slice.dy()/2)/dd4hep::mm, (x_slice.position().y()+x_slice.dy()/2)/dd4hep::mm, "G4_Si");
                            dd4hep::setPrintLevel(oldLevel);
                        }
                    slice.setPlacement(slice_phv);
                    
                    slice_num++;
                    
                    slice_number++;
                }
                else{
                    for (int kx=1;kx>=-1;kx-=2){
                        double phi1 = 0*deg;
                        if(kx==1){phi1 =0*deg;}
                        else{phi1 = 90*deg;}
                        if(ky==-1){phi1+=90*deg;
                        if(kx==1)phi1+=180*deg;}//invert the geometry
                        Volume slice_vol(slice_name,Tube(x_slice.rmin(),x_slice.rmax(),x_slice.z()/2,phi1,phi1+90*deg),slice_material);
                        slice_vol.setSensitiveDetector(sens);
                        slice_vol.setVisAttributes(description,x_slice.visStr());
                        PlacedVolume slice_phv = envelope.placeVolume(slice_vol,Position(kx*x_slice.position().x(),ky*x_slice.position().y(),kz*x_slice.position().z()));
	                    slice_phv.addPhysVolID("side",kz).addPhysVolID("stave",ky).addPhysVolID("module",l+1).addPhysVolID("layer",layer_num  ).addPhysVolID("slice",slice_number);
                        slice.setPlacement(slice_phv);
                        if(ky==1 && kz==1){
                            dd4hep::PrintLevel printLevel = dd4hep::ERROR;
                            if (x_det.hasAttr(_Unicode(printLevel))) {
                                printLevel = dd4hep::printLevel(x_det.attr<std::string>(_Unicode(printLevel)));
                            }
                            dd4hep::PrintLevel oldLevel = dd4hep::setPrintLevel(printLevel);
                            dd4hep::printout(dd4hep::INFO, "Construct", "Disk ->layer: r0 = %f r1 = %f mat = %s", (x_slice.position().y())/dd4hep::mm, (x_slice.position().y()+x_slice.rmax())/dd4hep::mm, "G4_Si");
                            dd4hep::setPrintLevel(oldLevel);
                        }
                        
                        slice_number++;
                    }
                    slice_num++;
                    
                }
                    
                  
            }
            l=l+1;
            logical_disk_id++;  
            }
        
            
        
    layer_num++;
    }
//////////////////////////////////////////////////////////////////
//build 1st LYSO
  
    cellSizeVector = seg.cellDimensions( encoder.getValue() ); 

    for(int ky=-1;ky<=1;ky=ky+2) {
        string module_name = _toString(3,"_module%d")+_toString(ky,"_stave%d");
    
        for (int i = 0; i < 14; i++) {
            encoder["layer"] = 4 ;
            cellSizeVector = seg.segmentation()->cellDimensions( encoder.getValue() ); 
            int num = 0;//calculate the number of crystals allowed to be placed in each line
            crystal_id=0;//stand for the id of LYSO crystals
            double yc = 12.0 + dy / mm * (i + 1);
            double xc = dx / mm;
            num = (int) 2 * ((std::sqrt(56 * 56 - yc * yc) / xc) - 1);
          
            string layer_name      = det_name+ module_name+_toString(kz*(i+1),"_layer%d");
            int j = 0;
            int half = 0;
            if (num % 2 == 1) {
                half = (num - 1) / 2; // half amount of total number each line
                j = -half;
                while (j <= half) {
                    string crystal_name = layer_name +_toString(crystal_id,"_slice%d");
                    DetElement crystalDE(layer_name, _toString(crystal_num,"_slice%d"), x_det.id());
                    Box crystalBox(0.5*dx, 0.5*dy, 0.5*dz);
                    Volume crystalVol(crystal_name, crystalBox, mat);

                    crystalVol.setVisAttributes(description, component.visStr());

                    crystalVol.setSensitiveDetector(sens);
                    Transform3D transform(RotationZYX(rotz, roty, rotx), Translation3D(x + j * dx, ky*(y - 0.5*dy-i * dy), kz*z));
                    PlacedVolume LYSO_phv =envelope.placeVolume(crystalVol, transform);

                    if (x_det.hasAttr(_U(id))) LYSO_phv.addPhysVolID("system", x_det.id());
                    LYSO_phv.addPhysVolID("side",kz).addPhysVolID("stave",ky).addPhysVolID("module",3).addPhysVolID("layer",i+1  ).addPhysVolID("slice",crystal_id);
                    
                    crystalDE.setPlacement(LYSO_phv);
                    j++;
                    crystal_id++;
                    crystal_num++;
                }
            } else {
                half = num / 2;
                j = -half;
                while (j < half) {
                    string crystal_name = layer_name+_toString(crystal_id,"_slice%d");
                    DetElement crystalDE(layer_name, _toString(crystal_num,"_slice%d"), x_det.id());

                    Box crystalBox(0.5*dx, 0.5*dy, 0.5*dz);
                    Volume crystalVol(crystal_name, crystalBox, mat);

                    crystalVol.setVisAttributes(description, component.visStr());
                    crystalVol.setSensitiveDetector(sens);
                    Transform3D transform(RotationZYX(rotz, roty, rotx),
                                            Translation3D(x + (j + 0.5) * dx, ky*(y -0.5*dy- i * dy), kz*z));
                    PlacedVolume LYSO_phv = envelope.placeVolume(crystalVol, transform);

                    if (x_det.hasAttr(_U(id))) LYSO_phv.addPhysVolID("system", x_det.id());
                    LYSO_phv.addPhysVolID("side",kz).addPhysVolID("stave",ky).addPhysVolID("module",3).addPhysVolID("layer",i+1  ).addPhysVolID("slice",crystal_id);
                    
                    crystalDE.setPlacement(LYSO_phv);
                    j++;
                    crystal_id++;
                    crystal_num++;
                }
            }
            if(ky==-1 && kz==1){
                dd4hep::PrintLevel printLevel = dd4hep::ERROR;
                            if (x_det.hasAttr(_Unicode(printLevel))) {
                                printLevel = dd4hep::printLevel(x_det.attr<std::string>(_Unicode(printLevel)));
                            }
                dd4hep::PrintLevel oldLevel = dd4hep::setPrintLevel(printLevel);
                dd4hep::printout(dd4hep::INFO, "Construct", "1st_LYSO ->layer %d : crystal number =%d y = %f x0 = %f x1 =%f mat = %s",(i+1),crystal_id,ky*(y - 0.5*dy-i * dy)/dd4hep::mm, (x-half*dx)/dd4hep::mm,(x+half*dx)/dd4hep::mm, "LYSO");
                dd4hep::setPrintLevel(oldLevel);
            }
        }
      
    }
//////////////////////////////////////////////////////////////////////
// build 2nd LYSO
    xml_coll_t k_second(second_LYSO_layer,_U(slice));
    xml_comp_t component_second = k_second;
    double dx2 = component_second.dx();
    double dy2 = component_second.dy();
    double dz2 = component_second.dz();
    Material mat2 = description.material(component_second.materialStr());
    xml_dim_t pos_second = component_second.position();
    double x2  = pos_second.x();
    double y2  = pos_second.y();
    double z2  = pos_second.z();
    

    for(int ky=-1;ky<=1;ky=ky+2) {
        string module_name = _toString(5,"_module%d")+_toString(ky,"_stave%d");
    
        for (int i = 0; i < 12; i++) {
            encoder["layer"] = 4 ;
            cellSizeVector = seg.segmentation()->cellDimensions( encoder.getValue() ); 
            int num = 0;//calculate the number of crystals allowed to be placed in each line
            crystal_id=0;//stand for the id of LYSO crystals
            double yc =12.0+ dy2/mm * (i+1);
            double xc = dx2 / mm;
            num = (int) 2 * ((std::sqrt(100.0*100.0 - yc * yc) / xc) - 1);
            string layer_name      = det_name+ module_name+_toString(kz*(i+1),"_layer%d");
            int j = 0;
            int half = 0;
            if (num % 2 == 1) {
                half = (num - 1) / 2;
                j = -half;
                while (j <= half) {
                        string crystal_name = layer_name +_toString(crystal_id,"_slice%d");
                        DetElement crystalDE(layer_name, _toString(crystal_num,"_slice%d"), x_det.id());
                        Box crystalBox(0.5*dx2, 0.5*dy2, 0.5*dz2);
                        Volume crystalVol(crystal_name, crystalBox, mat2);
                
                        crystalVol.setVisAttributes(description, component_second.visStr());

                        crystalVol.setSensitiveDetector(sens);
                        Transform3D transform(RotationZYX(0, 0, 0), Translation3D(x2 + j * dx2, ky*(y2 -0.5*dy2- i * dy2), kz*z2));
                        PlacedVolume LYSO_phv =envelope.placeVolume(crystalVol, transform);

                        if (x_det.hasAttr(_U(id))) LYSO_phv.addPhysVolID("system", x_det.id());
                        LYSO_phv.addPhysVolID("side",kz).addPhysVolID("stave",ky).addPhysVolID("module",5).addPhysVolID("layer",i+1  ).addPhysVolID("slice",crystal_id);
                        
                        crystalDE.setPlacement(LYSO_phv);
                        j++;
                        crystal_id++;
                        crystal_num++;
                }
            } else {
                half = num / 2;
                j = -half;
                while (j < half) {
                        string crystal_name = layer_name +_toString(crystal_id,"_slice%d");
                        DetElement crystalDE(layer_name, _toString(crystal_num,"_slice%d"), x_det.id());
                        Box crystalBox(0.5*dx2, 0.5*dy2, 0.5*dz2);
                        Volume crystalVol(crystal_name, crystalBox, mat2);
                
                        crystalVol.setVisAttributes(description, component_second.visStr());

                        crystalVol.setSensitiveDetector(sens);
                        Transform3D transform(RotationZYX(0, 0, 0), Translation3D(x2 + j * dx2, ky*(y2 -0.5*dy2- i * dy2), kz*z2));
                        PlacedVolume LYSO_phv =envelope.placeVolume(crystalVol, transform);

                        if (x_det.hasAttr(_U(id))) LYSO_phv.addPhysVolID("system", x_det.id());
                        LYSO_phv.addPhysVolID("side",kz).addPhysVolID("stave",ky).addPhysVolID("module",5).addPhysVolID("layer",i+1  ).addPhysVolID("slice",crystal_id);
                        
                        crystalDE.setPlacement(LYSO_phv);
                        j++;
                        crystal_id++;
                        crystal_num++;
                    
                }
            }
            if(ky==1 && kz==1){
                dd4hep::PrintLevel printLevel = dd4hep::ERROR;
                            if (x_det.hasAttr(_Unicode(printLevel))) {
                                printLevel = dd4hep::printLevel(x_det.attr<std::string>(_Unicode(printLevel)));
                            }
                dd4hep::PrintLevel oldLevel = dd4hep::setPrintLevel(printLevel);
                dd4hep::printout(dd4hep::INFO, "Construct", "2nd_LYSO ->layer %d : crystal number =%d y = %f x0 = %f x1 =%f mat = %s",i+1,crystal_id,ky*(y2 - 0.5*dy2-i * dy2)/dd4hep::mm, (x2-half*dx2)/dd4hep::mm,(x2+half*dx2)/dd4hep::mm, "LYSO");
                dd4hep::setPrintLevel(oldLevel);
            }
        }
    }
}
    cal.addExtension< LayeredCalorimeterData >( caloData ) ; 
    return cal;
}

DECLARE_DETELEMENT(Lumical_v01, create_detector)
