#ifndef Edm4hepWriterAnaElemTool_h
#define Edm4hepWriterAnaElemTool_h

#include <map>

#include "GaudiKernel/AlgTool.h"
#include "k4FWCore/DataHandle.h"
#include "DetSimInterface/IAnaElemTool.h"
#include "DetSimInterface/CommonUserEventInfo.hh"
#include "DetSimInterface/CommonUserTrackInfo.hh"

#include "DetInterface/IGeomSvc.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/CaloHitContributionCollection.h"

class Edm4hepWriterAnaElemTool: public extends<AlgTool, IAnaElemTool> {

public:

    using extends::extends;

    /// IAnaElemTool interface
    // Run
    virtual void BeginOfRunAction(const G4Run*) override;
    virtual void EndOfRunAction(const G4Run*) override;

    // Event
    virtual void BeginOfEventAction(const G4Event*) override;
    virtual void EndOfEventAction(const G4Event*) override;

    // Tracking
    virtual void PreUserTrackingAction(const G4Track*) override;
    virtual void PostUserTrackingAction(const G4Track*) override;

    // Stepping
    virtual void UserSteppingAction(const G4Step*) override;


    /// Overriding initialize and finalize
    StatusCode initialize() override;
    StatusCode finalize() override;

private:
    // In order to associate MCParticle with contribution, we need to access MC Particle.
    // - collection MCParticle: the particles in Generator
    DataHandle<edm4hep::MCParticleCollection> m_mcParGenCol{"MCParticleGen", 
            Gaudi::DataHandle::Writer, this};
    // - collection MCParticleG4: the simulated particles in Geant4
    DataHandle<edm4hep::MCParticleCollection> m_mcParCol{"MCParticle", 
            Gaudi::DataHandle::Writer, this};
    edm4hep::MCParticleCollection* mcCol;

    // Maintain the collections in a map, avoid to define a new collection in the header.
    // Key is the collection name with "Collection".
    // For calo hit contrib collection, the key is same as sim calo hit collection.
    std::map<std::string, DataHandle<edm4hep::SimTrackerHitCollection>*> m_trackerColMap;
    std::map<std::string, DataHandle<edm4hep::SimCalorimeterHitCollection>*> m_calorimeterColMap;
    std::map<std::string, DataHandle<edm4hep::CaloHitContributionCollection>*> m_caloContribColMap;

    // the name here is without suffix "Collection"
    Gaudi::Property<std::vector<std::string>> m_trackerColNames{this, 
        "TrackerCollections",
	{"VXD", "ITKBarrel", "ITKEndcap", "TPC", "TPCLowPt", "TPCSpacePoint",
	 "OTKBarrel", "OTKEndcap", "COIL", "MuonBarrel", "MuonEndcap",
	 "SIT", "SET", "FTD"},
        "Names of the Tracker collections (without suffix Collection)"};
    Gaudi::Property<std::vector<std::string>> m_calorimeterColNames{this, 
        "CalorimeterCollections",
        {"Lumical", 
         "EcalBarrel", "EcalEndcaps", "EcalEndcapRing", 
         "HcalBarrel", "HcalEndcaps", "HcalEndcapRing"}, 
        "Names of the Calorimeter collections (without suffix Collection)"};

    Gaudi::Property<double> m_sectrk_Ek{this, "SecTrackEk", 100., "Ek (MeV) threshold to record a secondary track"};
    Gaudi::Property<double> m_sectrk_rho{this, "SecTrackRho", 1830., "rho (mm) threshold to record a secondary track"};
    Gaudi::Property<double> m_sectrk_z{this, "SecTrackZ", 2900., "+/- z (mm) threshold to record a secondary track"};

    Gaudi::Property<bool> m_istrk2primary{this, "IsTrk2Primary", true, "For m_track2primary, the value is primary or ancestor"};
private:
    // in order to associate the hit contribution with the primary track or ancestor track,
    // we have a bookkeeping of every track.
    // The primary or ancestor track will assign the same key/value.

    // Following is an example:
    //    1 -> 1,
    //    2 -> 2,
    //    3 -> 1,
    // Now, if parent of trk #4 is trk #3, using the mapping {3->1} could 
    // locate the primary trk #1.

    std::map<int, int> m_track2primary;

    CommonUserEventInfo* m_userinfo = nullptr;

    // get the limitation of R/Z in tracker
    SmartIF<IGeomSvc> m_geosvc;
    double R = 0;
    double Z = 0;

    bool verboseOutput = false;
};

#endif
