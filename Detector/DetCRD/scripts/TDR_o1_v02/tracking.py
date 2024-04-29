#!/usr/bin/env python
import os
from Gaudi.Configuration import *

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="sim00.root")

from Configurables import RndmGenSvc, HepRndm__Engine_CLHEP__RanluxEngine_
seed = [12340]
# rndmengine = HepRndm__Engine_CLHEP__RanluxEngine_() # The default engine in Gaudi                                                                                                                                            
rndmengine = HepRndm__Engine_CLHEP__HepJamesRandom_("RndmGenSvc.Engine") # The default engine in Geant4                                                                                                                        
rndmengine.SetSingleton = True
rndmengine.Seeds = seed

rndmgensvc = RndmGenSvc("RndmGenSvc")
rndmgensvc.Engine = rndmengine.name()

geometry_option = "TDR_o1_v02/TDR_o1_v02.xml"

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

from Configurables import PodioInput
podioinput = PodioInput("PodioReader", collections=[
#    "EventHeader",
    "MCParticle",
    "VXDCollection",
    "SITCollection",
    "TPCCollection",
    "SETCollection",
    "FTDCollection"
    ])

# digitization
vxdhitname  = "VXDTrackerHits"
sithitname  = "SITTrackerHits"
gashitname  = "TPCTrackerHits"
sethitname  = "SETTrackerHits"
setspname   = "SETSpacePoints"
ftdhitname  = "FTDTrackerHits"
ftdspname   = "FTDSpacePoints"
from Configurables import PlanarDigiAlg
digiVXD = PlanarDigiAlg("VXDDigi")
digiVXD.SimTrackHitCollection = "VXDCollection"
digiVXD.TrackerHitCollection = vxdhitname
digiVXD.TrackerHitAssociationCollection = "VXDTrackerHitAssociation"
digiVXD.ResolutionU = [0.004, 0.004, 0.004, 0.004, 0.004, 0.004]
digiVXD.ResolutionV = [0.004, 0.004, 0.004, 0.004, 0.004, 0.004]
digiVXD.UsePlanarTag = True
digiVXD.ParameterizeResolution = False
digiVXD.ParametersU = [5.60959e-03, 5.74913e-03, 7.03433e-03, 1.99516, -663.952, 3.752e-03, 0, -0.0704734, 0.0454867e-03, 1.07359]
digiVXD.ParametersV = [5.60959e-03, 5.74913e-03, 7.03433e-03, 1.99516, -663.952, 3.752e-03, 0, -0.0704734, 0.0454867e-03, 1.07359]
#digiVXD.OutputLevel = DEBUG

digiSIT = PlanarDigiAlg("SITDigi")
digiSIT.IsStrip = False
digiSIT.SimTrackHitCollection = "SITCollection"
digiSIT.TrackerHitCollection = sithitname
digiSIT.TrackerHitAssociationCollection = "SITTrackerHitAssociation"
digiSIT.ResolutionU = [0.0072]
digiSIT.ResolutionV = [0.086]
digiSIT.UsePlanarTag = True
digiSIT.ParameterizeResolution = False
digiSIT.ParametersU = [2.29655e-03, 0.965899e-03, 0.584699e-03, 17.0856, 84.566, 12.4695e-03, -0.0643059, 0.168662, 1.87998e-03, 0.514452]
digiSIT.ParametersV = [1.44629e-02, 2.20108e-03, 1.03044e-02, 4.39195e+00, 3.29641e+00, 1.55167e+18, -5.41954e+01, 5.72986e+00, -6.80699e-03, 5.04095e-01]
#digiSIT.OutputLevel = DEBUG

digiSET = PlanarDigiAlg("SETDigi")
digiSET.IsStrip = False
digiSET.SimTrackHitCollection = "SETCollection"
digiSET.TrackerHitCollection = sethitname
digiSET.TrackerHitAssociationCollection = "SETTrackerHitAssociation"
digiSET.ResolutionU = [0.0072]
digiSET.ResolutionV = [0.086]
digiSET.UsePlanarTag = True
digiSET.ParameterizeResolution = False
digiSET.ParametersU = [2.29655e-03, 0.965899e-03, 0.584699e-03, 17.0856, 84.566, 12.4695e-03, -0.0643059, 0.168662, 1.87998e-03, 0.514452]
digiSET.ParametersV = [1.44629e-02, 2.20108e-03, 1.03044e-02, 4.39195e+00, 3.29641e+00, 1.55167e+18, -5.41954e+01, 5.72986e+00, -6.80699e-03, 5.04095e-01]
#digiSET.OutputLevel = DEBUG

digiFTD = PlanarDigiAlg("FTDDigi")
digiFTD.IsStrip = False
digiFTD.SimTrackHitCollection = "FTDCollection"
digiFTD.TrackerHitCollection = ftdhitname
digiFTD.TrackerHitAssociationCollection = "FTDTrackerHitAssociation"
digiFTD.ResolutionU = [0.0072]
digiFTD.ResolutionV = [0.086]
digiFTD.UsePlanarTag = True
digiFTD.ParameterizeResolution = False
digiFTD.ParametersU = [2.29655e-03, 0.965899e-03, 0.584699e-03, 17.0856, 84.566, 12.4695e-03, -0.0643059, 0.168662, 1.87998e-03, 0.514452]
digiFTD.ParametersV = [1.44629e-02, 2.20108e-03, 1.03044e-02, 4.39195e+00, 3.29641e+00, 1.55167e+18, -5.41954e+01, 5.72986e+00, -6.80699e-03, 5.04095e-01]
#digiFTD.OutputLevel = DEBUG

from Configurables import TPCDigiAlg
digiTPC = TPCDigiAlg("TPCDigi")
digiTPC.TPCCollection = "TPCCollection"
digiTPC.TPCLowPtCollection = "TPCLowPtCollection"
digiTPC.TPCTrackerHitsCol = gashitname
#digiTPC.OutputLevel = DEBUG

# tracking
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

from Configurables import SiliconTrackingAlg
tracking = SiliconTrackingAlg("SiliconTracking")
tracking.LayerCombinationsFTD = [4,2,0, 6,2,0, 6,4,0, 6,4,2, 5,3,1, 7,3,1, 7,5,1, 7,5,3,
                                 4,2,1, 6,2,1, 6,4,1, 6,4,3, 5,3,0, 7,3,0, 7,5,0, 7,5,2,
                                 4,3,0, 6,3,0, 6,5,0, 6,5,2, 5,2,1, 7,2,1, 7,4,1, 7,4,3,
                                 5,2,0, 7,2,0, 7,4,0, 7,4,2, 4,3,1, 6,3,1, 6,5,1, 6,5,3]
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
tracking.MinDistCutAttach = 5
tracking.Chi2FitCut = 100
tracking.MaxChi2PerHit = 100
#tracking.Chi2WZTriplet = 0.1
#tracking.Chi2WZQuartet = 0.1
#tracking.Chi2WZSeptet  = 0.1
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
full.SiTracks  = "SubsetTracks"
full.OutputTracks  = "CompleteTracks" # default name
full.VTXHitToTrackDistance = 5.
full.FTDHitToTrackDistance = 5.
full.SITHitToTrackDistance = 3.
full.SETHitToTrackDistance = 5.
full.MinChi2ProbForSiliconTracks = 0
#full.OutputLevel = DEBUG

from Configurables import TrackParticleRelationAlg
tpr = TrackParticleRelationAlg("Track2Particle")
tpr.MCParticleCollection = "MCParticle"
tpr.TrackList = ["CompleteTracks"]
#tpr.OutputLevel = DEBUG

# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "rec00.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [podioinput, digiVXD, digiSIT, digiSET, digiFTD, digiTPC, tracking, forward, subset, clupatra, full, tpr, out],
    EvtSel = 'NONE',
    EvtMax = 5,
    ExtSvc = [rndmengine, rndmgensvc, dsvc, evtseeder, geosvc, gearsvc, tracksystemsvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = ERROR
)
