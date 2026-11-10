# New BP matched-sample recoil and pT plots

The user requested recoil mass and pT resolution from the completed new BP
flats while the matched GSF rerun runs. No fit/production/source/library/card
changes or submissions are part of this analysis. Artifacts are uncommitted at
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/`.

Input snapshot:all200 completed jobs from
`higgs_eeHX_ISRFSRON_BP_TPCFIX_200jobs_50evt_20261110`, source REC jobs1--200,
entries0--49,10,000 events. File size/mtime identities are in snapshot.json.
Only new TPC-fixed data are used. Baseline KF means CompleteTracks. The new
matched GSF campaign has200 submitted markers,77 started,0 complete,0 failed
at the bounded status check; running paths remain frozen. That is a historical
point-in-time marker check, not a scheduler inventory.

## Estimator and truth definitions

Main curves use the existing complete scan110 minimum-NLL location selection
(kinds1/3 only, all candidates valid and same observation signature), with the
existing per-electron calorimeter fallback if that check fails. Kind0 null and
kind2 truth-only sites are not ranked; no new location policy or refit is made.
Successful scans use their own saved full IP directions. A fallback uses KF
directions and either KF plus neutral energy, charged/KF inverse-variance pT
weighting, or KF retention. The shared-neutral guard preserves successful
scans and prevents a new additive fallback from double-adding one cluster.
No truth-location/energy-override curve or simple-only extra method is shown.
Strict complete-scan pT/recoil controls use separate identical restricted
populations for Baseline KF and BP. Main and strict populations must not be
compared as if they were the same cohort.

pT truth is each selected stable generator electron after generator FSR and
before tracker material. Recoil is nominal240GeV minus the two electron
four-vectors with electron mass. No generator recoil/residual curve is drawn.
No extra photon is added to the pair after the estimator's own recovery.
Invalid masses stay in the comparison denominators. No mass/pT/angle physics
cut is applied; recoil requires reconstructed opposite charges and both selected
partners. Angular diagnostic bins use KF angle and N>=30. No Gaussian peak fit,
tail-survival or radiation-quantile figure is made.

Material-eBrem labels use positive recorded photon birth energy and the matched
primary electron's saved G4 ID, with interval-truth ID fallback if necessary.
FSR labels use persisted generator-origin links with unique final-electron MC
ownership, never angular/process inference. No additional photon-energy cut.
Unresolved ownership is unknown rather than neither; absence refers only to
recorder coverage. pT uses each track's flags; event flags OR over both selected
electrons. This is the same raw recorded-radiation convention as the GSF study.
Category differences alone do not isolate causal FSR effects.

Detailed definitions, CSV schema, estimator/fallback policy and script invocation
are retained in the per-analysis README. No old deleted physics files or old
GSF fit results are used in these plots; the independent overlap check reuses
only cached, checked REC truth labels on identical REC identities.

## Verified sample and numerical results

All200 completed jobs /10,000 source events are included. There are19,855
selected reconstructed tracks:145 source events have only one selected
reconstructed partner. Twelve additional pairs have the same reconstructed
charge, leaving9,843 opposite-charge pairs. All selected tracks have usable
Baseline KF and composite estimates. Strict complete-scan counts are17,300
tracks and7,478 pairs. Every strict scan has actual mask3 and source110.

The main estimator keeps17,300 scans; fallback routes are1,110 neutral
additions,1,350 charged-energy weighted estimates,95 KF-retained tracks.
There are1,619 incomplete scans and936 absent candidate scans. Fallback
availability is not fit efficiency. No shared-neutral pair was found.

| Track category | Tracks | Baseline KF central68% [%] | BP + fallback central68% [%] |
| --- | ---: | ---: | ---: |
| neither | 2,115 | 0.2084 | 0.6248 |
| ebrem only | 5,878 | 8.2991 | 1.2853 |
| fsr only | 3,065 | 0.2068 | 3.5675 |
| both | 8,797 | 8.5207 | 4.6744 |

Inclusive central68% half-width:4.5362% KF versus2.3209% composite BP.
RMS worsens from21.4758% to31.8565%; absolute pT error above100% rises from
0.0151% to1.6923%. Narrower central width does not establish tail safety.
On the different strict-only population, widths are3.1549% KF versus1.9178%
scan110. This is not a same-cohort comparison to the main population.

Truth pT refers to the stable outgoing generator electron after generator FSR
and before material loss. The split shows eBrem-only central recovery and
strong FSR-only/no-eBrem broadening relative to that bare-electron reference.
These category populations are not kinematically matched and no quantile
uncertainty/causal test is performed.

Undefined recoil counts in the main9843-pair population are2 KF and104
composite BP. On7478 strict pairs they are1 KF and3 BP. Undefined masses
remain in each histogram denominator. Four event groups contain118 neither,
1507 eBrem-only,584 FSR-only and7634 both. Unknown truth labels are zero.

All400 BP/REC file identities remain unchanged, track IDs are unique, and
input-event, track/pair, routing and category accounting closes. Recorded
BP radiation labels agree on4571 common selected tracks with an independent
previously checked truth cache from the identical REC file identities.
Sixteen PNG/PDF stems match. Recoil core/category and pT category figures were
visually inspected; footer margins and fallback-direction titles were corrected
using cached CSVs, without changing data/statistics or rereading ROOT inputs.

## Gallery verified

The same dedicated personal plot-gallery agent reviewed the README/plots,
organized gallery.json and validated all16 PNGs exactly once across8 sections.
It safely stopped the previous managed GSF gallery PID48142 and launched this
BP directory on ailogin002.ihep.ac.cn,localhost8000,PID384311. Proxy-bypassed
page and PNG checks returned200(image/png for the PNG). Main review corrected
wording to distinguish native BP scan directions from KF fallback directions,
and estimator ECAL/neutral recovery(which can include FSR) from any additional
photon addition to the recoil. The availability section does not claim pure
fit publication efficiency. Validation and page/PNG200 checks pass after those
manifest corrections. The main agent reviewed exact-once coverage and the
inclusive/category pT/recoil figure layouts.

URL:http://127.0.0.1:8000/ on that host. Remote forwarding:
`ssh -L 8000:127.0.0.1:8000 zhangcg@ailogin002.ihep.ac.cn`.
No production runtime or job was affected by the plotting-server handoff.
No matched GSF-versus-BP performance claim is made until the new GSF sample is
complete and compared on identical event/track populations.
