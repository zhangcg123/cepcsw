#!/usr/bin/env python
import os, sys
from Gaudi.Configuration import *

seed = int(os.getenv("GSF_LIGHT_SEED", "1"))
output_dir = os.getenv("GSF_LIGHT_OUTPUT_DIR", "/tmp/gsf_light_global_bh_seed1_5_manual/outputs")

selected_event_indices_by_seed = {
    1: [0, 1, 2, 4, 5, 6, 7, 8, 10, 12, 13, 15, 16, 18, 19, 20, 22, 24, 26, 29, 30, 31, 33, 36, 37, 44, 45, 47, 48, 52, 54, 55, 58, 60, 62, 63, 64, 67, 68, 69, 71, 72, 74, 75, 76, 77, 78, 79, 80, 81, 82, 85, 87, 88, 89, 90, 91, 92, 93, 94, 97, 98],
    2: [0, 3, 4, 5, 7, 8, 9, 10, 11, 12, 18, 20, 21, 23, 27, 28, 29, 30, 32, 34, 35, 40, 42, 44, 49, 51, 54, 65, 70, 71, 73, 74, 75, 79, 83, 89, 94, 95, 98],
    3: [1, 4, 7, 8, 10, 11, 12, 13, 15, 17, 21, 22, 25, 26, 27, 28, 29, 30, 31, 35, 38, 39, 42, 43, 44, 45, 48, 49, 50, 53, 54, 55, 56, 61, 62, 63, 66, 67, 68, 72, 73, 77, 78, 80, 83, 87, 89, 95, 96, 98, 99],
    4: [2, 4, 5, 8, 10, 11, 13, 16, 24, 26, 33, 34, 39, 42, 44, 45, 46, 48, 51, 54, 60, 62, 63, 65, 66, 69, 72, 73, 75, 78, 79, 80, 83, 84, 85, 87, 95, 96, 98, 99],
    5: [0, 1, 4, 5, 6, 7, 8, 14, 16, 21, 23, 24, 25, 30, 35, 37, 38, 39, 41, 42, 43, 46, 50, 51, 59, 60, 63, 67, 69, 71, 73, 75, 80, 81, 83, 84, 85, 86, 88, 91, 93, 95, 98],
}

if seed not in selected_event_indices_by_seed:
    raise RuntimeError("GSF_LIGHT_SEED must be one of 1, 2, 3, 4, 5")

selected_event_indices = selected_event_indices_by_seed[seed]
evtmax = max(selected_event_indices) + 1
input_file = os.getenv("GSF_LIGHT_INPUT", "trk-e--2.0-85-{}.root".format(seed))

if not os.path.isdir(output_dir):
    os.makedirs(output_dir)

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input=input_file)

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
gsf.ElectronHypothesis = True; gsf.MaxComponents = 12
gsf.BHSplitThreshold = 1e-4; gsf.MSOn = True; gsf.ElossOn = True
gsf.VerboseDump = True; gsf.VerboseSplitDump = False; gsf.MaterialIPExtrapolation = False
gsf.KappaSeedCov = 1e-7
gsf.BHModel = "GlobalSim2GeV85"
gsf.SelectedEventIndices = selected_event_indices

from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = os.path.join(output_dir, "gsf_light_global_bh_seed{}.root".format(seed))
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr, MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")
ApplicationMgr(TopAlg=[podioinput, gsf, out], EvtSel='NONE', EvtMax=evtmax,
               ExtSvc=[dsvc, geosvc, tracksys, gearsvc, evtseeder], OutputLevel=INFO)
