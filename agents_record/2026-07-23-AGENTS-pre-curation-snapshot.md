# CEPCSW GSF Development

> Complete pre-curation snapshot captured on 2026-07-23; the annotation on this
> line is the only addition to the former live file.

## 1. Introduction and global status

This project develops an electron Gaussian Sum Filter (GSF) refit for CEPCSW.
Its goal is to model tracker-material bremsstrahlung and recover the electron
state at the interaction point more accurately than the standard
`CompleteTracks` result.

The intended physics chain is:

```text
Geant4 pre/post-material-step truth
  -> CEPC step-t/X0-conditioned Bethe-Heitler mixture
  -> multi-component filtering and smoothing
  -> validated interaction-point track parameters
```

Global status:

- `RecGsfTracking` builds, installs, reads `CompleteTracks`, and writes
  `GSFTracks`.
- The component measurement-update workflow is operational. It uses the
  baseline-compatible MarlinTrk `initialise -> addAndFit` path; focused
  single- and multi-component tests no longer reproduce the former recovery
  and catastrophic smoothing failures.
- The exact MarlinTrk prediction and innovation quantities are now exposed to
  `RecGsfTracking`, and component posterior weights use the full Gaussian
  innovation likelihood including `det(S)^(-1/2)` with stable log-space
  normalization. Focused events 11, 16, and 17 run successfully with these
  diagnostics, but this statistical correction has not recovered truth
  momentum.
- The exact accepted inter-surface transport Jacobian is also exposed through
  the user-authorized `MeasurementUpdate` extension. The optional
  `GaussianSumSmoothing` workflow is KL reduction-aware: it records the
  forward reduction graph, conditions pre-merge contributors backward through
  each KL node, applies exact RTS transport links, and moment-reduces the
  common-surface backward mixture. Focused event 11 completes 234/234 hits but
  its weighted-mixture IP pT remains at LCIO; events 16/17 and population
  physics validation remain pending. The separate reverse multi-component
  filter is preserved unchanged.
- The forward surface workflow now preserves filtered measurement history in
  each branch and applies process convolution only to a separate continuation
  state. Low-weight cutoff and KL reduction operate on common-surface states;
  focused events 11, 16, and 17 retain all 234 hits with finite KL distances
  and no measurement-update rejection.
- An opt-in reverse multi-component filtering workflow now traverses the
  audited focused hit sequence inward, reuses the exact posterior likelihood,
  applies direction-reversed process convolution and current-surface
  reduction, and can publish either the default highest-weight reverse branch
  or a moment-matched reverse mixture at the IP. Events
  11, 16, and 17 retain 234/234 hits with zero reverse rejection and IP pT of
  1.9785, 1.9970, and 2.2591 GeV, each closer to truth than LCIO.
  This workflow is a second reverse refit, not a Gaussian-sum smoother, and its
  starting state reuses forward measurement information. The reverse seed is
  now always the complete final forward mixture, with every full 5x5 component
  covariance scaled by `ReverseKappaSeedCov` (legacy name, default 100). The
  former non-default `BestBroad` and `IdentityBroad` single-component seed
  modes and `ReverseSeedMode` property have been removed.
- A five-event 2 GeV muon control shows that reverse filtering without the
  electron hypothesis leaves pT essentially unchanged from LCIO. Forcing the
  same electron BH and split/reduce strategy onto the muons also does not raise
  every pT. The workflow is therefore not a universal momentum inflator.
- True Geant4 pre/post-step data is the authoritative energy-loss truth.
  SimTrackerHit momentum is only a detector-level cross-check.
- The electron loss tail is physically established. At 1 GeV and theta 85
  degrees, the 200-electron/200-muon G4-step comparison measured mean event
  losses of 0.007116 and 0.000898 GeV with comparable material budgets.
- None of the available Bethe-Heitler models is validated for CEPC tracker
  steps.
- The selectable BH implementations are now only `ActsAtlas` and
  `CEPC2GeV85StepConditioned`; the former `Current` and `GlobalSim2GeV85`
  implementations and aliases have been removed.
- The optional `ActsAtlas` reference now matches the current official ACTS
  default at commit `900c9e5e`: the exact thresholds and analytic thin-step
  Gaussian plus one transformed six-component polynomial table through the
  `0.2 X0` cap. The former incompatible local low/high tables were removed.
  This remains a comparison model, not CEPC validation.
- Geant4 checks support provisional fractional-loss transfer of
  `CEPC2GeV85StepConditioned` across the tested 2--10 GeV pT range and from
  theta 85 to 20 degrees in unchanged geometry. The three samples have global
  fractional eBrem loss per accumulated X0 of 0.958, 0.973, and 0.987, with
  overlapping bootstrap intervals; positive-eBrem loss shapes are compatible.
  The 20-degree 10 GeV-pT sample has about 29.32 GeV total momentum. This is
  bounded truth-level compatibility, not low-energy, all-angle, independent,
  or complete-GSF validation.
- The final 10 GeV-pT, 20-degree reconstruction has 990 valid events from
  99/100 tuples. LCIO versus reverse GSF improves median pT residual from
  -1.063% to -0.293%, central-68 half-width from 8.713% to 3.897%, and events
  inside +/-2% from 592 to 702, but worsens RMS from 21.08% to 27.18% through
  new extreme tails. Shallow-angle component/history memory growth remains an
  operational issue; physics selection must be stabilized before pruning for
  memory.
- At 10 GeV and theta 85 degrees, inclusive electron LCIO versus reverse GSF
  improves central-68 pT half-width from 2.401% to 0.418% and the population
  inside +/-2% from 788/1000 to 857/1000, but GSF has worse full RMS from new
  tails. Muon LCIO has 0.132% central-68 half-width; forcing the electron BH
  GSF onto muons slightly broadens it to 0.148% and creates large outliers.
  This control favors a minority reverse radiative-selection problem over a
  gross 2-to-10-GeV BH scaling failure; the muon GSF configuration is an
  unphysical electron-hypothesis stress test.
- Categorized exact-pair tests now demonstrate interaction-point momentum
  recovery for many hard-bremsstrahlung events, but a substantial unrecovered
  tail and clean-track degradation remain. This is still a research
  implementation, not a validated production reconstruction algorithm.
- Broad GSF-versus-LCIO performance claims and mainline integration must wait
  for clean-track preservation, reproducible tail control, and independent
  held-out validation.

### Project laws and work scope

These constraints are active and mandatory:

- Keep implementation changes inside `Reconstruction/RecGsfTracking` unless
  the user explicitly authorizes a broader scope for a concrete reason.
- Do not modify KalTest, TrackSystemSvc, MarlinTrk, DDKalTest, or other shared
  CEPCSW packages to compensate for a GSF-specific workflow or state-management
  problem unless the user explicitly authorizes a narrow shared change for a
  demonstrated interface requirement.
- Do not hand-code a parallel Kalman measurement update when the baseline
  MarlinTrk interface can provide the required operation.
- Treat Geant4 pre/post-step records as the material-energy-loss truth. Do not
  present SimTrackerHit momentum as an exact material transition.
- Do not claim the Bethe-Heitler model or GSF physics performance is validated
  from successful execution, finite output, or improved chi-square alone.
  Validation requires demonstrated interaction-point momentum recovery against
  generator truth in categorized hard-loss events.
- Preserve unrelated working-tree changes. Stage files explicitly and keep
  source/documentation changes separate from generated ROOT files, logs,
  plots, tables, notebooks, and batch cards.
- Work only on the `optimizing` branch. Do not switch, create, rename, or
  delete local or remote branches unless the user explicitly revokes or
  replaces this restriction.
- Do not perform Git operations while carrying out the optimization work
  unless the user explicitly requests a specific Git action. Maintain
  `AGENTS.md` and dated `agents_record/` records so progress and rationale
  survive long autonomous runs without relying on commits.
- Keep `AGENTS.md` current and concise. Before replacing a focus or removing
  detail, preserve every unique item in a dated `agents_record/` entry. Do not
  lose information during status migration.
- Validate every GSF implementation step with comprehensive verbose component
  dumps on a focused event before proceeding. Once stable, repeat the check on
  hard-loss events 11, 16, and 17. Build success, finite output, or lower
  chi-square is not a sufficient gate.
- Load historical records only when regression evidence, design rationale,
  experiment comparison, or explicit provenance is needed. Historical detail
  must not override the current focus merely because it is more extensive.

### Compile and run

Run commands from the repository root. For a complete configured build and
install:

```bash
source setup.sh
./build.sh
```

For the normal focused GSF development cycle on the configured EL9/LCG 105
build:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecGsfTracking -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt
```

Run a Gaudi option file through that build environment:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py path/to/options.py
```

Use a small `SelectedEventIndices` list for component diagnostics. Generated
ROOT files and logs are outputs, not project-status records.

Historical evidence, resolved incidents, prior experiments, runbooks, and the
complete pre-curation project guide are preserved under `agents_record/`.
Load this file first. Load historical records only for regression analysis,
design rationale, experiment comparison, or an explicit provenance request.
When the project focus changes, preserve the outgoing focus in a dated
`agents_record/` entry and replace—not append to—the current-focus section.
Do not lose unique information during that migration.

## 2. Current focus

The active concentration is now improving the already favorable light-eBrem
performance by diagnosing its remaining tail state by state. The expanded
499-seed matched sample contains 2045 no-eBrem, 2148 light-eBrem, and 797
hard-eBrem events under reconstruction-aligned Geant4 surface ownership; seed
464 is excluded because its flat tuple contains no `gsf_tuple`. In the light
category, LCIO versus GSF has median residual -0.2131% versus -0.0971%, 1550
versus 1778 events inside 1%, and 1973 versus 2064 inside 5%. The global gain is
real, but 37 GSF light events remain beyond 10% and a small set is much worse
than LCIO.

The current-24 state audit of the structural positive-residual tail is
complete. All 30 durable topology-clean IDs were rerun with comprehensive
component dumps. Only 21 still have current GSF amplification of at least
+0.25 percentage point; nine low-amplification cases drift to identity-like
outputs and remain in the table as controls rather than being replaced. The
set contains 20 no-eBrem and 10 light-eBrem events; its durable ID table is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_candidates.csv`.
Persistent amplification selects discrete radiative modes concentrated at
surfaces 5--8. Exact decisive-state odds remain heterogeneous—some events are
likelihood-driven and others inherit overwhelming reverse-seed odds—while KL
does not create the locally auditable crossings. This agrees with the existing
19-overshoot/18-control mechanism. Exact drift, per-event states, odds,
signatures, definitions, and resume instructions are in
`agents_record/2026-07-14-positive-lcio-amplification-current24-state-audit.md`.
The direct comprehensive-dump `MaxComponents=12` rerun is rejected: versus 24
components it improves 10 events, worsens 16, and leaves four unchanged, while
raising the number above +0.25-point amplification from 21 to 26. In the 13
low-amplification controls, only one improves and nine worsen because lower
capacity reactivates radiative selections that 24 leaves on identity. Keep 24
as the default; exact eventwise changes and provenance are in
`agents_record/2026-07-14-positive-lcio-amplification-maxcomponents12-comparison.md`.
State traces show why: KL-12 merges nearby radiative fragments and sums their
weights while identity stays separate, so highest-component publication can
flip even when the identity state and likelihood evidence barely change.
Identity protection remains configurable through `ProtectIdentityLineage`
(default `true`) and `GSF_PROTECT_IDENTITY_LINEAGE` for cutoff and KL merging.
The rejected historical rank-pruning comparison and its former protection
semantics are preserved only in
`agents_record/2026-07-14-positive-lcio-amplification-topn12-rejection.md` and
`agents_record/2026-07-14-configurable-identity-protection.md`; that reducer
has now been removed from the implementation.
The ineffective experimental `ReductionMinHitsAfterSplit` control and its
component-age bookkeeping have been removed. Forward and reverse workflows now
keep every BH child through the target measurement update and exact innovation
likelihood, then apply posterior cutoff and KL reduction. Comprehensive
24-component checks are mechanically healthy with peak 120 components and no
rejection: 404/8 retains strong recovery at 1.9999 GeV, while 284/1 publishes
2.0275 GeV and clean-like 62/9 remains badly overcorrected at 2.0678 GeV.
This ordering is implemented but not physics validated; do not proceed to a
broad claim before the existing overshoot/control and hard-loss ladder. Exact
source semantics, counts, logs, and missing seed-1 limitation are in
`agents_record/2026-07-14-posterior-mixture-reduction-implementation.md`.

The optional smoother is now a single KL reduction-aware Gaussian-sum
workflow. The old rank-pruning reducer, its property branch, and the temporary
retained-lineage smoother have been removed. Event 11 builds a 2,966-node
forward/reduction graph, solves 50 active KL nodes backward, retains 234/234
hits, and stays at LCIO pT. A five-event extension is mechanically complete:
event 3 partially recovers from 1.7980 to 1.8725 GeV against 2.0004 GeV truth,
while events 5, 7, 9, and 11 remain essentially LCIO-like. Thus the backward
graph is nontrivial but heterogeneous and not performance validated. The
independent reverse-filter regression remains operational at 1.9838 GeV.
In six additional paired smoother/reverse runs, all tracks were complete but
neither method improved selected-sample mean absolute truth error; the reverse
filter worsened truth-like event 10 from LCIO 2.0006 to 1.9707 GeV, whereas the
smoother preserved it at 1.9999 GeV. This reinforces clean-track false-
correction risk and is not a population comparison.
On the durable 50-event MaxComponents=12 transition-7--8 reverse-overshoot
sample, the KL smoother completes 50/50 but is essentially LCIO-like: mean
absolute residual 2.5258% versus 2.5189% LCIO and 1.0458% recorded reverse,
with 15/50 versus 15/50 and 28/50 inside 1%. Only one smoother event moves more
than 1 MeV from LCIO. Thus it removes the selected positive overshoots by
forfeiting reverse hard-loss recovery, not by converging toward truth. Exact
eventwise output and provenance are in
`agents_record/2026-07-15-kl-smoother-maxcomp12-overshoot50-comparison.md`.
Events 16/17 were unavailable locally and remain required before broader use.
Implementation and validation are in
`agents_record/2026-07-15-kl-reduction-aware-gsf-smoother.md`.

The source-level smoother comparison is complete at pinned CMSSW commit
`d58a3bbc` and ACTS commit `40c9a031`. CMSSW runs a backward multi-component
GSF, but its published per-hit smoothed state is a single-Gaussian information
combination of the moment-collapsed forward-updated and backward-predicted
mixtures; it does not form a retained all-pairs component-product mixture.
ACTS seeds a second reverse multi-component filter from the inflated final
forward posterior, repeats measurement updates, and publishes a configurable
mean or maximum-weight collapse without combining stored forward states; it
explicitly performs no dedicated component smoothing. Thus ACTS is closer to
the CEPC reverse filter, while neither package directly implements the current
CEPC retained-graph RTS smoother. Do not change `RecGsfTracking` merely for
package pedigree. Exact line-referenced algorithms, material/reduction order,
publication semantics, corrections to the preliminary interpretation, and
bounded optional-control designs are in
`agents_record/2026-07-15-cmssw-acts-gsf-source-validation.md`.
Following that audit, the reverse seed was simplified to the full forward
mixture only and ACTS-style x100 covariance inflation was made the default.
A comprehensive same-code event-11 A/B gives 1.98304 GeV at scale 1 versus
1.98302 GeV at scale 100; both retain 234/234 hits with zero reverse rejection,
but the selected lineage, final component count, weight, and chi2 change. The
changed measurement leverage is not physics validated. Exact removed
semantics, source/configuration scope, validation, and the required comparison
ladder are in
`agents_record/2026-07-15-reverse-seed-mode-removal-and-covariance-scaling.md`.

A configurable, default-off `CmsGsfSmoothing` workflow now implements the
pinned CMSSW endpoint semantics: it seeds from the outermost forward
prediction, scales the full covariance by `CmsErrorRescaling` (default 100),
applies the outermost hit backward, records collapsed forward/backward
information combinations at interior surfaces, and publishes the collapsed
innermost backward-filtered mixture at the IP. It is mutually exclusive with
the other backward workflows. Comprehensive event 11 completes 234/234 hits
and gives pT 1.9895 GeV versus 2.0004 GeV truth, 1.7938 GeV LCIO, and about
1.9830 GeV for the independent reverse filter. This is mechanical validation
only; events 16/17 and populations remain required. Exact semantics,
configuration, outputs, and caveats are in
`agents_record/2026-07-15-cmssw-like-gsf-smoother-implementation.md`.
The unrelated default-off `NativeTrackSmoothing` diagnostic has been removed
entirely: it only called KalTest `SmoothAll()` independently on final branches
and was not a Gaussian-sum smoother. Historical provenance is preserved in
`agents_record/2026-07-15-native-track-smoothing-removal.md`.
A same-code five-event comparison at indices 3, 5, 7, 9, and 11 gives mean
absolute pT residuals of 4.8948% LCIO, 0.4064% independent reverse, 4.8968%
KL smoother, and 0.4604% CMSSW-like. Both reverse refits recover the selected
hard losses while the KL smoother remains LCIO-like; CMSSW-like wins two
events and ordinary reverse wins three, so the sample establishes no ranking.
Exact eventwise values, configurations, outputs, and a selected-entry tuple
caveat are in
`agents_record/2026-07-15-three-workflow-five-event-comparison.md`.
A 20-entry extension is mechanically complete. Three entries are short or
mismatched (36, 67, or 115 hits) and dominate inclusive RMS. On the remaining
17 long tracks, mean absolute pT residual is 1.5022% LCIO, 0.5045% reverse,
1.4986% KL, and 0.4690% CMSSW-like; CMSSW-like beats reverse on 7/17 and loses
on 10/17. Exact metrics, topology caveat, and outputs are in
`agents_record/2026-07-15-three-workflow-20-event-extension.md`.
The same-code 19-overshoot rerun compares only ordinary reverse and CMSSW-like,
per the user's instruction to omit KL from subsequent routine tests. LCIO,
reverse, and CMSSW-like mean absolute residuals are 4.0741%, 2.0871%, and
1.8199%; CMSSW-like wins 9/19 eventwise and reverse wins 10/19. CMSSW-like is
not a uniform overshoot damping—it lowers pT in eight events and raises it in
11—and its aggregate advantage includes partial recovery of current
identity-like 469/6. Exact eventwise results and outputs are in
`agents_record/2026-07-15-overshoot19-reverse-vs-cms.md`.
A fresh same-code rerun after restoring the 12-component default completes all
19 events in both workflows with matched hit counts. LCIO, reverse, and
CMSSW-like mean absolute residuals are 4.0741%, 1.8978%, and 1.8563%; RMS is
4.6760%, 2.2620%, and 2.0034%. Reverse wins 11/19 eventwise and CMSSW-like
wins 8/19. Thus the two current methods are close in mean absolute error;
reverse retains the eventwise majority and narrower central-68 half-width,
while CMSSW-like retains the lower RMS and three events inside 1%. Exact fresh
truth/LCIO/method pT values, configurations, outputs, and the 24-component
comparison are in
`agents_record/2026-07-16-overshoot19-current-reverse12-vs-cms12.md`.
The full new CMSSW-like MaxComponents=12 production has 499 usable files and 4,990 matched
events; seed 464 is the sole known broken tuple. In topology-clean light events
CMS-like improves width68 from 1.2575% LCIO to 0.4609% and inside-1% from
1549/2132 to 1776/2132; in hard events it shifts the median from -13.381% to
-0.6719%. It degrades the no-eBrem width68 from 0.1321% to 0.1940%, loses 81
clean events inside 1%, and creates extreme clean tails that raise clean RMS
from 2.911% to 22.325%. Plots, exact tables, audit, and caveats are in
`agents_record/2026-07-15-cms-like-maxcomp12-full-4990-pt-resolution.md`.
A direct 4,990-event pairing of ordinary reverse and CMSSW-like at
MaxComponents=12 finds a core/tail tradeoff. Reverse has narrower inclusive
width68 (0.4670% versus 0.5060%) and 18 more events inside 1%; CMSSW-like has
lower RMS (19.937% versus 20.801%) and wins absolute error 2539/4990 versus
2450/4990 with one tie. Reverse strongly wins no-eBrem eventwise, while
CMSSW-like wins most light/hard events and lowers their full-tail RMS. Exact
category tables and plots are in
`agents_record/2026-07-15-reverse-vs-cms-like-maxcomp12-full-comparison.md`.
The new MaxComponents=12 productions confirm that tradeoff on
the same 4,990 exact pairs. Inclusive reverse/CMS-like width68 is
0.4734%/0.5060%, RMS is 20.169%/19.937%, and inside-1% counts are 4059/4037;
CMS-like wins absolute error 2512/4990 versus 2475 for reverse with three ties.
On 4,858 topology-clean events the eventwise comparison is effectively tied,
but reverse retains the narrower clean core while CMS-like lowers light/hard
RMS. Fixed `[-2%,2%]` Gaussian core fits give sigma 0.1566% reverse and
0.2040% CMS-like, but chi2/ndf near 10 rejects a literal single-Gaussian
description, so quantile and tail metrics remain authoritative. Exact category
tables, fit definitions, plots, and audit are in
`agents_record/2026-07-15-reverse-vs-cms-like-new-maxcomp12-full-comparison.md`.
The CMSSW-like MaxComponents=12 light-eBrem transition categorization confirms
the same information boundary: only modest changes at 0--4, strong central
recovery at 5--11, and slight core degradation above 11 where LCIO is already
mostly truth-like. At 7--8 width68 improves from 1.7786% to 0.4633% and inside
1% from 306/522 to 451/522, but RMS worsens from 3.435% to 5.266% through new
tails. Exact six-bin results and plots are in
`agents_record/2026-07-15-cms-like-maxcomp12-transition-location.md`.

A direct survey of the existing raw tracker SimHit collections identifies 133
of 5,000 events with non-primary tracker activity; 20 have at least 20
secondary tracker hits and include conspicuous conversion/curling topologies.
Only 132 have current GSF tuples because seed 464/event 2 is missing with the
rest of seed 464. Exclude the full secondary-tracker-activity ID set from
single-track optimization counts and representative selection, but always show
its GSF resolution separately as a topology/control population. The stable ID
table, exact definition, resolution plot, and counts are preserved in
`agents_record/2026-07-13-secondary-tracker-topology-separation.md`.
After exclusion, the active single-track populations are 2032 no-eBrem, 2132
light-eBrem, and 694 hard-eBrem events; updated category plots show that the
central light/hard improvements remain, while separate GSF-only extreme tails
still make full RMS unstable.

A stratified survey of all 2148 light events shows that the gain is concentrated
above 1% owned loss. In the 0--1%, 1--3%, 3--5%, 5--7%, and 7--10% loss bins,
GSF puts 1302/1401, 219/349, 119/178, 64/108, and 74/112 events inside 1%,
versus LCIO's 1357, 112, 36, 20, and 25. Fifteen verbose representatives show
that apparent no-recovery cases must be split: some already have truth-like
LCIO/identity states and should not be corrected, while genuine missed
recoveries remain on identity with biased pT. Good, partial, and overshooting
recoveries generally diverge at hits 7--3 according to the selected loss child;
469/6 is a useful boundary case that changes only at hit 0.

Fresh exact decisive-hit odds distinguish several mechanisms. Missed 2/7 is
prior-limited at 1,020:1 before hit 0. Good 234/4 receives moderate prior and
likelihood support. Partial 309/6 receives a 13.6 likelihood preference for an
under-correcting branch. Overshoot 299/7 is measurement-dominated with a
62,727 likelihood ratio despite starting behind in prior odds. Low-loss false
correction 463/7 is a near-boundary hit-1 flip. KL aggregation is not the
primary cause in these comparisons. Exact values and the reproducible survey
are in
`agents_record/2026-07-13-topology-clean-resurvey-and-first-fresh-odds.md`.
Second representatives confirm the split: missed 166/6 is even more
prior-limited, partial 377/3 and overshoot 26/9 are inner-hit likelihood flips,
and low-loss 340/5 would still choose the false branch without merged weight.
An opt-in dominant-unmerged-lineage final selection was therefore tested and
rejected on all 2132 topology-clean light events: the central-68 half-width
worsens from 0.4357% to 0.4743%, the 1% population falls from 1769 to 1761, and
104 events worsen versus 59 improve. The default remains aggregate weight.

The fresh population audit now identifies dominant loss surface as the main
organizing variable. Losses at transitions 0--4 are commonly missed, losses at
7--11 are frequently recovered, and very late losses are mostly attached to
already truth-like LCIO states. An independent extraction of the existing
ten-file model source and the expanded 499-seed sample finds statistically
compatible radiative probabilities in every t/X0 bin: the largest total-tail
difference is 0.87 sigma. The largest of 40 component-weight shifts is 2.09
sigma and is based on only 70 versus 22 tail entries. Global process-core
reweighting is therefore rejected; exact counts, representative truth surfaces,
and the reproducible comparison are preserved in
`agents_record/2026-07-13-topology-clean-surface-and-process-core-audit.md`.
The first closely loss-matched trace confirms the interpretation: good 369/1
amplifies truth-compatible weight from 0.09% to 38.5% at the first decisive
inner hit and finishes at 81.5%, while missed 433/6 retains a compatible branch
but never raises it above 0.1% and finishes near 0.0003%. Thus this missed case
is not caused by KL deletion or absent process support; its transition-0 loss
has inadequate inward curvature leverage.

Layer dependency is now an explicit optimization boundary. In recovery-
eligible events, the current default reverse GSF recovers 2/30 at transitions
0--2, 5/60 at 3--4, 93/163 at 5--6, 193/213 at 7--8, and 31/34 at 9--11. For
matched 3--5% losses the progression is 0%, 17.6%, 78.3%, 98.2%, and 100%.
LCIO/GSF resolution panels confirm negligible change at 0--4, a boundary at
5--6, and strong core recovery at 7--11. Treat 0--4 as predominantly
information-limited, concentrate default optimization on remaining failures at
5--11, and preserve the full tables, plot provenance, smoother controls,
interpretation, and resume point in
`agents_record/2026-07-13-layer-dependency-current-default-status.md`.

The first complete population audit of transition-5--11 overshoots covers all
19 topology-clean light overshoots and 18 same-surface, loss-matched controls,
not a single representative. It finds a heterogeneous mix of prior- and
likelihood-supported wrong decisions, while KL reduction is not a recurring
cause. The common signal is discrete process-mode mismatch: 18/19 overshoots
select one radiative child, and the selected g2/g3 modes systematically
overcorrect relative to truth more than in matched controls. For g3 overshoots,
the median modeled loss is 10.54% against 5.53% truth; for g2 it is 2.43%
against 2.15%. The largest pT gain exceeds truth loss by a median 1.15
percentage points in overshoots versus 0.07 in controls. Preserve the original
g3 support because the previous hard replacement split failed. Full tables,
selected lineage signatures, and provenance are in
`agents_record/2026-07-13-transition-5-11-overshoot-population-audit.md`.

The rejected experimental g3-splitting control has been removed from active
source, configuration, steering, and survey tooling. Its three tested settings
each improved 8/19 overshoots and worsened 11/19. A stronger surface pattern is
now established: 15/19 overshoots select radiation one hit inward, while
controls more often select the truth surface. Truth-surface competitors survive
in 15/19 overshoots but are within 10:1 in only 5/19, versus 10/18 controls.
Representative forward/reverse consistency is only 7/10 truth-correct for
overshoots, so do not impose a hard surface rule. Removal provenance is in
`agents_record/2026-07-14-intermediate-support-removal.md`.

The default-off quantitative surface-lineage diagnostic is now implemented
without changing KL reduction or selection. It propagates aggregate BH-mode
mass by surface through KL merges. In selected components, overshoots have
median radiative mass 0.121 at the truth surface and 0.881 one hit inward,
versus 0.478 and 0.270 for controls; 16/19 overshoots favor inward mass, while
11/18 controls favor truth. For 299/7 the representative hit-8 lineage hides
32.6% aggregate radiative mass at truth hit 9. A normalized cosine score is
rejected because it over-amplifies tiny forward mass and badly under-corrects
469/6. An unnormalized overlap scan is promising only at modest strength and
has no calibrated probabilistic interpretation, so selection remains
aggregate weight. Exact implementation, validation, primary-track extraction
correction, tables, and resume point are in
`agents_record/2026-07-13-kl-surface-lineage-mass-diagnostic.md`.

A bounded noisy-OR surface-coincidence likelihood with a fixed 0.05
uninformative floor was subsequently implemented and rejected. It exactly
produced the intended 74/0 and 310/8 improvements with no matched-control
change, but same-code direct A/B tests found 2 improvements versus 7
worsenings among 9 changed light events and 0 improvements versus 4 worsenings
among 5 changed clean events. Clean 358/8 worsened from -0.4505% to -1.3243%.
Stored outcome tuples also show current/stored drift for existing pathologies
such as 116/5, so final-selection claims must use same-code A/B reruns. Default
selection remains aggregate weight; do not tune the rejected noisy-OR floor.
Formula, validation, exact tables, and resume point are in
`agents_record/2026-07-13-bounded-surface-consistency-rejection.md`.

The user has explicitly returned `MaxComponents=12` as the active default.
The C++ property, documentation, standard electron steering, reverse-template
fallback, and focused helper agree on 12. Earlier evidence favored 24 over 12
for the positive-LCIO-amplification sample, so this is a user-selected baseline
change rather than a new physics-validation conclusion. Preserve 24 as an
explicit comparison setting where needed; the superseded 24-default adoption
and validation are recorded in
`agents_record/2026-07-16-maxcomponents-12-default-restoration.md`.

A default-off weighted `Runnalls` pair-ranking cost is available for the
unchanged moment reducer; `SymmetricKL` remains default. A comprehensive 284/1
dump was mechanically clean. On the fixed 100-event topology-clean light
sample, Runnalls narrowed width68 from 0.4192% to 0.3760% but slightly worsened
mean absolute residual (0.6125% to 0.6194%), RMS, inside-1% count, and
eventwise truth error (13 wins, 18 losses, 69 ties). Do not promote or tune it
without the overshoot/control and clean/hard population ladder. Exact formula,
configuration, eventwise outputs, and caveats are in
`agents_record/2026-07-16-runnalls-reduction-100-event-test.md`.

A new default-off `CEPC2GeV85StepConditioned6` BH model is selectable. It is
extracted from the same 2,573,914 accepted Geant4 transitions and keeps the
current no-eBrem, 0--1%, and >20% components while replacing the former 1--5%
and 5--20% g2/g3 pair with truth-stratified 1--5%, 5--10%, and 10--20%
components. The total replaced probability is conserved at every t/X0 knot;
the current five-component model remains the default. The package builds and
installs. Comprehensive event 284/1 completes with 232/232 hits, zero reverse
rejection, and pT 2.0247 GeV versus 2.0275 GeV for the current model. Legacy
hard event 11 completes at 1.9794 GeV versus 1.9830 GeV for the current model;
both have the same 15 reverse-update rejections, so the new model introduces
no rejection regression there. Events 16/17 were not present in the available
`/tmp/gsf-match-tracks.root` input and remain required. This is model
construction and machinery validation only, not evidence of improvement.
Exact provenance is in
`agents_record/2026-07-14-conditioned6-g2-g3-replacement-model.md`.
The complete same-code MaxComponents=12 A/B on the user's 50 transition-7--8
overshoots rejects the six-component model as a replacement. It improves
28/50 and worsens 21/50, but mean absolute residual worsens from 1.0458% to
1.0638%, RMS worsens from 1.1348% to 1.1976%, the count inside 1% stays 28,
and total reverse rejections rise from 261 to 265. New tails at 345/7 and
102/4 reach +2.6146% and +2.9271%. Transition-8 and >=5%-loss subsets improve,
but transition-7 and the dominant 1--5%-loss subset worsen. Keep the model
default-off; exact eventwise results are in the linked implementation record.

The full `CEPC2GeV85StepConditioned6` production is now audited and rejects the
six-component model as the default replacement. There are 499 usable files and
4,990 exact matched events; seed 464 retains its known missing-tree limitation.
In 2,132 topology-clean light events, old/new MaxComponents=12 gives width68
0.4366%/0.4283%, 1,769/1,769 inside 1%, RMS 5.0929%/5.2747%, and 32/30 beyond
10%. However, no-eBrem loses two events inside 1%, hard-eBrem loses four, and
hard median shifts from -0.6633% to -0.7287%. Across all topology-clean events
505 improve, 505 worsen, and 3,848 are unchanged. Transition 7--8 loses two
events inside 1% and worsens RMS from 5.1883% to 5.9806%. New extreme failures
include 483/2, 144/2, 352/4, and 403/3, even though other old tails improve.
Keep the five-component model active and the six-component model default-off.
Exact tables, plots, and interpretation are in
`agents_record/2026-07-15-conditioned6-full-4990-event-comparison.md`.

A fresh 500-seed MaxComponents=24 flat-tuple set has 14 unusable seed files,
leaving 4,860 matched events. In the topology-clean surviving subset, GSF
versus LCIO improves the light-eBrem central-68 half-width from 1.2504% to
0.4517% and the hard-eBrem median from -13.332% to -0.665%, while slightly
broadening the no-eBrem central core from 0.1326% to 0.1388% and retaining
extreme GSF tails. Exact broken seeds, subset denominators, plots, and tables
are in `agents_record/2026-07-14-new-maxcomp24-tuple-pt-resolution-comparison.md`.

The corresponding fresh MaxComponents=12 set has 499 usable files. A direct
4,860-event match to the surviving 24 set gives nearly identical central
performance: inclusive width68 is 0.4610% for 12 versus 0.4626% for 24, while
12 has worse full RMS, a slightly broader no-eBrem core, and worse light/hard
RMS despite tiny central-count gains. This tradeoff does not reverse the
selected default of 24. Exact audits, category results, transition plots, and
matched tables are in
`agents_record/2026-07-14-new-maxcomp12-tuples-and-matched-12-vs-24-comparison.md`.

MaxComp=12 is now acceptable as the working capacity for broad overshoot
mechanism screening, while 24 remains the active default and validation
capacity. Nine stratified transition-7--8 overshoots show wrong discrete
surface/mode selection directly: seven select radiation inward of the dominant
truth transition, and the strong-loss cases inherit overwhelming radiative
odds with coarse g3 or mixed corrections. A bounded 24-component check improves
8/9 outputs but changes the reported radiative structure materially only for
26/9; only four improvements exceed 0.2 point. Exact traces and the rule for
when a 24 rerun is warranted are in
`agents_record/2026-07-14-maxcomp12-transition78-overshoot-stratified-diagnostics.md`.

The surface-6 pattern is now population-tested at MaxComp=12. Among all 57
transition-7--8 overshoots from +0.5% to +2%, surface 6 is selected first in
24 events and an inward-only surface in 45; among 57 same-transition,
nearest-loss controls inside +/-0.5%, the counts are only 7 and 19, while
truth-surface selections rise from 8 to 23 and 11 controls retain identity.
This is an overshoot-specific coupled loss-magnitude/surface degeneracy, not a
universal surface-6 preference. Full dumps, matching definition, exact tables,
plot, caveats, and interpretation are in
`agents_record/2026-07-14-maxcomp12-transition78-surface6-population-audit.md`.

A default-off counterfactual loss scan now tests the missing-BH-support
hypothesis without entering the live mixture. On the same 57 overshoots and 57
loss-matched controls it compares 0.5--12% trial losses at the truth surface
and one surface inward using exact cumulative MarlinTrk innovation evidence.
All 2,850 branches are complete. Only 13 overshoots prefer the optimized truth
surface versus 24 controls; 44/57 overshoots still prefer inward placement,
and only 6/57 have a truth-surface optimum at or above 5%, of which one prefers
truth over inward. The simple idea that a new 5--8% BH component would restore
the truth surface is rejected as the next model change. Implementation,
non-interference checks, variance caveat, exact tables, plots, and next
diagnostic are in
`agents_record/2026-07-14-counterfactual-truth-surface-loss-scan.md`.

The capacity tradeoff remains active evidence, not erased history. In the
uniform random 100-event topology-clean light sample, 12, 24, and 36 give
central-68 half-widths of 0.4144%, 0.3463%, and 0.3586%, with 85, 83, and 82
events inside +/-1%. Capacity can preserve genuine recovery modes, such as
404/8 at 24, and false inner-radiation modes, such as 284/1. Monitor clean-core
degradation, hard recovery, and memory growth under the new default. The full
decision, validation, configurations, traces, plots, and tables are in
`agents_record/2026-07-13-maxcomponents-24-default-adoption.md` and the three
linked capacity-study records there.

Proceed in this order:

1. Compare the 21 persistent current amplifications—especially the 17 events
   stored above +1 percentage point—directly with the full
   19-overshoot/18-control audit at the selected surface and mode. Keep the
   nine drifted IDs as controls and seek a physically interpretable
   discriminator before changing the model. Do not tune the rejected noisy-OR floor or
   modify KL reduction, rescale the global process prior, revive
   dominant-lineage publication, add an ad hoc measurement-evidence threshold,
   or tune covariance globally.
2. Validate any change first on all 19 overshoots and matched controls plus
   the five ordinary light representatives,
   clean 62/9 and hard 1/3, then events 11, 16, and 17 and the full 2045 clean,
   2148 light, and 797 hard categories. Require finite complete tracks without
   new rejection or covariance failures. Then use forced-BH muons and
   representative 10 GeV/85-degree and 10 GeV-pT/20-degree electrons as
   transfer/safety controls. Treat catastrophic-outlier changes as a secondary
   safety check rather than the optimization objective.
3. Return to seed 74/event 4 and the missing seed-464 tuple only after the
   light-tail mechanism is resolved without sacrificing clean or hard results.

Success means reducing the light-eBrem tail and improving its core without
weakening the demonstrated hard-loss recovery or broadening/biasing the LCIO
no-eBrem core. Independent held-out validation and broad energy/angle coverage
remain required before any production-performance claim.

Current non-goals: adding a new measurement-evidence selection threshold,
using WeightedMean, global covariance tuning, fitting SimHit momentum, treating
ACTS coefficients as CEPC validation, premature runtime optimization, or
additional shared-package changes.

The outgoing clean-overselection focus is preserved in
`agents_record/2026-07-12-clean-overselection-focus-superseded.md`. Exact
identity construction and category provenance remain in
`agents_record/2026-07-12-ebrem-only-identity-and-clean-control.md`; KL,
reverse-seed, forward-only, and RTS diagnostics remain in
`agents_record/2026-07-12-reverse-overselection-diagnostics.md`. The expanded
light-tail population, four-event state traces, and focused controls are in
`agents_record/2026-07-12-light-ebrem-tail-initial-diagnosis.md`. The full
loss-bin survey and fifteen representative state traces are in
`agents_record/2026-07-12-light-ebrem-stratified-survey.md`; exact odds and the
rejected capacity/stratum-split tests are in
`agents_record/2026-07-12-light-ebrem-prior-audit-and-split-test.md`. The
official ACTS synchronization, source provenance, and focused checks are in
`agents_record/2026-07-12-acts-default-synchronization.md`.
The 10 GeV material-step transfer test, global loss-per-radiation-length
comparison, reconstruction summary, and provenance correction are in
`agents_record/2026-07-13-2gev-conditioned-model-10gev-transfer-check.md`.
The normalized three-way electron/muon LCIO/GSF controls, configuration caveat,
and selection-mechanism interpretation are in
`agents_record/2026-07-13-10gev-electron-muon-lcio-gsf-controls.md`.
The tested BH transferability universe, precise per-transition/per-event/global
loss-per-X0 definitions, and 20-degree evidence are in
`agents_record/2026-07-13-cepc-conditioned-bh-transferability-universe.md`.
The final integrated physics picture, 20-degree reconstruction result,
operational memory caveat, exact resume point, and file handoff are in
`agents_record/2026-07-13-pre-disconnect-integrated-understanding-handoff.md`.
