#ifndef CylinderDigiAlg_H
#define CylinderDigiAlg_H

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
#include <DDRec/DetectorData.h>
#include "DetInterface/IGeomSvc.h"

class CylinderDigiAlg : public Algorithm{
 public:
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
#else
  using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
  using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
#endif

  CylinderDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
  virtual StatusCode initialize() ;
  virtual StatusCode execute() ; 
  virtual StatusCode finalize() ;
 
protected:
  
  SmartIF<IGeomSvc> m_geosvc;
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder;

  // Input collections
  DataHandle<edm4hep::SimTrackerHitCollection>            m_inputColHdls{"DriftChamberHitsCollection", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<CEPCSWTrackerHit3DCollection>               m_outputColHdls{"DCTrackerHits", Gaudi::DataHandle::Writer, this};
  DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> m_assColHdls{"DCTrackerHitAssociationCollection", Gaudi::DataHandle::Writer, this};

  Gaudi::Property<float> m_resRPhi{this, "ResRPhi", 0.1};
  Gaudi::Property<float> m_resZ   {this, "ResZ", 2.828};

  int m_nEvt=0;
};
#endif
