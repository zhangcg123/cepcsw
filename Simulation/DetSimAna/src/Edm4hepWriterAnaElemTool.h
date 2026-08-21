#ifndef Edm4hepWriterAnaElemTool_h
#define Edm4hepWriterAnaElemTool_h

#include <cstdint>
#include <map>
#include <vector>

#include "GaudiKernel/AlgTool.h"
#include "k4FWCore/DataHandle.h"
#include "DetSimInterface/IAnaElemTool.h"
#include "DetSimInterface/CommonUserEventInfo.hh"
#include "DetSimInterface/CommonUserTrackInfo.hh"
#include "G4ThreeVector.hh"

#include "DetInterface/IGeomSvc.h"

#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/CaloHitContributionCollection.h"
#include "GsfTruthEventData/G4MaterialStepCollection.h"
#include "GsfTruthEventData/SimTrackerHitG4StepLinkCollection.h"

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

    DataHandle<gsftruth::G4MaterialStepCollection> m_gsfTruthStepCol{
        "GsfG4MaterialSteps", Gaudi::DataHandle::Writer, this};
    DataHandle<gsftruth::SimTrackerHitG4StepLinkCollection> m_gsfTruthLinkCol{
        "GsfSimTrackerHitG4StepLinks", Gaudi::DataHandle::Writer, this};

    Gaudi::Property<bool> m_writeGsfTruthEventData{
        this, "WriteGsfTruthEventData", false,
        "Embed selected Geant4 material steps and exact SimTrackerHit provenance in event data"};
    Gaudi::Property<std::vector<int>> m_gsfTruthPdgs{
        this, "GsfTruthPDGs", {11, -11},
        "PDG codes whose Geant4 tracker steps are embedded"};
    Gaudi::Property<bool> m_gsfTruthPrimaryOnly{
        this, "GsfTruthPrimaryOnly", true,
        "Embed Geant4 steps only for primary tracks"};
    Gaudi::Property<bool> m_gsfTruthTrackerOnly{
        this, "GsfTruthTrackerOnly", true,
        "Embed only steps with a pre or post point inside the tracker bounds"};

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

    struct GsfTruthStepRecord {
        int trackID = -1;
        int parentID = -1;
        int stepNumber = -1;
        int pdg = 0;
        int charge = 0;
        int preStepStatus = 0;
        int postStepStatus = 0;
        int processSubtype = 0;
        int preVolumeCopyNo = 0;
        int postVolumeCopyNo = 0;
        int preSensitive = 0;
        int postSensitive = 0;
        edm4hep::Vector3d prePosition{};
        edm4hep::Vector3d postPosition{};
        edm4hep::Vector3f preMomentum{};
        edm4hep::Vector3f postMomentum{};
        float momentumLoss = 0.0f;
        float retainedMomentumFraction = 0.0f;
        float preKineticEnergy = 0.0f;
        float postKineticEnergy = 0.0f;
        float energyDeposit = 0.0f;
        float nonIonizingEnergyDeposit = 0.0f;
        float stepLength = 0.0f;
        float materialRadiationLength = 0.0f;
        float stepTX0 = 0.0f;
        float preTrackLength = 0.0f;
        float postTrackLength = 0.0f;
        float preGlobalTime = 0.0f;
        float postGlobalTime = 0.0f;
    };

    std::vector<GsfTruthStepRecord> m_gsfTruthSteps;

    bool acceptGsfTruthPdg(int pdg) const;
    bool insideGsfTruthTracker(const G4ThreeVector& position) const;
    void recordGsfTruthStep(const G4Step* step);
};

#endif
