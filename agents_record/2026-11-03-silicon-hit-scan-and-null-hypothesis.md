# Opt-in reconstructed silicon hit scan and no-breakpoint hypothesis

User authorized adding a hit-location scan over VXD/ITK, including a no-eBrem
hypothesis. Implemented on `breakpoint-delayed-neutral`; no branch/remote
operation or project-default installation change was made.

## Interface and algorithm

New `IntervalSelectionMode="HitScan"` requires `FreeLossFit=True` and an empty
`BreakpointIntervals`. Truth and Manual selection, default Truth steering and
reserved/unimplemented Auto are unchanged.

`HitLocationScan` calls the maintained `BreakpointFitter` / `FreeLossFitter`.
It implements no new KF, measurement update or RTS smoother. Candidates are
outward/radius-ordered reconstructed hits with subdetector IDs 1 (VXD), 2 (ITK
barrel), 3 (ITK endcap), excluding the final track hit. All TPC/OTK measurements
are retained, but neither detector supplies a candidate location in this version.
No truth ownership or truth eBrem threshold selects candidates.

The selector always uses tracker-only **000**, independently of the downstream
beam/charged/neutral switches. It first evaluates a separate no-breakpoint 5D
fit, then invokes free-loss optimization at every candidate with the same seed,
SigmaLogLoss, bounds and tracker measurements. NLL means the existing normalized
**-2 log L**, including log-determinant and normalization terms. Candidate and
null measurement dimensions must match. A zero prior center with a finite
SigmaLogLoss is NOT substituted for the separate null model.

`HitScanMinNllGain` / `BP_HIT_SCAN_MIN_NLL_GAIN` defaults to 0. Select a candidate
only if `null_nll2 - best_candidate_nll2 > HitScanMinNllGain`; exact null ties
prefer no breakpoint. Exact candidate ties retain the innermost candidate.
Zero is a raw minimum-NLL ranking rule, **not calibrated detection significance**.
In particular, arbitrarily small positive gains can win at zero threshold.

The selected index, or empty null list, then feeds the existing endpoint and
cumulative-constraint machinery. No earlier fitted posterior is passed in as a
new prior. TruthOverride remains a separate diagnostic, never scan steering.
With TruthOverride=False, ownership matching is not invoked by the selector;
the existing MC/PFO/photon tuple diagnostics still read simulation collections.
This does not make the entire input schema real-data-only.

Example maintained-card steering:

```python
fit.IntervalSelectionMode = "HitScan"
fit.BreakpointIntervals = []
fit.FreeLossFit = True
fit.HitScanMinNllGain = 0.0
fit.TruthOverride = False
```

The new threshold is frozen by the batch helper. Other batch defaults were not
changed. The maintained card checks `fit.getDefaultProperties()` before assigning
the new property: old installed plugins can still run Truth/Manual, but an
explicit HitScan/threshold request gets an updated-runtime error. Gaudi can
report `hasattr` false for a declared but not explicitly assigned property; that
initial guard was caught in smoke tests and replaced before completion.

## Persisted scan diagnostics

The main flat tree always books 23 `hit_scan_*` fields. Other selection modes
leave status 0 and candidate vectors empty.

- Status: 0 not run; 1 null preferred; 2 breakpoint preferred; 3 no supported
  silicon candidate; -1 null fit failed; -2 all candidate fits failed.
- Null NLL and IP omega; measurement dimensions; best candidate index/NLL;
  second-best candidate NLL; null-minus-best gain; selected index; threshold.
- Aligned candidate hit/system/layer, free-fit and Minuit status, errors,
  normalized NLL, optimized prior center, posterior b/variance and IP omega.
- Failed-candidate count: a partial scan is explicitly not an exhaustive
  minimum. All-candidate failure retains a no-breakpoint refit but is tagged
  failed, never silently labelled a clean/no-eBrem track.

`best_hit` can exist while `selected_hit=-1` because the null won. The original
`selected_breakpoint_interval` contains only the actual selected hypothesis.
Selection-level status 5 denotes a completed scan decision (including the
separate no-candidate status); -3 denotes scan failure. Earlier codes remain.

## Build and validation

RecBreakpoint module built successfully in the LCG105 build. The combined build
command's second requested test target was not exposed by the configured full
project; the new test and transport test were instead freshly configured,
compiled and passed with standalone package CMake/CTest under `/tmp`.
Existing filesystem clock-skew warnings (generated files ahead of the worker
clock), and existing external-header warnings were emitted. No clock or shared
package sources were changed to suppress them.

- 52 Python batch tests pass, including frozen scan controls and old/new
  run-card property compatibility.
- 2 freshly built standalone CTests pass: HitLocationScan and Transport.
- 13 existing numerical test executables pass (transport, ownership/matching,
  truth ECAL tuple, likelihood, diffuse, neutral/charged candidate and energy,
  beam spot, photon projection and delayed neutral). These existing executables
  were not claimed to have been freshly rebuilt by this configured build.
- Installed-before versus updated-library Truth-mode reruns on seed1 events
  **11,16,17,38** reproduce **all 926 original branches across both trees**, all
  scalar/vector values over 4 track rows, exactly with NaNs compared as equal.
  Only the 23 new scan fields are added and remain inactive in Truth mode.
- Six local scanner events pass scope/vector alignment, minimum/null decision,
  selected-index propagation, candidate failure bookkeeping, and main free-loss
  NLL/IP-curvature reproduction checks. Every candidate optimization succeeded.
- An additional event11 run with threshold 1e9 checks the null decision path:
  the candidate scores are unchanged, no breakpoint is selected, free-loss
  copies the ordinary 5D result, and its NLL/IP curvature match the scanner's
  independently evaluated null. This huge threshold is a test, not physics tuning.
- The new maintained card also ran successfully for EvtMax=0 against the old
  installed runtime in Truth mode, proving default-card compatibility.

## Focused scanner results (not population validation)

Source: `sim_large_barrel_20261026/rec-barrel-1.root`. True loss fractions below
use same-parent photon birth-energy sums / initial electron energy; not legacy
step multiplicities. Truth-selected locations below are references only, not
inputs to HitScan. Residual = 100*(pT_reco/pT_truth - 1), in percent.
All scan decisions in this table use threshold zero and unconstrained 000.

| Event | Truth loss [%] | Truth-selected hit | Candidates | Scan-selected hit | NLL gain | KF residual [%] | Scan free-loss residual [%] |
|---|---:|---:|---:|---:|---:|---:|---:|
| 11 | 0 | none | 8 | 7 | 0.378386 | -0.208432 | -0.259800 |
| 16 | 0.099465 | 8 | 9 | 6 | 1.385734 | -0.288000 | +1.494222 |
| 17 | 0.378444 | 9 (TPC, outside candidate scope) | 9 | 8 | 0.022304 | -0.182654 | -0.041211 |
| 38 | 2.666820 | 5 | 9 | 0 | 1.319679e-7 | -2.685639 | -2.695638 |
| 5 | 19.450082 | 3 | 8 | 6 | 11.617958 | -19.459462 | -16.413580 |
| 31 | 8.541022 | 6 | 9 | 6 | 97.113394 | -8.197376 | +3.298295 |

Event5 has one recorded same-parent photon; event31 has two. The no-photon
control11 selecting a weak non-null minimum, event38's near-zero gain and
mislocated hard-loss examples demonstrate that the new machinery is NOT an
established accurate locator or a production eBrem classifier. No threshold
calibration, efficiency/purity study, endcap runtime validation or claim of
99% location correctness follows. Neighboring-site ambiguity, first/last-hit
information limits, multiple losses, current linearized models and existing
optimizer acceptance behavior remain separate limitations. No optimizer,
loss-map, bound, charged/neutral treatment or acceptance repair was added.

## Artifacts and deployment

Local test area: `/tmp/bp-hit-scan-dKKpu9/`, containing wrapper cards,
`installed_truth.root`, `updated_truth.root`, `scan.root`, `scan_null.root`,
`scan_hard.root`, verbose logs, `check.py`, `check_scan.py`, `regression.json`
and `scan_validation.json`. Temporary outputs may expire; the numerical table
and interface contract above are durable. No ROOT outputs are staged/committed.

Updated package staged ONLY under `/tmp/bp-hit-scan-dKKpu9/runtime` using a
package-only CMake install with that explicit prefix. Local new-library runs
prepend its lib/python directories after sourcing the normal project setup.

Installed-default library SHA256 before and after:
`911962a6b7e9e91d533018fc699c522a7e3c3d68e50dd58dbbdea774872ec986`.
Tested updated library SHA256:
`f588c82a1ed06f1a8ca0b1a405e60a59c423ba332f8f96334b7b847208bb2911`.

**Not installed into the project-default runtime.** No Condor jobs, remote
publication, default-mode promotion or new population plots. Review/authorize
deployment before attempting HitScan through the normal installed batch runtime.
Current Truth/Manual jobs and already-frozen cards remain unchanged.
