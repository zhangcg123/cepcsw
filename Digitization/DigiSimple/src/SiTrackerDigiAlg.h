#ifndef SiTrackerDigiAlg_H
#define SiTrackerDigiAlg_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/Algorithm.h"
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/SimTrackerHitCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#endif
#include "DigiTool/IDigiTool.h"

class SiTrackerDigiAlg : public Algorithm{
 public:
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
#else
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
#endif

  SiTrackerDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
  virtual StatusCode initialize();
  virtual StatusCode execute();
  virtual StatusCode finalize();
 
protected:
  // Input collections
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputColHdls{"VXDCollection", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<CEPCSWTrackerHit3DCollection>               m_outputColHdls{"VXDTrackerHits", Gaudi::DataHandle::Writer, this};
  DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> m_assColHdls{"VXDTrackerHitAssociationCollection", Gaudi::DataHandle::Writer, this};

  Gaudi::Property<std::string> m_digiToolName{this, "DigiTool", "SmearDigiTool/VXD"};

  ToolHandle<IDigiTool> m_digiTool;
  int m_nEvt=0;
};
#endif
