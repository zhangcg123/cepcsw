#ifndef HCAL_DIGI_ALG_H 
#define HCAL_DIGI_ALG_H

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "edm4hep/MCParticleCollection.h"
#include "edm4hep/MutableCaloHitContribution.h"
#include "edm4hep/MutableSimCalorimeterHit.h"
#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoCaloParticleAssociationCollection.h"
#include "edm4hep/MCParticle.h"

#include <DDRec/DetectorData.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DD4hep/Segmentations.h> 
#include "DetInterface/IGeomSvc.h"
#include "TVector3.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TString.h"
#include "TH3.h"
#include "TH2.h"
#include "TH1.h"
#include "TF1.h"

#include <cstdlib>
#include "time.h"
#include <TTimeStamp.h> 
#include <ctime>

#define C 299.79  // unit: mm/ns
#define PI 3.141592653

class HcalDigiAlg : public GaudiAlgorithm
{
 
public:
 
  HcalDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
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

  StatusCode MergeHits(const edm4hep::SimCalorimeterHitCollection& m_col, std::vector<edm4hep::SimCalorimeterHit>& m_hits);
  edm4hep::MutableSimCalorimeterHit find(const std::vector<edm4hep::MutableSimCalorimeterHit>& m_col, unsigned long long& cellid) const;

	void Clear();

protected:

  SmartIF<IGeomSvc> m_geosvc;
  typedef std::vector<float> FloatVec;
  typedef std::map<const edm4hep::MCParticle, float> MCParticleToEnergyWeightMap;

	int _nEvt ;
	TRandom3 rndm;
	TFile* m_wfile;
	TTree* t_simHit;
  TH2D* GSTileResMap; 

  double m_totE, m_totE_truth;
  double m_MC_EPx, m_MC_EPy, m_MC_EPz;
	FloatVec m_simHit_x, m_simHit_y, m_simHit_z, m_simHit_E, m_simHit_Etruth, m_simHit_Eatt, 
           m_simHit_system, m_simHit_stave, m_simHit_layer, m_simHit_tile, m_simHit_idx, m_simHit_idy, m_simHit_row, m_simHit_phi, 
           m_simHit_HG, m_simHit_LG, m_simHit_rawQ, m_simHit_Npe_scint, m_simHit_Npe_sipm;
  FloatVec m_step_x, m_step_y, m_step_LY;
  std::vector<unsigned long long> m_simHit_cellID;
  std::vector<int> m_simHit_steps;

  dd4hep::Detector* m_dd4hep;
	dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
	//dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  std::map<std::string, dd4hep::DDSegmentation::BitFieldCoder*> map_readout_decoder;

  TF1* f_DarkNoise = nullptr;

	Gaudi::Property<float> m_scale{ this, "Scale", 1 };

  // Input collections
  typedef DataHandle<edm4hep::SimCalorimeterHitCollection>              SimCaloType;
  typedef DataHandle<edm4hep::CalorimeterHitCollection>                 CaloType;
  typedef DataHandle<edm4hep::MCRecoCaloAssociationCollection>          CaloSimAssoType;      //Calorimeter - SimCalorimeter
  typedef DataHandle<edm4hep::MCRecoCaloParticleAssociationCollection>  CaloParticleAssoType; //Calorimeter - MCParticle

  Gaudi::Property< std::vector<std::string> > name_SimCaloHit{ this, "SimCaloHitCollection", {"HcalBarrelCollection"} };
  Gaudi::Property< std::vector<std::string> > name_Readout{ this, "ReadOutName", {"HcalBarrelCollection"} };
  Gaudi::Property< std::vector<std::string> > name_CaloHit{ this, "CaloHitCollection", {"HCALBarrel"} };
  Gaudi::Property< std::vector<std::string> > name_CaloAsso{ this, "CaloAssociationCollection", {"HCALBarrelAssoCol"} };
  Gaudi::Property< std::vector<std::string> > name_CaloMCPAsso{ this, "CaloMCPAssociationCollection", {"HCALBarrelParticleAssoCol"} };

  std::vector<SimCaloType*> _inputSimHitCollection;
  std::vector<CaloType*> _outputHitCollection;
  std::vector<CaloSimAssoType*> _outputCaloSimAssoCol;
  std::vector<CaloParticleAssoType*> _outputCaloMCPAssoCol;

  DataHandle<edm4hep::MCParticleCollection> m_MCParticleCol{"MCParticle", Gaudi::DataHandle::Reader, this};
  mutable Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};

  //Input parameters
  mutable Gaudi::Property<int>   _writeNtuple{this,  "WriteNtuple", 1, "Write ntuple"};
  mutable Gaudi::Property<int>   _Nskip{this,  "SkipEvt", 0, "Skip event"};
  mutable Gaudi::Property<float> _seed{this,   "Seed", 2131, "Random Seed"};
  mutable Gaudi::Property<float> r_cali{this,  "CalibrHCAL", 1, "Global calibration coefficients for HCAL"};
  mutable Gaudi::Property<int>   _UseRelDigi{this,   "UseRealisticDigi",  1, "If use the realistic model"};

  //add digitization parameters from AHCAL prototype
  //Scintillation and general
  mutable Gaudi::Property<float> _MIPCali{this,   "MIPResponse",  0.007126, "MIP response (/GeV)"};
  mutable Gaudi::Property<float> _MIPLY{this,   "MIPLY",  80, "Detected light yield (p.e./MIP)"};
  mutable Gaudi::Property<float> _Eth_Mip{this,   "MIPThreshold", 0.1, "Energy Threshold (/MIP)"};
  mutable Gaudi::Property<float> _TileRes{this,   "TileNonUniformity",  0., "Non-uniformity level of one tile response"};

  mutable Gaudi::Property<int> _UseTileLYMap{this,   "UseTileLYMap",  1., "Use the tile light yield map"};
  mutable Gaudi::Property<std::string> _TileLYMapFile{this,   "TileLYMapFile",  "", "Use the tile light yield map"};
  mutable Gaudi::Property<std::string> _EffAttenLength{this,   "EffAttenLength",  "23mm", "Effictive attenuation length (mm)"};
  mutable Gaudi::Property<float> _TempCoef{this,   "LYTempCoef",  0, "Temperature dependence of light yield (%/K)"};

  //Temperature
  mutable Gaudi::Property<float> _TempVariation{this,   "TemperatureVariation",  1., "Temperature control variation (K)"};

  //SiPM
  mutable Gaudi::Property<int>   _Pixel{this,   "SiPMPixel",  57600, "number of SiPM pixels"};
  mutable Gaudi::Property<float> _SiPMDCR{this,   "SiPMDCR", 1600, "SiPM Dark Count Rate (Hz)"};
  mutable Gaudi::Property<float> _SiPMXTalk{this,  "SiPMCT", 0.12, "SiPM crosstalk Probability"};
  mutable Gaudi::Property<float> _TimeInterval{this,  "TimeInterval", 0.000002, "Time interval for one readout (s)"};
  mutable Gaudi::Property<float> _SiPMGainTempCoef{this,   "SiPMGainTempCoef", -0.03, "Temperature dependence of SiPM gain (%/K)"}; // doi:10.1016/j.nima.2016.09.053
  mutable Gaudi::Property<float> _SiPMDCRTempCoef{this,   "SiPMDCRTempCoef", 3.34/80, "Temperature dependence of SiPM DCR (10^{k*deltaT})"}; // doi:10.1016/j.nima.2016.09.053

  //ADC
  mutable Gaudi::Property<int> _ADC{this,   "ADC", 8192, "Total ADC conuts"};
  mutable Gaudi::Property<int> _ADCSwitch{this,   "ADCSwitch", 8000, "switching point of different gain mode"};
  mutable Gaudi::Property<float> _GainRatio_12{this,  "GainRatio_12", 50, "Gain-1 over Gain-2"};
  mutable Gaudi::Property<float> _GainRatio_23{this,  "GainRatio_23", 60, "Gain-2 over Gain-3"};
  mutable Gaudi::Property<float> _SiPMGainMean{this,    "SiPMGainMean", 2, "SiPM gain: ADC per p.e. for HG (ADC)"};
  mutable Gaudi::Property<float> _SiPMGainSigma{this,   "SiPMGainSigma", 0.08, "Fluctuation of single photoelctron ADC around the mean value for single device (%)"};
  mutable Gaudi::Property<float> _SiPMNoiseSigma{this,  "SiPMNoiseSigma", 0, "Sigma of SiPM noise (ADC)"};
  mutable Gaudi::Property<float> _Pedestal{this,  "Pedestal", 50, "ADC value of pedestal"};
  mutable Gaudi::Property<float> _PedestalNoiseSigma{this,  "PedestalSigma", 4, "Sigma of electronic noise (ADC)"};

  // Output collections
  //DataHandle<edm4hep::CalorimeterHitCollection>    w_DigiCaloCol{"DigiCaloCol", Gaudi::DataHandle::Writer, this};
  //DataHandle<edm4hep::MCRecoCaloAssociationCollection>    w_CaloAssociationCol{"HCALBarrelAssoCol", Gaudi::DataHandle::Writer, this};
  //DataHandle<edm4hep::MCRecoCaloParticleAssociationCollection>    w_MCPCaloAssociationCol{"HCALBarrelParticleAssoCol", Gaudi::DataHandle::Writer, this};
};

#endif
