#!/usr/bin/env python
import os
from Gaudi.Configuration import *

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="CaloDigi_TDR_o1_v01.root")

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
pidsvc.ParFile = os.path.join(cepcswdatatop, "CEPCSWData/offline-data/Service/SimplePIDSvc/data/tdr25.1.1/dNdx_TPC.root")

from Configurables import PodioInput
podioinput = PodioInput("PodioReader", collections=[
#    "EventHeader",
    "MCParticle",
    "VXDCollection",
    "ITKBarrelCollection",
    "TPCCollection",
    "OTKBarrelCollection",
    "FTDCollection",
    "MuonBarrelCollection",
    "MuonEndcapCollection"
    ])


##################
# Digitization
##################

## Config ##
vxdhitname  = "VXDTrackerHits"
sithitname  = "ITKBarrelTrackerHits"
gashitname  = "TPCTrackerHits"
sethitname  = "OTKBarrelTrackerHits"
setspname   = "OTKBarrelSpacePoints"
ftdhitname  = "FTDTrackerHits"
ftdspname   = "FTDSpacePoints"
from Configurables import SmearDigiTool,SiTrackerDigiAlg

## VXD ##
vxdtool = SmearDigiTool("VXD")
vxdtool.ResolutionU = [0.005]
vxdtool.ResolutionV = [0.005]
#vxdtool.OutputLevel = DEBUG

digiVXD = SiTrackerDigiAlg("VXDDigi")
digiVXD.SimTrackHitCollection = "VXDCollection"
digiVXD.TrackerHitCollection = vxdhitname
digiVXD.TrackerHitAssociationCollection = "VXDTrackerHitAssociation"
digiVXD.DigiTool = "SmearDigiTool/VXD"
#digiVXD.OutputLevel = DEBUG

## ITKBarrel ##
itkbtool = SmearDigiTool("ITKBarrel")
itkbtool.ResolutionU = [0.008]
itkbtool.ResolutionV = [0.040]
#itkbtool.OutputLevel = DEBUG

digiITKB = SiTrackerDigiAlg("ITKBarrelDigi")
digiITKB.SimTrackHitCollection = "ITKBarrelCollection"
digiITKB.TrackerHitCollection = sithitname
digiITKB.TrackerHitAssociationCollection = "ITKBarrelTrackerHitAssociation"
digiITKB.DigiTool = "SmearDigiTool/ITKBarrel"
#digiITKB.OutputLevel = DEBUG

## OTKBarrel ##
otkbtool = SmearDigiTool("OTKBarrel")
otkbtool.ResolutionU = [0.010]
otkbtool.ResolutionV = [1.000]
#otkbtool.OutputLevel = DEBUG

digiOTKB = SiTrackerDigiAlg("OTKBarrelDigi")
digiOTKB.SimTrackHitCollection = "OTKBarrelCollection"
digiOTKB.TrackerHitCollection = sethitname
digiOTKB.TrackerHitAssociationCollection = "OTKBarrelTrackerHitAssociation"
digiOTKB.DigiTool = "SmearDigiTool/OTKBarrel"
#digiOTKB.OutputLevel = DEBUG

## FTD ##
ftdtool = SmearDigiTool("FTD")
ftdtool.ResolutionU = [0.0072]
ftdtool.ResolutionV = [0.086]
#ftdtool.OutputLevel = DEBUG

digiFTD = SiTrackerDigiAlg("FTDDigi")
digiFTD.SimTrackHitCollection = "FTDCollection"
digiFTD.TrackerHitCollection = ftdhitname
digiFTD.TrackerHitAssociationCollection = "FTDTrackerHitAssociation"
digiFTD.DigiTool = "SmearDigiTool/FTD"
#digiFTD.OutputLevel = DEBUG

## TPC ##
from Configurables import TPCDigiAlg
digiTPC = TPCDigiAlg("TPCDigi")
digiTPC.TPCCollection = "TPCCollection"
digiTPC.TPCLowPtCollection = "TPCLowPtCollection"
digiTPC.TPCTrackerHitsCol = gashitname
#default value, modify them according to future Garfield simulation results
#digiTPC.PixelClustering = True
#digiTPC.PointResolutionRPhi = 0.144
#digiTPC.DiffusionCoeffRPhi = 0.0323
#digiTPC.PointResolutionZ = 0.4
#digiTPC.DiffusionCoeffZ = 0.23
#digiTPC.N_eff = 30
#digiTPC.OutputLevel = DEBUG


## Muon Detector ##
from Configurables import MuonDigiAlg
digiMuon = MuonDigiAlg("MuonDigiAlg")
digiMuon.MuonBarrelHitsCollection = "MuonBarrelCollection"
digiMuon.MuonEndcapHitsCollection = "MuonEndcapCollection"
digiMuon.MuonBarrelTrackerHits = "MuonBarrelTrackerHits"
digiMuon.MuonEndcapTrackerHits = "MuonEndcapTrackerHits"
digiMuon.WriteNtuple = 0
digiMuon.OutFileName = "Digi_MUON.root"
#########################################

################
# Tracking
################
from Configurables import KalTestTool
# Close multiple scattering and smooth, used by clupatra
kt010 = KalTestTool("KalTest010")
kt010.MSOn = False
kt010.Smooth = False
#kt010.OutputLevel = DEBUG

# Open multiple scattering, energy loss and smooth (default)
kt111 = KalTestTool("KalTest111")
#kt111.OutputLevel = DEBUG

# Close smooth
kt110 = KalTestTool("KalTest110")
kt110.Smooth = False
#kt110.OutputLevel = DEBUG

# Close energy loss
kt101 = KalTestTool("KalTest101")
kt101.EnergyLossOn = False
#kt101.OutputLevel = DEBUG

from Configurables import SiliconTrackingAlg
tracking = SiliconTrackingAlg("SiliconTracking")
tracking.LayerCombinations = [8,7,6, 8,7,5, 8,7,4, 8,7,3, 8,7,2, 8,7,1, 8,7,0,
                              8,6,5, 8,6,4, 8,6,3, 8,6,2, 8,6,1, 8,6,0,
                              7,6,5, 7,6,4, 7,6,3, 7,6,2, 7,6,1, 7,6,0,
                              7,5,3, 7,5,2, 7,5,1, 7,5,0, 7,4,3, 7,4,2, 7,4,1, 7,4,0,
                              6,5,3, 6,5,2, 6,5,1, 6,5,0, 6,4,3, 6,4,2, 6,4,1, 6,4,0,
                              6,3,2, 6,3,1, 6,3,0, 6,2,1, 6,2,0, 6,1,0,
                              5,3,2, 5,3,1, 5,3,0, 5,2,1, 5,2,0, 5,1,0,
                              4,3,2, 4,3,1, 4,3,0, 4,2,1, 4,2,0, 4,1,0,
                              3,2,1, 3,2,0, 3,1,0, 2,1,0]
tracking.LayerCombinationsFTD = []
tracking.HeaderCol = "EventHeader"
tracking.VTXHitCollection = vxdhitname
tracking.SITHitCollection = sithitname
tracking.FTDPixelHitCollection = ftdhitname
tracking.FTDSpacePointCollection = ftdspname
tracking.SITRawHitCollection = "NotNeedForPixelSIT"
tracking.FTDRawHitCollection = ftdhitname
tracking.UseSIT = True
tracking.SmoothOn = False
tracking.NDivisionsInTheta = 10
tracking.NDivisionsInPhi = 60
tracking.NDivisionsInPhiFTD = 16
tracking.MinDistCutAttach = 50
# for p=1GeV, theta=10degree, Chi2FitCut = 1500, HelixMaxChi2 = 1000000, Chi2WZ = 0.02
tracking.Chi2FitCut = 200
tracking.MaxChi2PerHit = 200
tracking.HelixMaxChi2  = 50000
tracking.Chi2WZTriplet = 0.1
tracking.Chi2WZQuartet = 0.1
tracking.Chi2WZSeptet  = 0.1
#tracking.FitterTool = "KalTestTool/KalTest111"
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
#forward.FitterTool = "KalTestTool/KalTest110"
#forward.OutputLevel = DEBUG

from Configurables import TrackSubsetAlg
subset = TrackSubsetAlg("TrackSubset")
subset.TrackInputCollections = ["ForwardTracks", "SiTracks"]
subset.RawTrackerHitCollections = [vxdhitname, sithitname, ftdhitname, ftdspname]
subset.TrackSubsetCollection = "SubsetTracks"
#subset.FitterTool = "KalTestTool/KalTest111"
#subset.OutputLevel = DEBUG

from Configurables import ClupatraAlg
clupatra = ClupatraAlg("Clupatra")
clupatra.TPCHitCollection = gashitname
#clupatra.OutputLevel = DEBUG

from Configurables import FullLDCTrackingAlg
full = FullLDCTrackingAlg("FullTracking")
full.VTXTrackerHits = vxdhitname
full.SITTrackerHits = sithitname
full.TPCTrackerHits = gashitname
full.SETTrackerHits = sethitname
full.FTDPixelTrackerHits = ftdhitname
full.FTDSpacePoints = ftdspname
full.SITRawHits     = "NotNeedForPixelSIT"
full.SETRawHits     = "NotNeedForPixelSET"
full.FTDRawHits     = ftdhitname
full.TPCTracks = "ClupatraTracks" # add standalone TPC track
full.SiTracks  = "SiTracks"
full.OutputTracks  = "CompleteTracks" # default name
full.FTDHitToTrackDistance = 5.
full.SITHitToTrackDistance = 3.
full.SETHitToTrackDistance = 5.
full.MinChi2ProbForSiliconTracks = 0
full.MaxChi2PerHit = 200
full.ForceSiTPCMerging = True
full.ForceTPCSegmentsMerging = True
#full.OutputLevel = DEBUG

from Configurables import TPCDndxAlg
tpc_dndx = TPCDndxAlg("TPCDndxAlg")
tpc_dndx.Method = "Simple"

from Configurables import TofRecAlg
tof = TofRecAlg("TofRecAlg")
#tof.OutputLevel = DEBUG

from Configurables import TrackParticleRelationAlg
tpr = TrackParticleRelationAlg("Track2Particle")
tpr.MCParticleCollection = "MCParticle"
tpr.TrackList = ["CompleteTracks"]
tpr.TrackerAssociationList = ["VXDTrackerHitAssociation", "ITKBarrelTrackerHitAssociation", "OTKBarrelTrackerHitAssociation", "FTDTrackerHitAssociation"]
#tpr.OutputLevel = DEBUG


from Configurables import TrueMuonTagAlg
tmt = TrueMuonTagAlg("TrueMuonTag")
tmt.MCParticleCollection = "MCParticle"
tmt.TrackList = ["CompleteTracks"]
tmt.TrackerAssociationList = ["VXDTrackerHitAssociation", "ITKBarrelTrackerHitAssociation", "OTKBarrelTrackerHitAssociation", "FTDTrackerHitAssociation", "TPCTrackerHitAss"]
tmt.MuonTagEfficiency = 0.95 # muon true tag efficiency, default is 1.0 (100%)
tmt.MuonDetTanTheta = 1.2 # muon det barrel/endcap separation tan(theta)
#tmt.OutputLevel = DEBUG

# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "Tracking_TDR_o1_v01.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [podioinput, digiVXD, digiITKB, digiOTKB, digiFTD, digiTPC, digiMuon, tracking, forward, subset, clupatra, full, tpr, tpc_dndx, tof, tmt, out],
    EvtSel = 'NONE',
    EvtMax = 10,
    ExtSvc = [rndmengine, rndmgensvc, dsvc, evtseeder, geosvc, gearsvc, tracksystemsvc, pidsvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = ERROR
)
