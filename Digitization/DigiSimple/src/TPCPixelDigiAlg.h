#ifndef TPCPixelDigiAlg_H
#define TPCPixelDigiAlg_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/Algorithm.h"
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/SimTrackerHitCollection.h"

#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
// TODO: SimPrimaryIonizationClusterCollection is removed from EDM4hep
#else
#include "edm4hep/SimPrimaryIonizationClusterCollection.h"
#include "edm4hep/SimPrimaryIonizationCluster.h"
#endif

#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#endif

#include "DigiTool/IDigiTool.h"

class TPCPixelDigiAlg : public Algorithm{
 public:
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
#else
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
#endif
  
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
  DataHandle<CEPCSWTrackerHit3DCollection>               m_outputColHdls{"TPCTrackerHits", Gaudi::DataHandle::Writer, this};
  DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> m_assColHdls{"TPCTrackerHitsAss", Gaudi::DataHandle::Writer, this};

  Gaudi::Property<std::string> m_digiToolName{this, "DigiTool", "TPCPixelDigiTool"};

  ToolHandle<IDigiTool> m_digiTool;
  int m_nEvt=0;
};
#endif
