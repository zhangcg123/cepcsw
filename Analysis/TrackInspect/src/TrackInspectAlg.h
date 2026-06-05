#ifndef HITSCANPROCESSOR_H
#define HITSCANPROCESSOR_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"

#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/TrackCollection.h"
#include "edm4hep/TrackerHit.h"
#include "edm4hep/MCParticle.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
#include "edm4hep/TrackerHit3DCollection.h"
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"
#else
#include "edm4hep/TrackerHitCollection.h"
#include "edm4hep/MCRecoTrackerAssociationCollection.h"
#endif

#include "GaudiKernel/NTuple.h"

class TrackInspectAlg : public Algorithm {
    public :
        TrackInspectAlg(const std::string& name, ISvcLocator* pSvcLocator);
        ~TrackInspectAlg(){};

        StatusCode initialize() override;
        StatusCode execute() override;
        StatusCode finalize() override;

    private :

#if edm4hep_VERSION >= EDM4HEP_VERSION(1, 0, 0)
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::TrackerHitSimTrackerHitLinkCollection;
    using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHit3DCollection;
#else
    using CEPCSWTrackerHitSimTrackerHitLinkCollection = edm4hep::MCRecoTrackerAssociationCollection;
    using CEPCSWTrackerHit3DCollection = edm4hep::TrackerHitCollection;
#endif
    
        // TruthMatchProcessor
        DataHandle<edm4hep::TrackCollection> _inTrackColHdl{"InputTrackCollection", Gaudi::DataHandle::Reader, this};
        DataHandle<edm4hep::MCParticleCollection> _inMCParticleColHdl{"InputMCParticleCollection", Gaudi::DataHandle::Reader, this};

        DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> _TPCRelColHdl{"TPCTrackerHitRelations", Gaudi::DataHandle::Reader, this};
        DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> _VXDRelColHdl{"VXDTrackerHitRelations", Gaudi::DataHandle::Reader, this};
        DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> _SITRelColHdl{"SITTrackerHitRelations", Gaudi::DataHandle::Reader, this};
        DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> _SETRelColHdl{"SETTrackerHitRelations", Gaudi::DataHandle::Reader, this};
        DataHandle<CEPCSWTrackerHitSimTrackerHitLinkCollection> _FTDRelColHdl{"FTDTrackerHitRelations", Gaudi::DataHandle::Reader, this};
        Gaudi::Property<double> _weight{this, "Weight", 0.5};

        Gaudi::Property<bool> _useTPC{this, "useTPC", true};
        Gaudi::Property<bool> _useVXD{this, "useVXD", true};
        Gaudi::Property<bool> _useSIT{this, "useSIT", true};
        Gaudi::Property<bool> _useSET{this, "useSET", true};
        Gaudi::Property<bool> _useFTD{this, "useFTD", true};

        std::map<std::pair<edm4hep::TrackerHit, edm4hep::MCParticle>, double> hitmap;
        std::vector<std::tuple<edm4hep::MCParticle, edm4hep::Track, double>> matchvec;
        double match(edm4hep::MCParticle, edm4hep::Track);

        void initializeRelationCollections(std::vector<const CEPCSWTrackerHitSimTrackerHitLinkCollection*> &relCols);

        int _nEvt;
        std::string m_thisName;

        // TrackingEfficiency
        void Fill(edm4hep::MCParticle, edm4hep::Track);
        std::vector<edm4hep::Track> MCParticleTrackAssociator(edm4hep::MCParticle);

        std::map<edm4hep::MCParticle, std::vector<edm4hep::SimTrackerHit>> mcpHitMap;
        std::string treeFileName;

        NTuple::Tuple* m_tuple;

        NTuple::Item<long>    m_nParticles;
        NTuple::Array<double> vx;
        NTuple::Array<double> vy;
        NTuple::Array<double> vz;
        NTuple::Array<double> ex;
        NTuple::Array<double> ey;
        NTuple::Array<double> ez;
        NTuple::Array<double> Omega;
        NTuple::Array<double> D0;
        NTuple::Array<double> Z0;
        NTuple::Array<double> Phi;
        NTuple::Array<double> TanLambda;
        NTuple::Array<double> TRUEPX;
        NTuple::Array<double> TRUEPY;
        NTuple::Array<double> TRUEPZ;
        NTuple::Array<double> TRUEPE;
        NTuple::Array<double> TRUEPT;
        NTuple::Array<double> TRUEP;
        NTuple::Array<double> TRUEETA;
        NTuple::Array<double> TRUEY;
        NTuple::Array<double> TRUETHETA;
        NTuple::Array<int> eventNumber;
        NTuple::Array<int> particleNumber;
        NTuple::Array<int> totalCandidates;
        NTuple::Array<int> nCandidate;
        NTuple::Array<int> pid;
        NTuple::Array<int> nHits;
        NTuple::Array<int> WMSelectionVariable;
        // TTree tree;
        // TTree tuple;
        // TFile treeFile;
};

#endif
