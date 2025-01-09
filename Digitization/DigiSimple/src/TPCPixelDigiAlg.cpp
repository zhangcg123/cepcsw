#include "TPCPixelDigiAlg.h"


#include "edm4hep/Vector3f.h"

#include "DD4hep/Detector.h"
#include <DD4hep/Objects.h>
#include "DD4hep/DD4hepUnits.h"
#include "DDRec/Vector3D.h"
#include "DDRec/ISurface.h"

#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/IRndmGen.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"

DECLARE_COMPONENT(TPCPixelDigiAlg)

TPCPixelDigiAlg::TPCPixelDigiAlg(const std::string& name, ISvcLocator* svcLoc)
: GaudiAlgorithm(name, svcLoc) {
  // Input collections
  declareProperty("SimTrackHitCollection", m_inputColHdls, "Handle of the Input SimTrackerHit collection");


  // Output collections
  declareProperty("TPCTrackerHits", m_outputColHdls, "Handle of the output TrackerHit collection");
  declareProperty("TPCTrackerHitAss", m_assColHdls, "Handle of the Association collection between SimTrackerHit and TrackerHit");
}

StatusCode TPCPixelDigiAlg::initialize() {
  m_digiTool = ToolHandle<IDigiTool>(m_digiToolName.value());

  info() << "DigiTool " << m_digiTool.typeAndName() << " found" << endmsg;

  info() << "TPCPixelDigiAlg::initialized" << endmsg;
  return GaudiAlgorithm::initialize();
}


StatusCode TPCPixelDigiAlg::execute(){
  auto trkCol = m_outputColHdls.createAndPut();
  auto assCol = m_assColHdls.createAndPut();

  const edm4hep::SimTrackerHitCollection* simCol = nullptr;
  try {
    simCol = m_inputColHdls.get();
  }
  catch(GaudiException &e){
    debug() << "Collection " << m_inputColHdls.fullKey() << " test is unavailable in event " << m_nEvt << endmsg;
    return StatusCode::SUCCESS;
  }
  if (simCol->size() == 0) return StatusCode::SUCCESS;
  debug() << m_inputColHdls.fullKey() << " has cluser "<< simCol->size() << endmsg;

  m_digiTool->Call(simCol, trkCol, assCol);

  debug() << "Created " << trkCol->size() << " hits, "<<simCol->size()<<" clusters"
          << simCol->size() - trkCol->size() << " hits got dismissed for being out of boundary"
          << endmsg;

  m_nEvt++;

  return StatusCode::SUCCESS;
}

StatusCode TPCPixelDigiAlg::finalize(){
  info() << "Processed " << m_nEvt << " events " << endmsg;
  return GaudiAlgorithm::finalize();
}
