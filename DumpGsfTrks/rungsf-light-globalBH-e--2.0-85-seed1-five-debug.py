#!/usr/bin/env python
import os, sys
from Gaudi.Configuration import *

# Focused debug card for seed-1 light tracker-eBrem events that remain sane
# after the covariance/reducer fixes. Event 15 is intentionally excluded here;
# keep it as a separate stress case for the remaining tanLambda pathology.
selected_event_indices = [22, 37, 47, 89, 94]
evtmax = max(selected_event_indices) + 1

input_file = os.getenv("GSF_DEBUG_INPUT", "trk-e--2.0-85-1.root")
output_dir = os.getenv("GSF_DEBUG_OUTPUT_DIR", "/tmp/gsf_seed1_five_event_debug/outputs")
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
gsf.VerboseDump = True; gsf.VerboseSplitDump = True; gsf.MaterialIPExtrapolation = False
gsf.KappaSeedCov = 1e-7
gsf.BHModel = "GlobalSim2GeV85"
gsf.SelectedEventIndices = selected_event_indices

from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = os.path.join(output_dir, "gsf_light_global_bh_seed1_five_debug.root")
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr, MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")
ApplicationMgr(TopAlg=[podioinput, gsf, out], EvtSel="NONE", EvtMax=evtmax,
               ExtSvc=[dsvc, geosvc, tracksys, gearsvc, evtseeder], OutputLevel=INFO)
