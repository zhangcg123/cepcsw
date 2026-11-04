# CEPCSW GSF Development

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

`RecGsfTracking` builds, installs, and reads `CompleteTracks`. Smoother and
reverse runs write three row-aligned endpoint views: BestBranch to
`GSFTracksBestBranch`, the moment-matched endpoint to
`GSFTracksWeightedMean`, and the maximum of the complete five-dimensional IP
mixture density to `GSFTracksFullMixtureMode`. FullMixtureMode is
automatic/default-on, has a persisted optimization-status collection and
flat-tuple fields, and is mechanically available but not physics-validated.
Its definition and gates are in
`agents_record/2026-08-24-full-mixture-mode-endpoint.md`; historical CMS-like
extensions remain in dated records only.

Positive-weight final components and complete component lineage are persisted
automatically for smoother and reverse. The `final_mixture_component_*`,
`lineage_node_*`, and `lineage_edge_*` flat vectors retain the final mixture
and every evaluated seed, BH child, measurement result, KL output, cutoff, and
merge. Reverse additionally records same-surface two-filter products
`B_smoothed[i] = F_updated[i] x B_predicted[i]` only at interior surfaces.
Each product is formed inside its reverse-surface step from buffered
`B_predicted[i]` candidates, immediately before the same buffered measurement
results are committed to live `B_updated[i]`.
Direct product candidates persist their pair prior, five-dimensional overlap
chi-square/log-determinant, log weight, normalized pre-pruning posterior,
backward-predicted state, and explicitly named smoothed state.
They also persist signed F/B curvature and pT differences, their approximate
zero-cross-covariance variances, and a passive direction-signed
five-dimensional compatibility score. Its magnitude is the chi-square CDF of
the full F/B state difference and its direction is the signed pT difference.
The score is meaningful on the exact identity pair only; it is not a
calibrated physical posterior and never steers the fit.
They also persist the signed forward/backward differences
`B_predicted-F_updated` in kappa and transverse momentum, with passive
independence-approximation variances; the exact schema contract is in
`agents_record/2026-09-01-forward-backward-delta-diagnostics.md`.
The boundaries reuse live mixtures: `B_smoothed[0] = B_updated[0]` and
`B_smoothed[N-1] = F_updated[N-1]`. Interior product states never propagate or
publish. The compiled/default `InwardWeightMode=LocalMeasurement` also leaves
their weights diagnostic-only. Experimental `SmoothedMarginal` instead sums
the normalized unreduced direct-pair weights over all forward partners for
each backward candidate and attaches that marginal to the corresponding live
`B_updated[i]` state before cutoff, reduction, and further inward propagation.
It deliberately reuses overlapping forward evidence at successive surfaces
and is not a calibrated Bayesian posterior. Its active contract is in
`agents_record/2026-08-31-smoothed-marginal-inward-weighting.md`; the original
passive contracts are in
`agents_record/2026-08-25-final-mixture-component-flat-tuple.md`,
`agents_record/2026-08-25-component-lineage-dag-flat-tuple.md`, and
`agents_record/2026-08-29-smoothed-diagnostic-only-publication.md`; the
explicit boundary correction and focused gate are in
`agents_record/2026-08-29-smoothed-boundary-state-contract.md`, and the inline
construction regression is in
`agents_record/2026-08-29-inline-smoothed-surface-construction.md`; the
surface-local evidence schema is in
`agents_record/2026-08-29-smoothed-surface-local-evidence.md`.

Reverse consumes one `SharedForwardFilterResult` and publishes the terminal
inward mixture `B_updated[0] = measurement[0] x B_predicted[0]`. A positive
`InwardSeedCovarianceScale` copies and scales the final forward population; a
finite value at or below zero builds one fresh standard-KF-style seed, updates
the outermost hit, and first revisits hit `N-2`. The common initializer uses a
direction-local two-dimensional-hit prefit: the three innermost hits outward
and the three outermost hits inward, followed by the loose `FullLDCTracking`
covariance and an explicit boundary-hit MarlinTrk update.
`ForwardSeed=1` and `BackwardSeed=1` independently select the complete
FullLDCTracking-style loose diagonal covariance for the direction-local
prefits. Every finite positive value uniformly scales all five seed variances;
zero and negative values are invalid. `BackwardSeed` is inert when a positive
`InwardSeedCovarianceScale` selects a copied/scaled forward mixture. The
implementation gates and retired curvature-only controls are in
`agents_record/2026-08-28-standard-kf-gsf-initializer.md` and
`agents_record/2026-08-29-fresh-inward-standard-kf-initialization.md`, with
the current scaling contract in
`agents_record/2026-09-06-directional-seed-covariance-scales.md`.

The former CMS-like compatibility alias and `CmsGsfSmoothing` property are
retired because they had become exactly equivalent to reverse while publishing
no distinct endpoint. Historical tuples, source codes 3/4, and dated evidence
remain interpretable; the migration and exact reverse regression gate are in
`agents_record/2026-08-29-cms-like-workflow-retirement.md`. Forward filtering,
an independent reverse multi-component refit, and a KL reduction-aware
experimental smoother remain mechanically operational.
A default-off ECAL component-re-ranking prototype is also mechanically
operational. It preserves `GSFTracksBestBranch` and writes its paired result separately;
its focused evidence is promising only for retained bimodal alternatives and
is not population-validated.
A separate default-off truth BH-loss oracle can replace existing BH-call
responses on explicitly selected tracks while leaving the downstream GSF
workflow unchanged. It is a mechanism diagnostic only and never production
steering.
The normal simulation event can now optionally embed exact
`SimTrackerHit -> Geant4-step` provenance in two PODIO collections. The
default-off oracle's current batch source follows the standard reconstructed-
hit truth associations into those collections, so the maintained workflow no
longer requires or supports a side material tuple or prejoined CSV input.
The active `dump_gsftrk.sh` worker accepts one `STAGES` subset of `sim`, `trk`,
and `gsf` (default `trk,gsf`) and executes selected stages in physical order.
Each stage consumes a predecessor produced in the same job or, when that
predecessor is omitted, the corresponding existing tuple from the input tuple
path. The maintained GSF card writes only the flat tuple; its GSF collections
pass directly in memory to `RecGsfFlatTuple` and are not serialized through
`PodioOutput`. After verifying the flat tuple, the worker removes any tracker
tuple produced by its own `trk` stage; a `gsf`-only job retains its external
tracker input. Calorimeter digitization/reconstruction and `EcalCluster` are
not part of this worker while the ECAL prototype is paused.
A passive interval recorder now persists, in the final
GSF EDM and flat tuple, fractionally integrated Geant4 t/X0/eBrem truth,
DD4hep t/X0 between the same exact truth hooks, and summaries of the actual
forward/reverse component paths. These values never steer the GSF. The
compiled, active reverse-template, and maintained-card default is on.

The active production candidate remains the reverse multi-component refit. It
starts from the complete final forward mixture, scales each full covariance by
`InwardSeedCovarianceScale` (default 100), repeats measurement updates inward,
and publishes the selected branch, moment-matched mixture, and full
joint-density mode in separate row-aligned collections. It has demonstrated
interaction-point momentum recovery in many
hard-bremsstrahlung events and favorable central light/hard performance, but
it also creates clean-track degradation and extreme tails. The KL smoother is
largely LCIO-like and forfeits much of the hard-loss recovery.
The maintained comparison card now deliberately selects
`InwardSeedCovarianceScale=-1` for the fresh-inward-seed campaign. This is
campaign steering only; it does not change the compiled or active-template
default 100. Its reverse branch selects the compiled and active-template
`InwardWeightMode=LocalMeasurement` control; `SmoothedMarginal` remains a
default-off experiment.
Directional BH child creation is independently configurable. The compiled and
inherited active reverse-template defaults are
`ForwardBHSplitting=false, InwardBHSplitting=false`, so an unsteered fit
creates no BH children in either direction. The maintained double-off
diagnostic is historical; the maintained live beam-boundary campaign now
explicitly enables both gates. These gates do not disable
material-path evaluation, passive interval recording, deterministic energy
loss, multiple scattering, propagation, or measurement updates.

A default-off reverse-only live beam-boundary experiment is implemented inside
`RecGsfTracking`, outside the MarlinTrk classes. The forward three-hit prefit
is moved to the beam pivot and constrained before optional beam-to-hit-0 BH
splitting and the first real-hit update. After the ordinary reverse recursion
updates hit 0, the same canonical boundary t/X0 may split again in the inward
direction; each child propagates to the beam, receives the transverse `drho`
likelihood, and enters the ordinary cutoff/KL and endpoint publication. Its
nominal comparison widths are 0.0145 mm horizontally and 3.6e-5 mm vertically.
The exact contract and focused mechanical gate are recorded in
`agents_record/2026-09-07-live-beam-boundary-gsf.md`; the superseded endpoint-
copy prototype remains documented in the earlier dated handoff record.

The active defaults are `MaterialPathMode=DD4hepBetweenSurfaces`,
`ForwardSeed=1` and `BackwardSeed=1` (the complete FullLDCTracking-style
loose covariance for both direction-local prefits), `MaxComponents=10`,
`ComponentWeightCutoff=1e-4`, `SymmetricKL` reduction ranking,
identity-lineage protection enabled, `ForwardBHSplitting=false`,
`InwardBHSplitting=false`, and `CEPCRuntimeCategoryAligned9Clear`. Despite the
retained selector name, this BH model now has one effective identity plus nine
globally fitted radiative Gaussians. Preserve 12 and 24 components and
`CurrentSurface` as explicit comparison settings. `ActsAtlas` is the only
alternative BH model. Neither model is validated for production physics.

Geant4 pre/post-step data is the authoritative energy-loss truth.
SimTrackerHit momentum is only a detector-level cross-check. Existing Geant4
studies establish a real electron loss tail and bounded fractional-loss
transfer compatibility over the tested 2--10 GeV and theta 85--20 degree
samples. None of the available Bethe-Heitler models is validated for general
CEPC tracker steps.

Population studies show real central recovery, especially for losses at
transitions 5--11, but also new extreme tails. Losses at transitions 0--4 are
predominantly information-limited. Forced-electron-hypothesis muon controls do
not show a universal momentum inflation, but do show clean-core broadening and
outliers. This is a research implementation, not a validated production
algorithm. Broad performance claims and mainline integration require clean
track preservation, reproducible tail control, and independent held-out
validation.

Historical evidence, resolved incidents, exact experiment tables, runbooks,
and superseded decisions live under `agents_record/`. Load historical records
only for regression evidence, design rationale, experiment comparison, or
explicit provenance. Historical detail does not override this live status.

### Project laws and work scope

- Keep implementation changes inside `Reconstruction/RecGsfTracking` unless
  the user explicitly authorizes broader scope for a concrete reason.
  Current explicit exception: develop `Reconstruction/RecBreakpoint` and its
  dedicated card/build registration. Keep existing GSF and shared KF sources
  and maintained workflow cards unchanged for this experiment.
- Do not modify KalTest, TrackSystemSvc, MarlinTrk, DDKalTest, or other shared
  CEPCSW packages to compensate for a GSF-specific state-management problem
  unless the user explicitly authorizes a narrow shared interface change.
- Do not hand-code a parallel Kalman measurement update when the baseline
  MarlinTrk interface can provide the required operation.
- Treat Geant4 pre/post-step records as material-energy-loss truth. Do not
  present SimTrackerHit momentum as an exact material transition.
- Do not claim Bethe-Heitler or GSF validation from successful execution,
  finite output, improved chi-square, or selected-sample improvement alone.
  Validation requires interaction-point momentum recovery against generator
  truth in categorized hard-loss events, clean-track safety, and held-out
  population checks.
- Preserve unrelated working-tree changes. Keep source/documentation changes
  separate from generated ROOT files, logs, plots, tables, notebooks, and
  batch cards.
- Use `dev` as the active development branch. Do not switch, create, rename,
  delete, merge, or rebase branches unless the user explicitly requests the
  specific branch operation.
  The user-authorized breakpoint experiment is on `breakpoint` (renamed from
  `test_breakpoint`), branched from `dev`. The user authorized publishing this
  branch to origin; other remote branch changes require a separate request.
- Use Git frequently during development: inspect status and diffs, and create
  focused checkpoint commits after coherent, proportionately verified core
  implementation or project-knowledge changes. Track, commit, and push all
  C/C++ implementation and header changes across the repository, together
  with documentation, `AGENTS.md`, `.agents/` maintenance content, and durable
  `agents_record/` status/history records. This Git rule does not broaden the
  separate implementation-scope law: edits outside
  `Reconstruction/RecGsfTracking` still require explicit authorization. Also
  track the specifically maintained workflow card `DumpGsfTrks/gsf.py.bk`,
  whose complete property steering is part of the documentation contract
  except for the deliberate inherited
  `RecordTruthMaterialIntervals=true` default.
  Keep other run cards/options, analysis scripts, build files, generated ROOT
  files, logs, plots, tables, notebooks, batch cards, and experiment outputs
  uncommitted unless the user explicitly authorizes a specific exception. Do
  not change branches unless the user explicitly requests it.
  Explicit breakpoint publication exception: also track its dedicated
  run_breakpoint.py, subbreakpointjobs.sh, dump_breakpoint.sh, batch helper,
  package build files/registration and the required DumpGsfTrks/trk.py.bk.
  Never stage or commit generated ROOT files for this publication.
- Keep `AGENTS.md` limited to global status, active laws, essential commands,
  and the current focus. Before replacing or removing unique detail, preserve
  it in a dated `agents_record/` entry; replace rather than append focus.
- Whenever a `RecGsfTracking` configurable property is added, removed,
  renamed, or its default/allowed values change, assign a dedicated sub-agent
  to audit the complete option surface. In the same change, synchronize the
  authoritative property reference in
  `Reconstruction/RecGsfTracking/README.md` and the explicit effective
  steering in `DumpGsfTrks/gsf.py.bk`; document intentional historical-card
  differences in `DumpGsfTrks/README.md`.
- Validate every implementation step with comprehensive verbose component
  dumps on a focused event. Once mechanically stable, repeat on hard-loss
  events 11, 16, and 17 before population validation. Build success, finite
  output, or lower chi-square is not a sufficient gate.
- Exclude the stable 133-event secondary-tracker-activity set from
  single-track optimization counts and representative selection, but always
  report it separately as a topology/control population.
- Use same-code direct A/B reruns for final-selection claims; stored outputs
  can drift as the implementation changes.

### Compile and run

Run commands from the repository root. For a complete configured build:

```bash
source setup.sh
./build.sh
```

For the normal focused EL9/LCG 105 development cycle:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt \
  --target RecGsfTracking RecGsfFlatTuple -j4
cmake --install build.105.0.0.x86_64-el9-gcc11-opt
```

Run Gaudi options through that environment:

```bash
source setup.sh
build.105.0.0.x86_64-el9-gcc11-opt/run \
  gaudirun.py path/to/options.py
```

Use a small `SelectedEventIndices` list for component diagnostics. Generated
ROOT files and logs are outputs, not status records.

For browser plots, use the user's dedicated localhost port 8000 and the
gallery server by default: `source ~/.bashrc; plotweb_start /absolute/plot/dir`.
Its home page shows all PNG previews together; `plotweb_status` and
`plotweb_stop` manage it. Stop the current server before launching a different
plot directory. The maintained personal implementation is the `plot-gallery`
skill at `~/.agents/skills/plot-gallery/`; its launcher and server are independent
of CEPCSW. The former `TrackingPerformanceStudies/plot_gallery_server.py` remains
available as a historical project copy. Do not start a plain
directory-listing HTTP server for future plot requests. Delegate the complete
plot-server request to one dedicated sub-agent: inspect the requested directory
and analysis notes; author/update its `gallery.json` with `schema_version: 1`,
`title`, `intro`, and ordered `sections` (`title`, `description`, `patterns`);
verify every PNG appears in exactly one meaningful category; safely stop the
previous plot server and launch the requested directory on localhost port 8000;
verify the gallery page and a PNG response; then report the URL and server PID
to the main agent. The main agent relays the verified result to the user. Keep
category semantics out of the generic server; it displays uncategorized new
plots separately until the manifest is updated. Treat each per-analysis
manifest as an uncommitted analysis artifact.

## 2. Current focus

**TRUTH ENDPOINTS PLUS CONFIGURABLE LOCATION SCANS (2026-11-04).**
Work on `breakpoint-delayed-neutral`; preserve `breakpoint`. No branch/remote,
PFA/shared-KF, loss-map, optimizer bound/acceptance or truth-owner change is
authorized. The independent acceptance audit remains open.

The user requires every enabled free-loss approach to retain its normal
truth-location result, independently of which approaches scan locations.
Use `IntervalSelectionMode="Truth"` and the existing FreeLossFit/beam/charged/
neutral switches. New `LocationScanSetups=[]` (compiled/standard-card default)
adds no scans; [110] adds only110, [0,100,110] adds those three. Allowed entries
are unique0/100/110/111, canonically sorted. Nonempty lists require Truth and
FreeLossFit;100 needs beam,110 beam+charged,111 all three enabled. TruthOverride
is a separate loss-prior oracle, not required for truth-location free fits.

Normal endpoint families run first at the eligible truth owner and remain
published in their existing collections/flat branches. Only afterwards are
requested location scans run from original inputs, not any fitted posterior.
They record `location_scan_*` without selecting/overwriting an endpoint.
An optional scan exception cannot invalidate preserved truth results.
The list controls actual scans, not just saving: omitted predecessor approaches
supply raw observations but are not optimized secretly. Failure can retain only
an earlier successful REQUESTED setup at the same point; with none, save an
invalid/NaN entry. In a111-only scan its inapplicable null is therefore invalid,
not a fabricated110 result.

The maintained Higgs launcher now prepares Truth plus configurable scans,
default0/100/110, with fresh default output
`higgs_location_scan_truth_20261104`. Override BP_LOCATION_SCAN_SETUPS
(comma-separated, explicitly empty disables scans). All enabled truth
approaches remain saved regardless of this list. Generic batch forwarding,
frozen-card steering and worker schema checks are synchronized. No real
campaign prepared/submitted; only one dry-run test card under /tmp.
Existing frozen cards remain unchanged. Legacy HitScanAll stays an explicit
scan-only comparison with inactive normal endpoints and its old ordering,
default0/100/110 plus optional111.

Point definitions are unchanged: 5D null, VXD/ITK, exact first TPC row0,
optional eligible truth reference last if outside that list. Keep all hits.
No online ranking/gain cut; truth never supplies scan loss amounts.
ChargedEcalEnergyMode still controls110: ElectronOnly uses C for electron;
ElectronPhoton uses C or C+N for the sum. Passive neutral collection stays
active for110 even with111 OFF;111 always uses separate C/N. Sum flag plus
actual mask/source/status remain authoritative. Existing ownership guards,
SigmaLogLoss0.001 and prior-center bound[0,1] are unchanged.

Validation:15 CTests and61 batch tests pass. Direct comparisons with the
previous project-installed module on events11/16/17 preserve every one of
913 non-scan branches per row with110-only scanning (2,739 branch-row values).
Legacy event5 reproduces all947 branches exactly.111-only event5 preserves
all four truth-location free-loss families and all913 main branches;8 scan
candidates have separate mask7,1 fails Minuit and stays invalid (no acceptance
repair). Verbose evidence and deployment details:
`agents_record/2026-11-04-truth-endpoints-configurable-scans.md`.
User confirmed no active jobs and authorized install after validation.
The package-only project-default update is installed; no temporary runtime
pin is used by batch workers. No population performance claim follows.

Earlier campaign `higgs_location_scan_20261104` has249 files but only80
verified complete jobs (16,000 events);169 were killed at the10-hour limit.
Partial files are not usable as completed outputs. The old200 scan ROOTs in
`higgs_silicon_scan_20261103` remain deleted by prior request; logs/cards and
historical plots remain. New200-event default scans can still hit the runtime
limit; no chunking/rerun of that failed batch is part of this control change.
The completed80-file analysis remains at
TrackingPerformanceStudies/breakpoint_higgs_location_scan_20261104.
It includes truth-assisted TPC recoil routing and truth/fitted loss histograms;
the sample is runtime-selected. No-brem/null model selection is NOT applied
in those offline minimum-site plots. Server root remains TrackingPerformanceStudies.

Complete outgoing AGENTS:
`agents_record/2026-11-04-agents-before-configurable-truth-scans.md`.
Earlier contracts:
- `agents_record/2026-11-04-full-higgs-three-setup-scan.md`
- `agents_record/2026-11-04-honor-electron-only.md`
- `agents_record/2026-11-04-tpc-row0-location-scan.md`
- `agents_record/2026-11-04-location-scan-full-ip-state.md`
- `agents_record/2026-10-26-effective-boundary-truth-owners.md`
- `agents_record/2026-10-26-breakpoint-independent-audit-handoff.md`
