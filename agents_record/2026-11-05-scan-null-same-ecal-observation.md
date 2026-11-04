# Same ECAL observation for the location-scan null and candidates

Status: implemented, directly regression-tested, and installed in the project default.
The user explicitly confirmed no active breakpoint jobs and authorized
implementation and installation after validation. No jobs were submitted,
interrupted or rewritten. The original pending reminder is retained in
`2026-11-05-pending-scan-null-ecal-observation.md`.

## Scope and mechanism

Only `LocationFitScan` null observations are corrected. This serves optional
Truth-mode LocationScanSetups and legacy scan-only HitScanAll. Legacy ranked
HitScan, normal truth-location endpoint families, non-null candidate/reference
fits, loss maps, Minuit bounds/acceptance, truth owners, defaults and schemas
are unchanged. No GSF, PFA, shared-KF, branch or remote change is authorized.

With ElectronPhoton, charged C and a collected neutral N, both null and
candidate110 use measured C+N and sigma=hypot(sigma_C,sigma_N). Neutral collection
is independent of truth-breakpoint eligibility and the scanned point. With
ElectronOnly or no collected N, both use C as before. Without a charged
measurement the prior requested-setup fallback rules remain unchanged.

The null still has no breakpoint intervals, loss coordinate, loss-prior
center/variance or Minuit step. It predicts electron energy with the existing
native 5D KF/RTS and terminal ECAL update. The candidate predicts electron or
electron+photon energy and calls the original free-loss optimizer unchanged.
No filter/smoother or measurement update is reimplemented.

The existing sum helper accepts an explicit internal allowNoBreakpoint opt-in
(default false). Only the scan null uses it to combine/validate the raw parts
once; it then clears the consumed sum request and selects ElectronOnly before
calling the normal fitter. Ordinary no-breakpoint fits do not opt in. The
returned actual summed-observation flag is retained on the scan null.
The production fitter's default behavior remains unchanged.

For a successful resolved110 null, kind=0, actual mask=3 with beam+ECAL,
resolved_ecal_sum_applied=1, prior_center/fitted_log_loss/fitted_variance=NaN,
and minuit_status=-99. It scores one scalar ECAL observation, not a separate
neutral update. 000/100 and111-only null handling are unchanged. In a scan
requesting110 then111, the inapplicable111 null can inherit the corrected110,
but is not a valid same-observation null for separate C/N111.

No configurable property, run-card or batch-helper change is needed. Old
tuples are not relabeled: their resolved null had sum flag0 and used C only.
Rerun breakpoint tuples to obtain corrected null scores; REC/sim need not be
regenerated. Existing offline minimum non-null-site plots did not compare
null/candidate NLLs and are not numerically changed by this correction.
Same observations do not require equal fitted parameter counts, but likelihood
gain still requires boundary/location-search calibration; it is not automatically
a one-degree-of-freedom chi-square or eBrem probability. No classifier/ranking
or calibrated threshold is introduced.

## Verification and runtime evidence

Evidence directory: `/tmp/bp-scan-null-ecal-HOSXG1`.
Package build passed. All15 CTests pass (final6.70s); all62 Python batch-helper
tests pass (4.947s). Sum routing tests cover both opt-in states, missing C/N,
no-breakpoint preservation and all energy/error fields with zero, negative,
infinite and NaN inputs. Existing charged projection/derivative/native scalar
update tests remain active.

Direct previous-install/build comparisons completed with verbose component
dumps on gun seed1 events5/11/16/17/38, Higgs seed1 events0/17/22 and an explicit
ElectronOnly event5. They include no eligible truth owner, no neutral, missing
charged ECAL and resolved inputs. Across13 track rows, every one of11,869
non-scan branch-row values and the event-side tree agree exactly, NaN-aware.
All348 non-null scan entries agree exactly, including the passive truth sites,
NLLs, pT/errors, IP means/covariances, prior/posterior loss, dimensions, masks,
source, failure and Minuit flags. There are382 unchanged scan entries in total
and exactly5 changed entries: resolved ElectronPhoton110 nulls. Two have no
eligible truth owner. Each corrected null retains the same measurement dimension
and actual mask3, gets sum flag1, has no Minuit/loss coordinate, and satisfies
NLL2=quadratic+logDet+measurementDimensions*log(2*pi). Missing charged ECAL
(Higgs0 track1, mask1) remains unchanged despite a collected neutral. ElectronOnly
event5 preserves all913 non-scan branches, all30 scan entries and its side tree.

Corrected null scores (event:track, job seed1; these are not published endpoints):

| Case | Eligible truth hit | Old null NLL2 | Corrected null NLL2 |
|---|---:|---:|---:|
| gun 5:0 | 3 | 108.003737193 | 1316.178959255 |
| higgs 0:2 | -1 | 9635.971391519 | 5103.462829382 |
| higgs 17:0 | 21 | 42.129117124 | 284.366948823 |
| higgs 17:1 | 209 | 312.034518893 | 6460.837782895 |
| higgs 22:0 | -1 | -32.866210483 | 3458.967422124 |

The13 rows deliberately include short/secondary-control tracks. These are
mechanical regression counts, not a single-electron optimization population,
efficiency study or physics-performance validation. Exact comparisons and
null changes are in validation.json/validation.log in the evidence directory.

Old installed library SHA256:
`565b276140c1a9cee33a4d2c46090b56e945bc5af9f4e200840be631ec77752d`.
Built library SHA256:
`25fb531ed7774c8c3f9be6607935660d088557ac335ac79fa7d0365f64c872c9`.
The old module is backed up as previous_RecBreakpoint.so in the evidence
directory. Baselines load project setup then direct gaudirun.py; new tests use
the build/run wrapper. Driver logs print the actual library path/hash, not an
inferred version. Tests write only temporary outputs.

Installed ONLY RecBreakpoint via its105 package cmake_install.cmake, including
its library, generated Python configurable and discovery metadata; no full-
project/PFA/shared-KF install. Previous configuration/discovery files are
backed up under previous-config in the evidence directory. Installed and build
library hashes are identical to the built hash above. A direct project-setup
gaudirun.py EvtMax0 probe prints the installed path/hash and initializes/books/
finalizes successfully; its two trees have zero rows. This verifies default
deployment, not another physics test. Batch still uses the project-default
setup, with no temporary library pin and no new steering/card requirement.
Existing generated cards/data are unchanged. No batch was submitted.

Build/test runs retained external-header warnings and the build-wrapper's
legacy dictionary PCM diagnostics. No shared-source workaround was added.

## Project knowledge preservation

The complete pre-edit AGENTS is preserved byte-for-byte in
`2026-11-05-agents-before-scan-null-ecal-fix.md`. No history directory moves or
deletions occur. Global status/laws/essential commands remain protected;
only the current focus is replaced after verification/deployment. The
independent optimizer-acceptance audit remains open.
