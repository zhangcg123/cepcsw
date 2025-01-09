#ifndef TPCPixelDigiAlg_H
#define TPCPixelDigiAlg_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/SimPrimaryIonizationClusterCollection.h"
#include "edm4hep/SimPrimaryIonizationCluster.h"

#include "DigiTool/IDigiTool.h"

class TPCPixelDigiAlg : public GaudiAlgorithm{
 public:
  
  TPCPixelDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();
 
protected:
  // Input collections
  // DataHandle<edm4hep::SimPrimaryIonizationClusterCollection>            m_inputColHdls{"SimPrimaryIonizationClusterCollection", Gaudi::DataHandle::Reader, this};
  // DataHandle<edm4hep::SimPrimaryIonizationClusterCollection>            m_inputIonClusterCol{"SimPrimaryIonizationClusterCollection", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputColHdls{"TPCCollection", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<edm4hep::TrackerHitCollection>               m_outputColHdls{"TPCTrackerHits", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoTrackerAssociationCollection> m_assColHdls{"TPCTrackerHitsAss", Gaudi::DataHandle::Writer, this};

  Gaudi::Property<std::string> m_digiToolName{this, "DigiTool", "TPCPixelDigiTool"};

  ToolHandle<IDigiTool> m_digiTool;
  int m_nEvt=0;
};
#endif
