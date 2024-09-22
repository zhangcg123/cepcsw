#!/usr/bin/env python

import os

from Gaudi.Configuration import *

# NTupleSvc().Output = ["MyTuples DATAFILE='Examples/options/TPC_out.root' OPT='NEW' TYP='ROOT'"]

from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_()
rndmengine.SetSingleton = True
rndmengine.Seeds = [1]

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")

from Configurables import MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")

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

from Configurables import GearSvc
gearsvc = GearSvc("GearSvc")
#gearsvc.GearXMLFile = "Detector/DetCEPCv4/compact/FullDetGear.xml"

##############################################################################
# Generator
from Configurables import GenAlgo
from Configurables import GtGunTool
from Configurables import StdHepRdr
from Configurables import SLCIORdr
from Configurables import HepMCRdr
from Configurables import GenPrinter

gun = GtGunTool("GtGunTool")
gun.Particles = ["e-"]
#gun.EnergyMins = [1]
#gun.EnergyMaxs = [50]
#gun.ThetaMins = [50]
#gun.ThetaMaxs = [130]
#gun.PhiMins = [-90]
#gun.PhiMaxs = [90]
gun.EnergyMins = [10]
gun.EnergyMaxs = [10]
gun.ThetaMins = [90]
gun.ThetaMaxs = [90]
gun.PhiMins = [0]
gun.PhiMaxs = [0]

stdheprdr = StdHepRdr("StdHepRdr")
stdheprdr.Input = "/cefs/data/stdhep/CEPC240/higgs/Higgs_10M/data/E240.Pnnh_gg.e0.p0.whizard195/nnh_gg.e0.p0.00001.stdhep"
# stdheprdr.Input = "GenFile"

genprinter = GenPrinter("GenPrinter")

genalg = GenAlgo("GenAlgo")
# genalg.GenTools = ["GtGunTool"]
genalg.GenTools = ["StdHepRdr"]

##############################################################################
# Detector simulation
from Configurables import DetSimSvc
detsimsvc = DetSimSvc("DetSimSvc")

from Configurables import DetSimAlg
detsimalg = DetSimAlg("DetSimAlg")
# detsimalg.VisMacs = ["vis.mac"]
detsimalg.RunCmds = [
#    "/physics_lists/factory/addOptical"
]
detsimalg.PhysicsList = "FTFP_BERT"
detsimalg.AnaElems = ["Edm4hepWriterAnaElemTool"]
detsimalg.RootDetElem = "WorldDetElemTool"

from Configurables import AnExampleDetElemTool
example_dettool = AnExampleDetElemTool("AnExampleDetElemTool")

##############################################################################
# Tracker

from Configurables import TimeProjectionChamberSensDetTool
tpc_sensdettool = TimeProjectionChamberSensDetTool("TimeProjectionChamberSensDetTool")
tpc_sensdettool.TypeOption = 1

from Configurables import TrackSystemSvc
tracksystemsvc = TrackSystemSvc("TrackSystemSvc")

vxdhitname  = "VXDTrackerHits"
sithitname  = "SITTrackerHits"
sitspname   = "SITSpacePoints"
tpchitname  = "TPCTrackerHits"
sethitname  = "SETTrackerHits"
setspname   = "SETSpacePoints"
ftdspname   = "FTDSpacePoints"
ftdhitname = "FTDTrackerHits"
from Configurables import PlanarDigiAlg
digiVXD = PlanarDigiAlg("VXDDigi")
digiVXD.SimTrackHitCollection = "VXDCollection"
digiVXD.TrackerHitCollection = vxdhitname
digiVXD.TrackerHitAssociationCollection = "VXDTrackerHitAssociation"
digiVXD.ResolutionU = [0.0028, 0.006, 0.004, 0.004, 0.004, 0.004]
digiVXD.ResolutionV = [0.0028, 0.006, 0.004, 0.004, 0.004, 0.004]

digiSIT = PlanarDigiAlg("SITDigi")
digiSIT.IsStrip = 1
digiSIT.SimTrackHitCollection = "SITCollection"
digiSIT.TrackerHitCollection = sithitname
digiSIT.TrackerHitAssociationCollection = "SITTrackerHitAssociation"
digiSIT.ResolutionU = [0.007]
digiSIT.ResolutionV = [0.000]

digiSET = PlanarDigiAlg("SETDigi")
digiSET.IsStrip = 1
digiSET.SimTrackHitCollection = "SETCollection"
digiSET.TrackerHitCollection = sethitname
digiSET.TrackerHitAssociationCollection = "SETTrackerHitAssociation"
digiSET.ResolutionU = [0.007]
digiSET.ResolutionV = [0.000]

digiFTD = PlanarDigiAlg("FTDDigi")
digiFTD.SimTrackHitCollection = "FTDCollection"
digiFTD.TrackerHitCollection = ftdhitname
digiFTD.TrackerHitAssociationCollection = "FTDTrackerHitAssociation"
digiFTD.ResolutionU = [0.003, 0.003, 0.007, 0.007, 0.007, 0.007, 0.007, 0.007]
digiFTD.ResolutionV = [0.003, 0.003, 0,     0,     0,     0,     0,     0    ]
#digiFTD.OutputLevel = DEBUG

from Configurables import SpacePointBuilderAlg
spSIT = SpacePointBuilderAlg("SITBuilder")
spSIT.TrackerHitCollection = sithitname
spSIT.TrackerHitAssociationCollection = "SITTrackerHitAssociation"
spSIT.SpacePointCollection = sitspname
spSIT.SpacePointAssociationCollection = "SITSpacePointAssociation"
#spSIT.OutputLevel = DEBUG

spFTD = SpacePointBuilderAlg("FTDBuilder")
spFTD.TrackerHitCollection = ftdhitname
spFTD.TrackerHitAssociationCollection = "FTDTrackerHitAssociation"
spFTD.SpacePointCollection = ftdspname
spFTD.SpacePointAssociationCollection = "FTDSpacePointAssociation"
#spFTD.OutputLevel = DEBUG

from Configurables import TPCDigiAlg
digiTPC = TPCDigiAlg("TPCDigi")
digiTPC.TPCCollection = "TPCCollection"
digiTPC.TPCLowPtCollection = "TPCLowPtCollection"
digiTPC.TPCTrackerHitsCol = tpchitname
digiTPC.TPCTrackerHitAssCol = "TPCTrackerHitAssociation"
#digiTPC.OutputLevel = DEBUG

from Configurables import ClupatraAlg
clupatra = ClupatraAlg("Clupatra")
clupatra.TPCHitCollection = tpchitname
#clupatra.OutputLevel = DEBUG

from Configurables import SiliconTrackingAlg
tracking = SiliconTrackingAlg("SiliconTracking")
tracking.HeaderCol = "EventHeader"
tracking.VTXHitCollection = vxdhitname
tracking.SITHitCollection = sitspname
tracking.FTDPixelHitCollection = ftdhitname
tracking.FTDSpacePointCollection = ftdspname
tracking.SITRawHitCollection = sithitname
tracking.FTDRawHitCollection = ftdhitname
tracking.UseSIT = 1
tracking.SmoothOn = 0
#tracking.OutputLevel = DEBUG

from Configurables import ForwardTrackingAlg
forward = ForwardTrackingAlg("ForwardTracking")
forward.FTDPixelHitCollection = ftdhitname
forward.FTDSpacePointCollection = ftdspname
forward.FTDRawHitCollection = ftdhitname
forward.Chi2ProbCut = 0.0
forward.HitsPerTrackMin = 3
forward.BestSubsetFinder = "SubsetSimple"
forward.Criteria = ["Crit2_DeltaPhi","Crit2_StraightTrackRatio","Crit3_3DAngle","Crit3_ChangeRZRatio","Crit3_IPCircleDist","Crit4_3DAngleChange","Crit4_DistToExtrapolation",
                    "Crit2_DeltaRho","Crit2_RZRatio","Crit3_PT"]
forward.CriteriaMin = [0,  0.9,  0,  0.995, 0,  0.8, 0,   20,  1.002, 0.1,      0,   0.99, 0,    0.999, 0,   0.99, 0]
forward.CriteriaMax = [30, 1.02, 10, 1.015, 20, 1.3, 1.0, 150, 1.08,  99999999, 0.8, 1.01, 0.35, 1.001, 1.5, 1.01, 0.05]
#forward.OutputLevel = DEBUG

from Configurables import TrackSubsetAlg
subset = TrackSubsetAlg("TrackSubset")
subset.TrackInputCollections = ["ForwardTracks", "SiTracks"]
subset.RawTrackerHitCollections = [vxdhitname, sithitname, ftdhitname, sitspname, ftdspname]
subset.TrackSubsetCollection = "SubsetTracks"
#subset.OutputLevel = DEBUG

from Configurables import FullLDCTrackingAlg
full = FullLDCTrackingAlg("FullTracking")
full.VTXTrackerHits = vxdhitname
full.SITTrackerHits = sitspname
full.TPCTrackerHits = tpchitname
full.SETTrackerHits = setspname
full.FTDPixelTrackerHits = ftdhitname
full.FTDSpacePoints = ftdspname
full.SITRawHits     = sithitname
full.SETRawHits     = sethitname
full.FTDRawHits     = ftdhitname
full.TPCTracks = "ClupatraTracks"
full.SiTracks  = "SubsetTracks"
full.OutputTracks  = "MarlinTrkTracks"
#full.OutputLevel = DEBUG
'''
from Configurables import DumpMCParticleAlg
dumpMC = DumpMCParticleAlg("DumpMC")
dumpMC.MCParticleCollection = "MCParticle"

from Configurables import DumpTrackAlg
dumpFu = DumpTrackAlg("DumpFu")
dumpFu.TrackCollection = "MarlinTrkTracks"
#dumpFu.OutputLevel = DEBUG

dumpCl = DumpTrackAlg("DumpCl")
dumpCl.TrackCollection = "ClupatraTracks"
#dumpCl.OutputLevel = DEBUG

dumpSu = DumpTrackAlg("DumpSu")
dumpSu.TrackCollection = "SubsetTracks"
#dumpSu.OutputLevel = DEBUG

dumpSi = DumpTrackAlg("DumpSi")
dumpSi.TrackCollection = "SiTracks"
#dumpSi.OutputLevel = DEBUG

dumpFo = DumpTrackAlg("DumpFo")
dumpFo.TrackCollection = "ForwardTracks"
#dumpFo.OutputLevel = DEBUG
'''
##############################################################################
# Calorimeter

from Configurables import SimHitMergeAlg
simHitMerge = SimHitMergeAlg("SimHitMergeAlg")
simHitMerge.sanity_check = False
simHitMerge.InputCollections=["EcalBarrelCollection", "EcalEndcapsCollection","EcalEndcapRingCollection", "HcalBarrelCollection", "HcalEndcapsCollection", "HcalEndcapRingCollection"]
simHitMerge.OutputCollections=["EcalBarrelCollectionMerged", "EcalEndcapsCollectionMerged", "EcalEndcapRingCollectionMerged", "HcalBarrelCollectionMerged", "HcalEndcapsCollectionMerged", "HcalEndcapRingCollectionMerged"]

# simHitMerge.InputCollections=["EcalBarrelCollection", "EcalEndcapsCollection"]
# simHitMerge.OutputCollections=["EcalBarrelCollectionMerged", "EcalEndcapsCollectionMerged"]
##############################################################################
from Configurables import G2CDArborAlg
caloDigi = G2CDArborAlg("G2CDArborAlg")
caloDigi.ReadLCIO = False
caloDigi.ECALCollections = ["EcalBarrelCollectionMerged", "EcalEndcapsCollectionMerged", "EcalEndcapRingCollectionMerged"]
caloDigi.HCALCollections = ["HcalBarrelCollectionMerged", "HcalEndcapsCollectionMerged", "HcalEndcapRingCollectionMerged"]
caloDigi.ECALReadOutNames = ["EcalBarrelCollection", "EcalEndcapsCollection", "EcalEndcapRingCollection"]
caloDigi.HCALReadOutNames = ["HcalBarrelCollection", "HcalEndcapsCollection", "HcalEndcapRingCollection"]
caloDigi.DigiECALCollection = ["ECALBarrel", "ECALEndcap", "ECALOther"]
caloDigi.DigiHCALCollection = ["HCALBarrel", "HCALEndcap", "HCALOther"]
caloDigi.EventReportEvery = 100
# caloDigi.CalibrECAL = [46.538, 93.0769]  # Yudan
caloDigi.CalibrECAL = [48.16, 96.32]
caloDigi.HCALThreshold = 0.12

caloDigi.PolyaParaA = 1.1
caloDigi.PolyaParaB = 1.0
caloDigi.PolyaParaC = 0.0
##############################################################################
# Reconstruction: Arbor

from Configurables import MarlinArbor
marlinArbor = MarlinArbor("MarlinArbor")
marlinArbor.ReadLCIO = False
marlinArbor.ECALCollections =["ECALBarrel","ECALEndcap","ECALOther"]
marlinArbor.HCALCollections =["HCALBarrel","HCALEndcap","HCALOther"]
marlinArbor.ECALReadOutNames = ["EcalBarrelCollection", "EcalEndcapsCollection", "EcalEndcapRingCollection"]
marlinArbor.HCALReadOutNames = ["HcalBarrelCollection", "HcalEndcapsCollection", "HcalEndcapRingCollection"]
##############################################################################
from Configurables import BushConnect
bushconnect = BushConnect("BushConnect")
bushconnect.ReadLCIO = False
##############################################################################
# BMR analysis
from Configurables import TotalInvMass
totalInvM = TotalInvMass("TotalInvMass")
totalInvM.TreeOutputFile = "Examples/options/CEPCV4_simu_reco_Arbor_nnHgg_BMRAna.root"
# totalInvM.TreeOutputFile = "BMRFile"
totalInvM.MCPCollectionName = "MCParticleGen"
# totalInvM.MCPCollectionName = "MCParticle"
##############################################################################
# write PODIO file
from Configurables import PodioOutput
write = PodioOutput("write")
write.filename = "Examples/options/CEPCV4_simu_reco_Arbor_nnHgg_Reco.root"
# write.filename = "RecoFile"
write.outputCommands = ["keep *"]
##############################################################################
# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr(
    # TopAlg = [genalg, detsimalg, digiVXD, digiSIT, digiSET, digiFTD, spSIT, spFTD, digiTPC, clupatra, tracking, forward, subset, full, simHitMerge, caloDigi, write],
    TopAlg = [genalg, detsimalg, digiVXD, digiSIT, digiSET, digiFTD, spSIT, spFTD, digiTPC, clupatra, tracking, forward, subset, full, simHitMerge, caloDigi, marlinArbor, bushconnect, totalInvM, write],
    EvtSel = 'NONE',
    EvtMax = 1,
    ExtSvc = [rndmengine, dsvc, evtseeder, geosvc, gearsvc, tracksystemsvc],
    HistogramPersistency='ROOT',
    OutputLevel=INFO
)
