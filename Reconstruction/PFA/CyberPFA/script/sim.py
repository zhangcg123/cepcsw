#!/usr/bin/env python
import os
from Gaudi.Configuration import *

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")

from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_
seed = [12340]
# rndmengine = HepRndm__Engine_CLHEP__RanluxEngine_() # The default engine in Gaudi
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_("RndmGenSvc.Engine") # The default engine in Geant4
rndmengine.SetSingleton = True
rndmengine.Seeds = seed

rndmgensvc = RndmGenSvc("RndmGenSvc")
rndmgensvc.Engine = rndmengine.name()

# option for standalone tracker study
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

##############################################################################
# Physics Generator
##############################################################################
from Configurables import GenAlgo
from Configurables import GtGunTool
from Configurables import StdHepRdr
from Configurables import SLCIORdr
from Configurables import HepMCRdr
from Configurables import GenPrinter

#gun = GtGunTool("GtGunTool")
#gun.PositionXs = [0]
#gun.PositionYs = [0]
#gun.PositionZs = [0]
#gun.Particles = ["pi-"]
#gun.EnergyMins = [10]
#gun.EnergyMaxs = [10]
#gun.ThetaMins  = [60]
#gun.ThetaMaxs  = [120]
#gun.PhiMins    = [0]
#gun.PhiMaxs    = [360]
#genprinter = GenPrinter("GenPrinter")

stdheprdr = StdHepRdr("StdHepRdr")
stdheprdr.Input = "/cefs/data/stdhep/CEPC240/higgs/update_from_LiangHao_1M/data/E240.Pnnh_gg.e0.p0.whizard195/nnh_gg.e0.p0.00001.stdhep"


genalg = GenAlgo("GenAlgo")
#genalg.GenTools = ["GtGunTool"]
genalg.GenTools = ["StdHepRdr"]

##############################################################################
# Detector Simulation
##############################################################################
from Configurables import DetSimSvc
detsimsvc = DetSimSvc("DetSimSvc")

from Configurables import DetSimAlg
detsimalg = DetSimAlg("DetSimAlg")
detsimalg.RandomSeeds = seed
# detsimalg.VisMacs = ["vis.mac"]
detsimalg.RunCmds = [
#    "/tracking/verbose 1",
]
detsimalg.AnaElems = [
    # example_anatool.name()
    # "ExampleAnaElemTool"
    "Edm4hepWriterAnaElemTool"
]
detsimalg.RootDetElem = "WorldDetElemTool"

from Configurables import TimeProjectionChamberSensDetTool
tpc_sensdettool = TimeProjectionChamberSensDetTool("TimeProjectionChamberSensDetTool")
tpc_sensdettool.TypeOption = 1

    
from Configurables import CalorimeterSensDetTool
from Configurables import DriftChamberSensDetTool
cal_sensdettool = CalorimeterSensDetTool("CalorimeterSensDetTool")
cal_sensdettool.CalNamesMergeDisable = ["EcalBarrel", "CaloDetectorEndcap", "HcalBarrel", "HcalEndcaps"]
cal_sensdettool.CalNamesApplyBirks = ["EcalBarrel", "CaloDetectorEndcap", "HcalBarrel","HcalEndcaps"]
cal_sensdettool.CalNamesBirksConstants = [0.008415, 0.008415, 0.126, 0.126] # BGO and Glass scintillator

# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "Sim_TDR_o1_v01_E240_nnHgg.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [genalg, detsimalg, out],
    EvtSel = 'NONE',
    EvtMax = 3,
    ExtSvc = [rndmengine, rndmgensvc, dsvc, geosvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = INFO
)
