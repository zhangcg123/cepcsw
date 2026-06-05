#ifndef Calo_DIGI_ALG_H
#define Calo_DIGI_ALG_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
#include "edm4hep/CaloHitSimCaloHitLinkCollection.h"
#else
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#endif
#include <DDRec/DetectorData.h>
#include <DDRec/CellIDPositionConverter.h>
#include "DetInterface/IGeomSvc.h"




class CaloDigiAlg : public Algorithm
{
 
public:
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
    using CEPCSWCaloHitSimCaloHitLinkCollection = edm4hep::CaloHitSimCaloHitLinkCollection;
#else
    using CEPCSWCaloHitSimCaloHitLinkCollection = edm4hep::MCRecoCaloAssociationCollection;
#endif
 
  CaloDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
  /** Called at the begin of the job before anything is read.
   * Use to initialize the processor, e.g. book histograms.
   */
  virtual StatusCode initialize() ;
 
  /** Called for every event - the working horse.
   */
  virtual StatusCode execute() ; 
 
  /** Called after data processing for clean up.
   */
  virtual StatusCode finalize() ;
 
protected:

  SmartIF<IGeomSvc> m_geosvc;
  typedef std::vector<float> FloatVec;

  int _nEvt ;
  float m_length;
  dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
 
  Gaudi::Property<float> m_scale{ this, "Scale", 1 };

  // Input collections
  DataHandle<edm4hep::SimCalorimeterHitCollection> r_SimCaloCol{"SimCaloCol", Gaudi::DataHandle::Reader, this};
  // Output collections
  DataHandle<edm4hep::CalorimeterHitCollection>    w_DigiCaloCol{"DigiCaloCol", Gaudi::DataHandle::Writer, this};
  DataHandle<CEPCSWCaloHitSimCaloHitLinkCollection>    w_CaloAssociationCol{"MCRecoCaloAssociationCollection", Gaudi::DataHandle::Writer, this};
};

#endif
