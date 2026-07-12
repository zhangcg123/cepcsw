# eBrem-only identity repair and clean-track control

Date: 2026-07-12

## Why the conditioned artifact was replaced

The first `CEPC2GeV85StepConditioned` artifact fitted total Geant4 transition
retention, `p_after/p_before`.  The reconstruction card also had
`ElossOn=True`, so ordinary deterministic material energy loss was represented
both by MarlinTrk and by the BH mixture.  Moreover, component zero was only an
exact identity at zero material; its fitted mean moved below one and its weight
reached zero at thick knots.

The extractor now has `--loss-source total|ebrem`.  The stable reconstruction
artifact uses the eBrem-attributed loss
`z = 1 - ebrem_step_loss_sum_GeV/p_before_GeV`; transitions with no eBrem step
form an exact `z=1` atom.  The source contains 2,573,914 accepted transitions
inside `t/X0 < 0.03`, including 9,528 eBrem rows; 783 thicker source rows are
excluded from this first reconstruction range.  The eight fitted identity
weights are 0.999216, 0.996980, 0.992560, 0.955170, 0.910551, 0.852615,
0.812898, and 0.746967.  Identity mean is exactly one at every knot, the dense
model normalizes to 4.44e-16, and tail probability is 1.57e-11 at
`t/X0=1e-12`.

Stable artifacts and inspection products are under
`Reconstruction/RecGsfTracking/data/CEPC2GeV85StepConditioned/`.  The source
and audit are `cepc2gev85_step_conditioned_source.csv` and its audit JSON.

## Identity-lineage implementation

`GsfComponent` carries `noRadiationLineage`.  Splitting preserves it only for
an identity parent taking the exact identity child.  Low-weight cutoff retains
the identity lineage, and KL reduction does not merge identity and radiative
lineages when more than one retained component is allowed.  Reverse
initialization copies the flag, and verbose dumps expose it as `noRad=`.

The package built and installed successfully.  Focused seed 1 event 9, with no
owned eBrem, retained the identity as best and gave truth/LCIO/GSF pT of
2.0004/2.0039/2.00392 GeV.  Seed 1 event 3, with 10.4% cumulative owned loss,
gave 2.0004/1.7980/1.9912 GeV, showing that identity protection did not remove
the demonstrated hard-loss recovery.

## Correct surface-owned event categories

The earlier volume-token category was not aligned with reconstruction
ownership.  The authoritative matched 1000-event transition table is
`/tmp/matched-1000-surface-transitions.csv`; its durable category table is
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/` followed by
`surface_owned_ebrem_event_categories.csv`.  It contains 407 no-eBrem, 437
light-eBrem, and 156 hard-eBrem events.  The former token classification both
missed radiation on owned support intervals and counted radiation outside the
fitted measurement-surface span.

## Clean-event controls

All 407 surface-owned no-eBrem events completed with the eBrem-only BH and
reverse BestBranch.  LCIO median, central-68 width, and RMS residual are
-0.01821%, 0.278607%, and 3.03661%; the BH result is +0.004486%, 0.321295%,
and 3.13629%.  Counts inside 1% are 395 and 384.  Thus the median bias was
removed, but a minority tail still broadens the core.

The identical 407 events were then run with BH splitting disabled while
retaining multiple scattering, deterministic energy loss, and reverse
filtering.  Reverse no-BH gives median -0.014639%, width 0.275195%, RMS
3.036209%, and 396 events inside 1%.  This matches or slightly improves LCIO,
so the reverse refit alone is not the clean-core degradation mechanism and the
conditional reverse-message investigation is not activated.

Eventwise BH versus no-BH comparison finds 40/407 changes above 0.1%, 19 above
0.5%, 15 above 1%, and 2 above 5%.  The worst is seed 23 entry 8: truth 2.00036
GeV, no-BH 1.99839 GeV, and BH BestBranch 2.30485 GeV.  Its verbose log is
`/tmp/gsf-focus-23-8.log`.  The identity lineage survives to the reverse IP at
weight 0.291 and pT 1.99839 GeV, but a KL-merged radiative component reaches
weight 0.377 and is selected at 2.30485 GeV.  A difficult inner measurement at
hit 5 gives identity delta-chi2 61.65 while radiative hypotheses give roughly
17--20; by hit 2 a merged radiative cluster has weight 0.889.  Forward retained
lineage smoothing still favors identity at weight 0.735.  Therefore the
remaining clean degradation is localized radiative over-selection driven by a
measurement fluctuation plus the flexibility and weight aggregation of
radiative clusters.  Post-KL `BestBranch` is the heaviest merged component,
not necessarily one unmerged physical lineage.

## Hard-event category result

The corrected eBrem-only model completed 155/156 surface-owned hard-eBrem
events; seed 74 entry 4 remains the known failure.  LCIO versus reverse
BestBranch median residual is -12.8099% versus -0.2915%; events inside 1% are
48 versus 74 and inside 5% are 52 versus 87.  RMS improves from 35.2127% to
34.1263%.  A large unrecovered tail remains: the central-68 width changes only
from 50.9833% to 47.8167%.  These are execution and categorized-recovery
results, not independent validation, because the fit and evaluation use the
same production phase.

## Next optimization question

Do not add the user-excluded measurement-evidence selection threshold.  The
next controlled study should separate unmerged lineage posterior weight from
KL-cluster weight and determine whether reduction/BestBranch semantics can
avoid clean-event cluster over-selection without losing the hard-event gains.
No shared-package change or reverse-refit rewrite is currently justified.

The available exact-pair `/tmp/gsf-match-tracks.root` ends after event 11, so a
requested verbose selection of 11, 16, and 17 could only execute event 11.
That event remained finite with 234/234 hits and pT 1.98366 GeV, but its reverse
summary contained 2 rejected component updates.  Events 16 and 17 require the
original longer exact-pair input (or newly produced exact-pair replacements)
before the three-event zero-rejection gate can be repeated.  The durable log is
`/tmp/gsf-identity-events-11-16-17.log`; this partial check does not satisfy the
full focused validation gate.
