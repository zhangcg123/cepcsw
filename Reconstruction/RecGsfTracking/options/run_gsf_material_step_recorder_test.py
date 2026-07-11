#!/usr/bin/env python
import os
import sys
from Gaudi.Configuration import *

EvtMax = int(os.getenv("GSF_STEPREC_EVTMAX", "5"))
seed = [int(os.getenv("GSF_STEPREC_SEED", "12340"))]
gun_energy = float(os.getenv("GSF_STEPREC_ENERGY_GEV", "1.0"))
gun_theta = float(os.getenv("GSF_STEPREC_THETA_DEG", "85"))
step_output = os.getenv("GSF_STEPREC_OUTPUT", "gsf_material_steps_test.root")
edm_output = os.getenv(
    "GSF_STEPREC_EDM_OUTPUT", "gsf_material_steps_test_edm4hep.root")

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")

from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__HepJamesRandom_
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_("RndmGenSvc.Engine")
rndmengine.SetSingleton = True
rndmengine.Seeds = seed
props = rndmengine.getProperties()
if "ThreadSafe" in props:
    rndmengine.ThreadSafe = False
rndmgensvc = RndmGenSvc("RndmGenSvc")
rndmgensvc.Engine = rndmengine.name()

geometry_option = "TDR_o1_v01/TDR_o1_v01.xml"
if not os.getenv("DETCRDROOT"):
    print("Can't find the geometry. Please setup envvar DETCRDROOT.")
    sys.exit(-1)
geometry_path = os.path.join(os.getenv("DETCRDROOT"), "compact", geometry_option)
if not os.path.exists(geometry_path):
    print("Can't find the compact geometry file: %s" % geometry_path)
    sys.exit(-1)

from Configurables import DetGeomSvc
geosvc = DetGeomSvc("GeomSvc")
geosvc.compact = geometry_path

from Configurables import GenAlgo, GtGunTool
gun = GtGunTool("GtGunTool")
gun.PositionXs = [0]
gun.PositionYs = [0]
gun.PositionZs = [0]
gun.Particles = ["e-"]
gun.EnergyMins = [gun_energy]
gun.EnergyMaxs = [gun_energy]
gun.ThetaMins = [gun_theta]
gun.ThetaMaxs = [gun_theta]
gun.PhiMins = [0]
gun.PhiMaxs = [360]

genalg = GenAlgo("GenAlgo")
genalg.GenTools = ["GtGunTool"]

from Configurables import DetSimSvc
detsimsvc = DetSimSvc("DetSimSvc")

from Configurables import Edm4hepWriterAnaElemTool
edm4hep_writer = Edm4hepWriterAnaElemTool("Edm4hepWriterAnaElemTool")
edm4hep_writer.TrackerCollections = [
    "VXD", "ITKBarrel", "ITKEndcap", "TPC", "TPCLowPt", "TPCSpacePoint",
    "OTKBarrel", "OTKEndcap", "COIL", "MuonBarrel", "MuonEndcap",
]

from Configurables import GsfMaterialStepRecorderAnaElemTool
steprec = GsfMaterialStepRecorderAnaElemTool("GsfMaterialStepRecorderAnaElemTool")
steprec.OutputFile = step_output
steprec.PDGs = [11, -11]
steprec.PrimaryOnly = True
steprec.TrackerOnly = True
steprec.MinStepLengthMm = 0.0
steprec.MinAbsLossGeV = 0.0
steprec.RecordZeroLoss = True

from Configurables import DetSimAlg
detsimalg = DetSimAlg("DetSimAlg")
detsimalg.RandomSeeds = seed
detsimalg.RunCmds = []
detsimalg.AnaElems = [
    "Edm4hepWriterAnaElemTool",
    "GsfMaterialStepRecorderAnaElemTool",
]
detsimalg.RootDetElem = "WorldDetElemTool"

from Configurables import TimeProjectionChamberSensDetTool, TrackHeedSimTool
tpc_sensdettool = TimeProjectionChamberSensDetTool("TimeProjectionChamberSensDetTool")
tpc_sensdettool.TypeOption = 1
tpc_sensdettool.DoHeedSim = False
tpc_sensdettool.DedxSimTool = "TrackHeedSimTool"

dedx_simtool = TrackHeedSimTool("TrackHeedSimTool")
dedx_simtool.detector = "TPC"
dedx_simtool.only_primary = False
dedx_simtool.use_max_step = False
dedx_simtool.max_step = 1
dedx_simtool.save_mc = True

from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = edm_output
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr
ApplicationMgr(
    TopAlg=[genalg, detsimalg, out],
    EvtSel="NONE",
    EvtMax=EvtMax,
    ExtSvc=[rndmengine, rndmgensvc, dsvc, geosvc],
    HistogramPersistency="ROOT",
    OutputLevel=INFO,
)
