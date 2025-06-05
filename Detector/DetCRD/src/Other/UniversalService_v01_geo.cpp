//====================================================================
//  cepcgeo - CEPC universal sevice models in DD4hep 
//--------------------------------------------------------------------
//  FU Chengdong, IHEP
//  email: fucd@ihep.ac.cn
//  $Id$
//====================================================================
#include "DD4hep/DetFactoryHelper.h"
#include "DD4hep/DD4hepUnits.h"
#include "DD4hep/DetType.h"
#include "DD4hep/Printout.h"
#include "XML/Utilities.h"
#include <cmath>

using dd4hep::Solid;
using dd4hep::DetElement;
using dd4hep::Material;
using dd4hep::Position;
using dd4hep::RotationZYX;
using dd4hep::Transform3D;
using dd4hep::Rotation3D;
using dd4hep::Volume;
using dd4hep::PlacedVolume;
using dd4hep::_toString;

/* reference from 
   dd4hep::xml::createPlacedEnvelope(dd4hep::Detector& description, dd4hep::xml::Handle_t e, dd4hep::DetElement sdet)
*/

static dd4hep::Ref_t create_element(dd4hep::Detector& theDetector, xml_h e, dd4hep::SensitiveDetector sens)  {

  xml_det_t   x_det    = e;
  Material    air      = theDetector.air();
  int         det_id   = x_det.id();
  std::string name     = x_det.nameStr();
  DetElement  det(name, det_id);

  Volume envelope = dd4hep::xml::createPlacedEnvelope(theDetector, e, det);
  dd4hep::xml::setDetectorTypeFlag(e, det);
  if (theDetector.buildType()==dd4hep::BUILD_ENVELOPE) return det;
  std::string env_vis = x_det.hasAttr(_U(vis)) ? x_det.visStr() : "SeeThrough";
  envelope.setAttributes(theDetector, x_det.regionStr(), x_det.limitsStr(), env_vis);

  dd4hep::PrintLevel printLevel = dd4hep::ERROR;
  if (x_det.hasAttr(_Unicode(printLevel))) {
    printLevel = dd4hep::printLevel(x_det.attr<std::string>(_Unicode(printLevel)));
  }
  dd4hep::PrintLevel oldLevel = dd4hep::setPrintLevel(printLevel);

  dd4hep::printout(dd4hep::INFO, "Construct", "** building UniversalService_v01 %s", name);

  bool reflect = x_det.reflect(false);

  PlacedVolume  pv;
  for (xml_coll_t com_i(x_det,_U(component)); com_i; ++com_i) {
    xml_comp_t  x_com(com_i);
    xml_comp_t  x_shp    = x_com.child(_U(shape));
    std::string com_name = x_com.nameStr();

    Position    com_pos;
    RotationZYX com_rot;

    if (x_com.hasChild(_U(position))) {
      xml_comp_t x_pos = x_com.position();
      com_pos = Position(x_pos.x(), x_pos.y(), x_pos.z());
    }
    if (x_com.hasChild(_U(rotation))) {
      xml_comp_t x_rot = x_com.rotation();
      com_rot = RotationZYX(x_rot.z(), x_rot.y(), x_rot.x());
    }
    int    nphi     = x_com.nphi();
    int    nz       = x_com.nz();
    double z_offset = x_com.z_offset(0);
    if (nz>1 && z_offset==0)
      dd4hep::printout(dd4hep::ERROR, "Construct", "missing z_offset while nz>1");
    else if (nz<=0)
      dd4hep::printout(dd4hep::ERROR, "Construct", "nz<=0");

    if (nphi<=0) dd4hep::printout(dd4hep::ERROR, "Construct", "nphi<=0");

    Solid  com_solid(x_shp.createShape());

    if (!com_solid.isValid()) {
      throw std::runtime_error(std::string("Cannot create component volume : ") + x_shp.typeStr() +
			       std::string(" for service " ) + name + std::string(" ") + com_name);
    }

    Material com_mat = theDetector.material(x_shp.materialStr());

    Volume component = Volume(name+"_"+com_name, com_solid, com_mat);

    double dphi = 2.0*M_PI/nphi;
    for (int iphi = 0; iphi < nphi; iphi++) {
      double phi = iphi*dphi;
      RotationZYX rpt_rot(phi, 0, 0);
      RotationZYX rot = rpt_rot*com_rot;
      double x = com_pos.x()*cos(phi) - com_pos.y()*sin(phi);
      double y = com_pos.x()*sin(phi) + com_pos.y()*cos(phi);
      for (int iz = 0; iz < nz; iz++) {
	double z = com_pos.z() + iz*z_offset;
	Position pos(x, y, z);
	pv = envelope.placeVolume(component, Transform3D(rot, pos));
	dd4hep::printout(dd4hep::INFO, "Construct", "put component at (%f,%f,%f) by rot (%f,%f,%f)", pos.x(), pos.y(), pos.z(), rot.Psi(), rot.Theta(), rot.Phi());

	DetElement cdet(det, name+"_"+com_name+_toString(iphi, "_phi%d")+_toString(iz, "_z%d"), det_id);
	cdet.setPlacement(pv);

	if (reflect) {
	  Position    reflect_pos(x, y, -z);
	  RotationZYX reflect_rot = rpt_rot*RotationZYX(0, 0, M_PI)*com_rot;
	  pv = envelope.placeVolume(component, Transform3D(reflect_rot, reflect_pos));
	  DetElement rdet(det, name+"_"+com_name+_toString(iphi, "_phi%d")+_toString(iz, "_z%dr"), det_id);
	  rdet.setPlacement(pv);
	}
      }
    }

    component.setVisAttributes(theDetector.visAttributes(x_com.visStr()));
  }

  dd4hep::printout(dd4hep::INFO, "Construct", "UniversalService_v01 %s done.", name);
  dd4hep::setPrintLevel(oldLevel);

  return det;
}
DECLARE_DETELEMENT(UniversalService_v01,create_element)
