#!/usr/bin/env python

import os
from Gaudi.Configuration import *
from Configurables import k4DataSvc
dsvc = k4DataSvc("EventDataSvc")
#########################################################################
# read LCIO files
from Configurables import LCIOInput
read = LCIOInput("read")
read.inputs = [
"/cefs/higgs/PFAData/Sample/Generator/FullGeo_Baseline/E240_nnH_gg/Reco/Reco_Baseline_E240_nnH_gg_00001_0.slcio"
]
read.mode = 1
read.collections = [
        "MCParticle:MCParticle",
        "SimCalorimeterHit:EcalBarrelSiliconCollection",
        "SimCalorimeterHit:EcalBarrelSiliconPreShowerCollection",
        "SimCalorimeterHit:EcalEndcapRingCollection",
        "SimCalorimeterHit:EcalEndcapRingPreShowerCollection",
        "SimCalorimeterHit:EcalEndcapSiliconCollection",
        "SimCalorimeterHit:EcalEndcapSiliconPreShowerCollection",
        "SimCalorimeterHit:HcalBarrelCollection",
        "SimCalorimeterHit:HcalEndCapRingsCollection",
        "SimCalorimeterHit:HcalEndCapsCollection",
        "SimCalorimeterHit:LumiCalCollection",
        "SimCalorimeterHit:MuonEndCapCollection",
        "Track:ClupatraTrackSegments",
        "Track:ClupatraTracks",
        "Track:ForwardTracks",
        "Track:MarlinTrkTracks",
        "Track:SiTracks",
        "Track:SubsetTracks",
        "TrackerHit:FTDSpacePoints",
        "TrackerHit:SETSpacePoints",
        "TrackerHit:SITSpacePoints",
        "TrackerHit:TPCTrackerHits",
        "TrackerHitPlane:FTDPixelTrackerHits",
        "TrackerHitPlane:FTDStripTrackerHits",
        "TrackerHitPlane:SETTrackerHits",
        "TrackerHitPlane:SITTrackerHits",
        "TrackerHitPlane:VXDTrackerHits",
        "SimTrackerHit:COILCollection",
        "SimTrackerHit:FTD_PIXELCollection",
        "SimTrackerHit:FTD_STRIPCollection",
        "SimTrackerHit:SETCollection",
        "SimTrackerHit:SITCollection",
        "SimTrackerHit:TPCCollection",
        "SimTrackerHit:TPCSpacePointCollection",
        "SimTrackerHit:VXDCollection",
        "CalorimeterHit:LCAL",
        "CalorimeterHit:LHCAL"
]
#########################################################################
geometry_option = "CepC_v4.xml"

if not os.getenv("DETCEPCV4ROOT"):
    print("Can't find the geometry. Please setup envvar DETCEPCV4ROOT." )
    sys.exit(-1)

geometry_path = os.path.join(os.getenv("DETCEPCV4ROOT"), "compact", geometry_option)
if not os.path.exists(geometry_path):
    print("Can't find the compact geometry file: %s"%geometry_path)
    sys.exit(-1)

from Configurables import GeomSvc
geosvc = GeomSvc("GeomSvc")
geosvc.compact = geometry_path
########################################################################
from Configurables import GearSvc
gearSvc  = GearSvc("GearSvc")
# gearSvc.GearXMLFile = "Detector/DetCEPCv4/compact/FullDetGear.xml"
# gearSvc.GearXMLFile = "/cefs/higgs/PFAData/Sample/Generator/FullGeo_Baseline/E240_nnH_gg/Simu/GearOutput.xml"
gearSvc.GearXMLFile = "Detector/DetCEPCv4/compact/CEPCV4_FullDet_GearOutput.xml"
##############################################################################
from Configurables import G2CDArborAlg
caloDigi = G2CDArborAlg("G2CDArborAlg")
caloDigi.ReadLCIO = True
caloDigi.ECALCollections = ["EcalBarrelSiliconCollection","EcalEndcapSiliconCollection","EcalEndcapRingCollection"]
caloDigi.HCALCollections = ["HcalBarrelCollection","HcalEndCapsCollection","HcalEndCapRingsCollection"]
caloDigi.DigiECALCollection = ["ECALBarrel","ECALEndcap","ECALOther"]
caloDigi.DigiHCALCollection = ["HCALBarrel","HCALEndcap","HCALOther"]

caloDigi.CalibrECAL = [48.16, 96.32]
caloDigi.HCALThreshold = 0.12

caloDigi.PolyaParaA = 1.1
caloDigi.PolyaParaB = 1.0
caloDigi.PolyaParaC = 0.0

caloDigi.EventReportEvery = 1
##############################################################################
from Configurables import MarlinArbor
marlinArbor = MarlinArbor("MarlinArbor")
marlinArbor.ReadLCIO = True
marlinArbor.ECALCollections =["ECALBarrel","ECALEndcap","ECALOther"]
marlinArbor.HCALCollections =["HCALBarrel","HCALEndcap","HCALOther"]
marlinArbor.ECALReadOutNames= ["EcalBarrelCollection","EcalEndcapsCollection","EcalEndcapRingCollection"]
marlinArbor.HCALReadOutNames= ["HcalBarrelCollection","HcalEndcapsCollection","HcalEndcapRingCollection"]
##############################################################################
from Configurables import BushConnect
bushconnect = BushConnect("BushConnect")
bushconnect.ReadLCIO = True
##############################################################################
from Configurables import TotalInvMass
totalInvM = TotalInvMass("TotalInvMass")
totalInvM.TreeOutputFile = "Examples/options/LCIO_read_Arbor_nnHgg_BMRAna.root"
##############################################################################
from Configurables import PodioOutput
write = PodioOutput("write")
write.filename = "Examples/options/LCIO_read_Arbor_nnHgg_Reco.root"
write.outputCommands = ["keep *"]
#########################################################################
# ApplicationMgr
from Configurables import ApplicationMgr
ApplicationMgr( TopAlg = [read, caloDigi, marlinArbor, bushconnect, totalInvM, write],
                EvtSel = 'NONE',
                EvtMax = 2,
                ExtSvc = [dsvc, geosvc, gearSvc],
                OutputLevel=DEBUG
)
