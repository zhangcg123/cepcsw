#include "GeomSvc.h"
#include "gearimpl/GearParametersImpl.h"
#include "TMath.h"
#include "TMaterial.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"

#include "DD4hep/Detector.h"
#include "DD4hep/Plugins.h"
#include "DDG4/Geant4Converter.h"
#include "DDG4/Geant4Mapping.h"
#include "DDRec/DetectorData.h"

#include <iomanip>
#include <iostream>

DECLARE_COMPONENT(GeomSvc)

GeomSvc::GeomSvc(const std::string& name, ISvcLocator* svc)
: base_class(name, svc), m_dd4hep_geo(nullptr){

}

GeomSvc::~GeomSvc() {

}

StatusCode
GeomSvc::initialize() {
  StatusCode sc = Service::initialize();

  m_dd4hep_geo = &(dd4hep::Detector::getInstance());
  // if DEBUG, set
  //dd4hep::PrintLevel level = msgLevel(MSG::INFO) ? dd4hep::printLevel() : dd4hep::setPrintLevel(dd4hep::DEBUG);
  // if failed to load the compact, a runtime error will be thrown.
  m_dd4hep_geo->fromCompact(m_dd4hep_xmls.value());
  // recover to old level, if not, too many DD4hep print
  //dd4hep::setPrintLevel(level);

  return sc;
}

StatusCode
GeomSvc::finalize() {
  StatusCode sc;

  // m_surface_manager has added as extension of Detector, so not delete?
  //if (m_surface_manager != nullptr) delete m_surface_manager;
  dd4hep::Detector::destroyInstance();

  return sc;
}

dd4hep::DetElement
GeomSvc::getDD4HepGeo() {
    if (lcdd()) {
        return lcdd()->world();
    }
    return dd4hep::DetElement();
}

dd4hep::Detector*
GeomSvc::lcdd() {
    return m_dd4hep_geo;
}


IGeomSvc::Decoder*
GeomSvc::getDecoder(const std::string& readout_name) {

    IGeomSvc::Decoder* decoder = nullptr;

    if (!lcdd()) {
        error() << "Failed to get lcdd()" << endmsg;
        return decoder;
    }

    auto readouts = m_dd4hep_geo->readouts();
    if (readouts.find(readout_name) == readouts.end()) {
        error() << "Failed to find readout name '" << readout_name << "'"
                << " in DD4hep::readouts. "
                << endmsg;
        return decoder;
    }
    
    dd4hep::Readout readout = lcdd()->readout(readout_name);
    auto m_idspec = readout.idSpec(); 

    decoder = m_idspec.decoder();

    if (!decoder) {
        error() << "Failed to get the decoder with readout '"
                << readout_name << "'" << endmsg;
    }

    return decoder;

}

const dd4hep::rec::SurfaceMap*
GeomSvc::getSurfaceMap(const std::string& det_name) {
  if (m_surface_manager == nullptr) {
    dd4hep::rec::SurfaceManager* surfaceMgr = nullptr;
    // first check whether exist
    try {
      surfaceMgr = m_dd4hep_geo->extension<dd4hep::rec::SurfaceManager>();
    }
    catch (std::runtime_error& e) {
      info() << e.what() << " " << surfaceMgr << endmsg;
      surfaceMgr = nullptr;
    }

    if (surfaceMgr) {
      m_surface_manager = surfaceMgr;
    }
    else {
      m_dd4hep_geo->addExtension<dd4hep::rec::SurfaceManager>(m_surface_manager = new dd4hep::rec::SurfaceManager(*m_dd4hep_geo));
    }
  }
  return m_surface_manager->map(det_name);
}
