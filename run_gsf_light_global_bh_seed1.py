#!/usr/bin/env python
import os, sys
from Gaudi.Configuration import *

evtmax = 99

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="trk-e--2.0-85-1.root")

geometry_option = "TDR_o1_v01/TDR_o1_v01-onlyTracker.xml"
geometry_path = os.path.join(os.getenv("DETCRDROOT"), "compact", geometry_option)

from Configurables import DetGeomSvc
geosvc = DetGeomSvc("GeomSvc"); geosvc.compact = geometry_path

from Configurables import PodioInput
podioinput = PodioInput("PodioReader", collections=[
    "MCParticle",
    "VXDCollection", "ITKBarrelCollection", "TPCCollection", "OTKBarrelCollection",
    "VXDTrackerHits", "ITKBarrelTrackerHits", "TPCTrackerHits", "OTKBarrelTrackerHits",
    "VXDTrackerHitAssociation", "ITKBarrelTrackerHitAssociation",
    "TPCTrackerHitAss", "OTKBarrelTrackerHitAssociation",
    "CompleteTracks",
])

from Configurables import RecGsfTracking, TrackSystemSvc, GearSvc
tracksys = TrackSystemSvc("TrackSystemSvc")
gearsvc = GearSvc("GearSvc")

gsf = RecGsfTracking("RecGsfTracking")
gsf.ElectronHypothesis = True; gsf.MaxComponents = 12; gsf.ReductionTargetComponents = 3
gsf.BHSplitThreshold = 1e-4; gsf.MSOn = True; gsf.ElossOn = True
gsf.VerboseDump = True; gsf.VerboseSplitDump = False; gsf.MaterialIPExtrapolation = False
gsf.KappaSeedCov = 1e-7
gsf.BHModel = "GlobalSim2GeV85"
gsf.SelectedEventIndices = [0, 1, 2, 4, 5, 6, 7, 8, 10, 12, 13, 15, 16, 18, 19, 20, 22, 24, 26, 29, 30, 31, 33, 36, 37, 44, 45, 47, 48, 52, 54, 55, 58, 60, 62, 63, 64, 67, 68, 69, 71, 72, 74, 75, 76, 77, 78, 79, 80, 81, 82, 85, 87, 88, 89, 90, 91, 92, 93, 94, 97, 98]  # 1e-7=tight(trust LCIO); 1e-4=loose(more KF freedom)

from Configurables import PodioOutput
out = PodioOutput("outputalg"); out.filename = "/tmp/gsf_light_global_bh_seed1_5_manual/outputs/gsf_light_global_bh_seed1.root"; out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr, MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")
ApplicationMgr(TopAlg=[podioinput, gsf, out], EvtSel='NONE', EvtMax=evtmax,
               ExtSvc=[dsvc, geosvc, tracksys, gearsvc, evtseeder], OutputLevel=INFO)
