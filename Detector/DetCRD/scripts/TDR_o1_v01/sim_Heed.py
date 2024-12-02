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

gun = GtGunTool("GtGunTool")
gun.PositionXs = [0]
gun.PositionYs = [0]
gun.PositionZs = [0]
gun.Particles = ["mu-"]
gun.EnergyMins = [1]
gun.EnergyMaxs = [100]
gun.ThetaMins  = [8]
gun.ThetaMaxs  = [172]
gun.PhiMins    = [0]
gun.PhiMaxs    = [360]

genprinter = GenPrinter("GenPrinter")

genalg = GenAlgo("GenAlgo")
genalg.GenTools = ["GtGunTool"]

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
tpc_sensdettool.DoHeedSim = True
dedxoption = "TrackHeedSimTool"
tpc_sensdettool.DedxSimTool = dedxoption

from Configurables import TrackHeedSimTool
dedx_simtool = TrackHeedSimTool("TrackHeedSimTool")
dedx_simtool.detector = "TPC"
dedx_simtool.only_primary = False#True
dedx_simtool.use_max_step = False#True
dedx_simtool.max_step = 1#mm
dedx_simtool.save_mc = True



from Configurables import MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")
    
# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "simHeed00.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [genalg, detsimalg, out],
    EvtSel = 'NONE',
    EvtMax = 10,
    ExtSvc = [rndmengine, rndmgensvc, dsvc, geosvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = ERROR
)
