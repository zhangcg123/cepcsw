#!/usr/bin/env python
import os
from Gaudi.Configuration import *

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="rec00.root")

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

from Configurables import GeomSvc
geosvc = GeomSvc("GeomSvc")
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
    "HcalBarrelCollection", 
    "HcalBarrelContributionCollection", 
    "CompleteTracks", 
    "CompleteTracksParticleAssociation"
    ])

########## Digitalization ################

##ECAL##
from Configurables import EcalDigiAlg
EcalDigi = EcalDigiAlg("EcalDigiAlg")
EcalDigi.ReadOutName = "EcalBarrelCollection"
EcalDigi.SimCaloHitCollection = "EcalBarrelCollection"
EcalDigi.CaloHitCollection = "ECALBarrel"
EcalDigi.CaloAssociationCollection = "ECALBarrelAssoCol"
EcalDigi.CaloMCPAssociationCollection = "ECALBarrelParticleAssoCol"
EcalDigi.SkipEvt = 0
EcalDigi.Seed = 2079
#Digitalization parameters
EcalDigi.CalibrECAL = 1.
EcalDigi.AttenuationLength = 7e10
EcalDigi.TimeResolution = 0.5        #unit: ns
EcalDigi.ChargeThresholdFrac = 0.05
EcalDigi.Debug=1
EcalDigi.WriteNtuple = 0
EcalDigi.OutFileName = "Digi_ECAL.root"
#########################################

##HCAL##
from Configurables import HcalDigiAlg
HcalDigi = HcalDigiAlg("HcalDigiAlg")
HcalDigi.ReadOutName = "HcalBarrelCollection"
HcalDigi.SimCaloHitCollection = "HcalBarrelCollection"
HcalDigi.CaloHitCollection = "HCALBarrel"
HcalDigi.CaloAssociationCollection = "HCALBarrelAssoCol"
HcalDigi.CaloMCPAssociationCollection = "HCALBarrelParticleAssoCol"
HcalDigi.SkipEvt = 0
HcalDigi.Seed = 2079
#Digitalization parameters
HcalDigi.MIPResponse = 0.01  # 0.5 MeV / MIP
HcalDigi.MIPThreshold = 0.5    # Unit: MIP
HcalDigi.CalibrHCAL = 1.
HcalDigi.Debug=0
HcalDigi.WriteNtuple = 0
HcalDigi.OutFileName = "Digi_HCAL.root"


# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "CaloDigi_TDR_o1_v01_00.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [podioinput, EcalDigi, HcalDigi, out],
    EvtSel = 'NONE',
    EvtMax = 5,
    ExtSvc = [dsvc, rndmengine, rndmgensvc, geosvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = ERROR
)
