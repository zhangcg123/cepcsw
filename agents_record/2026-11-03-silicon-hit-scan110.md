# Silicon location scan scored with free-loss 110

User requested using free-loss **110** for the NLL gain and testing more
inner-silicon eBrem events. This supersedes only the tracker-only scoring
contract in `2026-11-03-silicon-hit-scan-and-null-hypothesis.md`, not its
historical numerical results. Branch remains `breakpoint-delayed-neutral`.

## Scoring and implementation

The existing `HitLocationScan` now inherits enabled/available beam and charged
ECAL observations. With both available it compares:

```
gain = NLL2(no-breakpoint 5D, beam + charged ECAL)
     - min_over_silicon_hits NLL2(free-loss breakpoint, beam + charged ECAL)
NLL2 = existing normalized -2 log L
```

Both hypotheses use the same measured beam/cluster inputs and errors, tracker
hits and seed. The 5D null has no loss coordinate or loss prior and uses
ElectronOnly. An unresolved breakpoint candidate uses configured ElectronPhoton.
Candidate Gaussian SigmaLogLoss remains 0.001; Minuit optimizes its prior center
over the existing [0,1] bounds. There is no new optimizer, covariance treatment,
loss mapping or significance calibration. Log-determinant and normalization
terms remain included by the existing likelihood implementation.

Scope remains reconstructed VXD/ITK hits, excluding the last track hit; TPC/OTK
remain measurements. No truth location/loss steers a scan. Separate neutral
measurements are excluded from scoring. If a collected neutral routes the
charged measurement to ElectronOnly, that existing routing is respected.
Unavailable charged input leaves only the available beam input; the actual
score mask is saved. Turning both switches off reproduces the earlier 000 scan.
Existing endpoint families are still fitted after the shared location decision.

No new controller: use the existing BeamSpotConstraint/ChargedEcalMeasurement
switches. New always-booked passive fields are `hit_scan_constraint_mask`
(beam=1, charged=2) and `hit_scan_charged_energy_mode`; 25 scan fields total.
README and maintained run-card comments describe the scoring contract.
Truth/Manual selection and default Truth steering remain unchanged.

## Six hard events and a control

Inputs: `sim_large_barrel_20261026/rec-barrel-{1,2,3}.root`. Hard-event selection
used existing frozen diagnostics only to choose tests: one track row/event,
one recorded same-parent photon, photon birth energy / initial electron energy
>1%, exactly one eligible truth-selected silicon breakpoint, ordinary physical
ownership (effective_owner_rule=0), valid charged measurement and no collected
separate neutral. Three VXD and three ITK events were chosen before the scans.
Truth is used below as a reference, not by the selector. Event1:11 is a
no-recorded-photon control. This is not a random or representative population.

Hit indices below are zero-based **ordered reconstructed-hit indices**, not
detector layer numbers. Residual = 100*(pT_reco-pT_truth)/pT_truth at IP.
All scans use threshold zero, free-loss 110, ElectronPhoton for candidates,
truth override/diffuse/backward filters OFF and verbose component dumps ON.

| Seed:event | Photon loss [%] | Truth-owned hit | Scan hit | Candidates | Gain in NLL2 | KF residual [%] | Scan free-loss 110 residual [%] | Truth-location free-loss 110 residual [%] |
|---|---:|---|---|---:|---:|---:|---:|---:|
| 1:38 | 2.666820 | VXD h5 | VXD h0 | 9 | 10.205547 | -2.685639 | -0.442512 | -0.494561 |
| 1:79 | 6.988072 | VXD h5 | VXD h5 | 9 | 265.805159 | -6.944192 | +2.896996 | +2.896996 |
| 1:112 | 3.552839 | ITK h6 | ITK h9 | 10 | 17.561872 | -3.482321 | -1.861786 | -1.396546 |
| 2:45 | 4.961242 | ITK h8 | ITK h8 | 9 | 166.469036 | -4.454243 | +1.430608 | +1.430608 |
| 2:189 | 2.284023 | VXD h0 | VXD h5 | 10 | 6.111415 | -2.274262 | -0.623925 | -0.629505 |
| 3:40 | 2.226259 | ITK h7 | VXD h5 | 9 | 15.873250 | -2.229331 | +0.650142 | -1.686691 |
| 1:11 control | 0 | none | ITK h7 | 8 | 1.996840 | -0.208432 | -0.513638 | -0.401622 |

The added truth-location column uses the same free-loss 110 fit with ONLY the
breakpoint location selected by Truth mode; TruthOverride=False, so the loss
amount is still fitted, not supplied from truth. The control has no breakpoint.
Independent reruns reproduce the original scan's candidate NLL/IP omega at all
six true locations, and the control reproduces the scan's 5D null NLL. Correct
location alone does not guarantee a small IP pT residual.

Event2:45 candidates h0/h1 failed with "No converged valid Minuit minimum";
all other candidate optimizations succeeded. Its minimum is over seven valid
candidates, not an exhaustive nine-candidate minimum.

All six hard events improve absolute IP pT residual over their stored KF
baseline, but only **2/6 select the exact truth-owned hit**. Two correct-location
events overshoot IP pT; the control invents a breakpoint and worsens. Do not
claim accurate localization or clean-track preservation from this sample.

For displaced selections, NLL2 at the true hit versus selected hit is:

| Event | True-hit NLL2 | Selected-hit NLL2 |
|---|---:|---:|
| 1:38 | 47.367623 | 46.970994 |
| 1:112 | -41.022431 | -44.454899 |
| 2:189 | -108.334557 | -108.405952 |
| 3:40 | -34.270457 | -48.721044 |

The weak separation of some locations is distinct from large null-versus-loss
gain. Gain zero is a ranking rule, not a calibrated no-eBrem test.

## Checks and deployment

- RecBreakpoint rebuilt successfully; existing external-header/clock-skew
  warnings remain. Only a package install into a new /tmp prefix was performed.
- 52 Python batch tests pass; 2 freshly built standalone CTests pass.
- Every new scan row passes candidate scope/vector alignment, minimum/gain,
  selected-location and no-truth-selection checks. The final published
  charged-ECAL free-loss endpoint reproduces the scan winner's NLL2 to 1e-7
  and IP omega exactly; no optimizer result is inferred from a different endpoint.
- New-library 000 reruns of seed1 events11/38 reproduce all old main-tree values
  exactly (NaN-aware), **1,784 row/branch comparisons**, including all old scan
  scores and endpoints. The two additional fields describe inactive constraints.
- Additional event11 110 run with threshold1e9 preserves all candidate/null
  scores but selects the 5D null. Its downstream 110 NLL/IP omega reproduce
  the scan's null. Huge threshold is only a decision-path test.
- Seed1 events11/16/17/38 rerun in existing Truth mode reproduce all
  **926 original branches across both trees** exactly against the installed-
  before reference. No earlier physics endpoint changes.
- All local Gaudi jobs terminated successfully. No Condor jobs or PFA/shared-KF
  changes; no remote publication or branch operation.

Artifacts: `/tmp/bp-hit-scan110-gR1LL9/`: `run_case.py`, `null_case.py`,
`candidates.json`, `s{1,2,3}_110.root/log`, `s1_000.root/log`,
`null110.root/log`, `updated_truth.root/log`, `check_results.py`, `results.json`
and the staged `runtime/`. Added truth-location artifacts are
`truth_location.py`, `s{1,2,3}_truth_location110_rerun.root/log`,
`compare_truth_location.py`, and `truth_location_comparison.json`. The initial
wrapper changed mode after the card's input-selection stage without adding hit
associations; it produced failed rows and was corrected in /tmp before these
reruns. Those initial outputs are NOT used. No production-card change was needed.
Temporary files may expire; this table is durable.

Tested staged library SHA256:
`5f756445881ec9eb18ae4656afecb9a0c822208d34ce1713737a07c2b834fc1f`.
Project-default installed library remains unchanged:
`911962a6b7e9e91d533018fc699c522a7e3c3d68e50dd58dbbdea774872ec986`.

**Not installed into the default runtime.** No population/endcap validation,
threshold tuning, multi-breakpoint scan or acceptance repair was performed.
