"""Read existing REC; write baseline-KF interval-identification features.

No breakpoint fit, ECAL input, truth steering, or EDM output. Existing output
files are refused. Run through the CEPCSW build-tree gaudirun environment.
"""
import os
from Gaudi.Configuration import INFO
from Configurables import ApplicationMgr, DetGeomSvc, GearSvc, k4DataSvc, PodioInput
from RecBreakpoint.RecBreakpointConf import RecBreakpointIdentification

input_file = os.environ.get("BPID_INPUT", "rec.root")
output_file = os.environ.get("BPID_OUTPUT", "breakpoint_identification.root")
record_truth = os.environ.get("BPID_TRUTH", "1") == "1"
geometry = DetGeomSvc("GeomSvc")
geometry.compact = os.path.join(os.environ["DETCRDROOT"], "compact/TDR_o1_v01/TDR_o1_v01-onlyTracker.xml")
gear = GearSvc("GearSvc")
data = k4DataSvc("EventDataSvc", input=input_file)
collections = ["CompleteTracks", "VXDTrackerHits", "ITKBarrelTrackerHits",
               "ITKEndcapTrackerHits", "TPCTrackerHits", "OTKBarrelTrackerHits", "OTKEndcapTrackerHits"]
if record_truth:
    collections += ["MCParticle", "CompleteTracksParticleAssociation", "GsfG4MaterialSteps", "GsfSimTrackerHitG4StepLinks",
                    "VXDCollection", "ITKBarrelCollection", "ITKEndcapCollection", "TPCCollection", "OTKBarrelCollection", "OTKEndcapCollection",
                    "VXDTrackerHitAssociation", "ITKBarrelTrackerHitAssociation", "ITKEndcapTrackerHitAssociation",
                    "TPCTrackerHitAss", "OTKBarrelTrackerHitAssociation", "OTKEndcapTrackerHitAssociation"]
reader = PodioInput("PodioReader", collections=collections)
diagnostic = RecBreakpointIdentification("IntervalIdentification")
diagnostic.InputTracks = "CompleteTracks"
diagnostic.EventHeaders = "EventHeader"
diagnostic.ReadEventHeader = False  # current REC has no EventHeader; use source_file + event_index
diagnostic.TupleFile = output_file
diagnostic.SourceFile = input_file
diagnostic.GeometryTag = geometry.compact
diagnostic.RecordTruth = record_truth
diagnostic.TruthSteps = "GsfG4MaterialSteps"
diagnostic.TruthLinks = "GsfSimTrackerHitG4StepLinks"
diagnostic.TrackTruthAssociations = "CompleteTracksParticleAssociation"
diagnostic.VXDHitAssociations = "VXDTrackerHitAssociation"
diagnostic.ITKBarrelHitAssociations = "ITKBarrelTrackerHitAssociation"
diagnostic.ITKEndcapHitAssociations = "ITKEndcapTrackerHitAssociation"
diagnostic.TPCHitAssociations = "TPCTrackerHitAss"
diagnostic.OTKBarrelHitAssociations = "OTKBarrelTrackerHitAssociation"
diagnostic.OTKEndcapHitAssociations = "OTKEndcapTrackerHitAssociation"
# Standard KF settings, NOT the augmented-breakpoint settings.
diagnostic.MultipleScattering = True
diagnostic.EnergyLoss = True
diagnostic.MaxChi2PerHit = 100.0
# Variances of EDM (d0,phi,omega,z0,tanLambda), used separately in each direction.
diagnostic.SeedVariances = [1e6, 1e2, 1e-4, 1e6, 1e2]
diagnostic.TruthEndpointToleranceMM = 5.0  # provenance validation, never nearest-position matching
diagnostic.SelectedEventIndices = [int(x) for x in os.environ.get("BPID_SELECTED", "").split(",") if x]
diagnostic.VerboseDiagnostics = os.environ.get("BPID_VERBOSE", "0") == "1"
ApplicationMgr(TopAlg=[reader, diagnostic], EvtSel="NONE", EvtMax=int(os.environ.get("BPID_EVENTS", "10")),
               ExtSvc=[data, geometry, gear], OutputLevel=INFO)
