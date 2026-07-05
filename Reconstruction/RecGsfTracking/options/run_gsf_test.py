#!/usr/bin/env python
import os, sys
from Gaudi.Configuration import *

evtmax = 5

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="trk-e--1.0-85-1.root")

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
gsf.VerboseDump = True; gsf.MaterialIPExtrapolation = False
gsf.KappaSeedCov = 1e-7  # 1e-7=tight(trust LCIO); 1e-4=loose(more KF freedom)

from Configurables import PodioOutput
out = PodioOutput("outputalg"); out.filename = "gsf_test.root"; out.outputCommands = ["keep *"]

from Configurables import ApplicationMgr, MarlinEvtSeeder
evtseeder = MarlinEvtSeeder("EventSeeder")
ApplicationMgr(TopAlg=[podioinput, gsf, out], EvtSel='NONE', EvtMax=evtmax,
               ExtSvc=[dsvc, geosvc, tracksys, gearsvc, evtseeder], OutputLevel=INFO)