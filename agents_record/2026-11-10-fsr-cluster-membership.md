# Generator FSR membership in pre-reclustering ECAL clusters

## Authority and scope

The user asked how many FSR photons enter the track-matched charged cluster
and how many form separate neutral clusters. This is an offline truth study
on existing radiation-on REC events. No simulation, tracking, PFA or fitter
implementation, original run card, installed library, branch or batch job is
changed. Temporary analysis cards replay the existing PFA code only; no new
REC production or BP/GSF fit runs. The complete outgoing AGENTS is preserved
byte-for-byte in [the snapshot](2026-11-10-agents-before-fsr-cluster-membership.md).
Earlier plot decisions/results remain in
[the matched comparison record](2026-11-10-matched-gsf-bp-overlays.md).

## Available truth and measurement definitions

The current REC retains generator-origin FSR tags, original-photon transport
checkpoints, digitized calorimeter hits, their MC associations, and pre-PFO/
cluster snapshots. Raw simulated ECAL/HCAL hits were pruned previously; this
study does not regenerate them. The cluster snapshots create new synthetic
3D hit objects with display cell IDs and no original-hit/MC truth relation.
The BP neutral event tree exports photon truth separately, against final PFO
neutral diagnostics; it does not contain photon-to-precluster truth links.
Thus nearest-angle matching alone cannot establish photon shower membership.

FSR identity is the saved generator-origin status1 assignment to the final
stable production electron. It is not inferred from direction or a G4 creator
process. The simulation preserves original generator MC indices, then appends
saved secondaries. IsTrk2Primary=False labels hits by retained MC ancestors;
follow the saved MC parent graph to include FSR conversion/shower descendants.
This separates FSR showers from tracker material photons of the primary electron.

The existing PFA WriteAna diagnostic exposes per-cluster MC contribution
weights. A temporary card reads REC calorimeter/tracking inputs, stops after
TrackClusterConnectingAlg, before PFOReclusteringAlg, and enables WriteAna.
Its independent pre-PFO/ECAL snapshot is checked against the existing REC.
The existing code is called unchanged; only analysis steering is temporary.
No MC angular matching supplies an association.

## Reproduction gates and limits

The first two events of REC job1 reproduce68 exported snapshot leaves exactly,
including NaN patterns, and yield6 generator-tagged FSR photons. A larger
check reveals collection permutations and some genuinely differing cluster
constituents. Comparing row numbers is therefore insufficient.

The maintained analysis matches exact cluster energy/position and the full
multiset of hit cellID/type/energy/xyz, then checks the PFO track-index set and
charge. All classifications use the ORIGINAL stored cluster indices and PFO
relations. An event whose clusters/ownership cannot be reproduced remains
unresolved in the photon denominator; its replay output is not substituted
for the original. No pre-existing PFA code is changed to force reproduction.
No cause of the remaining replay differences has been established.

Existing debug MC-weight rows omit MC indices. Match their exact float32
PDG/momentum/endpoint representation to original MC candidates, accepting only
identical FSR-ancestor sets across candidates. PDG is also float32 in these
legacy debug rows, important for large nuclear PDG codes. Generator FSR identity
still comes from the explicit generator index. Ambiguity is never guessed.

A positive normalized MC weight means a photon contributes to that cluster;
it need not dominate it. Keep parent-charged-only, neutral-only, both, other
charged, no retained cluster signal and unresolved categories. A separate
FSR-dominated neutral cluster requires that photon and its descendants supply
more than50% of its MC-labelled content. Neutral means charge0 and no tracks;
this is a separate reconstructed object, not an angular isolation cut.

Cluster energy times normalized MC weight is a reconstructed-response proxy,
not raw deposited energy or true photon energy. The actual saved BP charged
cluster indices and nominal neutral collection indices are also inspected.
Being in the neutral collection window does not prove the observation was
applied in a successful110 fit, particularly for null hypotheses.

## Fixed population and artifacts

The declared sample is1000 events: REC jobs1,11,...,191, first50 entries each,
from higgs_eeHX_ISRFSRON_REC_TPCFIX_500jobs_200evt_20261110. It is a fixed subset
of the200-job BP comparison, with no BP/GSF fit-success selection. Photon scope
is FSR belonging to reconstructed selected production-electron tracks. Missing
reconstructed partners are not silently counted as associated tracks.
All photons are counted; cumulative thresholds10MeV,100MeV,1GeV and5GeV
show the effects of infrared/soft populations separately. Parent material-loss
bins retain f<1% versus f>=1% from the existing exact truth-energy cache.

Artifacts are uncommitted under
TrackingPerformanceStudies/fsr_cluster_membership_20261110/. replay.py uses
only the original PFA implementation; run_sample.py creates the fixed native
replays. analyze.py validates physical cluster/track identity and traces MC
ancestry. summarize.py checks every selected-track FSR birth-energy sum
against the existing matched cache, then writes photon/cluster CSVs and counts.
The native snapshots contain only pre-PFO/ECAL diagnostic collections and are
not replacement REC files. The original plotting directory is untouched.

The native run initially used a strict raw-row equality gate, whose failures
are preserved in sample.log. The corrected analysis permits collection permutations for physical ECAL
objects while requiring identical cluster content and track/charge ownership;
genuinely unmatched events remain unresolved. Native ROOT outputs
are reused for that analysis correction, without another PFA execution.
Population completion, numerical results and final gates are recorded below.

## Completed population and results

All20 native PFA executions completed50 events each. The initial driver exited
nonzero at its strict raw-row comparison, not at PFA execution. The original
failures are preserved in initial-row-order-gate.json/sample.log; the final
sample_jobs.json records canonical validation.928/1000 events reproduce exact
physical ECAL cluster/hit content and track/charge ownership.72 remain
unresolved, with131 FSR photons; their replay outputs are not substituted.
No cause of the remaining differences is established and no older code is fixed.

There are1979 selected reconstructed electrons and1800 tagged FSR photons;
1669 assignments resolve. At Eγ>=1GeV:420 photons,387 resolved,33 unresolved.
| Photon outcome (overlapping) | All | Eγ>=1GeV |
| --- | ---: | ---: |
| Own-electron charged cluster | 782 | 299 |
| Any separate neutral cluster | 472 | 303 |
| Both charged and neutral | 333 | 224 |
| FSR-dominated neutral (>50% MC content) | 240 | 206 |
| Both charged and FSR-dominated neutral | 143 | 135 |
| Unresolved replay | 131 | 33 |

The exclusive all-photon classes are449 own-charged/no-neutral,139 neutral/
no-own-charged,333 both,22 only-other-charged,723 no retained usable signal,
3 only-invalid-cluster,131 unresolved; sum1800. Energetic classes75,79,224,8,
1,0,33; sum420. FSR-dominated neutral means >50% MC-labelled content from
that photon/descendants, not mere contamination.240 photons yield276 distinct
such neutral clusters;206 energetic photons yield242 clusters. Some photons
fragment into multiple objects.143(135 energetic) also reach the parent charged
cluster. Counts are photons unless explicitly labelled clusters.

Among resolved energetic photons:77.3% own-charged,78.3% any neutral,57.9% both,
53.2% with an FSR-dominated neutral cluster. Do not extrapolate these survey
fractions to all10000 events or present them as validated reconstruction
probabilities. Contributions to both categories are common, not a binary fate.

MC-attributed reconstructed-response shares(all/energetic): own charged26.9/26.0%,
neutral68.3/69.2%, other charged4.8/4.9%. This proxy is not raw deposited energy
or true photon energy. Actual BP nominal neutral collection includes283 photons,
175 energetic; this does not prove application in a valid110 fit. Parent no/light
energetic photons223(208 resolved):157 own charged,161 any neutral,113 neutral
majority. Parent hard197(179 resolved):142 own charged,142 any neutral,93
neutral majority. Parent categories retain the prior exact material-loss fraction.

## Verification and retained limitations

Unique1800 photon identities, all selected-track FSR birth-energy closure,
usable-link positivity/finiteness, category/response accounting, neutral-majority
flags,20 unchanged REC size/mtime identities and canonical matching edge cases
pass. Synthetic checks preserve cluster-index permutations and reject changed
constituents/track ownership; legacy float32 nuclear-PDG handling also passes.
Non-finite-energy or empty clusters are excluded as unusable, matching BP
measurement requirements. Three photons have no other usable signal; mixed
valid/invalid cases retain only usable contributions. None are silently labelled
as successful usable-cluster associations.

The source/replay gate is deliberately conservative: any unresolved physical
cluster/ownership mismatch makes all FSR assignments in that event unresolved,
even if a particular electron might be unaffected. Direct persisted precluster
MC weights are absent, so exact assignment for those72 original events is not
established by this study. It remains outside scope to modify the older PFA or
infer assignments from angular proximity. Generator labels, recorded ancestry
and existing PFA MC-weight diagnostics supply all accepted assignments.

Artifacts and complete tables are under
TrackingPerformanceStudies/fsr_cluster_membership_20261110/README.md,
summary.json, photons.csv and photon_cluster_links.csv. validation.json,
runtime-sha256.json, per-job analysis.json and event_validation.csv preserve the
checks. Native snapshots/debug outputs occupy about578MB; they are generated,
uncommitted analysis artifacts. The original eight plots, source REC/BP/GSF
files and production/runtime paths remain unchanged.

## Project-memory coverage

The outgoing current-focus heading/content maps to the complete dated snapshot
and the prior matched-comparison record. Global status, all active laws/scope
and essential compile/run instructions are byte-identical. AGENTS retains
exactly two substantive sections and now names this FSR study as the current
focus. No history directory was renamed or migrated; no history file was
removed, so a directory-move manifest is not applicable. All new live links
resolve under agents_record/. Analysis/native outputs remain uncommitted.
