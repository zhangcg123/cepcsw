#ifndef AnalysisPIDAlg_h
#define AnalysisPIDAlg_h 1

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"

#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/TrackerHit.h"
#include "edm4hep/TrackCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#include "edm4hep/TrackMCParticleLinkCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#endif
#include "edm4hep/RecDqdx.h"
#include "edm4hep/RecDqdxCollection.h"
#include "edm4cepc/RecTofCollection.h"
#include "edm4hep/TrackState.h"
#include "edm4hep/Vector3d.h"

#include "TFile.h"
#include "TTree.h"
#include <random>
#include "GaudiKernel/NTuple.h"

class AnalysisPIDAlg : public Algorithm {
 public:
  // Constructor of this form must be provided
  AnalysisPIDAlg( const std::string& name, ISvcLocator* pSvcLocator );

  // Three mandatory member functions of any algorithm
  StatusCode initialize() override;
  StatusCode execute() override;
  StatusCode finalize() override;

 private:


#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
    using CEPCSWTrackMCParticleLinkCollection = edm4hep::TrackMCParticleLinkCollection;
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
    using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
#else
    using CEPCSWTrackMCParticleLinkCollection = edm4hep::MCRecoTrackParticleAssociationCollection;
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
    using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
#endif
    
    
  DataHandle<edm4hep::TrackCollection> _FultrkCol{"CompleteTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<CEPCSWTrackMCParticleLinkCollection> _FultrkParAssCol{"CompleteTracksParticleAssociation", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::RecDqdxCollection> _inDndxColHdl{"DndxTracks", Gaudi::DataHandle::Reader, this};
  DataHandle<edm4hep::RecTofCollection> _inTofColHdl{"RecTofCollection", Gaudi::DataHandle::Reader, this};

  Gaudi::Property<std::string> m_outputFile{this, "OutputFile", "pid.root"};

  std::vector<double> genpx, genpy, genpz, genE, genp, genM, gentheta, genphi, endx, endy, endz, endr;
  std::vector<int> PDG, genstatus, simstatus, recoPDG, tpcrecoPDG, tofrecoPDG;
  std::vector<bool> isdecayintrker, iscreatedinsim, isbackscatter, isstopped;
  std::vector<bool> matchedtpc, matchedtof;
  std::vector<int> truthidx, tpcidx, tofidx;

  std::vector<std::vector<double>> tof_chi2s, tof_expt;
  std::vector<std::vector<double>> tpc_chi2s, tpc_expdndxs;
  std::vector<std::vector<double>> tof_chis, tpc_chis, tot_chi2s;
  std::vector<double> tof_meast, tof_measterr;
  std::vector<double> tpc_measdndx, tpc_measdndxerr;
  std::vector<double> tof_chi2s_1, tof_expt_1;
  std::vector<double> tpc_chi2s_1, tpc_expdndxs_1;
  std::vector<double> tof_chis_1, tpc_chis_1, tot_chi2s_1;
  
  int _nEvt;
  const std::map<int, int> PDGIDs = { 
    {0, -11},
    {1, -13},
    {2, 211},
    {3, 321},
    {4, 2212},
  };

  double tpcdndx, tpcdndxerr, toft, tofterr;
  int Ndndxtrk, Ntoftrk, Nfulltrk, Nfullass;
  double max_weight, weight;
  int max_weight_idx, ass_idx, dndx_index, tof_index; 
  double p1, p2, p3, x1, y1;
  bool matched1, matched2;
  int recpdg, tpcrecpdg, tofrecpdg, minchi2idx;

  TFile* m_file;
  TTree* m_tree;

  void ClearVars(){

    Ndndxtrk = 0;
    Ntoftrk = 0;
    Nfullass = 0;
    Nfulltrk = 0;

    genpx.clear(); genpy.clear(); genpz.clear(); genE.clear();
    genp.clear(); genM.clear(); gentheta.clear(); genphi.clear();
    endx.clear(); endy.clear(); endz.clear(); endr.clear();
    PDG.clear(); genstatus.clear(); simstatus.clear();
    recoPDG.clear(); tpcrecoPDG.clear(); tofrecoPDG.clear();
    isdecayintrker.clear(); iscreatedinsim.clear();
    isbackscatter.clear(); isstopped.clear();

    tof_chi2s.clear();
    tof_chis.clear();
    tof_meast.clear();
    tof_measterr.clear();
    tof_expt.clear();

    tpc_chi2s.clear();
    tpc_chis.clear();
    tpc_expdndxs.clear();
    tpc_measdndx.clear();
    tpc_measdndxerr.clear();

    tot_chi2s.clear();

    matchedtpc.clear();
    matchedtof.clear();
    truthidx.clear();
    tpcidx.clear();
    tofidx.clear();
  }

};
#endif
