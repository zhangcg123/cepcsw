# Topology-clear ECAL category and transition screen

Date: 2026-08-17

## Scope and definitions

The broad ECAL population screen was reclassified using truth records matching
the new 100-event-per-seed flat tuples. The analysis joins 40,091 finite paired
LCIO, ordinary-GSF, and ECAL-constrained-GSF events from 401 seeds.

Topology comes from the matching `tuples285/sim-e--2.0-85-<seed>.root` files.
An event is topology-clear only when all six tracker SimHit-to-MC associations
have MC index zero. Any association index greater than zero places the event in
the separately reported secondary-tracker-activity control.

Energy-loss truth comes from
`tuples285/oldgsf/gsf_material_steps-e--2.0-85-<seed>.root`. The primary
electron selection is `track_id=1`, `parent_id=0`, `pdg=11`. Geant4 steps are
assigned with outgoing ownership from one sensitive-entry anchor up to, but
not including, the next sensitive entry; TPC upper half-layers are omitted as
duplicate measurement anchors. Only owned `process_subtype=3` eBrem losses
enter the category:

- no-eBrem: no owned primary-electron eBrem step;
- light-eBrem: owned eBrem exists and both maximum single-transition and
  cumulative fractional loss are below 10%;
- hard-eBrem: either quantity reaches 10%.

The dominant transition is the eBrem-containing transition with the largest
total momentum loss, matching the established transition-location study.
Transition bins are 0--2, 3--4, 5--6, 7--8, 9--11, and >=12. Because these
indices are not universal detector layers across a broad-angle sample, the
owner path is also grouped physically as VXD, ITK, or TPC.

## Mechanical validation

- All 401 flat-tuple seeds had matching 100-event simulation and material-step
  truth; all 40,091 paired rows joined exactly.
- On seed 7, the temporary aggregator reproduced all 100 categories, cumulative
  owned-loss values, and dominant transition indices from the established
  `build_g4_transition_dataset.py` output with zero mismatches.
- The established transition builder produced 23,198 transitions from seed 7;
  the direct aggregator used the same anchor count and ownership.
- A ROOT `TTreeFormula` cross-check reproduced the selective uproot topology
  count (for example seed 7 entry 5 has three secondary ITK-barrel SimHits).

## Population split

| population | no-eBrem | light-eBrem | hard-eBrem | total |
|---|---:|---:|---:|---:|
| topology-clear | 14,924 | 16,638 | 4,119 | 35,681 |
| secondary activity | 239 | 1,371 | 2,800 | 4,410 |

The 11.0% secondary-control fraction is much larger than the older fixed-angle
sample and is strongly enriched in hard eBrem. It must not be mixed into the
optimization population.

## Topology-clear category resolution

All residuals are `100*(pT_reco/pT_truth-1)` on identical paired events.

| category | method | median (%) | width68 (%) | abs >10% | abs >50% | abs >100% |
|---|---|---:|---:|---:|---:|---:|
| no-eBrem | LCIO | -0.00538 | 0.18261 | 110 | 24 | 0 |
| no-eBrem | GSF | +0.00474 | 0.22351 | 148 | 45 | 17 |
| no-eBrem | GSF + ECAL | +0.00471 | 0.22351 | 137 | 33 | 7 |
| light-eBrem | LCIO | -0.2302 | 1.27077 | 135 | 17 | 0 |
| light-eBrem | GSF | -0.1338 | 0.61031 | 173 | 42 | 17 |
| light-eBrem | GSF + ECAL | -0.1337 | 0.61051 | 156 | 26 | 4 |
| hard-eBrem | LCIO | -15.323 | 23.8696 | 2,765 | 617 | 0 |
| hard-eBrem | GSF | -1.283 | 21.9499 | 1,658 | 630 | 76 |
| hard-eBrem | GSF + ECAL | -1.266 | 21.7941 | 1,637 | 606 | 59 |

The tracker-only GSF broadens the no-eBrem central core relative to LCIO and
creates positive catastrophic tails. It substantially improves the light core
and hard median, with a mixed hard width/tail tradeoff. ECAL changes too few
events to alter central widths, but reduces catastrophic tails in all three
categories.

## ECAL branch changes and clean safety

| category | events | changed | improved | worsened | ordinary <=1% changed | core improved | core worsened | moved >1% | moved >3% |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| no-eBrem | 14,924 | 27 | 15 | 12 | 13 | 2 | 11 | 6 | 0 |
| light-eBrem | 16,638 | 52 | 32 | 20 | 20 | 2 | 18 | 15 | 1 |
| hard-eBrem | 4,119 | 71 | 59 | 12 | 2 | 1 | 1 | 1 | 1 |
| total | 35,681 | 150 | 106 | 44 | 35 | 5 | 30 | 22 | 2 |

No topology-clear core intervention moved beyond 10%. Nevertheless, 30 of 35
ordinary <=1% branch changes increased truth error, so the present selector is
not clean-track safe and remains default-off.

## Transition-location results

For topology-clear light eBrem, counts by dominant transition are 557, 1,331,
3,878, 4,019, 625, and 6,228. Ordinary GSF width68 evolves as 2.00%, 1.76%,
1.18%, 0.588%, 0.549%, and 0.286%, respectively. ECAL leaves those central
widths essentially unchanged but removes many sparse positive tails, including
reducing the light ITK-region greater-than-100% count from 10 to one.

For topology-clear hard eBrem:

| transition | N | LCIO median/w68 (%) | GSF median/w68 (%) | GSF+ECAL median/w68 (%) | ECAL improved/worsened changes |
|---|---:|---:|---:|---:|---:|
| 0--2 | 142 | -25.9 / 25.7 | -22.8 / 25.3 | -22.8 / 25.3 | 3 / 0 |
| 3--4 | 342 | -26.5 / 24.1 | -20.8 / 17.1 | -20.0 / 16.8 | 17 / 6 |
| 5--6 | 1,002 | -25.2 / 24.4 | -7.55 / 30.3 | -7.32 / 30.2 | 28 / 4 |
| 7--8 | 1,060 | -23.3 / 21.2 | -1.91 / 27.2 | -1.91 / 27.2 | 2 / 2 |
| 9--11 | 146 | -21.7 / 26.6 | -2.57 / 31.9 | -2.57 / 31.9 | 0 / 0 |
| >=12 | 1,427 | -0.506 / 4.42 | -0.0743 / 1.56 | -0.0745 / 1.58 | 9 / 0 |

This independently repeats the known information boundary: early hard losses
are poorly constrained; 5--11 allows strong median recovery but retains broad
ambiguity; late/TPC-owned losses are much easier. ECAL acts most frequently on
hard transitions 3--6 and improves 45 of 55 changes there.

The physical-region view reaches the same conclusion. Light counts are
3,596 VXD, 6,773 ITK, and 6,269 TPC; hard counts are 924, 1,760, and 1,435.
For hard loss, ordinary GSF median/width68 are -16.53%/24.55% in VXD,
-2.12%/29.18% in ITK, and -0.0745%/1.60% in TPC. Transition index should not
be interpreted as a literal detector layer outside this accompanying region
view.

## Generated artifacts and next gate

Generated, intentionally uncommitted plots and CSVs are under:

`TrackingPerformanceStudies/gsf_ecal_constraint_broad_electron_2026-08-17/topology_and_g4_categories/`

The directory contains the full classified paired table; topology-clear
category views; a separate secondary-control view; light/hard transition and
physical-region views; category, transition, region, ECAL-outcome, and clean-
safety summaries.

The next diagnostic should compare the 44 worsened topology-clear changes,
especially the 30 ordinary-core degradations, with successful hard transition-
3--6 changes using cluster matching, p/E, energy closure, angle, and surviving
component score. Do not tune on this evaluated sample and then call the result
validated: freeze any proposed revision and test it on an independent new
broad sample.
