#ifndef HCAL_DIGI_ALG_H 
#define HCAL_DIGI_ALG_H

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
#include "TH1.h"

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

	void Clear();

protected:

  SmartIF<IGeomSvc> m_geosvc;
  typedef std::vector<float> FloatVec;
  typedef std::map<const edm4hep::MCParticle, float> MCParticleToEnergyWeightMap;

	int _nEvt ;
	TRandom3 rndm;
	TFile* m_wfile;
	TTree* t_simHit;

  double m_totE;
	FloatVec m_simHit_x, m_simHit_y, m_simHit_z, m_simHit_E, m_simHit_slice, m_simHit_layer, m_simHit_tower, m_simHit_stave, m_simHit_module, m_simHit_HG, m_simHit_LG;
  std::vector<unsigned long long> m_simHit_cellID;
  std::vector<int> m_simHit_steps;


	dd4hep::rec::CellIDPositionConverter* m_cellIDConverter;
	dd4hep::DDSegmentation::BitFieldCoder* m_decoder;

	Gaudi::Property<float> m_scale{ this, "Scale", 1 };

  // Input collections
  DataHandle<edm4hep::SimCalorimeterHitCollection> r_SimCaloCol{"SimCaloCol", Gaudi::DataHandle::Reader, this};
  mutable Gaudi::Property<std::string> _readoutName{this, "ReadOutName", "CaloHitsCollection", "Readout name"};
  mutable Gaudi::Property<std::string> _filename{this, "OutFileName", "testout.root", "Output file name"};

  //Input parameters
  mutable Gaudi::Property<int>   _writeNtuple{this,  "WriteNtuple", 1, "Write ntuple"};
  mutable Gaudi::Property<int>   _Nskip{this,  "SkipEvt", 0, "Skip event"};
  mutable Gaudi::Property<float> _seed{this,   "Seed", 2131, "Random Seed"};
  mutable Gaudi::Property<int>  _Debug{this,   "Debug", 0, "Debug level"};
  mutable Gaudi::Property<float> r_cali{this,  "CalibrHCAL", 1, "Global calibration coefficients for HCAL"};
  mutable Gaudi::Property<int>   _UseRelDigi{this,   "UseRealisticDigi",  1, "If use the realistic model"};

  //add digitization parameters from AHCAL prototype
  mutable Gaudi::Property<float> _MIPCali{this,   "MIPResponse",  0.000461, "MIP response (/GeV)"};
  mutable Gaudi::Property<float> _Eth_Mip{this,   "MIPThreshold", 0.5, "Energy Threshold (/MIP)"};
  mutable Gaudi::Property<int>   _Pixel{this,   "SiPMPixel",  7284, "number of SiPM pixels"};
  mutable Gaudi::Property<float> _ADCError{this,   "ADCError",  0.0002, "ADC Error (/ADC)"};
  mutable Gaudi::Property<float> _MIPADC{this,   "MIPADCMean",  345.7, "Mean value of MIP response adc (/ADC)"};
  mutable Gaudi::Property<float> _TileRes{this,   "TileNonUniformity",  0.05, "Non-uniformity level of one tile response"};
  mutable Gaudi::Property<float> _PeADCMean{this,   "PeADCMean",  30.0, "Mean value of single photons adc (/ADC)"};
  mutable Gaudi::Property<float> _PeADCSigma{this,   "PeADCSigma",  3.5, "Sigma of single photons adc (/ADC)"};
  mutable Gaudi::Property<float> _BaselineHG{this,   "ADCBaselineHG",  377.4, "Mean value of HG baseline adc (/ADC)"};
  mutable Gaudi::Property<float> _BaselineSigmaHG{this,   "ADCBaselineSigmaHG",  3.3, "Sigma of HG baseline adc (/ADC)"};
  mutable Gaudi::Property<float> _BaselineLG{this,   "ADCBaselineLG",  373.9, "Mean value of LG baseline adc (/ADC)"};
  mutable Gaudi::Property<float> _BaselineSigmaLG{this,   "ADCBaselineSigmaLG",  2.2, "Sigma of LG baseline adc (/ADC)"};
  mutable Gaudi::Property<float> _HLRatio{this,   "ADCHLRatio",  29.9, "The ratio of HG to LG"};
  mutable Gaudi::Property<float> _ADCSwitch{this,   "ADCSwitch",  2930, "transition point from HG to LG (/ADC)"};
  mutable Gaudi::Property<float> _ADCLimit{this,   "ADCLimit",  3000, "ADC saturation of LG (/ADC)"};

  // Output collections
  DataHandle<edm4hep::CalorimeterHitCollection>    w_DigiCaloCol{"DigiCaloCol", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoCaloAssociationCollection>    w_CaloAssociationCol{"HCALBarrelAssoCol", Gaudi::DataHandle::Writer, this};
  DataHandle<edm4hep::MCRecoCaloParticleAssociationCollection>    w_MCPCaloAssociationCol{"HCALBarrelParticleAssoCol", Gaudi::DataHandle::Writer, this};
};

#endif
