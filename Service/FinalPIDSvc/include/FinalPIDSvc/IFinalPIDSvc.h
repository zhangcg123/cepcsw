#ifndef I_FinalPID_SVC_H
#define I_FinalPID_SVC_H

#include "GaudiKernel/IService.h"
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackState.h"
#include "edm4cepc/RecTof.h"
#include "edm4cepc/RecTofCollection.h"
#include "edm4hep/RecDqdx.h"
#include "edm4hep/RecDqdxCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit.h"
#include "edm4hep/TrackMCParticleLinkCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackParticleAssociationCollection.h"
#endif

#include <xgboost/c_api.h>

/**
 * @class IFinalPIDSvc
 * @brief Interface for PID implementation
 * @author Geliang Liu (glliu@ihep.ac.cn)
*/

class IFinalPIDSvc: virtual public IService {
public:
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
    using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
    using CEPCSWTrackMCParticleLinkCollection = edm4hep::TrackMCParticleLinkCollection;
#else
    using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
    using CEPCSWTrackMCParticleLinkCollection = edm4hep::MCRecoTrackParticleAssociationCollection;
#endif
    DeclareInterfaceID(IFinalPIDSvc, 0, 1);
    
    virtual ~IFinalPIDSvc() = default;

    virtual void SetCollections( const CEPCSWTrackerHit3DCollection* barrelhits, const CEPCSWTrackerHit3DCollection* endcaphits, const edm4hep::RecTofCollection* tofcol, const edm4hep::RecDqdxCollection* dqdxcol, const edm4hep::ReconstructedParticleCollection* PFO) = 0;

    virtual void MatchMuonHitsToTracks() = 0;

    virtual void SetWP_mu(int muWP) = 0;
    virtual void SetWP_ele(int eleWP) = 0;
    virtual void SetWP_pho(double phoWP) = 0;
    virtual void Set_dR_max(double dR_max) = 0;

    virtual bool LoadPFO(const edm4hep::ReconstructedParticle pfo) = 0;

    virtual void ApplyModel() = 0;

    virtual int GetType() = 0;

    virtual double GetChi2Total(int i_pdg) = 0;
    virtual double GetChi2TPC(int i_pdg) = 0;
    virtual double GetChi2TOF(int i_pdg) = 0;

    virtual float GetProb(int i_pdg) = 0;

    virtual double GetP() = 0;
    virtual double GetTheta() = 0;
    virtual double GetPhi() = 0;

    virtual double GetTof() = 0;
    virtual double GetDndx() = 0;

    virtual double GetE(bool isHCAL) = 0;
    virtual double GetEp(bool isHCAL) = 0;
    virtual double GetL(bool isHCAL) = 0;
    virtual double GetR90(bool isHCAL) = 0;
    virtual double GetWeta2(bool isHCAL) = 0;
    virtual double GetWphi2(bool isHCAL) = 0;
    virtual double GetTimeFirst(bool isHCAL) = 0;
    virtual double GetTimeLast(bool isHCAL) = 0;
    
    virtual double GetMindR(int i) = 0;
    virtual double GetMindR_last() = 0;
    virtual double Getdd(int i) = 0;
    virtual double Getdd_last() = 0;

    virtual int GetCharge() = 0;
    virtual int GetNhcal() = 0;
    virtual int GetNmuon() = 0;
};

#endif

