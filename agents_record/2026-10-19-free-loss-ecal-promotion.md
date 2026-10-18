# Configurable ECAL term in RecBreakpoint free-loss fitting

The local `breakpoint` source and project installation now expose
`FreeLossEcal` (compiled and run-card default false; card environment
`BP_FREE_LOSS_ECAL=1`). The batch-card generator freezes this environment
control. It requires `FreeLossFit=True` and `LossStateMode=Persistent6D`.

For a track with exactly one selected breakpoint and an accepted hit-supported
neutral cluster (`collected_neutral_ecal_status=2`), the existing scalar
free-loss prior-center search adds a Gaussian ECAL energy likelihood to its
tracker-hit objective. At each trial's first smoothed post-break state,
`E_pred = p_post * (exp(b)-1)`. The variance is the selected neutral-energy
variance plus the propagated six-dimensional post-state covariance. An
ECAL-informed starting point supplements the original scan. The KF/RTS hit
updates and separate beam-guided free-loss fit do not change. With no accepted
cluster, the base free-loss result is exactly the tracker-only result. Failed
fits retain the existing input-KF fallback. The base free-loss NLL includes
the ECAL term only when enabled and applicable.

## Same-day publication correction

The first promotion wrote ECAL-guided results into the existing `free_loss_*`
outputs, replacing tracker-only free loss. This did not meet the intended
parallel-output contract. The follow-up preserves `free_loss_*` as tracker-only
and adds `free_loss_ecal_*` flat branches and separate RTS/backward EDM
collections. When the ECAL option is off or no accepted neutral is present,
the new family copies the tracker-only result. This correction supersedes the
first paragraph's output/NLL destination statement; the ECAL likelihood is
now saved under `free_loss_ecal_likelihood_*` only.

Same-run focused gate on seed 35 event 4: base `free_loss_rts_pt` remains
30.0358993935 GeV while `free_loss_ecal_rts_pt` is 28.1217969539 GeV.
Both result statuses are 2. With the ECAL switch off, the new endpoint and
NLL copy the base exactly (status 1); with the switch on but no accepted
neutral (event 26), they also copy exactly (status 1). When the shared
`FreeLossBackwardFilter` switch is enabled, both backward endpoints and their
row-aligned indices are present: 30.0216997 versus 28.1084829 GeV, indices
0/0 for the focused event. The corrected build and installation were
revalidated after these changes.

## Maintained-card steering decision

The user subsequently chose `FreeLossEcal=True` as the maintained breakpoint
run-card default, while keeping all four backward filters off. The compiled
`FreeLossEcal` default remains false because the compiled `FreeLossFit`
default is false and the ECAL option requires it; the maintained card enables
both. If `BP_FREE_LOSS_FIT=0`, the card defaults the ECAL option to off as
well. `BP_FREE_LOSS_ECAL=0` explicitly restores the tracker-only control.
Previously generated batch cards retain their frozen steering. This changes
which experimental outputs are produced by a new run, not the evidence or
physics-validation status above.
The maintained-card default was exercised on seed 35 event 4 without an
ECAL-switch or backward-filter override: tracker-only pT 30.0358994 GeV,
ECAL-guided pT 28.121797 GeV, ECAL result status 2, all four backward flags
zero, and both free-loss backward indices -1. All 39 batch-helper tests pass.

Mechanical checks: RecBreakpoint built and installed in
`InstallArea/x86_64-el9-gcc11-opt`; installed and build plugin SHA256 hashes
agree. RecBreakpointLikelihood and RecBreakpointNeutralLossCandidate CTests
pass; 39 batch-helper tests pass. Same-source event 35:4 gives tracker-only
free-loss pT 30.0358994 GeV and ECAL-guided 28.121797 GeV (truth
28.5984802 GeV). No-accepted-neutral event 35:26 is bit-identical on/off in
pT, fitted b, and NLL. The isolated precursor tested 31 selected tracks and
12 additional selected hard-loss tracks; see `/tmp/recbp_ecal_free_4igiFK/RESULTS.md`
while that temporary directory exists. This is not held-out population
validation. In particular, two selected neutral misassociations drove pT
from about 41/48 GeV to 83/91 GeV, so the switch stays off by default.

## Outgoing focus preserved from AGENTS.md

Before this ECAL promotion, active work on local `breakpoint` was migration of
eBrem identification to the same layer-owned truth used by RecBreakpoint.
Both consumers call `TruthBHLossEventData::matchBreakpoints`; identification
retains all eligible owner groups, while the recovery fitter chooses its
largest group. Index i denotes loss at hit[i], not a direction-dependent gap.
Physical hook-to-hook material diagnostics remain separate. GSF remains
unmigrated.

Identification schema3 saves shared ownership records and `truth_label_valid`.
The conservative training gate excludes any row with an ineligible emission;
unknown ownership is not a no-eBrem label. Local full-context model readers
require schema3 and use separate `layer_owned_v1` caches/models. The >1%-of-
primary-momentum track target is any qualifying owner group. Baseline KF
features and fitting algorithms were unchanged by that migration. The batch
helper uses the rebuilt plugin, so future jobs produce schema3 without an
additional install.

Validation then covered 20 local events (22 rows), including entries 11/16/17.
Ownership matched RecBreakpoint in all 22 rows; all non-truth branches were
exactly equal to the pre-change producer and truth-disabled rerun. Extraction
retained 12 valid-label tracks and excluded 10 incomplete-ownership rows. Six
CTests and 36 batch-helper tests passed. No ML retraining or new efficiency
claim followed. Details: `2026-10-18-identification-layer-owned-truth.md`.

Pending identification work remains: regenerate identification tuples into a
new output directory, rebuild training caches, count coverage by detector and
loss, then retrain/evaluate with seed-separated data. Do not mix old
hook-labelled tuples/caches with schema3. Reviewed passive ownership remains
barrel-only; unknown/endcap structures fail closed. Contracts are in
`RecBreakpoint/docs/interval-identification-tuple.md` and
`truth-layer-ownership.md`. Earlier breakpoint evidence is in
`2026-10-18-breakpoint-layer-owned-truth.md` and
`2026-10-18-AGENTS-before-identification-ownership.md`.
