"""Run from the CEPCSW environment with gaudirun.py.

Edit the controls below, or use the BP_* environment variables for isolated
diagnostic jobs. Existing GSF/tracker cards are not involved.
"""
import os

# Reject retired steering before loading Gaudi or opening any input/output.
if "BP_LOSS_PRIOR_MODE" in os.environ:
    raise ValueError("BP_LOSS_PRIOR_MODE/LossPriorMode was removed; ordinary breakpoint fits use a Gaussian loss prior")
if os.environ.get("BP_LOSS_STATE_MODE", "Persistent6D") != "Persistent6D":
    raise ValueError("BP_LOSS_STATE_MODE only accepts Persistent6D; LocalMarginal was removed")
for _retired in ("BP_ABSOLUTE_NEUTRAL_RTS", "BP_ABSOLUTE_NEUTRAL_DIFFUSE_REFERENCE",
                 "BP_ABSOLUTE_NEUTRAL_REFERENCE_SOURCE", "BP_ECAL_LOSS_REFERENCE_MODE",
                 "BP_FREE_LOSS_ECAL"):
    if _retired in os.environ:
        raise ValueError(_retired + " was removed with neutral-cluster-constrained breakpoint fits")

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

# Zero-based ordered-hit intervals: 5 means a loss AT hit[5], before outward
# propagation to hit[6]; inward propagation reaches hit[5] before inverse loss.
# Used only with IntervalSelectionMode=Manual. Empty then means the 5D reference.
# At most one interval is supported by the persistent loss fit.
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
fit.OutputTracksTruthOverrideRTS = "BreakpointTracksTruthOverrideRTS"
fit.OutputTracksTruthOverrideBackwardFilter = "BreakpointTracksTruthOverrideBackwardFilter"
fit.OutputTracksDiffuseAugmentedRTS = "BreakpointTracksDiffuseAugmentedRTS"
fit.OutputTracksBeamSpotRTS = "BreakpointTracksBeamSpotRTS"
fit.OutputTracksBeamSpotFreeLossRTS = "BreakpointTracksBeamSpotFreeLossRTS"
fit.OutputTracksChargedEcalRTS = "BreakpointTracksChargedEcalRTS"
fit.OutputTracksChargedEcalFreeLossRTS = "BreakpointTracksChargedEcalFreeLossRTS"
fit.OutputTracksNeutralEcalRTS = "BreakpointTracksNeutralEcalRTS"
fit.OutputTracksNeutralEcalFreeLossRTS = "BreakpointTracksNeutralEcalFreeLossRTS"
fit.BreakpointIntervals = breakpoint_intervals
# WHERE: choose the breakpoint intervals shared by ordinary and truth-override fits.
# Truth (default): select ONLY the eligible layer-owned breakpoint with the
# largest summed G4 eBrem momentum loss (ties: innermost). Every fit shares it.
# Both halves of one sensor and uniquely owned support map to the same hit.
# Shared/unassigned material, missing/ambiguous hits and the last hit cannot
# select a breakpoint; their emissions/statuses remain saved in the flat tuple.
# No eligible loss means no breakpoint, NOT necessarily no physical eBrem.
# Other loss locations are not fitted;
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
# These switches skip the corresponding inward KF computation, not just output
# serialization. Disabled backward collections are empty and tuple indices are -1.
# Their flat endpoint values are NaN; RTS results remain available.
fit.OrdinaryBackwardFilter = os.environ.get("BP_ORDINARY_BACKWARD_FILTER", "0") == "1"
fit.FreeLossBackwardFilter = os.environ.get("BP_FREE_LOSS_BACKWARD_FILTER", "0") == "1"
fit.TruthOverrideBackwardFilter = os.environ.get("BP_TRUTH_BACKWARD_FILTER", "0") == "1"
# Three usable 2D hits in radius order; N//2 is the middle (upper for even N).
# FirstThree restores the original short-lever-arm prefit for comparisons.
fit.SeedHitSelection = os.environ.get("BP_SEED_HIT_SELECTION", "FirstMiddleLast")
# RTS is always produced; the three backward continuations are independently optional.
# Reject the retired environment control rather than silently ignoring it.
if "BP_BACKWARD_MODE" in os.environ:
    raise ValueError("BP_BACKWARD_MODE/BackwardMode was removed; use the three backward-filter switches")
# Compatibility property for previously generated cards; only Persistent6D
# remains supported. The backward continuation retains its local-joint fit.
fit.LossStateMode = "Persistent6D"
# Independent exact-diffuse augmented 6D KF/RTS experiment (compiled/card on).
# Off or NoReference; the diffuse output then copies ordinary RTS.
# At the one selected interval, b=log(p_before/p_after) starts with no finite
# Gaussian prior. Downstream hits identify b; SigmaLogLoss is NOT used here.
# 0: disabled ordinary-RTS copy; 1: no-interval ordinary copy;
# 2: diffuse fitted; -1: underidentified/failed input-KF copy.
# Negative fitted b is retained as a diagnostic, not silently clipped to 0.
# Its finite innovation chi2 excludes the diffuse-consuming coordinate; it is
# NOT an absolute likelihood comparable with Gaussian-prior fits.
fit.DiffuseAugmentedRTS = os.environ.get("BP_DIFFUSE_AUGMENTED_RTS", "1") == "1"
# Reconstructed hit-supported neutral collection, also used by the optional KF measurement.
fit.NeutralLossThetaWindowMrad = float(os.environ.get("BP_NEUTRAL_THETA_MRAD", "10"))
fit.NeutralLossPhiWindowMrad = float(os.environ.get("BP_NEUTRAL_PHI_MRAD", "200"))
# Per-cluster sigma_E [GeV] = a*sqrt(E [GeV]) + c*E [GeV]; independent cluster
# variances are added. These are provisional resolution assumptions.
fit.NeutralLossStochasticError = float(os.environ.get("BP_NEUTRAL_STOCHASTIC", "0.011"))
fit.NeutralLossConstantError = float(os.environ.get("BP_NEUTRAL_CONSTANT", "0.004"))
# The ordinary fit uses the MeanLogLoss/SigmaLogLoss Gaussian prior;
# the separate diffuse output above does not.
# The optional Minuit pair below optimizes the prior center with the SAME sigma;
# it remains separate from the ordinary fit and the truth-centred pair.
# Optional outer prior-center fit: normalized full-track likelihood, not chi2 alone.
# Ordinary RTS is always retained. True adds optimized results in
# the FreeLoss RTS/backward collections on one selected interval, using
# Persistent6D for every trial/refit;
# their covariances include the loss posterior variance and correlations.
# Minuit's error on the chosen prior center is NOT another SigmaLogLoss.
# False copies ordinary RTS and, only if enabled, its backward result into
# the corresponding extra outputs, without another fit.
# Empty lists copy ordinary results. Manual multiple intervals
# or failed optimization copy the INPUT CompleteTracks KF into both free outputs.
# Success requires a converged, valid Minuit minimum (not just a finite scan).
# free_loss_result_status: 0 absent, 1 ordinary copy, 2 optimized pair, 3 KF fallback.
# KF fallback has IP parameters/covariance and free_loss_kf_chi2; unavailable
# per-hit refit diagnostics remain empty/NaN. Ordinary/truth pairs stay separate.
# Maintained standalone/batch card default is ON; set BP_FREE_LOSS_FIT=0 to disable.
fit.FreeLossFit = os.environ.get("BP_FREE_LOSS_FIT", "1") == "1"
# Cumulative constraint stages for ordinary augmented KF/RTS and free-loss.
# The original rts_* and free_loss_rts_* remain unconstrained. Each ON switch
# saves TWO RTS families, accumulating enabled constraints in this output order:
# beam -> charged -> neutral. All ON: beam; beam+charged; beam+charged+neutral.
# This is output-stage order; the fitter applies measurements at their physical sites.
# Each stage starts from the original seed/prior with its accumulated measurements,
# not from an earlier fitted posterior. Each free stage has its own optimization.
# OFF: empty collection, NaN scalar endpoints, empty vectors, index -1.
# ON but unavailable/failed: preceding accepted stage copy, constraint_status 1/-1.
# constraint_mask records applied observations: beam=1, charged=2, neutral=4.
# FreeLossFit=False copies the constrained ordinary result into its free variant.
# Diffuse/truth-centered fits and original backward endpoints remain unchanged.
# Old free-loss-only environment names must not silently disable a requested measurement.
for retired, replacement in (("BP_FREE_LOSS_CHARGED_ECAL_MEASUREMENT", "BP_CHARGED_ECAL_MEASUREMENT"),
                             ("BP_FREE_LOSS_NEUTRAL_ECAL_MEASUREMENT", "BP_NEUTRAL_ECAL_MEASUREMENT")):
    if retired in os.environ:
        raise ValueError(retired + " was renamed to " + replacement)
# Experimental terminal scalar KF update: one linked charged ECAL shower
# constrains energy (not position) before RTS smoothing. Requires a selected breakpoint.
# Uses native KalTest detector transport F/Q and Filter(), as tracker hits do.
# CyberPFA may have used the input track momentum when constructing this shower.
fit.ChargedEcalMeasurement = os.environ.get("BP_CHARGED_ECAL_MEASUREMENT", "0") == "1"
# Sub-options of ChargedEcalMeasurement, shared by ordinary and free-loss RTS.
# Shared charged/neutral source: PreReclustering uses CyberPFOPreReclustering
# and its EcalClusterPreReclustering links; FinalPFO uses CyberPFOPID/EcalCluster.
# Missing pre collections are an error, never a silent switch to final PFOs.
# Final-PFO diagnostic branches/neutral_pfos tree remain final-PFO information.
fit.EcalConstraintInput = os.environ.get("BP_ECAL_CONSTRAINT_INPUT", "PreReclustering")
# Empty: use the charged shower directly linked by the selected PFO.
# Optional alternate energy collection (e.g. TrkMergedECAL): unique hit-provenance
# matching is retained. Legacy campaign: FinalPFO plus TrkMergedECAL.
fit.ChargedEcalInputClusters = os.environ.get("BP_CHARGED_ECAL_INPUT_CLUSTERS", "")
# ElectronOnly: predicted E = electron momentum at the ECAL face (mass ignored).
# ElectronPhoton: predicted E = electron momentum * exp(b), the unresolved
# electron+photon sum for ONE breakpoint and no other energy loss to ECAL.
# ElectronPhoton requires ElossOn=False and NeutralEcalMeasurement=False;
# do not observe the same photon again with the separate neutral constraint.
fit.ChargedEcalEnergyMode = os.environ.get("BP_CHARGED_ECAL_ENERGY_MODE", "ElectronOnly")
# Optional measurement in the final cumulative neutral-stage families. Uses the
# collected hit-supported ECAL energy as a scalar observation of p_after*(exp(b)-1)
# at the breakpoint, updating all six coordinates and their correlations.
# Uses the same native KalTest Filter() as charged energy; no photon-position hit.
# No usable neutral/no selected breakpoint retains earlier enabled constraints;
# a failed stage also retains the preceding accepted result and records the error.
# These six extra families are RTS-only. The measured energy is never changed;
# fitted and smoothed photon-energy estimates are recorded separately.
fit.NeutralEcalMeasurement = os.environ.get("BP_NEUTRAL_ECAL_MEASUREMENT", "0") == "1"
fit.ChargedEcalStochasticError = float(os.environ.get("BP_CHARGED_ECAL_STOCHASTIC", "0.011"))
fit.ChargedEcalConstantError = float(os.environ.get("BP_CHARGED_ECAL_CONSTANT", "0.004"))
fit.FreeLossMaxLogLoss = float(os.environ.get("BP_FREE_LOSS_MAX_LOG_LOSS", "1.0"))  # bounds prior center, not fitted b
fit.FreeLossMaxCallsPerStart = int(os.environ.get("BP_FREE_LOSS_MAX_CALLS", "180"))
fit.FreeLossTolerance = float(os.environ.get("BP_FREE_LOSS_TOLERANCE", "0.001"))
# Separate beam-only ordinary/free-loss RTS families; original endpoints unchanged.
# Beam KF measurement BEFORE the first tracker hit.
# Explicit transverse beam site (no z hit), then native material-aware transport
# to hit 0 using MSOn/ElossOn. Its F/Q are retained for RTS back to the beam.
# Publish the smoothed beam state at IP; do not traverse boundary material twice.
# Included ONCE in the likelihood; no separate beam-guided objective or extra penalty.
# Also works with no breakpoint and with FreeLossFit=False. Default OFF.
for retired in ("BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE", "BP_BEAM_FREE_LOSS_BACKWARD_FILTER"):
    if retired in os.environ:
        raise ValueError(retired + " was removed; use BP_BEAM_SPOT_CONSTRAINT")
fit.BeamSpotConstraint = os.environ.get("BP_BEAM_SPOT_CONSTRAINT", "0") == "1"
fit.BeamSpotX = float(os.environ.get("BP_BEAM_SPOT_X", "0.0"))  # mm
fit.BeamSpotY = float(os.environ.get("BP_BEAM_SPOT_Y", "0.0"))  # mm
fit.BeamSpotSigmaX = float(os.environ.get("BP_BEAM_SPOT_SIGMA_X", "0.0145"))  # mm
fit.BeamSpotSigmaY = float(os.environ.get("BP_BEAM_SPOT_SIGMA_Y", "3.6e-5"))  # mm
# HOW MUCH, for the extra pair only: True (default) sets the b PRIOR CENTERS
# to layer-owned G4 values at the SAME selected breakpoints. SigmaLogLoss is the SAME
# as in the ordinary fit; hits can still update b and its posterior variance.
# Both pairs use the same one-pass fitting code.
# It neither changes the interval selection nor corrects unselected intervals.
# Example: Manual=[5], actual eBrem only in interval 7 -> prior center at 5 is
# b=0 (z=1), but the fitted b may move. Interval 7 is not added automatically.
# Primary RTS and enabled backward use MeanLogLoss/SigmaLogLoss. FreeLossFit changes
# only the additional FreeLoss pair. The truth pair STILL uses the configured
# positive sigma and truth prior center, not Minuit or a fixed-loss oracle.
# FreeLossFit uses that same sigma but an optimized prior center instead.
# False, or an empty effective interval list: save an ordinary RTS copy and
# only the separately enabled backward copy. Truth selection still reads truth.
# The truth-prior pair changes only the loss-prior center.
fit.TruthOverride = os.environ.get("BP_TRUTH_OVERRIDE", "1") == "1"
# Repeated relinearization was retired; enabled endpoints use one pass.
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
if fit.EcalConstraintInput not in ("PreReclustering", "FinalPFO"):
    raise ValueError("EcalConstraintInput must be PreReclustering or FinalPFO")
if fit.EcalConstraintInput == "PreReclustering":
    _pre_collections = ["CyberPFOPreReclustering", "EcalClusterPreReclustering",
                        "EcalHitsPreReclustering", "HcalClusterPreReclustering",
                        "HcalHitsPreReclustering"]
    # The legacy PodioInput can dereference a missing collection instead of failing cleanly.
    import ROOT
    _input = ROOT.TFile.Open(input_file, "READ")
    if not _input or _input.IsZombie():
        raise ValueError("Cannot open breakpoint input: " + input_file)
    try:
        _events = _input.Get("events")
        _missing = [name for name in _pre_collections if not _events or not _events.GetBranch(name)]
    finally:
        _input.Close()
    if _missing:
        raise ValueError("Missing pre-reclustering inputs: " + ", ".join(_missing) +
                         "; regenerate rec with the PFA snapshot exporter or explicitly set "
                         "EcalConstraintInput=FinalPFO (BP_ECAL_CONSTRAINT_INPUT=FinalPFO)")
    reader.collections += _pre_collections
if fit.ChargedEcalMeasurement:
    _charged_source = str(fit.ChargedEcalInputClusters)
    if _charged_source and _charged_source not in reader.collections:
        reader.collections.append(_charged_source)
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
