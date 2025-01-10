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
#include "TF1.h"
#include "TGraph.h"

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
  double GetBarLengthFromGeo(unsigned long long id); 
	edm4hep::MutableSimCalorimeterHit find(const std::vector<edm4hep::MutableSimCalorimeterHit>& m_col, unsigned long long& cellid) const;
  // double Digitization(double edepCry, double fCrosstalkChannel);
  float EnergyDigi(float ScinGen, float sEcalCryMipLY, float sEcalSiPMGainMean, float sEcalSiPMDCR, 
                    TF1* f_SiPMResponse, TF1* f_SiPMSigmaDet, TF1* f_SiPMSigmaRecp, TF1* f_SiPMSigmaRecm, TF1* f_AsymGauss, TF1* f_DarkNoise, TF1* f_ADCNonLin,
                    int& outLO, int& outNDC, int& outNDetPE, float& outPedestal, float& outADC, float& outADCGain);

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

  double totE_Truth, totE_Truth_MIP, totE_Digi;
  double mean_CT, ECALTemp;
  FloatVec m_step_t;  // yyy: time of each step
	FloatVec m_step_x, m_step_y, m_step_z, m_step_E, m_step_T1, m_step_T2, m_stepBar_x, m_stepBar_y, m_stepBar_z;
	FloatVec m_simBar_x, m_simBar_y, m_simBar_z, 
            m_simBar_E_Truth, m_simBar_Scint, 
            m_simBar_ChannelTemp, m_simBar_CryTID, m_simBar_SiPMNIEL, m_simBar_CryIntLY, m_simBar_SiPMGain, m_simBar_SiPMDCR,
            m_simBar_LO1, m_simBar_LO2, m_simBar_NDC1, m_simBar_NDC2, m_simBar_NDetPE1, m_simBar_NDetPE2, 
            m_simBar_Pedestal1, m_simBar_Pedestal2, m_simBar_ADC1, m_simBar_ADC2, m_simBar_ADCGain1, m_simBar_ADCGain2,
            m_simBar_T1, m_simBar_T2, 
            m_simBar_Q1_Att, m_simBar_Q2_Att, m_simBar_Q1_Digi, m_simBar_Q2_Digi, 
            m_simBar_length,
            m_simBar_bar, m_simBar_dlayer, m_simBar_slayer, m_simBar_stave, m_simBar_part, m_simBar_module, m_simBar_system;
  std::vector<unsigned long long> m_simBar_cellID;


  dd4hep::Detector* m_dd4hep;
	dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
	//dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  std::map<std::string, dd4hep::DDSegmentation::BitFieldCoder*> map_readout_decoder;
  dd4hep::VolumeManager m_volumeManager;

	TF1* f_SiPMResponse = nullptr;
  TF1* f_SiPMSigmaDet = nullptr;
  TF1* f_SiPMSigmaRecp = nullptr;
  TF1* f_SiPMSigmaRecm = nullptr;
  TF1* f_AsymGauss = nullptr;
  TF1* f_DarkNoise = nullptr;
  TF1* f_ADCNonLin = nullptr;
  TGraph* g_SiPMDCR_vs_NIEL = nullptr;
  TGraph* g_CryLYRatio_vs_TID = nullptr;

  // Input and collections
  typedef DataHandle<edm4hep::SimCalorimeterHitCollection>              SimCaloType;
  typedef DataHandle<edm4hep::CalorimeterHitCollection>                 CaloType;
  typedef DataHandle<edm4hep::MCRecoCaloAssociationCollection>          CaloSimAssoType;      //Calorimeter - SimCalorimeter
  typedef DataHandle<edm4hep::MCRecoCaloParticleAssociationCollection>  CaloParticleAssoType; //Calorimeter - MCParticle

  Gaudi::Property< std::vector<std::string> > name_SimCaloHit{ this, "SimCaloHitCollection", {"EcalBarrelCollection"} };
  Gaudi::Property< std::vector<std::string> > name_Readout{ this, "ReadOutName", {"EcalBarrelCollection"} };
  Gaudi::Property< std::vector<std::string> > name_CaloHit{ this, "CaloHitCollection", {"ECALBarrel"} };
  Gaudi::Property< std::vector<std::string> > name_CaloAsso{ this, "CaloAssociationCollection", {"ECALBarrelAssoCol"} };
  Gaudi::Property< std::vector<std::string> > name_CaloMCPAsso{ this, "CaloMCPAssociationCollection", {"ECALBarrelParticleAssoCol"} };

  std::vector<SimCaloType*> _inputSimHitCollection;
  std::vector<CaloType*> _outputHitCollection;
  std::vector<CaloSimAssoType*> _outputCaloSimAssoCol;
  std::vector<CaloParticleAssoType*> _outputCaloMCPAssoCol;

  //DataHandle<edm4hep::SimCalorimeterHitCollection> r_SimCaloCol{"SimCaloCol", Gaudi::DataHandle::Reader, this};

  //mutable Gaudi::Property<std::string> _readoutName{this, "ReadOutName", "CaloHitsCollection", "Readout name"};
  mutable Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};
  mutable Gaudi::Property<int>   _UseRelDigi{this,   "UseRealisticDigi",  1, "If use the realistic model"};

  //Input parameters
  mutable Gaudi::Property<int>   _writeNtuple{this,  "WriteNtuple", 1, "Write ntuple"};
  mutable Gaudi::Property<int>   _Nskip{this,  "SkipEvt", 0, "Skip event"};
  mutable Gaudi::Property<float> _seed{this,   "Seed", 2131, "Random Seed"};
  mutable Gaudi::Property<int>  _Debug{this,   "Debug", 0, "Debug level"};
  mutable Gaudi::Property<float> Latt{this, 	"AttenuationLength", 8000, "Crystal Attenuation Length(mm)"};
  mutable Gaudi::Property<float> Tres{this, 	"TimeResolution", 0.1, "Crystal time resolution in one side (ns)"};
  mutable Gaudi::Property<float> nMat{this, 	"MatRefractive", 2.15, "Material refractive index of crystal"};
  mutable Gaudi::Property<float> Tinit{this, 	"InitalTime", 2, "Start time (ns)"}; 
  mutable Gaudi::Property<float> _Qthfrac  {this, 	"ChargeThresholdFrac", 0.05, "Charge threshold fraction"};

  mutable Gaudi::Property<int> fUseDigiScint{this, "UseDigiScint", 1, "Add scintillation effect in digitization"};
  mutable Gaudi::Property<int> fUseCryTemp{this, "UseCryTemp", 1, "Add crystal temperature effect in digitization"};
  mutable Gaudi::Property<int> fUseCryTempCor{this, "UseCryTempCor", 1, "Add correction on crystal temperature effect in digitization"};
  mutable Gaudi::Property<int> fUseSiPMTemp{this, "UseSiPMTemp", 1, "Add SiPM temperature effect in digitization"};
  mutable Gaudi::Property<int> fUseSiPMTempCor{this, "UseSiPMTempCor", 1, "Add correction on SiPM temperature effect in digitization"};
  mutable Gaudi::Property<int> fUseCryTID{this, "UseCryTID", 0, "Add TID effect on crystal"};
  mutable Gaudi::Property<int> fUseCryTIDCor{this, "UseCryTIDCor", 0, "Add correction on TID effect on crystal"};
  mutable Gaudi::Property<int> fUseSiPMNIEL{this, "UseSiPMNIEL", 1, "Add NIEL effect on SiPM"};
  mutable Gaudi::Property<int> fUseSiPMNIELCor{this, "UseSiPMNIELCor", 0, "Add NIEL effect on SiPM"};

  mutable Gaudi::Property<int> fADC{this, 	"ADC", 8192, "Total ADC conuts"};
  mutable Gaudi::Property<int> fNofGain{this, 	"NofGain", 3, "Number of gain modes"};
  mutable Gaudi::Property<int> fADCSwitch{this, 	"ADCSwitch", 8000, "switching point of different gain mode"};
  mutable Gaudi::Property<float> fPedestal{this, 	"Pedestal", 50, "ADC value of pedestal"};
  mutable Gaudi::Property<int> fSiPMDigiVerbose{this, 	"SiPMDigiVerbose", 1, "SiPM Digitization verbose. 0:w/o response, w/o correction; 1:w/ response, w/o correction; 2:w/ response, w/ simple correction; 3:w/ response, w/ full correction;"};
  mutable Gaudi::Property<float> fGainRatio_12{this, 	"GainRatio_12", 50, "Gain-1 over Gain-2"};
  mutable Gaudi::Property<float> fGainRatio_23{this, 	"GainRatio_23", 60, "Gain-2 over Gain-3"};
  mutable Gaudi::Property<float> fADCNonLin{this, 	"ADCNonLinearity", 0.01, "ADC non-linearity"};
  mutable Gaudi::Property<float> fEcalMIPEnergy{this, 	"EcalMIPEnergy", 8.9, "MIP energy deposit in 1cm BGO (MeV/MIP)"};
  mutable Gaudi::Property<float> fEcalCryMipLY{this, 	"EcalCryMipLY", 200, "Detected light yield (p.e./MIP)"};
  mutable Gaudi::Property<float> fEcalCryIntLY{this, 	"EcalCryIntLY", 8200, "Intrinsic light yield (ph/MeV)"};
  mutable Gaudi::Property<float> fEcalSiPMPDE{this, 	"EcalSiPMPDE", 0.25, "SiPM PDE"};
  mutable Gaudi::Property<float> fEcalSiPMDCR{this, 	"EcalSiPMDCR", 2500000, "SiPM Dark Count Rate (Hz)"};
  mutable Gaudi::Property<float> fEcalSiPMCT{this, 	"EcalSiPMCT", 0.12, "SiPM crosstalk Probability"};
  mutable Gaudi::Property<float> fEcalTimeInterval{this, 	"EcalTimeInterval", 0.00000015, "Time interval for one readout (s)"};
  //mutable Gaudi::Property<float> fEcalCryAtt{this, 	"EcalCryAtt", fEcalCryMipLY/(fEcalCryIntLY*fEcalSiPMPDE*fEcalMIPEnergy), "Ratio of light attenuation, changed with SiPM PDE to achieve the assigned MIP light yield"};
  
  mutable Gaudi::Property<float> fEcalSiPMGainMean{this, 	"EcalSiPMGainMean", 50, "SiPM gain: ADC per p.e. for Gain-1 (ADC)"};
  mutable Gaudi::Property<float> fEcalSiPMGainSigma{this, 	"EcalSiPMGainSigma", 0.08, "Fluctuation of single photoelctron ADC around the mean value for single device (%)"};
  mutable Gaudi::Property<float> fEcalASICNoiseSigma{this, 	"EcalASICNoiseSigma", 4, "Sigma of ASIC noise (ADC), which does not depend on gain"};
  mutable Gaudi::Property<float> fEcalFEENoiseSigma{this, 	"EcalFEENoiseSigma", 0, "Sigma of front-end-electronics noise (ADC), which depends on gain"};
  mutable Gaudi::Property<float> fEcalMIP_Thre{this, 	"EcalMIP_Thre", 0.05, "Energy threshold for single readout channel (MIP)"};

  mutable Gaudi::Property<float> fEcalTempRef{this, 	"EcalTempRef", 298.15, "Reference temperature, top layer (K)"};
  mutable Gaudi::Property<float> fEcalTempFluc{this, 	"EcalTempFluc", 3, "ECAL temperature fluctuation around 25 degrees (K)"};
  mutable Gaudi::Property<float> fEcalTempGrad{this, 	"EcalTempGrad", 3./27, "The radial temperature gradient in the ECAL (K/crystal)"};
  mutable Gaudi::Property<float> fEcalBGOTempCoef{this, 	"EcalBGOTempCoef", -0.0138, "Temperature dependence of BGO light yield (%/K)"}; // doi:10.1007/s11433-014-5548-4
  mutable Gaudi::Property<float> fEcalSiPMGainTempCoef{this, 	"EcalSiPMGainTempCoef", -0.03, "Temperature dependence of SiPM gain (%/K)"}; // doi:10.1016/j.nima.2016.09.053
  mutable Gaudi::Property<float> fEcalSiPMDCRTempCoef{this, 	"EcalSiPMDCRTempCoef", 3.34/80, "Temperature dependence of SiPM DCR (10^{k*deltaT})"}; // doi:10.1016/j.nima.2016.09.053

  mutable Gaudi::Property<float> fEcalCryTID{this, 	"EcalCryTID", 10, "Total ionizing dose in crystal (rad})"};
  mutable Gaudi::Property<float> fEcalSiPMNIEL{this, 	"EcalSiPMNIEL", 10, "Non-ionizing energy loss in SiPM, expressed as the equivalent 1 MeV neutron flux (cm-2s-1})"};

  // mutable Gaudi::Property<float> fEcalCryIntLYFlu{this, 	"EcalCryIntLYFlu", 0.1, "Fluctuation of crystal intrinsic light yield"};
  // mutable Gaudi::Property<float> fEcalSiPMPDEFlu{this, 	"EcalSiPMPDEFlu", 0.10, "Fluctuation of SiPM PDE"};
  // mutable Gaudi::Property<float> fEcalCryLYUn{this, 	"EcalCryLYUn", 0.00, "Uncertainty of light yield calibration"};
  // mutable Gaudi::Property<float> fEcalSiPMGainUn{this, 	"EcalSiPMGainUn", 0.00, "Uncertainty of SiPM gain calibration"};
  // mutable Gaudi::Property<float> fEcalSiPMGainMeanFlu{this, 	"EcalSiPMGainMeanFlu", 0.15, "Fluctuation of SiPM gain"};
  // mutable Gaudi::Property<float> fEcalADCError{this, 	"EcalADCError", 0.0, "ADC precision"};


  // Output collections
  //DataHandle<edm4hep::CalorimeterHitCollection>    w_DigiCaloCol{"DigiCaloCol", Gaudi::DataHandle::Writer, this};
  //DataHandle<edm4hep::MCRecoCaloAssociationCollection>    w_CaloAssociationCol{"ECALBarrelAssoCol", Gaudi::DataHandle::Writer, this};
  //DataHandle<edm4hep::MCRecoCaloParticleAssociationCollection>    w_MCPCaloAssociationCol{"ECALBarrelParticleAssoCol", Gaudi::DataHandle::Writer, this};
};

#endif
