#!/usr/bin/env python
import os
from Gaudi.Configuration import *

momenta_low = .502
theta_low = 85
inputseed = 2
particlename = 'mu-'
evtmax = 1000

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")

from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_
seed = [inputseed]
# rndmengine = HepRndm__Engine_CLHEP__RanluxEngine_() # The default engine in Gaudi
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_("RndmGenSvc.Engine") # The default engine in Geant4
rndmengine.SetSingleton = True
rndmengine.Seeds = seed
# avoid lock
props = rndmengine.getProperties()
if "ThreadSafe" in props:
    rndmengine.ThreadSafe = False

rndmgensvc = RndmGenSvc("RndmGenSvc")
rndmgensvc.Engine = rndmengine.name()

# option for standalone tracker study
#geometry_option = "TDR_o1_v01/TDR_o1_v01-oldVersion.xml"
#geometry_option = "TDR_o1_v01/TDR_o1_v01-patchOTK.xml"
geometry_option = "TDR_o1_v01/TDR_o1_v01.xml"
# geometry_option = "TDR_o1_v01/TDR_o1_v01-NonuniformField.xml"

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

##############################################################################
# Physics Generator
##############################################################################
from Configurables import GenAlgo
from Configurables import GtGunTool
from Configurables import StdHepRdr
from Configurables import SLCIORdr
from Configurables import HepMCRdr
from Configurables import GenPrinter

########### Particle Gun ##############
gun = GtGunTool("GtGunTool")
gun.PositionXs = [0]
gun.PositionYs = [0]
gun.PositionZs = [0]
gun.Particles = [particlename]
gun.EnergyMins = [momenta_low]
gun.EnergyMaxs = [momenta_low]
gun.ThetaMins  = [theta_low]
gun.ThetaMaxs  = [theta_low]
gun.PhiMins    = [0]
gun.PhiMaxs    = [360]

genprinter = GenPrinter("GenPrinter")

genalg = GenAlgo("GenAlgo")
genalg.GenTools = ["GtGunTool"]

############ Physics generator in stdhep
#stdheprdr = StdHepRdr("StdHepRdr")
#stdheprdr.Input = "/cefs/data/stdhep/CEPC240/higgs/update_from_LiangHao_1M/data/E240.Pnnh_gg.e0.p0.whizard195/nnh_gg.e0.p0.00001.stdhep"

#genalg = GenAlgo("GenAlgo")
#genalg.GenTools = ["StdHepRdr"]

##############################################################################
# Detector Simulation
##############################################################################
from Configurables import DetSimSvc
detsimsvc = DetSimSvc("DetSimSvc")

from Configurables import Edm4hepWriterAnaElemTool
edm4hep_writer = Edm4hepWriterAnaElemTool("Edm4hepWriterAnaElemTool")
edm4hep_writer.TrackerCollections = ["VXD", "ITKBarrel", "ITKEndcap", "TPC", "TPCLowPt", "TPCSpacePoint",
                                     "OTKBarrel", "OTKEndcap", "COIL", "MuonBarrel", "MuonEndcap"]

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

from Configurables import Edm4hepWriterAnaElemTool
detsim_anatool = Edm4hepWriterAnaElemTool("Edm4hepWriterAnaElemTool")
detsim_anatool.IsTrk2Primary = False # True: primary; False: ancestor

from Configurables import TimeProjectionChamberSensDetTool
tpc_sensdettool = TimeProjectionChamberSensDetTool("TimeProjectionChamberSensDetTool")
tpc_sensdettool.TypeOption = 1
tpc_sensdettool.DoHeedSim = False #True
dedxoption = "TrackHeedSimTool"
tpc_sensdettool.DedxSimTool = dedxoption

from Configurables import TrackHeedSimTool
dedx_simtool = TrackHeedSimTool("TrackHeedSimTool")
dedx_simtool.detector = "TPC"
dedx_simtool.only_primary = False#True
dedx_simtool.use_max_step = False#True
dedx_simtool.max_step = 1#mm
dedx_simtool.save_mc = True
#dedx_simtool.OutputLevel = DEBUG


from Configurables import CalorimeterSensDetTool
from Configurables import DriftChamberSensDetTool
cal_sensdettool = CalorimeterSensDetTool("CalorimeterSensDetTool")
cal_sensdettool.CalNamesMergeDisable = ["EcalBarrel", "EcalEndcap", "HcalBarrel", "HcalEndcap"]
cal_sensdettool.CalNamesApplyBirks = ["EcalBarrel", "EcalEndcap", "HcalBarrel","HcalEndcap"]
cal_sensdettool.CalNamesBirksConstants = [0.008415, 0.008415, 0.01, 0.01] # BGO and Glass scintillator


from Configurables import MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")

# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "sim-mu--0.5-85-2.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [genalg, detsimalg, out],
    EvtSel = 'NONE',
    EvtMax = evtmax,
    ExtSvc = [rndmengine, rndmgensvc, dsvc, geosvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = ERROR
)
