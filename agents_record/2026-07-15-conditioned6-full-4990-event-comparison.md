# Conditioned6 full 4,990-event comparison

Date: 2026-07-15

## Inputs and completeness

The user produced the large `CEPC2GeV85StepConditioned6`, MaxComponents=12
sample directly in the repository root as 500 `gsf_flat-e--2.0-85-SEED.root`
and 500 `gsf-e--2.0-85-SEED.root` files. The standard audit finds 499 usable
flat tuples and 4,990 finite matched events. Seed 464 has the known 951-byte
flat tuple with no `gsf_tuple`; this is the existing missing-seed limitation,
not a new model failure.

The comparison uses the existing old-model MaxComponents=12 sample and has an
exact common set of 4,990 event IDs. Topology-clean populations are 2,032
no-eBrem, 2,132 light-eBrem, and 694 hard-eBrem events. The 132 events with
secondary tracker activity remain included only in the explicitly inclusive
rows and are excluded from topology-clean optimization conclusions.

## Inclusive and topology-clean results

For all 4,990 events, old/new conditioned GSF gives:

- median residual: -0.06150% / -0.06052%;
- central-68 half-width: 0.46697% / 0.46849%;
- RMS: 20.8013% / 19.9566%;
- inside 1%: 4,055 / 4,048;
- inside 2%: 4,325 / 4,322;
- inside 5%: 4,484 / 4,482;
- inside 10%: 4,557 / 4,560.

For all 4,858 topology-clean events:

- median: -0.05813% / -0.05654%;
- central-68 half-width: 0.40740% / 0.40622%;
- RMS: 19.9336% / 19.0231%;
- inside 1%: 3,999 / 3,993;
- inside 2%: 4,265 / 4,262;
- inside 5%: 4,422 / 4,421;
- inside 10%: 4,492 / 4,495.

The lower total RMS is driven by changes among extreme outliers and is not a
clean-core improvement. Eventwise over all 4,990 events, the new model improves
519, worsens 528, and is unchanged at tuple precision for 3,943. Among
topology-clean events the split is exactly 505 improved and 505 worsened, with
3,848 unchanged.

## Topology-clean physics categories

No-eBrem, old/new:

- median: -0.00677% / -0.00661%;
- width68: 0.14352% / 0.14341%;
- RMS: 22.7196% / 21.2668%;
- inside 1%: 1,915 / 1,913;
- beyond 10%: 38 / 36.

Light-eBrem, old/new:

- median: -0.09431% / -0.09254%;
- width68: 0.43659% / 0.42826%;
- RMS: 5.0929% / 5.2747%;
- inside 1%: 1,769 / 1,769;
- inside 2%: 1,959 / 1,960;
- inside 5%: 2,055 / 2,056;
- inside 10%: 2,100 / 2,102;
- beyond 10%: 32 / 30.

Hard-eBrem, old/new:

- median: -0.66329% / -0.72870%;
- width68: 22.8740% / 22.7712%;
- RMS: 34.5025% / 33.5178%;
- inside 1%: 315 / 311;
- inside 2%: 357 / 355;
- inside 5%: 378 / 375;
- inside 10%: 398 / 397;
- beyond 10%: 296 / 297.

Thus the new model modestly narrows the light central 68% interval and removes
two >10% light tails, but it leaves the primary light ±1% count unchanged and
worsens light RMS. It also loses clean and hard events from the useful core.
This fails the active success requirement of improving light core/tail without
weakening clean or hard behavior.

## Transition dependence and eventwise tails

For topology-clean light events, old/new results by dominant transition are:

- 0--2: width68 1.5173% / 1.5223%, inside 1% 42/42;
- 3--4: width68 unchanged at 2.2597%, inside 1% 82/82;
- 5--6: width68 0.6473% / 0.6550%, inside 1% 382/384;
- 7--8: width68 0.4762% / 0.4728%, inside 1% 446/444, RMS 5.1883% / 5.9806%;
- 9--11: width68 0.4223% / 0.4184%, inside 1% 74/73;
- >11: width68 0.2070% / 0.2081%, inside 1% 743/744.

The large-sample transition-7--8 result therefore confirms the earlier warning:
the central width changes little while new tails worsen the RMS and the 1%
population falls.

Largest new-model worsenings include 483/2 light (+85.68% to +110.26%), 144/2
no-eBrem (-0.48% to -24.67%), 352/4 light (+2.90% to -26.13%), and 403/3
no-eBrem (+14.96% to +30.31%). Large improvements also occur, including 172/3
hard (+326.92% to +246.24%), 116/5 no-eBrem (+1005.98% to +939.04%), 261/1
light (-21.02% to +4.99%), and 47/2 light (+14.56% to -0.59%). These swaps show
discrete selection changes rather than a stable global resolution gain.

## Decision and outputs

`CEPC2GeV85StepConditioned6` is rejected as the default replacement. Keep it
selectable and default-off for diagnostics. The current five-component model
remains active.

New-model plots and tables:
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/conditioned6_maxcomp12_2026-07-15/`.

Exact three-way matched comparison:
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/plots/conditioned5_vs_conditioned6_maxcomp12_2026-07-15/`.

The generic labels/title options added to
`Reconstruction/RecGsfTracking/scripts/compare_maxcomponents_12_24_pt.py`
allow this same plotting workflow without mislabelling the compared models.
