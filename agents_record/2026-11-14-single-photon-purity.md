# Individual-photon purity and reconstructed selection — 2026-11-14

## Corrected question

The user wants neutral photon candidates whose cluster energy comes almost
entirely from one photon. Summing FSR and eBrem is insufficient: a cluster
containing 60% FSR and 40% eBrem is mixed, despite being photon-only. Two
different eBrem photons or two different FSR photons also remain separate.
Shower descendants of the SAME original photon belong together. Main-electron
showers classified as neutral PFOs are background for this task.

Define each cluster's purity as the largest individual tagged-photon response
weight divided by the sum of ALL native cluster-to-MC response weights. The
corresponding attributed energy is that fraction times reconstructed cluster
energy; its denominator is the full measured cluster energy. This is a native
MC response-attribution proxy, not an exact deposited-energy measurement.
The target is individual cluster purity >=95%. Origin labels never combine
different photons to meet this target.

## Inputs and identity attribution

Read all 500 CURRENT REC_ECALTRUTH files, 100000 events, from
`higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112/`.
Keep the same 55391 eligible unique baseline neutrals as the previous radius
study: hit-supported pre-reclustering, charge-zero trackless PFO clusters,
positive finite energy/position, selected production Z->ee electron windows
with theta/phi half-widths 25/200 mrad. Overlaps assigned once to the nearest
passing track. Retain the fixed upper-2% candidate energy cap 55.75722694396976
GeV. No event radiation-category, recoil-window or fit-validity gate.

Native `EcalClusterPreReclusteringParticleAssociation` weights and MC ancestry
are read afresh. FSR roots use generator `GsfGeneratorFsrLinks` status 1 and a
selected final electron owner. eBrem roots use saved G4 subtype-3 photon births
from selected electrons before their first ECAL entry, matched uniquely to MC
photons by their exact saved birth momentum and vertex. Each root's MC index
stays distinct; descendants are followed through MC parent links. Multiple-root
ancestry or an MC root mapping to distinct G4 births is ambiguous and never
credited to one photon. No such ambiguity was found in these candidate weights.
No angular match substitutes for native truth links.

eBrem birth-to-MC matching covers 121541/152534 births and
691705.0394/699978.0444 GeV, in the neutral-candidate-bearing events traversed.
These are NOT full-production photon totals. Unmatched births are not
identified photons and cannot increase the certified single-photon fraction.
The positive labels therefore concern identifiable saved roots, not complete
detector-wide photon truth. Photons outside the selected Z->ee scope remain
background. All 500 file sizes/mtimes are checked before and after reading.

## Corrected truth counts and examples

Of 55391 eligible neutral clusters:

| Definition | Count |
|---|---:|
| Summed FSR+eBrem response >=95% |6339|
| ONE photon response >=95% |3631|
| Dominant photon is FSR |1640|
| Dominant photon is eBrem |1991|
| Aggregate passes, individual photon fails |2708|

Every one of the 2708 aggregate false positives has at least two identifiable
photon roots. Example keys are (source job, zero-based local event, cluster):

- (1,112,24): 41.3335% FSR plus 58.4249% eBrem. Largest individual photon
  contribution is 58.4249%; fails.
- (1,66,23): 99.9434% summed eBrem, but the two largest individual photons
  contribute 50.1724% and 49.7408%; fails.
- (1,66,22): 99.9465% summed eBrem, but largest photon only 93.3166%; fails.
- (3,4,15): main electron shower 99.9867%, no tagged-photon contribution;
  isolation I300=1 does not make it a photon candidate.

All 3631 passing cluster IDs and dominant photon MC indices are saved in
`TrackingPerformanceStudies/single_photon_purity_20261114/truth_selected95.csv`.
`photon_contributions.csv` retains every individual photon fraction, not only
the winning photon. Keys always include the REC source job and local event.

## Reconstructed selection attempt

Re-evaluate distance/isolation against this corrected truth target. The old
R300/I>=0.46 working point optimized SUMMED radiation >=90% and does not
establish a single-photon classifier. The old conditional-species and distance
plots likewise measure contributions grouped by origin, not one-photon purity.

Split source jobs modulo 5: values 0/1/2 development (33029 candidates), value
3 calibration (11018), value 4 evaluation (11344). All event clusters stay in
one split. These are held out from THIS new rule search, not an independent
fresh production: earlier aggregate studies saw this same production.

Search an interpretable beam of 40 rules, up to four reconstructed cuts:
energy, distance to track entrance/main charged cluster, theta/phi displacement,
charged reference availability, inclusive isolation at R25/50/75/100/150/200/
300/400 mrad and neighbour counts. No truth/identity/origin value is a feature.
Require >=100 development and >=50 calibration candidates. Rank development
rules by Wilson-95% lower bound. Prefer a calibration rule reaching >=95%
true-positive fraction; none does. Freeze the highest calibration lower-bound
rule BEFORE evaluating. This finite beam search does not exclude better rules.

Frozen rule: track-entrance 3D distance >=300 mm AND I300>=0.7.
I300 = candidate E / (candidate E + all OTHER valid pre-ECAL cluster E within
300 mrad); includes main charged clusters and excludes only the candidate.

| Population | Selected | Individually >=95% | Fraction passing | Mean individual purity |
|---|---:|---:|---:|---:|
|Development|193|149|77.20%|—|
|Calibration|50|41|82.00%|96.29%|
|Evaluation baseline|11344|752|6.63%|49.17%|
|Frozen evaluation selection|61|48|78.69%|96.12%|

Evaluation retains 48/752=6.383% of true >=95% candidates and 2.771% of
dominant-photon-attributed reconstructed energy. Wilson-95% lower bound on its
passing fraction is 66.88%. Mean purity 96.12% MUST NOT be described as 95% of
selected clusters passing the individual purity requirement. The cuts enrich
clean photons, but do not establish a dependable >=95%-passing classifier.
No online/default selection is installed; no recoil improvement is tested.

## Checks, plots and preservation

The fresh native read reproduces every old grouped FSR/eBrem and main fraction;
all old candidate and geometry/isolation CSV fields are exact. Independent
CSV aggregation verifies per-root sums/maxima/second fraction, origin sums,
all 3631 truth IDs, all 2708 mixed false positives and frozen evaluation counts.
A separate iterative ancestry walk checks raw native links for the four plotted
examples. New logic includes ancestry, ambiguity and distinct-root self-checks.
`verification.json`, `read_summary.json` and `selection_summary.json` preserve
the checks and source identities. Four final renders are reviewed:

1. Individual-photon fraction versus summed radiation fraction.
2. Individual-photon purity versus inclusive isolation on evaluation only.
3. Passing-fraction/retention tradeoff for reconstructed cuts.
4. Actual single-photon, cross-origin, same-origin and electron examples.

Artifacts are uncommitted under
`TrackingPerformanceStudies/single_photon_purity_20261114/`.
Gallery publication adds these four plot pairs and explicit origin-sum warnings
to three legacy section descriptions. Existing images stay unchanged; both
user-removed sections remain absent. See `gallery_verification.json` for the
live-server/hash audit after publication. No source, REC, run card, fit,
installation or production-submission change.

The complete outgoing AGENTS focus is preserved in
`2026-11-14-agents-before-single-photon-purity.md`; older isolation results stay
valid for their original aggregate target only. Next physics question is how
to distinguish one compact photon shower from overlapping showers using more
reconstructed information; the present scalar isolation cuts have not solved it.
