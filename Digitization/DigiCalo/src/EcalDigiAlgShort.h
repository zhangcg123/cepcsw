#ifndef _ECAL_DIGI_ALG_SHORT_H
#define _ECAL_DIGI_ALG_SHORT_H

#include "k4FWCore/DataHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "edm4hep/EDM4hepVersion.h"
#include "edm4hep/MutableCaloHitContribution.h"
#include "edm4hep/MutableSimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/Cluster.h"
#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
#include "edm4hep/CaloHitSimCaloHitLinkCollection.h"
#include "edm4hep/CaloHitMCParticleLinkCollection.h"
#else
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoCaloParticleAssociationCollection.h"
#endif
#include "edm4hep/Vector3f.h"

#include <DDRec/DetectorData.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DD4hep/Detector.h>
#include <DD4hep/Objects.h>
#include <DD4hep/Segmentations.h>
#include "DetInterface/IGeomSvc.h"

#include "TVector3.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TMath.h"
#include "TH3.h"
#include "TH1.h"

#include <iostream>
#include <algorithm>
#include <map>
#include <random>
#include <math.h>
#include <cmath>
#include <cstdlib>
#include <TTimeStamp.h>
#include <ctime>
#include "time.h"

#include "HitStep.h"
#include "CaloCrystalShort.h"

const double C = 299.79;    // In mm/ns
const double PI = 3.141592653;

class EcalDigiAlgShort : public Algorithm
{
public:
#if edm4hep_VERSION >= EDM4HEP_VERSION(0, 99, 0)
    using CEPCSWCaloHitSimCaloHitLinkCollection = edm4hep::CaloHitSimCaloHitLinkCollection;
    using CEPCSWCaloHitMCParticleLinkCollection = edm4hep::CaloHitMCParticleLinkCollection;
#else
    using CEPCSWCaloHitSimCaloHitLinkCollection = edm4hep::MCRecoCaloAssociationCollection;
    using CEPCSWCaloHitMCParticleLinkCollection = edm4hep::MCRecoCaloParticleAssociationCollection;
#endif
    EcalDigiAlgShort(const std::string& name, ISvcLocator* svcLoc);

    virtual StatusCode initialize();

    virtual StatusCode execute();

    virtual StatusCode finalize();

    StatusCode MergeHits(const edm4hep::SimCalorimeterHitCollection& m_col, std::vector<edm4hep::SimCalorimeterHit>& m_hits);

    edm4hep::MutableSimCalorimeterHit find(const std::vector<edm4hep::MutableSimCalorimeterHit>& m_col, unsigned long long& cellid) const;

    const double EnergyDigi(const double& ScinGen, const double& sEcalCryMipLY);

    void Clear();

protected:
    std::string algname;
    int dettype;    // 1 for barrel, 2 for end-cap.
    SmartIF <IGeomSvc> m_geosvc;
    typedef std::vector<float> FloatVec;
    typedef std::map<const edm4hep::MCParticle, float> MCParticleToEnergyWeightMap;

    int _nEvt;
    float m_length;
    TRandom3 rndm;
    TFile* m_wfile;
    TTree* t_SimCont;
    TTree* t_SimHit;

    double totE_Truth, totE_Digi;
    FloatVec m_step_t;
    FloatVec m_step_x, m_step_y, m_step_z, m_step_E, m_step_T, m_stepHit_x, m_stepHit_y, m_stepHit_z;
    FloatVec m_simHit_x, m_simHit_y, m_simHit_z, m_simHit_T, m_simHit_Q_Truth, m_simHit_Q_Digi, m_simHit_module, m_simHit_stave, m_simHit_layer;
    FloatVec m_simHit_phi_x, m_simHit_z_y;    // Phi and z for barrel, x and y for end-cap
    std::vector<unsigned long long> m_simHit_cellID;

    dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
    dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
    dd4hep::Detector* m_dd4hep;

    Gaudi::Property<float> m_scale{this, "Scale", 1};

    // Input collections
    DataHandle <edm4hep::SimCalorimeterHitCollection> r_SimCaloCol{"SimCaloCol", Gaudi::DataHandle::Reader, this};
    mutable Gaudi::Property <std::string> _readoutName{this, "ReadOutName", "CaloHitsCollection", "Readout name"};
    mutable Gaudi::Property <std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};
    mutable Gaudi::Property<int> _UseRelDigi{this, "UseRealisticDigi", 1, "If use the realistic model"};

    // Input parameters
    mutable Gaudi::Property<int> _writeNtuple{this, "WriteNtuple", 1, "Write ntuple"};
    mutable Gaudi::Property<int> _Nskip{this, "SkipEvt", 0, "Skip event"};
    mutable Gaudi::Property<int> _seed{this, "Seed", 2024, "Random Seed"};
    mutable Gaudi::Property<int> _Debug{this, "Debug", 0, "Debug level"};
    mutable Gaudi::Property<float> _Eth{this, "EnergyThreshold", 0.001, "Energy Threshold (/GeV)"};
    mutable Gaudi::Property<float> r_cali{this, "CalibrECAL", 1, "Calibration coefficients for ECAL"};
    mutable Gaudi::Property<float> Latt{this, "AttenuationLength", 7000, "Crystal Attenuation Length(mm)"};
    mutable Gaudi::Property<float> Tres{this, "TimeResolution", 0.1, "Crystal time resolution in one side (ns)"};
    mutable Gaudi::Property<float> nMat{this, "MatRefractive", 2.15, "Material refractive index of crystal"};
    mutable Gaudi::Property<float> Tinit{this, "InitalTime", 2, "Start time (ns)"};

    mutable Gaudi::Property<float> _Qthfrac{this, "ChargeThresholdFrac", 0.05, "Charge threshold fraction"};

    mutable Gaudi::Property<int> fADC{this, "ADC", 4096, "Total ADC counts"};
    mutable Gaudi::Property<int> fNofGain{this, "NofGain", 3, "Number of gain modes"};
    mutable Gaudi::Property<int> fADCSwitch{this, "ADCSwitch", 4000, "switching point of different gain mode"};
    mutable Gaudi::Property<float> fGainRatio_12{this, "GainRatio_12", 15, "Gain-1 over Gain-2"};
    mutable Gaudi::Property<float> fGainRatio_23{this, "GainRatio_23", 10, "Gain-2 over Gain-3"};
    mutable Gaudi::Property<float> fEcalCryLen{this, "EcalCryLen", 41, "Crystal length (mm)"};
    mutable Gaudi::Property<float> fEcalCryMipLY{this, "EcalCryMipLY", 200, "Crystal light yield (p.e./MIP)"};
    mutable Gaudi::Property<float> fEcalMIPEnergy{this, "EcalMIPEnergy", 8.9, "MIP Energy deposit in 1cm BGO (MeV/MIP)"};
    mutable Gaudi::Property<int> fEcalSiPMPixels{this, "EcalSiPMPixels", 250000, "Pixels number of SiPM"};
    mutable Gaudi::Property<float> fEcalChargeADCMean{this, "EcalChargeADCMean", 5, "ADC per pe for Gain-1 (ADC)"};
    mutable Gaudi::Property<float> fEcalChargeADCSigma{this, "EcalChargeADCSigma", 2.5, "Sigma of ADC per pe for Gain-1 (ADC)"};
    mutable Gaudi::Property<float> fEcalADCError{this, "EcalADCError", 0.002, "ADC precision"};
    mutable Gaudi::Property<float> fEcalNoiseADCSigma{this, "EcalNoiseADCSigma", 3, "Sigma of electronic noise (ADC)"};
    mutable Gaudi::Property<float> fEcalMIP_Thre{this, "EcalMIP_Thre", 0.1, "Energy threshold for single readout end (MIP)"};

    // Output collections
    DataHandle <edm4hep::CalorimeterHitCollection> w_DigiCaloCol{"DigiCaloCol", Gaudi::DataHandle::Writer, this};
    DataHandle <CEPCSWCaloHitSimCaloHitLinkCollection> w_CaloAssociationCol{"ECALBarrelAssoCol", Gaudi::DataHandle::Writer, this};
    DataHandle <CEPCSWCaloHitMCParticleLinkCollection> w_MCPCaloAssociationCol{"ECALBarrelParticleAssoCol", Gaudi::DataHandle::Writer, this};
};

#endif
