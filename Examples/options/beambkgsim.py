#!/usr/bin/env python
#Author: Zhan Li <lizhan@ihep.ac.cn>
#Created [2024-03-07 Thu 14:53]

import os
import sys

import Gaudi.Configuration
from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_, k4DataSvc, GeomSvc
from Configurables import TimeProjectionChamberSensDetTool
from Configurables import GenAlgo
from Configurables import GtGunTool
from Configurables import StdHepRdr
from Configurables import SLCIORdr
from Configurables import HepMCRdr
from Configurables import GenPrinter
from Configurables import GtBeamBackgroundTool
from Configurables import DetSimSvc
from Configurables import DetSimAlg
from Configurables import AnExampleDetElemTool
from Configurables import PodioOutput
from Configurables import ApplicationMgr

seed = [42]

rndmengine = Gaudi.Configuration.HepRndm__Engine_CLHEP__HepJamesRandom_("RndmGenSvc.Engine")
rndmengine.SetSingleton = True
rndmengine.Seeds = seed

rndmgensvc = RndmGenSvc("RndmGenSvc")
rndmgensvc.Engine = rndmengine.name()


dsvc = k4DataSvc("EventDataSvc")
#geometry_option = "CepC_v4-onlyVXD.xml"
#geometry_option = "CepC_v4_onlyTracker.xml"
geometry_option = "CepC_v4.xml"

geometry_path = os.path.join(os.getenv("DETCEPCV4ROOT"), "compact", geometry_option)
geosvc = GeomSvc("GeomSvc")
geosvc.compact = geometry_path

#Previously I do not have these 2 lines
tpc_sensdettool = TimeProjectionChamberSensDetTool("TimeProjectionChamberSensDetTool")
tpc_sensdettool.TypeOption = 1


# Physics Generator
# ------------------ WARNING ------------------
# Files for code test only. 
# Contact @Haoyu Shi (shihy@ihep.ac.cn) for reliable simulation samples and rates. 
# ---------------------------------------------
bg = GtBeamBackgroundTool("GtBeamBackgroundTool")
bg.InputFileMap = {
  "BGB":"/cefs/higgs/guofy/CEPCSW_BeamBkgSim/run/BGB_Higgs.root",
  "BGC":"/cefs/higgs/guofy/CEPCSW_BeamBkgSim/run/BGC_Higgs.root",
  "BTH":"/cefs/higgs/guofy/CEPCSW_BeamBkgSim/run/BTH_Higgs.root",
#  "TSC":"",
#  "Pair":"",
  "SR":"/cefs/higgs/guofy/CEPCSW_BeamBkgSim/run/SRBkg/"
}
bg.InputFormatMap = {
  "BGB":"BeamBackgroundFileParserV1",
  "BGC":"BeamBackgroundFileParserV1",
  "BTH":"BeamBackgroundFileParserV1",
#  "TSC":"BeamBackgroundFileParserV1",
#  "Pair":"BeamBackgroundFileParserV1",
  "SR":"BeamBackgroundFileParserV2"
}
bg.InputRateMap = {  # in Hz
  "BGB":1e7, 
  "BGC":5e6,
  "BTH":5e5,
#  "TSC":"5e5",
#  "Pair":"",
  "SR":1
}
bg.TimeWindow = 1e-6
bg.InputBeamEnergy = 120.
bg.RotationAlongYMap = {
  "BGB":16.5e-3,
  "BGC":16.5e-3,
  "BTH":16.5e-3,
#  "TSC":"16.5e-3",
#  "Pair":"",
  "SR":0
}

gun = GtGunTool("GtGunTool")
gun.Particles = ["gamma"]
gun.EnergyMins = [10]
gun.EnergyMaxs = [10]
gun.ThetaMins = [90]
gun.ThetaMaxs = [90]
gun.PhiMins = [0]
gun.PhiMaxs = [0]

genprinter = GenPrinter("GenPrinter")

genalg = GenAlgo("GenAlgo")
genalg.GenTools = ["GtBeamBackgroundTool", "GtGunTool"]

detsimsvc = DetSimSvc("DetSimSvc")

detsimalg = DetSimAlg("DetSimAlg")
detsimalg.RandomSeeds = seed


detsimalg.RunCmds = []
detsimalg.AnaElems = [
    "Edm4hepWriterAnaElemTool"
]
detsimalg.RootDetElem = "WorldDetElemTool"

example_dettool = AnExampleDetElemTool("AnExampleDetElemTool")


# POD I/O
out = PodioOutput("outputalg")
out.filename = "beam-SETv0.root"
out.outputCommands = ["keep *"]

ApplicationMgr( TopAlg = [genalg, detsimalg, out],
                EvtSel = 'NONE',
                EvtMax = 3,
                ExtSvc = [rndmengine, rndmgensvc, dsvc, geosvc],
)
