#include "CalorimeterSensDetTool.h"

#include "G4VSensitiveDetector.hh"

#include "DetSimSD/CaloSensitiveDetector.h"

#include "DD4hep/Detector.h"

DECLARE_COMPONENT(CalorimeterSensDetTool);

StatusCode
CalorimeterSensDetTool::initialize() {
    StatusCode sc;


    m_geosvc = service<IGeomSvc>("GeomSvc");
    if (!m_geosvc) {
        error() << "Failed to find GeomSvc." << endmsg;
        return StatusCode::FAILURE;
    }


    return sc;
}

StatusCode
CalorimeterSensDetTool::finalize() {
    StatusCode sc;

    return sc;
}

G4VSensitiveDetector*
CalorimeterSensDetTool::createSD(const std::string& name) {

    dd4hep::Detector* dd4hep_geo = m_geosvc->lcdd();

    bool is_merge_enabled = true;
    for(auto cal_name : m_listCalsMergeDisable){
      if(cal_name==name){
	is_merge_enabled = false;
	break;
      }
    }

    CaloSensitiveDetector* sd = new CaloSensitiveDetector(name, *dd4hep_geo, is_merge_enabled);
    warning() << name << " set to merge true/false = " << is_merge_enabled << endmsg;


    if(m_listCalsApplyBirks.size()!=m_listCalsBirksConst.size()){
      info() << name << " is set to apply Birks law, but Hit collection and Birks constant collection can not match! " << endmsg;
    }
    else{
      for(int i=0; i<m_listCalsApplyBirks.size(); i++){
        if(m_listCalsApplyBirks[i]==name){
          info() << name << " will apply Birks law" << endmsg;
          sd->ApplyBirksLaw(m_listCalsBirksConst[i]);
          break;
        }
      }
    }

    return sd;
}


