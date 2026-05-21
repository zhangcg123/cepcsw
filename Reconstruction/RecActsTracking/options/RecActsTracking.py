#!/usr/bin/env python
import os

from Gaudi.Configuration import *

NTupleSvc().Output = ["MyTuples DATAFILE='test.root' OPT='NEW' TYP='ROOT'"]

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc", input="rec00.root")

from Configurables import PodioInput
podioinput = PodioInput("PodioReader", collections=[
    "MCParticle",
    "VXDCollection",
    "SITCollection",
    "VXDTrackerHits",
    "SITTrackerHits",
])

##############################################################################
# Geometry Svc
##############################################################################

# geometry_option = "TDR_o1_v01/TDR_o1_v01-onlyTracker.xml"
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

##############################################################################
# CEPCSWData
##############################################################################
cepcswdatatop ="/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/data/latest"

##############################################################################
# RecActsTracking
##############################################################################

from Configurables import RecActsTracking
actstracking = RecActsTracking("RecActsTracking")
actstracking.TGeoFile = os.path.join(cepcswdatatop, "CEPCSWData/offline-data/Reconstruction/RecActsTracking/data/tdr25.1.0/SiTrack-tgeo.root")
actstracking.TGeoConfigFile = os.path.join(cepcswdatatop, "CEPCSWData/offline-data/Reconstruction/RecActsTracking/data/tdr25.1.0/SiTrack-tgeo-config.json")
actstracking.MaterialMapFile = os.path.join(cepcswdatatop, "CEPCSWData/offline-data/Reconstruction/RecActsTracking/data/tdr25.1.0/SiTrack-material-maps.json")

# config for acts tracking
actstracking.onSurfaceTolerance = 2e-1
# actstracking.ExtendSeedRange = True
# actstracking.SeedDeltaRMin = 10
# actstracking.SeedDeltaRMax = 200
# actstracking.SeedRMax = 600
# actstracking.SeedRMin = 80
# actstracking.SeedImpactMax = 4
# actstracking.SeedRMinMiddle = 340
# actstracking.SeedRMaxMiddle = 380
actstracking.numMeasurementsCutOff = 2
actstracking.CKFchi2Cut = 1

##############################################################################
# output
##############################################################################
from Configurables import PodioOutput
out = PodioOutput("out")
out.filename = "ACTS_rec00.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr( TopAlg = [podioinput, actstracking, out],
                EvtSel = 'NONE',
                EvtMax = 5,
                ExtSvc = [dsvc, geosvc, evtseeder, gearsvc],
                # OutputLevel=DEBUG
)
