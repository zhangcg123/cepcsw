#ifndef FinalPIDAlg_h
#define FinalPIDAlg_h 1

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"

#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4cepc/RecTof.h"
#include "edm4cepc/RecTofCollection.h"
#include "edm4hep/RecDqdx.h"
#include "edm4hep/RecDqdxCollection.h"
#include "edm4hep/ParticleIDCollection.h"

class FinalPIDAlg : public Algorithm {
 public:
  // Constructor of this form must be provided
  FinalPIDAlg( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:
  DataHandle<edm4hep::ReconstructedParticleCollection> m_inPFOCol{"CyberPFO", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::RecTofCollection> m_inTofCol{"RecTofCollection", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::RecDqdxCollection> m_inDqdxCol{"DndxTracks", Gaudi::DataHandle::Reader, this};
  //DataHandle<edm4hep::ParticleIDCollection> m_PIDCol{"finalPID", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::ReconstructedParticleCollection> m_outPFOCol{"CyberPFOPID", Gaudi::DataHandle::Writer, this};
  Gaudi::Property<std::string> m_method{this, "Method", "TPC+TOF"};

  //void FillTPCPID(const edm4hep::ReconstructedParticleCollection* pfocol, const edm4hep::RecDqdxCollection* dqdxcol, edm4hep::ParticleIDCollection* pidcol);
  void FillTPCTOFPID(const edm4hep::RecDqdxCollection* dqdxcol, const edm4hep::RecTofCollection* tofcol, edm4hep::MutableReconstructedParticle& pfo);

  int _nEvt;
  bool _hasTPC;
  bool _hasTOF;

  const std::map<int, int> PDGIDs = {
    {0, -11},
    {1, -13},
    {2, 211},
    {3, 321},
    {4, 2212},
  };

};
#endif
