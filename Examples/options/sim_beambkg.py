#!/usr/bin/env python
import os
from Gaudi.Configuration import * 

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")

from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_

seed = [556]
Nevt = 1
yyy_input = "/cefs/higgs/songwz/summer24/bkgGenerator/pairHiggs/pair4Higgs/"
yyy_output = '../simdir/sim_bkg_556.root'

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

from Configurables import NTupleSvc
ntsvc = NTupleSvc("NTupleSvc")
#ntsvc.Output = ["MyTuples DATAFILE='result.root' OPT='NEW' TYP='ROOT'"]

##############################################################################
# Physics Generator
##############################################################################
from Configurables import GenAlgo
from Configurables import GtGunTool
from Configurables import StdHepRdr
from Configurables import SLCIORdr
from Configurables import HepMCRdr
from Configurables import GenPrinter
from Configurables import GtBeamBackgroundTool

# gun = GtGunTool("GtGunTool")
# gun.Particles =  ["mu-"]
# gun.PositionXs = [0.]
# gun.PositionYs = [0.]
# gun.PositionZs = [0.]
# gun.EnergyMins = [5.0] # GeV
# gun.EnergyMaxs = [5.0] # GeV
# gun.ThetaMins  = [90]   # deg
# gun.ThetaMaxs  = [90]   # deg
# gun.PhiMins    = [0]   # deg
# gun.PhiMaxs    = [0]   # deg
# gun.Times      = [0.75e3]  # ns

bg = GtBeamBackgroundTool("GtBeamBackgroundTool") 
Nbunch = 10
TbunchSpacing = 355. # 50MW, higgs mode, and in ns
bg.TimeWindow = Nbunch*TbunchSpacing
bg.InputFileMap = {
  "BGB":"/cefs/higgs/songwz/summer24/bkgGenerator/singleBeamHiggs/BGB.root",
  "BGC":"/cefs/higgs/songwz/summer24/bkgGenerator/singleBeamHiggs/BGC.root",
  "BTH":"/cefs/higgs/songwz/summer24/bkgGenerator/singleBeamHiggs/BTH.root",
}
bg.InputFileMap.update(dict([(f'pair{i}', yyy_input) for i in range(Nbunch)]))
bg.InputFormatMap = {
  "BGB":"BeamBackgroundFileParserV1",
  "BGC":"BeamBackgroundFileParserV1",
  "BTH":"BeamBackgroundFileParserV1",
}
bg.InputFormatMap.update(dict([(f'pair{i}', "BeamBackgroundFileParserV2") for i in range(Nbunch)]))
bg.InputRateMap = {  # in Hz
  "BGB":83280.65, 
  "BGC":884002.12,
  "BTH":623520.09,
}
bg.InputBeamEnergy = 120.
bg.RotationAlongYMap = {
  "BGB":16.5e-3,
  "BGC":16.5e-3,
  "BTH":16.5e-3
}
bg.RotationAlongYMap.update(dict([(f'pair{i}', 0) for i in range(Nbunch)]))
bg.TimeBkgMap = dict([(f'pair{i}', TbunchSpacing*i) for i in range(Nbunch)]) #ns
bg.NumberMcParticle = { #-1: pair use one file and single beam use rate*time; >=0: fixed number
  "BGB":-1,
  "BGC":-1,
  "BTH":-1
}
bg.NumberMcParticle.update(dict([(f'pair{i}', -1) for i in range(Nbunch)]))

# stdheprdr = StdHepRdr("StdHepRdr")
# stdheprdr.Input = "/cefs/data/stdhep/CEPC240/higgs/Higgs_10M/data/E240.Pnnh_gg.e0.p0.whizard195/nnh_gg.e0.p0.00100.stdhep"
# stdheprdr.StartTime  = 2.4e3 #ns

# # lciordr = SLCIORdr("SLCIORdr")
# # lciordr.Input = "/cefs/data/stdhep/lcio250/signal/Higgs/E250.Pbbh.whizard195/E250.Pbbh_X.e0.p0.whizard195/Pbbh_X.e0.p0.00001.slcio"
# # hepmcrdr = HepMCRdr("HepMCRdr")
# # hepmcrdr.Input = "example_UsingIterators.txt"

genprinter = GenPrinter("GenPrinter")

genalg = GenAlgo("GenAlgo")
genalg.GenTools = ["GtBeamBackgroundTool"]
# genalg.GenTools = ["GtGunTool"]
# genalg.GenTools = ["StdHepRdr"]
# genalg.GenTools = ["StdHepRdr", "GenPrinter"]
# genalg.GenTools = ["SLCIORdr", "GenPrinter"]
# genalg.GenTools = ["HepMCRdr", "GenPrinter"]

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
#    "/tracking/verbose 2",
]
detsimalg.AnaElems = [
    # example_anatool.name()
  # "ExampleAnaDoseElemTool",
    "Edm4hepWriterAnaElemTool"
]
detsimalg.RootDetElem = "WorldDetElemTool" 
detsimalg.PhysicsList = "QGSP_BERT_EMV"

# from Configurables import ExampleAnaDoseElemTool
# dosesimtool = ExampleAnaDoseElemTool("ExampleAnaDoseElemTool")
# dosesimtool.Gridnbins = [220, 220, 60]
# dosesimtool.Coormin = [0, 0, 0]
# dosesimtool.Coormax = [2200, 2200, 600]
# # dosesimtool.Gridnbins = [2200,1,4400]
# # dosesimtool.Coormin = [0,-0.5,-2200]
# # dosesimtool.Coormax = [2200,0.5,2200]
# dosesimtool.Regionhist = [20,1.e-12,10]
# dosesimtool.filename = "output/testdose_yyy_n_"
# dosesimtool.Dosecofffilename = "dosecoffe/"
# dosesimtool.HEHadroncut = 0.02

from Configurables import MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")

from Configurables import GearSvc
gearsvc = GearSvc("GearSvc")
#gearsvc.GearXMLFile = "../../Detector/DetCEPCv4/compact/FullDetGear.xml"

from Configurables import TrackSystemSvc
tracksystemsvc = TrackSystemSvc("TrackSystemSvc")

from Configurables import AnExampleDetElemTool
example_dettool = AnExampleDetElemTool("AnExampleDetElemTool")

from Configurables import TimeProjectionChamberSensDetTool
tpc_sensdettool = TimeProjectionChamberSensDetTool("TimeProjectionChamberSensDetTool")
tpc_sensdettool.TypeOption = 1

from Configurables import MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")

from Configurables import CalorimeterSensDetTool
from Configurables import DriftChamberSensDetTool
cal_sensdettool = CalorimeterSensDetTool("CalorimeterSensDetTool")
cal_sensdettool.CalNamesMergeDisable = ["CaloDetector"]
# cal_sensdettool.CalNamesApplyBirks = ["HcalBarrel"]


# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = yyy_output
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr(
    TopAlg = [genalg, detsimalg, out], #digiVXD, digiSIT, digiSET, digiFTD, spSET, digiTPC, tracking, forward, subset, full, 
    EvtSel = 'NONE',
    EvtMax = Nevt,
    ExtSvc = [rndmengine, rndmgensvc, dsvc, evtseeder, geosvc, gearsvc, tracksystemsvc],
    #ExtSvc = [rndmengine, rndmgensvc, dsvc, geosvc],
    OutputLevel=INFO
)
