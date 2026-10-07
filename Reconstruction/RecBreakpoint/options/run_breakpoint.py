"""Run from the CEPCSW environment with gaudirun.py.

Edit the controls below, or use the BP_* environment variables for isolated
diagnostic jobs. Existing GSF/tracker cards are not involved.
"""
import os

# Reject retired steering before loading Gaudi or opening any input/output.
if "BP_LOSS_PRIOR_MODE" in os.environ:
    raise ValueError("BP_LOSS_PRIOR_MODE/LossPriorMode was removed; ordinary breakpoint fits use a Gaussian loss prior")
for _retired in ("BP_ABSOLUTE_NEUTRAL_RTS", "BP_ABSOLUTE_NEUTRAL_DIFFUSE_REFERENCE",
                 "BP_ABSOLUTE_NEUTRAL_REFERENCE_SOURCE"):
    if _retired in os.environ:
        raise ValueError(_retired + " was removed; use BP_ECAL_LOSS_REFERENCE_MODE="
                         "Off/NoReference/PreReference/PostReference")

from Gaudi.Configuration import INFO
from Configurables import (
    ApplicationMgr, DetGeomSvc, GearSvc, PodioInput, RecBreakpoint,
    TrackSystemSvc, k4DataSvc,
)

# The calorimeter-reconstructed event retains CompleteTracks, tracker truth
# provenance, and ECAL/PFO collections for later external eBrem studies.
input_file = os.environ.get("BP_INPUT", "rec.root")
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
    "CompleteTracks", "CompleteTracksParticleAssociation", "MCParticle",
    "CyberPFOPID", "EcalCluster", "GsfG4BremsPhotons", "GsfG4BremsPhotonSteps",
    "VXDTrackerHits", "ITKBarrelTrackerHits", "ITKEndcapTrackerHits",
    "TPCTrackerHits", "OTKBarrelTrackerHits", "OTKEndcapTrackerHits",
])

fit = RecBreakpoint("RecBreakpoint")
fit.InputTracks = "CompleteTracks"
fit.OutputTracks = "BreakpointTracksRTS"
fit.OutputTracksBackwardFilter = "BreakpointTracksBackwardFilter"
fit.OutputTracksFreeLossRTS = "BreakpointTracksFreeLossRTS"
fit.OutputTracksFreeLossBackwardFilter = "BreakpointTracksFreeLossBackwardFilter"
fit.OutputTracksBeamGuidedFreeLossRTS = "BreakpointTracksBeamGuidedFreeLossRTS"
fit.OutputTracksBeamGuidedFreeLossBackwardFilter = "BreakpointTracksBeamGuidedFreeLossBackwardFilter"
fit.OutputTracksTruthOverrideRTS = "BreakpointTracksTruthOverrideRTS"
fit.OutputTracksTruthOverrideBackwardFilter = "BreakpointTracksTruthOverrideBackwardFilter"
fit.OutputTracksDiffuseAugmentedRTS = "BreakpointTracksDiffuseAugmentedRTS"
fit.OutputTracksAbsoluteNeutralRTS = "BreakpointTracksAbsoluteNeutralRTS"
fit.BreakpointIntervals = breakpoint_intervals
# WHERE: choose the breakpoint intervals shared by ordinary and truth-override fits.
# Truth (default): select ONLY the matched hit interval with the largest summed
# absolute G4 eBrem momentum loss (ties: innermost). All three pairs share it.
# Other loss intervals are not fitted. No positive loss means no breakpoint;
# keep BreakpointIntervals empty here, since the list is built for each track.
# Manual: use only BreakpointIntervals above, even if actual eBrem occurs elsewhere.
# Auto: reserved; initialization fails, regardless of TruthOverride.
# Selection provides locations, not truth loss amounts, to the ordinary fit.
fit.IntervalSelectionMode = os.environ.get("BP_INTERVAL_SELECTION_MODE", "Truth")
# b = log(p_before/p_after). Fractional loss is 1-exp(-b).
# MeanLogLoss is the center of the Gaussian prior on b at each selected edge.
# The default 0 represents the no-loss hypothesis; hits may update b.
# This first implementation uses one linearized Gaussian at each selected edge.
fit.MeanLogLoss = float(os.environ.get("BP_MEAN_LOG_LOSS", "0.0"))
# SigmaLogLoss is the standard deviation OF b, not log(sigma) and not the
# energy error itself. The prior variance used by the fitter is SigmaLogLoss**2.
# Batch value is set in subbreakpointjobs.sh and frozen into the generated card.
# The fallback below is for direct standalone use, without the submission script.
# This SAME prior sigma is used by ordinary, free-loss and truth-centred fits.
# Minuit varies the free-loss prior center; each trial/final refit retains sigma.
fit.SigmaLogLoss = float(os.environ.get("BP_SIGMA_LOG_LOSS", "0.001"))
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
fit.LossStateMode = os.environ.get("BP_LOSS_STATE_MODE", "Persistent6D")
# Independent exact-diffuse augmented 6D KF/RTS experiment (compiled/card on).
# Required by the default ECAL PostReference. To disable, also choose ECAL
# Off or NoReference; the diffuse output then copies ordinary RTS.
# At the one selected interval, b=log(p_before/p_after) starts with no finite
# Gaussian prior. Downstream hits identify b; SigmaLogLoss is NOT used here.
# 0: disabled ordinary-RTS copy; 1: no-interval ordinary copy;
# 2: diffuse fitted; -1: underidentified/failed input-KF copy.
# Negative fitted b is retained as a diagnostic, not silently clipped to 0.
# Its finite innovation chi2 excludes the diffuse-consuming coordinate; it is
# NOT an absolute likelihood comparable with Gaussian-prior fits.
fit.DiffuseAugmentedRTS = os.environ.get("BP_DIFFUSE_AUGMENTED_RTS", "1") == "1"
# One ECAL absolute-loss KF/RTS fitter, selected by this single controller:
# Off: copy ordinary RTS into the ECAL output.
# NoReference: evaluate the loss map/Jacobian at the live forward state at hit i.
# PreReference: use the diffuse smoothed state at i as the loss-map reference.
# PostReference (default): start with that same state; replace its curvature using
# p_diffuse(i+1)+E_neutral, with the charge sign from i+1. Keep pivot/direction i.
# PreReference/PostReference require DiffuseAugmentedRTS=True. Only their mean
# reference is used; the live seed/covariance are retained. No diffuse error
# is inserted as a prior. This changes one loss map, not the whole trajectory.
# All three active modes collect the same hit-supported neutral ECAL clusters
# around the input track's ECAL direction and initialize the sixth coordinate
# L=p_before-p_after with E_neutral and its error in GeV. Each runs the same
# 6D fitter; ECAL energy is not applied as a second measurement update.
# One selected interval is required. Missing interval/cluster or failed fit
# produces an ordinary RTS copy, with explicit status/error in the flat tuple.
fit.EcalLossReferenceMode = os.environ.get("BP_ECAL_LOSS_REFERENCE_MODE", "PostReference")
fit.NeutralLossThetaWindowMrad = float(os.environ.get("BP_NEUTRAL_THETA_MRAD", "10"))
fit.NeutralLossPhiWindowMrad = float(os.environ.get("BP_NEUTRAL_PHI_MRAD", "200"))
# Per-cluster sigma_E [GeV] = a*sqrt(E [GeV]) + c*E [GeV]; independent cluster
# variances are added. These are provisional resolution assumptions.
fit.NeutralLossStochasticError = float(os.environ.get("BP_NEUTRAL_STOCHASTIC", "0.011"))
fit.NeutralLossConstantError = float(os.environ.get("BP_NEUTRAL_CONSTANT", "0.004"))
# Both ordinary LossStateMode representations use the MeanLogLoss/SigmaLogLoss
# Gaussian prior; the separate diffuse output above does not.
# The optional Minuit pair below optimizes the prior center with the SAME sigma;
# it remains separate from the ordinary fit and the truth-centred pair.
# Optional outer prior-center fit: normalized full-track likelihood, not chi2 alone.
# Ordinary RTS/backward are ALWAYS retained. True adds optimized results in
# the FreeLoss RTS/backward collections on one selected interval, using the
# selected LossStateMode (Persistent6D or LocalMarginal) for every trial/refit;
# their covariances include the loss posterior variance and correlations.
# Minuit's error on the chosen prior center is NOT another SigmaLogLoss.
# False copies ordinary results
# into these extra collections/flat branches, without another fit.
# Empty lists copy ordinary results. Unsupported modes/manual multiple intervals
# or failed optimization copy the INPUT CompleteTracks KF into both free outputs.
# Success requires a converged, valid Minuit minimum (not just a finite scan).
# free_loss_result_status: 0 absent, 1 ordinary copy, 2 optimized pair, 3 KF fallback.
# KF fallback has IP parameters/covariance and free_loss_kf_chi2; unavailable
# per-hit refit diagnostics remain empty/NaN. Ordinary/truth pairs stay separate.
# Maintained standalone/batch card default is ON; set BP_FREE_LOSS_FIT=0 to disable.
fit.FreeLossFit = os.environ.get("BP_FREE_LOSS_FIT", "1") == "1"
fit.FreeLossMaxLogLoss = float(os.environ.get("BP_FREE_LOSS_MAX_LOG_LOSS", "1.0"))  # bounds prior center, not fitted b
fit.FreeLossMaxCallsPerStart = int(os.environ.get("BP_FREE_LOSS_MAX_CALLS", "180"))
fit.FreeLossTolerance = float(os.environ.get("BP_FREE_LOSS_TOLERANCE", "0.001"))
# A second free-loss optimizer adds one transverse beam-origin likelihood to
# the OBJECTIVE at every trial. No beam hit or update enters either KF/RTS fit.
# The ordinary FreeLoss pair above remains unchanged; when this switch is off,
# the beam-guided pair is an exact copy of that base pair. Both use the same
# detector hits, selected breakpoint and SigmaLogLoss.
fit.FreeLossBeamSpotObjective = os.environ.get("BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE", "1") == "1"
fit.BeamSpotX = float(os.environ.get("BP_BEAM_SPOT_X", "0.0"))  # mm
fit.BeamSpotY = float(os.environ.get("BP_BEAM_SPOT_Y", "0.0"))  # mm
fit.BeamSpotSigmaX = float(os.environ.get("BP_BEAM_SPOT_SIGMA_X", "0.0145"))  # mm
fit.BeamSpotSigmaY = float(os.environ.get("BP_BEAM_SPOT_SIGMA_Y", "3.6e-5"))  # mm
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
# FreeLossFit uses that same sigma but an optimized prior center instead.
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
