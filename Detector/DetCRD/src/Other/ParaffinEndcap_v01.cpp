#include "DD4hep/DetFactoryHelper.h"
#include "XML/Layering.h"
#include "XML/Utilities.h"
#include "DDRec/DetectorData.h"
#include "DDSegmentation/Segmentation.h"

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


    dd4hep::DetElement sdet("det",1770);

    dd4hep::Material det_mat(theDetector.material("ParaffinWax"));
    dd4hep::Volume det_vol(det_name+"_vol", dd4hep::Tube(dim.rmin(), dim.rmax(), dim.dz()), det_mat);

    dd4hep::Transform3D transform(dd4hep::Rotation3D(),
                                  dd4hep::Position(pos.x(),pos.y(),pos.z()));
    dd4hep::Transform3D transform_01(dd4hep::Rotation3D(),
                                  dd4hep::Position(pos.x(),pos.y(),-1*pos.z()));


    //Create caloData object to extend driver with data required for reconstruction

    dd4hep::PlacedVolume pv;
    dd4hep::DetElement both_endcap(det_name, x_det.id());
    dd4hep::Volume motherVol = theDetector.pickMotherVolume(both_endcap);
    dd4hep::DetElement sdetA = sdet;
    dd4hep::Ref_t(sdetA)->SetName((det_name+"_A").c_str());
    dd4hep::DetElement sdetB = sdet.clone(det_name+"_B",1769);
    dd4hep::Assembly assembly("assembly");
    pv = assembly.placeVolume(det_vol,transform);

    sdetA.setPlacement(pv);

    pv = assembly.placeVolume(det_vol,transform_01);

    sdetB.setPlacement(pv);

    pv = motherVol.placeVolume(assembly);
    both_endcap.setPlacement(pv);
    both_endcap.add(sdetA);
    both_endcap.add(sdetB);

    MYDEBUG("create_detector DONE. ");
    return both_endcap;


}

DECLARE_DETELEMENT(ParaffinEndcap_v01, create_detector)
