#!/usr/bin/env python
import os, sys
from Gaudi.Configuration import *

evtmax = 200

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="trk-e--0.5-85-2.root")

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
gsf.BHSplitThreshold = 0.01; gsf.MSOn = True; gsf.ElossOn = True
gsf.VerboseDump = False; gsf.MaterialIPExtrapolation = False

from Configurables import RecGsfFlatTuple
flat = RecGsfFlatTuple("RecGsfFlatTuple")
flat.OutputFile = "gsf_flat-e--0.5-85-2.root"
flat.BField = 3.0
flat.HitCollectionNames = ["VXDTrackerHits", "ITKBarrelTrackerHits", "TPCTrackerHits", "OTKBarrelTrackerHits"]

from Configurables import PodioOutput
out = PodioOutput("outputalg"); out.filename = "gsf-e--0.5-85-2.root"; out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr, MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")
ApplicationMgr(TopAlg=[podioinput, gsf, flat, out], EvtSel='NONE', EvtMax=evtmax,
               ExtSvc=[dsvc, geosvc, tracksys, gearsvc, evtseeder], OutputLevel=ERROR)
