# BestBranch/Baseline KF plots split by recorded radiation

The user requested that current comparison plots show only GSF BestBranch and
Baseline KF, then separate tracks/events with and without FSR and eBrem.
The user also explicitly requested the name **Baseline KF** for CompleteTracks
and no generator recoil curve unless asked. This record supersedes the display
policy in `2026-11-10-finished-gsf-recoil-pt-plots.md`; its initial numerical
extraction/closure evidence remains historical. The complete outgoing AGENTS
is preserved in `2026-11-10-agents-before-gsf-radiation-plots.md`.

## Scope and definitions

Reuse the immutable 87-job / 17,400-event completed GSF snapshot and its matched
new TPC-fixed REC inputs, under
`TrackingPerformanceStudies/gsf_finished_recoil_pt_20261110/`. This is a
runtime-selected subset, not a campaign-wide validation. Runtime cards,
libraries, production outputs and running jobs are unchanged. WeightedMean and
FullMixtureMode are removed from current figures. Generator truth remains the
pT-residual reference; generator recoil and recoil-residual figures are absent.
Old figures are preserved under `.analysis_archive`, outside the gallery root.

Per-track categories are neither, eBrem only, FSR only, and both. Use any
positive-energy recorded photon, with no additional analysis energy threshold:

- FSR ownership comes from the unique generator-history link in
  `GsfGeneratorFsrLinks`, connecting `GsfG4FsrPhotons` to the stable outgoing
  electron MC index. Validate generator photon PDG/status and energy closure.
  There is no angular or G4-process inference of FSR.
- Material eBrem comes from `GsfG4BremsPhotons`, with exact parent material-step
  relations, a primary electron/positron parent, creator subtype3, and birth
  inside the recorder's tracker bounds. Map the primary G4 track ID to MC via
  saved SimTrackerHit provenance and its MCParticle relation; require a unique
  owner and correct charge/PDG. This identifies recorded photons independently
  of fitter/location-scan eligibility. The producer reads exact G4 secondaries
  and applies no explicit photon-energy threshold; its generic MCParticle
  secondary threshold does not censor this separate photon collection.

"Without" therefore means no such recorded photon within recorder coverage,
not proof of zero radiation. Missing/ambiguous ownership is unknown, never
silently neither. Event categories OR each radiation flag over the two selected
reconstructed electrons; event-level both may involve different legs. Track
and event denominators are different and explicitly displayed.

pT comparisons use identical usable tracks within each category. Recoil
comparisons use identical complete pairs, keep undefined masses in histogram
denominators, and retain the user-approved **GSF BestBranch pT + Baseline KF
phi/tanLambda for both legs** approximation. Full GSF directions are absent for
the second track in this flat schema. Recoil is nominal240GeV minus two bare
leptons, with no FSR recovery. No Z-mass, angular or pT selection is added.

Scripts, CSV/JSON data, plots and gallery manifests are uncommitted analysis
artifacts. Durable definitions/results are recorded here.

## Radiation split results

Central68% pT half-widths below use identical usable tracks in each category.
Selected counts include GSF publication failures; common counts do not.

| Track category | Selected | Common usable | Baseline KF width [%] | BestBranch width [%] |
| --- | ---: | ---: | ---: | ---: |
| Neither | 3,521 | 3,393 | 0.2090 | 0.2154 |
| eBrem only | 10,189 | 9,388 | 5.4209 | 1.7073 |
| FSR only | 5,472 | 5,288 | 0.2075 | 0.2132 |
| Both | 15,335 | 14,107 | 5.8095 | 1.8822 |

The width reduction is concentrated in categories containing material eBrem;
neither and FSR-only have slightly larger BestBranch widths on this subset.
These are descriptive quantiles, without uncertainty estimates or kinematic
matching between different truth categories.

| Event category | Reconstructed pairs | Common complete pairs | Invalid recoil KF / BestBranch |
| --- | ---: | ---: | ---: |
| Neither | 174 | 162 | 0 / 0 |
| eBrem only | 2,537 | 2,187 | 1 / 23 |
| FSR only | 1,011 | 944 | 0 / 0 |
| Both | 13,399 | 11,577 | 3 / 115 |

All 34,517 selected tracks and 17,121 selected pairs have unambiguous labels;
unknown counts are zero. The category common totals close to 32,176 tracks
and 14,870 pairs. Photon relations, parent identity/PDG, FSR generator energy
closure, source identities, category totals and unique row joins all pass
assertions.

## Verification and reproducibility

Eighteen PNG/PDF pairs are complete: ten inclusive/coverage/recoil views and
eight radiation population/availability and pT/recoil category views. All
original job/event/track identities remain in the CSVs. Final independent
checks pass all174 GSF/REC size+mtime identities, unique track-label joins,
photon-count/flag equivalence, common sample closure, and18 matching PNG/PDF
stems. Extraction validates all source photon relations, material parent
identity/PDG/process, and generator photon energy/status. The first three
source events (six tracks) were manually inspected before full extraction.
The main agent visually inspected inclusive core/recoil and four new figures:
population, pT core, quantiles and recoil peak. No reconstruction or refit was
rerun; no current input, runtime card or installed library changed.

No bootstrap/quantile uncertainty calculation or pT/angle matching between
truth categories is performed. ECAL steering is off. These plots do not
establish a causal FSR effect or physics validation of either BH model/GSF.

## Gallery refresh verified

The existing dedicated plot-gallery agent updated gallery.json and validated
all18 PNGs exactly once across8 sections. It reused the healthy managed
PID48142 on ailogin002.ihep.ac.cn, serving the same directory; no restart was
needed. Proxy-bypassed HTTP checks returned200 for the page and200 image/png
for recoil_radiation_peak.png. The page includes the radiation section and
BestBranch/Baseline KF labels, with WeightedMean absent. The main agent reviewed
the new manifest and figure categories. URL:http://127.0.0.1:8000/ on that host.
Remote access uses
`ssh -L 8000:127.0.0.1:8000 zhangcg@ailogin002.ihep.ac.cn`.
