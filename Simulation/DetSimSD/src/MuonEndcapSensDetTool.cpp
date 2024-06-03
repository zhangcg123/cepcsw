#include "MuonEndcapSensDetTool.h"

#include "G4VSensitiveDetector.hh"

#include "DD4hep/Detector.h"

#include "MuonEndcapSensitiveDetector.h"
#include "TrackerCombineSensitiveDetector.h"

#include "CLHEP/Units/SystemOfUnits.h"

DECLARE_COMPONENT(MuonEndcapSensDetTool)

StatusCode MuonEndcapSensDetTool::initialize() {
  StatusCode sc;
  debug() << "initialize() " << endmsg;
  
  m_geosvc = service<IGeomSvc>("GeomSvc");
  if (!m_geosvc) {
    error() << "Failed to find GeomSvc." << endmsg;
    return StatusCode::FAILURE;
  }
  
  return AlgTool::initialize();
}

StatusCode MuonEndcapSensDetTool::finalize() {
  StatusCode sc;
  
  return sc;
}

G4VSensitiveDetector* MuonEndcapSensDetTool::createSD(const std::string& name) {
  debug() << "createSD for " << name << endmsg;
  
  dd4hep::Detector* dd4hep_geo = m_geosvc->lcdd();
  auto sens = dd4hep_geo->sensitiveDetector(name);
  G4VSensitiveDetector* sd = nullptr;
  if(sens.combineHits()){
    sd = new TrackerCombineSensitiveDetector(name, *dd4hep_geo);
  }
  else{
    sd = new MuonEndcapSensitiveDetector(name, *dd4hep_geo);
  }

  return sd;
}
