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
evtmax = 100
input_file = "trk-e--2.0-85-1.root"
edm_output = "gsf-reverse-e--2.0-85-1.root"
tuple_output = "gsf_flat-reverse-e--2.0-85-1.root"

# True for electron GSF/BH processing. Set False for the no-BH muon control.
electron_hypothesis = True

# Set, for example, [11] for comprehensive focused diagnostics. Leave empty
# for normal processing of every event up to evtmax.
selected_event_indices = []
verbose_components = False

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
gsf.BHModel = "GlobalSim2GeV85"
gsf.BHSplitThreshold = 1.0e-4

gsf.MaxComponents = 12
gsf.ReductionTargetComponents = 0  # 0 means MaxComponents
gsf.ReductionMode = "KL"
gsf.ReductionMinHitsAfterSplit = 0
gsf.ComponentWeightCutoff = 1.0e-8

gsf.MSOn = True
gsf.ElossOn = True
gsf.KappaSeedCov = 1.0e-7

# Enables inward multi-component filtering and publishes its best IP branch
# with matching reverse chi2/NDF metadata.
gsf.ReverseFiltering = True
gsf.GSFOutputMode = "BestBranch"
gsf.MaterialIPExtrapolation = False

gsf.SelectedEventIndices = selected_event_indices
gsf.VerboseDump = verbose_components
gsf.VerboseSplitDump = verbose_components
gsf.ComponentDebugDump = verbose_components
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
