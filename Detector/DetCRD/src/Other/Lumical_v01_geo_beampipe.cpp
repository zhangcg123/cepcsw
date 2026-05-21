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
    std::cout << "This is the Lumical_v02:"  << std::endl;

    xml_det_t x_det = e;
    string det_name = x_det.nameStr();
    DetElement cal(det_name, x_det.id());

// --- create an envelope volume and position it into the world ---------------------
    Volume envelope = dd4hep::xml::createPlacedEnvelope( description, e, cal );
    dd4hep::xml::setDetectorTypeFlag( e, cal ) ;
    if( description.buildType() == BUILD_ENVELOPE ) return cal;
    envelope.setVisAttributes(description, x_det.visStr());
    PlacedVolume pv;

    DetElement beampipe1_inner_DE(cal,"beampipe_Be_inner",x_det.id());
    DetElement beampipe1_outter_DE(cal,"beampipe_Be_outter",x_det.id());
    DetElement beampipe2_inner_DE(cal,"beampipe_Al_inner",x_det.id());
    DetElement beampipe2_outter_DE(cal,"beampipe_Al_outter",x_det.id());
    DetElement runway_DE(cal,"runway",x_det.id());

    for(int kz=1;kz>=-1;kz-=2){
/////////////////////////////////////////////////////////////////
//build Be beampipe
        xml_comp_t component_beampipe1_inner = x_det.child(_Unicode(beampipe_Be_inner));
        Material fBe = description.material(component_beampipe1_inner.materialStr());
        xml_dim_t pos_beampipe_Be = component_beampipe1_inner.position();

        xml_comp_t component_beampipe1_outter = x_det.child(_Unicode(beampipe_Be_outter));
    
        Tube beampipe_Be_inner(component_beampipe1_inner.rmin(),component_beampipe1_inner.rmax(),component_beampipe1_inner.z()*0.5,component_beampipe1_inner.phi1(),component_beampipe1_inner.phi2());
    
        Volume beampipe_Be_inner_Vol("beampipe_Be_inner", beampipe_Be_inner,fBe);
        beampipe_Be_inner_Vol.setVisAttributes(description, component_beampipe1_inner.visStr());
        Transform3D transform_beampipe_Be(RotationZYX(0,0,0), Translation3D(pos_beampipe_Be.x(), pos_beampipe_Be.y() , kz*pos_beampipe_Be.z()));
        envelope.placeVolume(beampipe_Be_inner_Vol, transform_beampipe_Be);

        Tube beampipe_Be_outter(component_beampipe1_outter.rmin(),component_beampipe1_outter.rmax(),component_beampipe1_outter.z()*0.5,component_beampipe1_outter.phi1(),component_beampipe1_outter.phi2());
        Volume beampipe_Be_outter_Vol("beampipe_Be_outter", beampipe_Be_outter,fBe);
        beampipe_Be_outter_Vol.setVisAttributes(description, component_beampipe1_outter.visStr());
        envelope.placeVolume(beampipe_Be_outter_Vol, transform_beampipe_Be);
    
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//build Al beampipe
        xml_comp_t component_beampipe2_inner = x_det.child(_Unicode(beampipe_Al_inner));
        Material fAl = description.material(component_beampipe2_inner.materialStr());
        xml_dim_t pos_beampipe_Al = component_beampipe2_inner.position();

        xml_comp_t component_beampipe2_outter = x_det.child(_Unicode(beampipe_Al_outter));
    

        Tube beampipe_Al_inner(component_beampipe2_inner.rmin(),component_beampipe2_inner.rmax(),component_beampipe2_inner.z()*0.5,component_beampipe2_inner.phi1(),component_beampipe2_inner.phi2());
        Volume beampipe_Al_inner_Vol("beampipe_Al_inner", beampipe_Al_inner,fAl);
        beampipe_Al_inner_Vol.setVisAttributes(description, component_beampipe2_inner.visStr());
        Transform3D transform_beampipe_Al(RotationZYX(0,0,0), Translation3D(pos_beampipe_Al.x(), pos_beampipe_Al.y() , kz*pos_beampipe_Al.z()));
        envelope.placeVolume(beampipe_Al_inner_Vol, transform_beampipe_Al);

        Tube beampipe_Al_outter(component_beampipe2_outter.rmin(),component_beampipe2_outter.rmax(),component_beampipe2_outter.z()*0.5,component_beampipe2_outter.phi1(),component_beampipe2_outter.phi2());
        Volume beampipe_Al_outter_Vol("beampipe_Al_outter", beampipe_Al_outter,fAl);
        beampipe_Al_outter_Vol.setVisAttributes(description, component_beampipe2_outter.visStr());
        envelope.placeVolume(beampipe_Al_outter_Vol, transform_beampipe_Al);
//////////////////////////////////////////////////////////////////////
//build runway

        xml_comp_t component_runway = x_det.child(_Unicode(runway));
    
        xml_dim_t pos_runway = component_runway.position();
        for(int kx = -1;kx<=1;kx+=2){
            double phi0=90*deg-kx*90*deg;
            Transform3D transform_runway(RotationZYX(90*deg,0,0), Translation3D(-kx*pos_runway.x(), pos_runway.y() , kz*pos_runway.z()));

            Tube runway(component_runway.rmin(),component_runway.rmax(),component_runway.z()*0.5,phi0,phi0+180*deg);
            Volume runway_Vol("runway", runway,fAl);
            runway_Vol.setVisAttributes(description, component_runway.visStr());
            envelope.placeVolume(runway_Vol, transform_runway);                                                                                                                                 
        }
///////////////////////////////////////////////////////////////////////////////////////
//build beampipe between flange
    
    for(xml_coll_t c(x_det,_U(beampipe));c;c++){
        int layer_num=1;
        xml_comp_t   x_layer = c;
        int count = 0;
        for(xml_coll_t k(x_layer,_U(slice)); k; k++)  {
            string module_name = _toString(0,"_module%d")+_toString(0,"_stave%d");
            int slice_number = 0;
            xml_comp_t x_slice = k;
            string layer_name      = det_name+module_name+_toString(layer_num,"_layer%d");
            for(int kx = -1;kx<=1;kx+=2){
                double phi0=90*deg-kx*90*deg;
                
                string slice_name      = layer_name+_toString(slice_number,"_slice%d");

                DetElement slice(slice_name,_toString(slice_number,"_slice%d"),x_det.id());
                Material slice_material  = description.material(x_slice.materialStr());

                if(count==0||count==2){
                    Volume slice_vol(slice_name,Tube(x_slice.rmin(),x_slice.rmax(),x_slice.z()/2,phi0,phi0+90*deg*(count+2)),slice_material);
                    slice_vol.setVisAttributes(description,x_slice.visStr());
                    Transform3D transform_beampipe(RotationZYX(90*deg,0,0), Translation3D(-kx*x_slice.position().x(),x_slice.position().y() , kz*x_slice.position().z()));
                    PlacedVolume slice_phv = envelope.placeVolume(slice_vol,transform_beampipe);
                    slice_phv.addPhysVolID("side",kz).addPhysVolID("module",0).addPhysVolID("layer",0  ).addPhysVolID("slice",layer_num);
                    //std::cout<<"side:"<<kz<<"module:"<<0<<"layer:"<<layer_num<<"slice:"<<layer_num<<"\n";
                    slice.setPlacement(slice_phv);
                    slice_number++;
                }
                else if (count==1){
                    
                    double rmin1 = x_slice.rmin1();
                    double rmax1 = x_slice.rmax1();
                    double rmin2 = x_slice.rmin2();
                    double rmax2 = x_slice.rmax2();
                    if(kz==-1){
                        double r1 = rmin1;
                        rmin1 = rmin2;
                        rmin2 = r1;
                        double r2 = rmax1;
                        rmax1 = rmax2;
                        rmax2 = r2;
                    }
                    Volume slice_vol(slice_name,ConeSegment(x_slice.z()/2,rmin1,rmax1,rmin2,rmax2,phi0,phi0+180*deg),slice_material);
                    slice_vol.setVisAttributes(description,x_slice.visStr());
                    Transform3D transform_beampipe(RotationZYX(90*deg,0,0), Translation3D(-kx*x_slice.position().x(),x_slice.position().y() , kz*x_slice.position().z()));
                    PlacedVolume slice_phv = envelope.placeVolume(slice_vol,transform_beampipe);
                    slice_phv.addPhysVolID("side",kz).addPhysVolID("module",0).addPhysVolID("layer",0  ).addPhysVolID("slice",layer_num);
                    //std::cout<<"side:"<<kz<<"module:"<<0<<"layer:"<<layer_num<<"slice:"<<layer_num<<"\n";
                    slice.setPlacement(slice_phv);
                    slice_number++;
                }
                
            }
            count++;
        }
        layer_num++;
    }
    }
    return cal;
}

DECLARE_DETELEMENT(Lumical_v01_standalone, create_detector)
