"""Run from the CEPCSW environment with gaudirun.py.

Edit the controls below, or use the BP_* environment variables for isolated
diagnostic jobs. Existing GSF/tracker cards are not involved.
"""
import os
from Gaudi.Configuration import INFO
from Configurables import (
    ApplicationMgr, DetGeomSvc, GearSvc, PodioInput, RecBreakpoint,
    TrackSystemSvc, k4DataSvc,
)

input_file = os.environ.get("BP_INPUT", "trk_v01.root")
output_file = os.environ.get("BP_OUTPUT", "breakpoint_flat.root")
event_count = int(os.environ.get("BP_EVENTS", "10"))
selected_events = [int(v) for v in os.environ.get("BP_SELECTED", "").split(",") if v]

# Zero-based ordered-hit intervals: 5 means hit[5] -> hit[6].
# Used only with IntervalSelectionMode=Manual. Empty then means the 5D reference.
# For example [5, 7] introduces two independent local loss parameters.
breakpoint_intervals = [int(v) for v in os.environ.get("BP_INTERVALS", "").split(",") if v]

dsvc = k4DataSvc("EventDataSvc", input=input_file)
geometry = DetGeomSvc("GeomSvc")
geometry.compact = os.path.join(
    os.environ["DETCRDROOT"], "compact/TDR_o1_v01/TDR_o1_v01-onlyTracker.xml"
)
gear = GearSvc("GearSvc")
track_system = TrackSystemSvc("TrackSystemSvc")
reader = PodioInput("PodioReader", collections=[
    "CompleteTracks", "MCParticle",
    "VXDTrackerHits", "ITKBarrelTrackerHits", "ITKEndcapTrackerHits",
    "TPCTrackerHits", "OTKBarrelTrackerHits", "OTKEndcapTrackerHits",
])

fit = RecBreakpoint("RecBreakpoint")
fit.InputTracks = "CompleteTracks"
fit.OutputTracks = "BreakpointTracksRTS"
fit.OutputTracksBackwardFilter = "BreakpointTracksBackwardFilter"
fit.OutputTracksFreeLossRTS = "BreakpointTracksFreeLossRTS"
fit.OutputTracksFreeLossBackwardFilter = "BreakpointTracksFreeLossBackwardFilter"
fit.OutputTracksTruthOverrideRTS = "BreakpointTracksTruthOverrideRTS"
fit.OutputTracksTruthOverrideBackwardFilter = "BreakpointTracksTruthOverrideBackwardFilter"
fit.BreakpointIntervals = breakpoint_intervals
# WHERE: choose the breakpoint intervals shared by ordinary and truth-override fits.
# Truth (default): select every matched hit interval with positive G4 eBrem loss;
# keep BreakpointIntervals empty here, since the list is built for each track.
# Manual: use only BreakpointIntervals above, even if actual eBrem occurs elsewhere.
# Auto: reserved; initialization fails, regardless of TruthOverride.
# Selection provides locations, not truth loss amounts, to the ordinary fit.
fit.IntervalSelectionMode = os.environ.get("BP_INTERVAL_SELECTION_MODE", "Truth")
# b = log(p_before/p_after). Fractional loss is 1-exp(-b).
# This first implementation uses one linearized Gaussian at each selected edge.
fit.MeanLogLoss = float(os.environ.get("BP_MEAN_LOG_LOSS", "0.0"))
# Batch value is set in subbreakpointjobs.sh and frozen into the generated card.
# The fallback below is for direct standalone use, without the submission script.
fit.SigmaLogLoss = float(os.environ.get("BP_SIGMA_LOG_LOSS", "0.05"))
fit.SeedScale = 1.0
# Scale the full copied first-forward endpoint covariance for BackwardFilter
# only. The mean and RTS are unchanged. Positive finite values; 1 preserves it.
# Default 100 inflates all covariance entries by 100 (standard deviations by 10).
fit.BackwardSeedScale = float(os.environ.get("BP_BACKWARD_SEED_SCALE", "100.0"))
# Three usable 2D hits in radius order; N//2 is the middle (upper for even N).
# FirstThree restores the original short-lever-arm prefit for comparisons.
fit.SeedHitSelection = os.environ.get("BP_SEED_HIT_SELECTION", "FirstMiddleLast")
# Both RTS and BackwardFilter are always produced from the common forward fit.
# Reject the retired environment control rather than silently ignoring it.
if "BP_BACKWARD_MODE" in os.environ:
    raise ValueError("BP_BACKWARD_MODE/BackwardMode was removed: both endpoints are always saved")
# Persistent6D retains b and the full 6x6 covariance at every downstream hit.
# One breakpoint at most. The backward continuation retains its local-joint
# loss treatment. LocalMarginal also supports multiple selected breakpoints.
fit.LossStateMode = os.environ.get("BP_LOSS_STATE_MODE", "LocalMarginal")
# Optional outer b fit: normalized full-track likelihood, not chi2 alone.
# Ordinary RTS/backward are ALWAYS retained. True adds optimized results in
# the FreeLoss RTS/backward collections on one selected LocalMarginal interval;
# their covariances are conditional on fitted b. False copies ordinary results
# into these extra collections/flat branches, without another fit.
# Empty lists, unsupported modes/multiple intervals and failed searches also
# copy ordinary results, with explicit free_loss_status/error diagnostics.
# free_loss_result_status: 0 absent, 1 ordinary copy, 2 optimized pair.
fit.FreeLossFit = os.environ.get("BP_FREE_LOSS_FIT", "0") == "1"
fit.FreeLossMaxLogLoss = float(os.environ.get("BP_FREE_LOSS_MAX_LOG_LOSS", "1.0"))
fit.FreeLossMaxCallsPerStart = int(os.environ.get("BP_FREE_LOSS_MAX_CALLS", "180"))
fit.FreeLossTolerance = float(os.environ.get("BP_FREE_LOSS_TOLERANCE", "0.001"))
# Numerical audit only: reverse-order and joint-smoothed forms of the SAME
# likelihood must agree. They are not alternative physics objectives.
fit.FreeLossCheckLikelihoods = os.environ.get("BP_FREE_LOSS_CHECK", "0") == "1"
# HOW MUCH, for the extra pair only: True (default) sets the b PRIOR CENTERS
# to matched G4 values at the SAME selected intervals. SigmaLogLoss is the SAME
# as in the ordinary fit; hits can still update b and its posterior variance.
# Both pairs use the same LossStateMode and one-pass fitting code.
# It neither changes the interval selection nor corrects unselected intervals.
# Example: Manual=[5], actual eBrem only in interval 7 -> prior center at 5 is
# b=0 (z=1), but the fitted b may move. Interval 7 is not added automatically.
# Primary RTS/backward always use MeanLogLoss/SigmaLogLoss. FreeLossFit changes
# only the additional FreeLoss pair. The truth pair STILL uses the configured
# positive sigma and truth prior center, not Minuit or a fixed-loss oracle.
# False, or an empty effective interval list: save ordinary RTS/backward copies
# in the truth-override outputs. Truth selection can still read truth when False.
# LossStateMode controls the ordinary fit; neither control overrides the other.
fit.TruthOverride = os.environ.get("BP_TRUTH_OVERRIDE", "1") == "1"
# Repeated relinearization was retired; both endpoints use one pass.
for retired in ("BP_MAX_ITERATIONS", "BP_ITERATION_TOLERANCE"):
    if retired in os.environ:
        raise ValueError(retired + " was removed; RecBreakpoint is one-pass only")
fit.MaxChi2PerHit = 1.e100
fit.MSOn = True
fit.ElossOn = False
fit.TruthDiagnostics = True  # optional scalar reference; never used in fitting
fit.TruthMaxEndpointDistance = 5.0  # mm, validation of associated hooks, not spatial matching
fit.VerboseDump = os.environ.get("BP_VERBOSE", "0") == "1"
fit.VerifyKFReference = os.environ.get("BP_VERIFY_KF", "0") == "1"
fit.SelectedEventIndices = selected_events
fit.OutputFile = output_file  # Existing files cause failure, never overwrite.
fit.OutputLevel = INFO

# Derive input needs from the final property values, including manual edits.
if fit.IntervalSelectionMode == "Truth" or (fit.TruthOverride and fit.BreakpointIntervals):
    reader.collections += [
        "VXDCollection", "ITKBarrelCollection", "ITKEndcapCollection",
        "TPCCollection", "OTKBarrelCollection", "OTKEndcapCollection",
        "VXDTrackerHitAssociation", "ITKBarrelTrackerHitAssociation", "ITKEndcapTrackerHitAssociation",
        "TPCTrackerHitAss", "OTKBarrelTrackerHitAssociation", "OTKEndcapTrackerHitAssociation",
        "GsfG4MaterialSteps", "GsfSimTrackerHitG4StepLinks",
    ]

# The flat tuple is written by RecBreakpoint. To persist the event collections:
# from Configurables import PodioOutput
# writer = PodioOutput("PodioWriter", filename="breakpoint_edm.root",
#                      outputCommands=["keep *"])
# Append writer to TopAlg below when wanted.
ApplicationMgr(TopAlg=[reader, fit], EvtSel="NONE", EvtMax=event_count,
               ExtSvc=[dsvc, geometry, gear, track_system], OutputLevel=INFO)
