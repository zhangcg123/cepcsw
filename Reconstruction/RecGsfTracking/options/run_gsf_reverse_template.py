#!/usr/bin/env python
"""Template card for the surface-local + reverse-GSF milestone.

Run from the CEPCSW repository root:

  source setup.sh
  build.105.0.0.x86_64-el9-gcc11-opt/run \
    gaudirun.py Reconstruction/RecGsfTracking/options/run_gsf_reverse_template.py
"""

import os
from Gaudi.Configuration import *

# ---- User inputs -----------------------------------------------------------
evtmax = int(os.getenv("GSF_EVTMAX", "100"))
input_file = os.getenv("GSF_INPUT_FILE", "trk-e--2.0-85-1.root")
edm_output = os.getenv("GSF_EDM_OUTPUT", "gsf-reverse-e--2.0-85-1.root")
tuple_output = os.getenv("GSF_TUPLE_OUTPUT", "gsf_flat-reverse-e--2.0-85-1.root")

# True for electron GSF/BH processing. Set False for the no-BH muon control.
electron_hypothesis = os.getenv(
    "GSF_ELECTRON_HYPOTHESIS", "1").lower() in ("1", "true", "yes")

# Set GSF_SELECTED_EVENT_INDICES to a comma-separated list such as "1,4,7".
# Leave it empty for normal processing of every event up to evtmax.
selected_event_indices = [
    int(index) for index in os.getenv("GSF_SELECTED_EVENT_INDICES", "").split(",")
    if index.strip()
]
# Disabled for normal production; opt in only for focused state-by-state runs.
verbose_components = os.getenv(
    "GSF_VERBOSE_COMPONENTS", "0").lower() in ("1", "true", "yes")

# ---- Input and geometry ----------------------------------------------------
from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input=input_file)

geometry = os.path.join(
    os.getenv("DETCRDROOT"),
    "compact",
    "TDR_o1_v01/TDR_o1_v01-onlyTracker.xml",
)

from Configurables import DetGeomSvc
geosvc = DetGeomSvc("GeomSvc")
geosvc.compact = geometry

from Configurables import PodioInput
podioinput = PodioInput("PodioReader", collections=[
    "MCParticle",
    "VXDCollection", "ITKBarrelCollection", "TPCCollection",
    "OTKBarrelCollection",
    "VXDTrackerHits", "ITKBarrelTrackerHits", "TPCTrackerHits",
    "OTKBarrelTrackerHits",
    "VXDTrackerHitAssociation", "ITKBarrelTrackerHitAssociation",
    "TPCTrackerHitAss", "OTKBarrelTrackerHitAssociation",
    "CompleteTracks",
])

# ---- GSF milestone configuration ------------------------------------------
from Configurables import RecGsfTracking, TrackSystemSvc, GearSvc
tracksys = TrackSystemSvc("TrackSystemSvc")
gearsvc = GearSvc("GearSvc")

gsf = RecGsfTracking("RecGsfTracking")
gsf.ElectronHypothesis = electron_hypothesis
gsf.BHModel = os.getenv("GSF_BH_MODEL", "CEPC2GeV85StepConditioned")
gsf.BHSplitThreshold = 1.0e-4
gsf.MaterialPathMode = os.getenv("GSF_MATERIAL_PATH_MODE", "CurrentSurface")

gsf.MaxComponents = 12
gsf.ReductionTargetComponents = 0  # 0 means MaxComponents
gsf.ReductionMode = "KL"
gsf.ReductionMinHitsAfterSplit = 0
gsf.ComponentWeightCutoff = 1.0e-8

gsf.MSOn = True
gsf.ElossOn = os.getenv("GSF_ELOSS_ON", "1").lower() in ("1", "true", "yes")
gsf.KappaSeedCov = 1.0e-7

# Enables inward multi-component filtering. ReverseOutputMode controls whether
# the published IP state is the moment-matched mixture or highest-weight branch.
gsf.ReverseFiltering = True
gsf.ReverseOutputMode = os.getenv("GSF_REVERSE_OUTPUT_MODE", "BestBranch")
gsf.MaterialIPExtrapolation = False

gsf.SelectedEventIndices = selected_event_indices
gsf.VerboseDump = verbose_components
gsf.VerboseSplitDump = verbose_components
gsf.ComponentDebugDump = verbose_components
material_transition_csv = os.getenv("GSF_MATERIAL_TRANSITION_CSV", "")
if material_transition_csv:
    gsf.MaterialTransitionCSV = material_transition_csv
gsf.OutputLevel = INFO

# ---- Flat analysis tuple ---------------------------------------------------
from Configurables import RecGsfFlatTuple
flat = RecGsfFlatTuple("RecGsfFlatTuple")
flat.OutputFile = tuple_output
flat.BField = 3.0
flat.HitCollectionNames = [
    "VXDTrackerHits",
    "ITKBarrelTrackerHits",
    "TPCTrackerHits",
    "OTKBarrelTrackerHits",
]

# ---- EDM output and application -------------------------------------------
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = edm_output
out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr, MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")
ApplicationMgr(
    TopAlg=[podioinput, gsf, flat, out],
    EvtSel="NONE",
    EvtMax=evtmax,
    ExtSvc=[dsvc, geosvc, tracksys, gearsvc, evtseeder],
    OutputLevel=INFO,
)
