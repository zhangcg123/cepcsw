#!/usr/bin/env python
import os, sys
from Gaudi.Configuration import *

from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")

# option for standalone tracker study
#geometry_option = "TDR_o1_v01/TDR_o1_v01-oldVersion.xml"
#geometry_option = "TDR_o1_v01/TDR_o1_v01-patchOTK.xml"
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

from Configurables import GeomMetaWriter
metaWriter = GeomMetaWriter("GeomMetaWriter")
    
# output
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "GeometryMetaData.root"
out.outputCommands = ["keep *"]

# ApplicationMgr
from Configurables import ApplicationMgr
mgr = ApplicationMgr(
    TopAlg = [metaWriter, out],
    EvtSel = 'NONE',
    EvtMax = 0,
    ExtSvc = [dsvc, geosvc],
    HistogramPersistency = 'ROOT',
    OutputLevel = ERROR
)
