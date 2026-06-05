#ifndef TRACKHELPER_H
#define TRACKHELPER_H
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/TrackState.h"
#include "edm4hep/TrackerHit.h"
#include "edm4hep/MCParticle.h"
#include "edm4hep/SimTrackerHit.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#else
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#endif
#include "TMatrixDSym.h"
#include "TVector3.h"
#include "DataHelper/HelixClass.h"

namespace CEPC{
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
#else
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
#endif
    
    //get track position and momentum from TrackState
    void getPosMomFromTrackState(const edm4hep::TrackState& trackState,
            double Bz, TVector3& pos,TVector3& mom,double& charge,
            TMatrixDSym& covMatrix_6);

    //Set track state from position, momentum and charge
    void getTrackStateFromPosMom(edm4hep::TrackState& trackState,double Bz,
            TVector3 pos,TVector3 mom,double charge,TMatrixDSym covMatrix_6);

    void getHelixFromPosMom(HelixClass& helix,double& xc,double& yc,
            double& R,double Bz,TVector3 seedPos,TVector3 seedMom,double charge);
    void getAssoMCParticle(
            const CEPCSWTrackerHitSimTrackerHitLinkCollection* assoHits,
            edm4hep::TrackerHit trackerHit,
            edm4hep::MCParticle& mcParticle,edm4hep::SimTrackerHit& simTrackerHit);
}

#endif
