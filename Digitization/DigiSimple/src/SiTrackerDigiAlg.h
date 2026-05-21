#ifndef SiTrackerDigiAlg_H
#define SiTrackerDigiAlg_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/Algorithm.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"

#include "DigiTool/IDigiTool.h"

class SiTrackerDigiAlg : public Algorithm{
 public:
  
  SiTrackerDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();
 
protected:
  // Input collections
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputColHdls{"VXDCollection", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<edm4hep::TrackerHitCollection>               m_outputColHdls{"VXDTrackerHits", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_assColHdls{"VXDTrackerHitAssociationCollection", Gaudi::DataHandle::Writer, this};

  Gaudi::Property<std::string> m_digiToolName{this, "DigiTool", "SmearDigiTool/VXD"};

  ToolHandle<IDigiTool> m_digiTool;
  int m_nEvt=0;
};
#endif
