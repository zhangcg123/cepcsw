#!/usr/bin/env python

import os
import sys
# sys.exit(0)

from Gaudi.Configuration import *

##############################################################################
# Random Number Svc
##############################################################################
from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_

seed = [42]

# rndmengine = HepRndm__Engine_CLHEP__RanluxEngine_() # The default engine in Gaudi
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_("RndmGenSvc.Engine") # The default engine in Geant4
rndmengine.SetSingleton = True
rndmengine.Seeds = seed

rndmgensvc = RndmGenSvc("RndmGenSvc")
rndmgensvc.Engine = rndmengine.name()


##############################################################################
# Event Data Svc
##############################################################################
from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")


##############################################################################
# Geometry Svc
##############################################################################

# geometry_option = "CepC_v4-onlyTracker.xml"
#geometry_option = "CepC_v4-onlyVXD.xml"
geometry_option = "CepC_v4.xml"

if not os.getenv("DETCEPCV4ROOT"):
    print("Can't find the geometry. Please setup envvar DETCEPCV4ROOT." )
    sys.exit(-1)

geometry_path = os.path.join(os.getenv("DETCEPCV4ROOT"), "compact", geometry_option)
if not os.path.exists(geometry_path):
    print("Can't find the compact geometry file: %s"%geometry_path)
    sys.exit(-1)

from Configurables import DetGeomSvc
geosvc = DetGeomSvc("GeomSvc")
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
gun.Particles = ["mu-"]
gun.EnergyMins = [120.] # GeV
gun.EnergyMaxs = [120.] # GeV

gun.ThetaMins = [0] # rad; 45deg
gun.ThetaMaxs = [180.] # rad; 45deg

gun.PhiMins = [0] # rad; 0deg
gun.PhiMaxs = [0.] # rad; 360deg

gun.PositionXs = [0.]
gun.PositionYs = [0.]
gun.PositionZs = [0.]
# stdheprdr = StdHepRdr("StdHepRdr")
# stdheprdr.Input = "/cefs/data/stdhep/CEPC250/2fermions/E250.Pbhabha.e0.p0.whizard195/bhabha.e0.p0.00001.stdhep"

# lciordr = SLCIORdr("SLCIORdr")
# lciordr.Input = "/cefs/data/stdhep/lcio250/signal/Higgs/E250.Pbbh.whizard195/E250.Pbbh_X.e0.p0.whizard195/Pbbh_X.e0.p0.00001.slcio"

# hepmcrdr = HepMCRdr("HepMCRdr")
# hepmcrdr.Input = "example_UsingIterators.txt"

genprinter = GenPrinter("GenPrinter")

genalg = GenAlgo("GenAlgo")
genalg.GenTools = ["GtGunTool"]
# genalg.GenTools = ["StdHepRdr"]
# genalg.GenTools = ["StdHepRdr", "GenPrinter"]
# genalg.GenTools = ["SLCIORdr", "GenPrinter"]
# genalg.GenTools = ["HepMCRdr", "GenPrinter"]

##############################################################################
# Detector Simulation
##############################################################################
from Configurables import DetSimSvc

detsimsvc = DetSimSvc("DetSimSvc")

# from Configurables import ExampleAnaElemTool
# example_anatool = ExampleAnaElemTool("ExampleAnaElemTool")

from Configurables import DetSimAlg

detsimalg = DetSimAlg("DetSimAlg")
detsimalg.RandomSeeds = seed

if int(os.environ.get("VIS", 0)):
      detsimalg.VisMacs = ["vis.mac"]

detsimalg.RunCmds = [
#    "/tracking/verbose 1",
]
detsimalg.AnaElems = [
    # example_anatool.name()
    # "ExampleAnaElemTool"
    "ExampleAnaDoseElemTool"
]
detsimalg.RootDetElem = "WorldDetElemTool"

from Configurables import ExampleAnaDoseElemTool
dosesimtool = ExampleAnaDoseElemTool("ExampleAnaDoseElemTool")
dosesimtool.Gridnbins = [450,1,1100]
dosesimtool.Coormin = [0,-0.5,-550]
dosesimtool.Coormax = [450,0.5,550]
dosesimtool.Regionhist = [20,1.e-12,10]
dosesimtool.filename = "testdose_"
dosesimtool.Dosecofffilename = "dosecoffe/"
dosesimtool.HEHadroncut = 0.02

from Configurables import CalorimeterSensDetTool
from Configurables import DriftChamberSensDetTool

from Configurables import TimeProjectionChamberSensDetTool
tpc_sensdettool = TimeProjectionChamberSensDetTool("TimeProjectionChamberSensDetTool")
tpc_sensdettool.TypeOption = 1


##############################################################################
# POD I/O
##############################################################################
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "test-dosesim.root"
out.outputCommands = ["keep *"]

##############################################################################
# ApplicationMgr
##############################################################################
#from Configurables import NTupleSvc
#ntsvc = NTupleSvc("NTupleSvc")
#ntsvc.Output = ["MyTuples DATAFILE='testout.root' OPT='NEW' TYP='ROOT'"]

from Configurables import ApplicationMgr
ApplicationMgr( TopAlg = [genalg, detsimalg, out],
                EvtSel = 'NONE',
                EvtMax = 10,
                ExtSvc = [rndmengine, rndmgensvc, dsvc, geosvc],
		OutputLevel=ERROR
)
