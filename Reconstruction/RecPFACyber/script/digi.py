#!/usr/bin/env python
import os
from Gaudi.Configuration import *

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="Sim_TDR_o1_v01.root")

from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_
seed = [12340]
# rndmengine = HepRndm__Engine_CLHEP__RanluxEngine_() # The default engine in Gaudi                                                                                                                                            
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_("RndmGenSvc.Engine") # The default engine in Geant4                                                                                                                        
rndmengine.SetSingleton = True
rndmengine.Seeds = seed

rndmgensvc = RndmGenSvc("RndmGenSvc")
rndmgensvc.Engine = rndmengine.name()

geometry_option = "TDR_o1_v01/TDR_o1_v01.xml"

if not os.getenv("DETCRDROOT"):
    print("Can't find the geometry. Please setup envvar DETCRDROOT." )
    sys.exit(-1)

geometry_path = os.path.join(os.getenv("DETCRDROOT"), "compact", geometry_option)
if not os.path.exists(geometry_path):
    print("Can't find the compact geometry file: %s"%geometry_path)
    sys.exit(-1)

from Configurables import DetGeomSvc
geosvc = DetGeomSvc("GeomSvc")
geosvc.compact = geometry_path

from Configurables import MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")

from Configurables import GearSvc
gearsvc = GearSvc("GearSvc")

from Configurables import TrackSystemSvc
tracksystemsvc = TrackSystemSvc("TrackSystemSvc")

from Configurables import SimplePIDSvc
pidsvc = SimplePIDSvc("SimplePIDSvc")
cepcswdatatop = "/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/data/latest"
pidsvc.ParFile = os.path.join(cepcswdatatop, "CEPCSWData/offline-data/Service/SimplePIDSvc/data/dNdx_TPC.root")

from Configurables import PodioInput
podioinput = PodioInput("PodioReader", collections=[
#    "EventHeader",
    "MCParticle",
    "EcalBarrelCollection", 
    "EcalBarrelContributionCollection",
    "EcalEndcapsCollection",
    "EcalEndcapsContributionCollection", 
    "HcalBarrelCollection", 
    "HcalBarrelContributionCollection", 
    "HcalEndcapsCollection",
    "HcalEndcapsContributionCollection"
    ])

########## Digitalization ################

##ECAL##
from Configurables import EcalDigiAlg
EcalDigi = EcalDigiAlg("EcalDigiAlg")
EcalDigi.SimCaloHitCollection = ["EcalBarrelCollection", "EcalEndcapsCollection"]
EcalDigi.ReadOutName = ["EcalBarrelCollection", "EcalEndcapsCollection"]
EcalDigi.CaloHitCollection = ["ECALBarrel", "ECALEndcaps"]
EcalDigi.CaloAssociationCollection = ["ECALBarrelAssoCol", "ECALEndcapsAssoCol"]
EcalDigi.CaloMCPAssociationCollection = ["ECALBarrelParticleAssoCol", "ECALEndcapsParticleAssoCol"]
EcalDigi.SkipEvt = 0
EcalDigi.Seed = 2079
#Digitalization parameters
EcalDigi.TimeResolution = 0.7            # 0.7 ns
EcalDigi.EcalMIPEnergy = 13.35            # MIP energy 13.35 MeV for 1.5 cm BGO
EcalDigi.EcalMIP_Thre = 0.05              # 0.05 mip at each side, 0.1 mip for one bar
EcalDigi.UseRealisticDigi = 1
# scintillation
EcalDigi.UseDigiScint = 1
EcalDigi.EcalCryIntLY = 8200             #intrinsic LY 8200 [p.e./MIP]
EcalDigi.EcalCryMipLY = 300              #Detected effective LY 300 [p.e./MIP]
EcalDigi.AttenuationLength = 1e8         # 8000 mm for 5% non-uniformity
# SiPM
EcalDigi.SiPMDigiVerbose = 2             # 0:w/o response, w/o correction; 1:w/ response, w/o correction; 2:w/ response, w/ simple correction; 3:w/ response, w/ full correction
EcalDigi.EcalSiPMPDE = 0.25              # NDL-EQR06, PDE 0.25
EcalDigi.EcalSiPMDCR = 0                 # NDL-EQR06, dark count rate 2500000 [Hz]
EcalDigi.EcalTimeInterval = 0.           # Time interval 0.000002 [s]. DCR*TimeInterval = dark count noise
EcalDigi.EcalSiPMCT = 0.                 # SiPM crosstalk Probability 12%
EcalDigi.EcalSiPMGainMean = 5            # 5 [ADC/p.e.]
EcalDigi.EcalSiPMGainSigma = 0.08        # 0.08
#EcalDigi.EcalSiPMNoiseSigma = 0          # 0
# ADC
EcalDigi.ADC = 8192                      # 13-bit, 8192
EcalDigi.ADCSwitch = 8000                # 8000
EcalDigi.Pedestal = 50                   # Pedestal 50 ADC
EcalDigi.GainRatio_12 = 30               # Gain ratio 30
EcalDigi.GainRatio_23 = 10               # Gain ratio 10
EcalDigi.EcalASICNoiseSigma = 4
EcalDigi.EcalFEENoiseSigma = 5
EcalDigi.ADCNonLinearity = 0             # ADC non-linearity 0
# temperature control
EcalDigi.UseCryTemp = 0
EcalDigi.UseCryTempCor = 0
EcalDigi.UseSiPMTemp = 0
EcalDigi.UseSiPMTempCor = 0
EcalDigi.EcalTempGrad = 3./27            # 3./27 = 3K from 0 to 27 layer
EcalDigi.EcalBGOTempCoef = -0.0138       # -0.0138 [%/K]
EcalDigi.EcalSiPMGainTempCoef = -0.03    # -0.03 [%/K]
EcalDigi.EcalSiPMDCRTempCoef = 3.34/80   # 3.34/80 [10^{k*deltaT}]
#########################################
EcalDigi.WriteNtuple = 0

##HCAL##
from Configurables import HcalDigiAlg
HcalDigi = HcalDigiAlg("HcalDigiAlg")
HcalDigi.SimCaloHitCollection = ["HcalBarrelCollection", "HcalEndcapsCollection"]
HcalDigi.ReadOutName = ["HcalBarrelCollection", "HcalEndcapsCollection"]
HcalDigi.CaloHitCollection = ["HCALBarrel", "HCALEndcaps"]
HcalDigi.CaloAssociationCollection = ["HCALBarrelAssoCol", "HCALEndcapsAssoCol"]
HcalDigi.CaloMCPAssociationCollection = ["HCALBarrelParticleAssoCol", "HCALEndcapsParticleAssoCol"]
HcalDigi.SkipEvt = 0
HcalDigi.Seed = 2079
HcalDigi.CalibrHCAL = 1.
#Digitalization parameters
HcalDigi.UseRealisticDigi = 0     #---------Flag to use digitization model.
HcalDigi.MIPResponse = 0.007126   # 0.007.126 GeV / MIP
HcalDigi.MIPThreshold = 0.1       # ----------Unit: MIP
HcalDigi.TemperatureVariation = 0 # Temperature variation 1K
# Scintillation
HcalDigi.UseTileLYMap = 0
HcalDigi.MIPLY = 80               # Glass LY
HcalDigi.LYTempCoef = 0           # Glass LY with temperature
HcalDigi.TileNonUniformity = 0.0
# SiPM
HcalDigi.SiPMPixel = 57600           #---------57600 for 6025PE (6*6 mm, 25 um pixel pitch)
HcalDigi.SiPMDCR = 1600              #---------1600 for 6025PE (6*6 mm, 25 um pixel pitch), 3200 for 6015PS ()
HcalDigi.SiPMCT = 0.0                #---------SiPM crosstalk Probability 0.12
HcalDigi.TimeInterval = 0.           #---------Shaping time 2 us
HcalDigi.SiPMGainTempCoef = 0        #---------Temperature dependence of SiPM gain (-3%/K)
HcalDigi.SiPMDCRTempCoef = 0         #---------Temperature dependence of SiPM DCR (10^{k*deltaT}, k=3.34/80)
# ADC
HcalDigi.ADC = 8192
HcalDigi.ADCSwitch = 1e7          # Switch at 8000
HcalDigi.GainRatio_12 = 50
HcalDigi.GainRatio_23 = 60
HcalDigi.SiPMGainMean = 20        # SiPM gain: 2 ADC / p.e.
HcalDigi.SiPMGainSigma = 0.08     # Fluctuation of ADC / p.e.
HcalDigi.SiPMNoiseSigma = 0       # SiPM noise sigma
HcalDigi.Pedestal = 50            # Pedestal 50 ADC
HcalDigi.PedestalSigma = 4        # Sigma of electronic noise (4 ADC)
HcalDigi.WriteNtuple = 0


# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "CaloDigi_TDR_o1_v01.root"
out.outputCommands = ["drop *", 
    "keep MCParticle",
    "keep VXDCollection",
    "keep ITKBarrelCollection",
    "keep TPCCollection",
    "keep OTKBarrelCollection",
    "keep FTDCollection",
    "keep MuonBarrelCollection",
    "keep MuonEndcapCollection",
    "keep ECALBarrel",
    "keep HCALBarrel",
    "keep ECALBarrelParticleAssoCol",
    "keep HCALBarrelParticleAssoCol",
    "keep ECALEndcaps",
    "keep HCALEndcaps",
    "keep ECALEndcapsParticleAssoCol",
    "keep HCALEndcapsParticleAssoCol" ]


# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [podioinput, EcalDigi, HcalDigi, out],
    EvtSel = 'NONE',
    EvtMax = 10,
    ExtSvc = [dsvc, rndmengine, rndmgensvc, geosvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = ERROR
)
