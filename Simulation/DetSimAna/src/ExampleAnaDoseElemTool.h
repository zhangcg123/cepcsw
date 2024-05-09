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

#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/NTuple.h"
#include "G4ThreeVector.hh"

#include <vector>
#include <fstream>
#include <string>

const G4int nbinscoeffdoseeqneu = 68;
const G4int nbinscoeffdoseeqproton = 33;
const G4int nbinscoeffdoseeqele = 49;
const G4int nbinscoeffdoseeqpos = 49;
const G4int nbinscoeffdoseeqhe = 24;
const G4int nbinscoeffdoseeqphoton = 64;
const G4int nbinscoeffdoseeqmup = 33;
const G4int nbinscoeffdoseeqmum = 33;
const G4int nbinscoeffdoseeqpip = 43;
const G4int nbinscoeffdoseeqpim = 43;
const G4int nbinscoeff1mevneueqfluenceneu = 1381;
const G4int nbinscoeff1mevneueqfluenceproton = 927;
const G4int nbinscoeff1mevneueqfluencepion = 900;
const G4int nbinscoeff1mevneueqfluenceele = 15;



class ExampleAnaDoseElemTool: public extends<AlgTool, IAnaElemTool> {

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
    bool verboseOutput = false;
    Gaudi::Property<std::vector<int>> m_gridnbins{this, "Gridnbins"};
    Gaudi::Property<std::vector<double>> m_coormin{this, "Coormin"};
    Gaudi::Property<std::vector<double>> m_coormax{this, "Coormax"};
    Gaudi::Property<std::vector<double>> m_regionhist{this, "Regionhist"};
    Gaudi::Property<double> m_hehadcut{this, "HEHadroncut"};
    Gaudi::Property<std::string> m_filename{this, "filename"};
    Gaudi::Property<std::string> m_dosecoefffilename{this, "Dosecofffilename","Simulation/DetSimAna/src/dosecoffe/"};

    std::vector<G4double> fVector; //absorbed dose
    std::vector<G4double> fnneu;   //track length
    std::vector<G4double> fdoseeqright; //dose eq
    std::vector<G4double> f1mevnfluen;  //1mev neutron equivalent fluence
    std::vector<G4double> fhehadfluen;  //high energy hadron fluence
    std::vector<G4int> fdet1;
    std::vector<G4int> fair;

    G4ThreeVector nbins,regionhist,coormin,coormax,vecwidth;
    G4double hehadcut;

    void Writefile(std::string);
    void Setdosevalue(const G4Step* );
    void Calcudose(std::vector<G4double>& fexample, const G4int constnbins, const G4double coeffexample[][2], const G4int ijkbin[], const G4double inputtracklength, const G4double inputkinenergy);
    void Readcoffe(std::string filepath, std::string filename, const G4int constnbins, G4double coeffexample[][2]);
    void Initialize();
    void Preparecoeff(const G4ThreeVector, const G4ThreeVector , const G4ThreeVector, const G4ThreeVector, const std::string, const G4double);

    static int cmppair(const std::pair< G4ThreeVector,G4double > p1, const std::pair< G4ThreeVector,G4double > p2) {return p1.second<p2.second;};

    G4double coeffneu[nbinscoeffdoseeqneu][2];
    G4double coeffproton[nbinscoeffdoseeqproton][2];
    G4double coeffele[nbinscoeffdoseeqele][2];
    G4double coeffpos[nbinscoeffdoseeqpos][2];
    G4double coeffhe[nbinscoeffdoseeqhe][2];
    G4double coeffphoton[nbinscoeffdoseeqphoton][2];
    G4double coeffmup[nbinscoeffdoseeqmup][2];
    G4double coeffmum[nbinscoeffdoseeqmum][2];
    G4double coeffpip[nbinscoeffdoseeqpip][2];
    G4double coeffpim[nbinscoeffdoseeqpim][2];
    G4double coeff1mevneueqfluenceneu[nbinscoeff1mevneueqfluenceneu][2];
    G4double coeff1mevneueqfluenceproton[nbinscoeff1mevneueqfluenceproton][2];
    G4double coeff1mevneueqfluencepion[nbinscoeff1mevneueqfluencepion][2];
    G4double coeff1mevneueqfluenceele[nbinscoeff1mevneueqfluenceele][2];

    std::ifstream m_inputFile_doseeqneu;
};

#endif
