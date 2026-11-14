# Full-angle ECAL photon-response accounting — 2026-11-15

Read existing native REC_ECALTRUTH files, not simulation/reconstruction/refits.
Same 500 files /100000 input events and198081 truth-covered selected tracks as
the earlier photon-birth-normalized collection tables; the349 selected tracks
without an electron ECAL entry remain excluded identically. Freeze the existing
55.75722694396976GeV neutral cap and76.59788360595702GeV charged cap. Do not
change BP/GSF/PFA code, cards, installation or production.

For each covered event inspect EVERY pre-reclustering ECAL cluster, every PFO
owner, and the full native cluster-to-MC response map. Generator-tagged FSR and
G4 eBrem births before the selected electron's first ECAL entry define origins.
Native parent traversal is the same as the earlier tables, retaining photon
energy that converted/absorbed before ECAL in the BIRTH truth denominator.
Count every cluster once, including both selected-electron neighbourhoods.
No generator recoil plot or fitted-track-success selection.

## Disjoint accounting

All finite positive-energy ECAL clusters are partitioned into:
- Selected input-track matched charged clusters retained/rejected by the frozen
  charged cap. Matching uses exactly the chosen CompleteTracks input index.
- Eligible neutrals inside25/200mrad theta/phi HALF windows, additional neutrals
  inside200/200, additional neutrals at ANY phi but theta<=200mrad, and neutrals
  at larger theta offsets. The angular test is a union over the covered selected
  tracks' Baseline KF AtCalorimeter position rays from nominal IP.
- Neutrals removed by the frozen neutral cap, across all angles.
- Other charged clusters and other charge-zero track-associated clusters.
- Positive-energy clusters failing hit/position validity, reported separately.
Nonfinite/nonpositive cluster energies have no positive response assigned and
are counted separately; their physical deposits cannot be inferred here.

Large-window collections additionally test200/1000 and1000/1000mrad half-width
rectangles, a1000mrad 3D opening-angle cone around either selected track's ECAL
ray, and unrestricted full angle. These cones are collection cones around the
electron references, distinct from prior isolation cones around neutral
candidates. The full-angle totals include barrel and endcap ECAL clusters.

Numerator=sum of measured cluster E times its normalized native origin fraction;
denominator=sum of corresponding photon BIRTH E on the unchanged truth cohort.
These are energy ratios of sums, not per-cluster mean purity. Main-electron
energy never enters the FSR/eBrem numerators. Native MC response maps are not
independently validated spatial deposited-energy truth; inherited projection
labels are not demonstrated wrong. Raw cut-rejected energy, including known
extreme response outliers, is diagnostic and is not validated recoverable energy.
No HCAL or original simulated ECAL deposit closure is claimed.

A secondary diagnostic applies the same existing charged cap to OTHER charged
clusters when adding them to collection curves. This is explicitly an extension
for comparison, not a newly fitted percentile or a BP algorithm change. Raw
other-charged response and its cap split are both retained in summary.json.

## Verification and artifacts

read.py checks every source identity before/after reading; native IDs, indices,
finite nonnegative weights, parent graph cycles, unique cluster PFO ownership,
per-cluster sums and exact geometric masks. Every job reproduces the old25/200
neutral origin sums and raw matched-charged origin sums, and its truth sums.
Full groups partition all finite positive ECAL response without double counting.
Boundary/phi-wrap self-checks are in window_selfcheck.json. analyze.py verifies
all500source identities and global old-table closure again.

verify.py independently recomputes response/group/window sums from saved target-
bearing cluster rows using scalar math.fsum. Fresh native checks of examples
independently traverse MC parents iteratively, compare attributed fractions,
and inspect PFO charge/tracks and each track's leading MC hit-vote association.
Other charged clusters can include photon-descended electron/positron tracks
AND alternative reconstructed tracks of the production electron; examples do
not measure the relative population fractions of these mechanisms.

Files: jobs/*json and *_clusters.csv, read.py/analyze.py/verify.py, summary.json,
energy_budget.csv, collection_windows.csv, examples.json, verification.json,
plot_files.json, and plots/*.png/pdf. Scripts/data/plots/manifests uncommitted.
Gallery update uses one plot-gallery sub-agent, adds only the two new plots,
and preserves all106 priorPNG/PDF hashes and all18sectionobjects/removals.
Expected final79PNGs/19sections/31PDFs; HTTP/hashchecks in gallery_verification.json.

## Results on all 100,000 input events

Every one of500jobs reproduced prior25/200neutral-origin sums and raw matched
charged-origin sums. Truth denominators remain FSR424983.0044955895GeV and
eBrem1233162.672157788GeV. All4280406pre-ECAL clusters in covered events are
inspected;13609 have nonpositive/nonfinite measured energy. Positive response
in invalid-hit/position or charge-zero track-associated groups is zero for
the tagged origins. Count4280406 covers99988events with at least one common
truth-covered selected track; input count remains100000.

|Response component (% of corresponding photon birth energy)|FSR|eBrem|
|---|---:|---:|
|Current25/200neutral collection|35.7364|21.6285|
|Matched charged, retained|23.3200|58.1048|
|Additional neutrals: theta25→200, phi<=200|7.8773|0.1143|
|Additional neutrals: theta<=200, phi beyond200 to fullphi|10.4219|1.9820|
|Additional neutrals: theta outside200, fullphi|9.9495|0.1762|
|Other charged clusters, raw|4.3655|10.7491|
|Matched charged rejected by study cap|0.8635|4.6167|
|Neutral clusters rejected by study cap, fullangle|5.6358|3.4136|
|ALL raw positive ECAL response|98.1700|100.7853|
|Signed birth-minus-response residual|+1.8300|-0.7853|

Thus extra full-angle eligible neutrals contain28.2488FSR and2.2725eBrem
percentage points. Other charged clusters contain4.3655/10.7491points. Study
cap-rejected clusters contain6.4992/8.0303points. These rows account for almost
all of the earlier apparent deficit. The negative eBrem residual is overresponse,
not negative physical loss or proof of an energy-conserving truth closure.

With matched charged retained and neutral caps still applied, the1000mrad
electron-centred collection cone gives82.7757%FSR/81.9566%eBrem. Fullangle
gives87.3052%/82.0058%. Adding other charged ORIGIN-attributed response under
the SAME charged cap gives87.1375%/92.6316%for the cone and91.6671%/92.6808%
for fullangle. This is truth accounting, not an implemented photon-recovery
algorithm. No full other-charged p4 or unrelated cluster energy is added.

The raw other-charged response above76.59788360595702GeV accounts for only
15.6384GeVFSR/.00368points and913.9281GeVeBrem/.07411points; this split is
retained. Raw total response includes known outlier clusters and should not
replace the user's frozen trims in reconstruction comparisons.

Unique eBrem birth-to-MC matches267049/338509births account for1212080.4929GeV;
unmatched71460births contain21082.1793GeV(1.7096%of total eBrem birth energy).
This is an attribution-completeness limit and overlaps the response accounting;
it is not an additional disjoint deficit term to add to the table.

The main finding is where native tagged response sits: FSR mainly outside the
original angular window; eBrem mainly in other charged and cut-rejected clusters.
It does not establish true photon flight angles from reconstructed cluster
centres, independent spatial purity, physical material loss, or calibrated
photon recovery. Original simulation-deposit/HCAL closure was not available.

## Completed independent verification

347175savedclusterrows reproduce both origins' disjoint-group response and all
6collection-window sums via independent scalar per-file math.fsum. All500source
identities, old-window/raw-charged closures, truth denominators and 4plot hashes
pass. Geometric boundary/phi-wrap checks pass; all3scripts compile. Both final
renders reviewed. The scalar checker accumulates per-file sums to bound memory.

Four fresh native checks reproduce origin fractions and cluster energies:
- 284/11/3: other charged,59.9438GeVFSR response; associated track27's leading
  MC408 is apositron descended from taggedFSR photon18, not a production leg.
- 147/172/0: other charged,75.8963GeVeBrem response; associated track4's leading
  MC7 is the productionelectron, but the chosen input track for that MC is3.
  This is an alternative reconstructed track, not evidence of a conversion.
- 347/76/38: neutral outsidephi200at theta<=200;55.5685GeVFSR response.
- 476/123/52: neutral outsidephi200at theta<=200;55.7292GeVeBrem response.
Indices are job/event/cluster, event/cluster zero-based. These focused examples
establish mechanisms, not their population fractions. Two earlier pilot native
checks are additionally in pilot_native_verification.json. Verification.json
contains all final native track/MC/ancestry details.

## Project-memory retention

Complete outgoing AGENTS.md is saved in
2026-11-15-agents-before-full-angle-ecal-photon-accounting.md. Global status,
active laws and essential compile/run instructions remain byte-identical; only
current focus is replaced. Analysis scripts, tables, logs, plots and gallery
manifest remain uncommitted. No source/card/build/install/default/production
changes or full simulation/reconstruction jobs occurred.

Gallery complete:79PNGs/19sections/31PDFs/110images. All106prior image hashes
and18sectionobjects unchanged; both newPNGHTTP200 andsource/local/servedhashes
match. Sameailogin002localhost8000PID443262, no restart. Main independently
rechecked old/new local hashes and manifest preservation. Existing removed
sections/variants remain absent.
