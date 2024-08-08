#ifndef _ECAL_DIGI_ALG_H
#define _ECAL_DIGI_ALG_H

#include "k4FWCore/DataHandle.h"
#include "GaudiAlg/GaudiAlgorithm.h"
#include "edm4hep/MutableCaloHitContribution.h"
#include "edm4hep/MutableSimCalorimeterHit.h"
#include "edm4hep/SimCalorimeterHit.h"
#include "edm4hep/CalorimeterHit.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include "edm4hep/MCRecoCaloAssociationCollection.h"
#include "edm4hep/MCRecoCaloParticleAssociationCollection.h"
#include "CaloBar.h"
#include "HitStep.h"

#include <DDRec/DetectorData.h>
#include <DDRec/CellIDPositionConverter.h>
#include <DD4hep/Segmentations.h> 
#include "DetInterface/IGeomSvc.h"
#include "TVector3.h"
#include "TRandom3.h"
#include "TFile.h"
#include "TString.h"
#include "TH3.h"
#include "TH1.h"

#include <cstdlib>
#include "time.h"
#include <TTimeStamp.h> 
#include <ctime>

#define C 299.79  // unit: mm/ns
#define PI 3.141592653

class EcalDigiAlg : public GaudiAlgorithm
{
 
public:
 
  EcalDigiAlg(const std::string& name, ISvcLocator* svcLoc);
 
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

	double GetBarLength(CaloBar& bar); //TODO: should read from geom file! 
	edm4hep::MutableSimCalorimeterHit find(const std::vector<edm4hep::MutableSimCalorimeterHit>& m_col, unsigned long long& cellid) const;
  // double Digitization(double edepCry, double fCrosstalkChannel);
  double EnergyDigi(double ScinGen, double sEcalCryMipLY);

	void Clear();

protected:

  SmartIF<IGeomSvc> m_geosvc;
  typedef std::vector<float> FloatVec;
  typedef std::map<const edm4hep::MCParticle, float> MCParticleToEnergyWeightMap;

	int _nEvt ;
	float m_length;
	TRandom3 rndm;
	TFile* m_wfile;
	TTree* t_SimCont;
	TTree* t_SimBar;
	
  double totE_Truth, totE_Digi;
  FloatVec m_step_t;  // yyy: time of each step
	FloatVec m_step_x, m_step_y, m_step_z, m_step_E, m_step_T1, m_step_T2, m_stepBar_x, m_stepBar_y, m_stepBar_z;
	FloatVec m_simBar_x, m_simBar_y, m_simBar_z, m_simBar_T1, m_simBar_T2, m_simBar_Q1_Truth, m_simBar_Q2_Truth, m_simBar_Q1_Digi, m_simBar_Q2_Digi, m_simBar_dlayer, m_simBar_stave, m_simBar_slayer, m_simBar_module;
  std::vector<unsigned long long> m_simBar_cellID;


	dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
	dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  dd4hep::Detector* m_dd4hep;

	Gaudi::Property<float> m_scale{ this, "Scale", 1 };

  // Input collections
  DataHandle<edm4hep::SimCalorimeterHitCollection> r_SimCaloCol{"SimCaloCol", Gaudi::DataHandle::Reader, this};
  mutable Gaudi::Property<std::string> _readoutName{this, "ReadOutName", "CaloHitsCollection", "Readout name"};
  mutable Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};
  mutable Gaudi::Property<int>   _UseRelDigi{this,   "UseRealisticDigi",  1, "If use the realistic model"};

  //Input parameters
  mutable Gaudi::Property<int>   _writeNtuple{this,  "WriteNtuple", 1, "Write ntuple"};
  mutable Gaudi::Property<int>   _Nskip{this,  "SkipEvt", 0, "Skip event"};
  mutable Gaudi::Property<float> _seed{this,   "Seed", 2131, "Random Seed"};
  mutable Gaudi::Property<int>  _Debug{this,   "Debug", 0, "Debug level"};
  mutable Gaudi::Property<float> _Eth {this,   "EnergyThreshold", 0.001, "Energy Threshold (/GeV)"};
  mutable Gaudi::Property<float> r_cali{this,  "CalibrECAL", 1, "Calibration coefficients for ECAL"};
  mutable Gaudi::Property<float> Latt{this, 	"AttenuationLength", 7000, "Crystal Attenuation Length(mm)"};
  mutable Gaudi::Property<float> Tres{this, 	"TimeResolution", 0.1, "Crystal time resolution in one side (ns)"};
  mutable Gaudi::Property<float> nMat{this, 	"MatRefractive", 2.15, "Material refractive index of crystal"};
  mutable Gaudi::Property<float> Tinit{this, 	"InitalTime", 2, "Start time (ns)"}; 
  
  mutable Gaudi::Property<float> _Qthfrac  {this, 	"ChargeThresholdFrac", 0.05, "Charge threshold fraction"};

  mutable Gaudi::Property<int> fADC{this, 	"ADC", 4096, "Total ADC conuts"};
  mutable Gaudi::Property<int> fNofGain{this, 	"NofGain", 3, "Number of gain modes"};
  mutable Gaudi::Property<int> fADCSwitch{this, 	"ADCSwitch", 4000, "switching point of different gain mode"};
  mutable Gaudi::Property<float> fGainRatio_12{this, 	"GainRatio_12", 15, "Gain-1 over Gain-2"};
  mutable Gaudi::Property<float> fGainRatio_23{this, 	"GainRatio_23", 10, "Gain-2 over Gain-3"};
  mutable Gaudi::Property<float> fEcalCryMipLY{this, 	"EcalCryMipLY", 100, "Crystal light yield (p.e./MIP)"};
  mutable Gaudi::Property<float> fEcalMIPEnergy{this, 	"EcalMIPEnergy", 8.9, "MIP Energy deposit in 1cm BGO (MeV/MIP)"};
  mutable Gaudi::Property<int> fEcalSiPMPixels{this, 	"EcalSiPMPixels", 250000, "Pixels number of SiPM"};
  mutable Gaudi::Property<float> fEcalChargeADCMean{this, 	"EcalChargeADCMean", 5, "ADC per p.e. for Gain-1 (ADC)"};
  mutable Gaudi::Property<float> fEcalChargeADCSigma{this, 	"EcalChargeADCSigma", 2.5, "Sigma of ADC per p.e. for Gain-1 (ADC)"};
  mutable Gaudi::Property<float> fEcalADCError{this, 	"EcalADCError", 0.002, "ADC precision"};
  mutable Gaudi::Property<float> fEcalNoiseADCSigma{this, 	"EcalNoiseADCSigma", 3, "Sigma of electronic noise (ADC)"};
  mutable Gaudi::Property<float> fEcalMIP_Thre{this, 	"EcalMIP_Thre", 0.1, "Energy threshold for single readout end (MIP)"};

  // Output collections
  DataHandle<edm4hep::CalorimeterHitCollection>    w_DigiCaloCol{"DigiCaloCol", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoCaloAssociationCollection>    w_CaloAssociationCol{"ECALBarrelAssoCol", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoCaloParticleAssociationCollection>    w_MCPCaloAssociationCol{"ECALBarrelParticleAssoCol", Gaudi::DataHandle::Writer, this};
};

#endif
