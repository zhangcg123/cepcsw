#!/usr/bin/env python
import os
from Gaudi.Configuration import *

evtmax = 5

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="trk-e--1.0-85-1.root")

from Configurables import PodioInput
podioinput = PodioInput("PodioReader", collections=[
    "MCParticle",
    "VXDCollection", "ITKBarrelCollection", "TPCCollection", "OTKBarrelCollection",
    "VXDTrackerHits", "ITKBarrelTrackerHits", "TPCTrackerHits", "OTKBarrelTrackerHits",
    "VXDTrackerHitAssociation", "ITKBarrelTrackerHitAssociation",
    "TPCTrackerHitAss", "OTKBarrelTrackerHitAssociation",
    "CompleteTracks",
])

from Configurables import RecGsfSimHitTuple
simtuple = RecGsfSimHitTuple("RecGsfSimHitTuple")
simtuple.OutputFile = "gsf_simhit_tuple_test.root"
simtuple.PrimaryOnly = True
simtuple.ElectronOnly = True
simtuple.SimHitCollectionNames = [
    "VXDCollection",
    "ITKBarrelCollection",
    "TPCCollection",
    "OTKBarrelCollection",
]

from Configurables import ApplicationMgr
ApplicationMgr(TopAlg=[podioinput, simtuple], EvtSel='NONE', EvtMax=evtmax,
               ExtSvc=[dsvc], OutputLevel=INFO)
