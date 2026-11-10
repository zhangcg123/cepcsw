# Finished GSF snapshot: pT and recoil plots

Historical initial figures and display policy. The user subsequently requested
BestBranch vs Baseline KF only, no generator recoil, and FSR/eBrem splits.
Current definitions/results/gallery are in
[the superseding record](2026-11-10-gsf-bestbranch-radiation-split.md).

The user requested recoil-mass and pT-resolution plots from finished GSF
tuples and a gallery on port8000. Runtime/fit files remain frozen and unchanged.
Only NEW normal-ISR/FSR TPC-fixed inputs are used. The analysis is under
`TrackingPerformanceStudies/gsf_finished_recoil_pt_20261110/`. Scripts, CSVs,
JSONs, PNG/PDFs and gallery manifest there are uncommitted analysis artifacts.

## Frozen population and state recovery

Snapshot time: 2026-11-10T10:34:23.658509+00:00.87 verified completed jobs,17,400 events.
Job/input identities and size/mtime snapshots are in snapshot.json and remain
unchanged after analysis. Later completions, failed jobs and partial outputs
are excluded. Completion/runtime selection can bias this subset.

Generator pT and CompleteTracks AtIP states are joined from each matching REC
using the tuple's original production-electron MC/track indices. Legacy scalar
mc_*/lcio_* fields are not a valid two-electron truth/reconstruction pairing.
Both tracks' BestBranch pT is recovered from the maximum-weight final component;
WeightedMean pT uses the inverse absolute weighted SIGNED curvature, not the
mean of pT values. All17,300 usable first-published-track values match native
scalar endpoints for EACH method within float-persistence tolerance.

Selected reconstructed tracks34,517; GSF published32,200 (93.29% conditional
on selection);24 published tracks lack component records. Main comparisons
use32,176 identical tracks. This is not overall track-reconstruction efficiency.
All generator pair slots are not counted as reconstructed tracks.

| Method | Median relative pT [%] | Central68% half-width [%] | RMS [%] | abs(error)>5% |
| --- | --- | --- | --- | --- |
|CompleteTracks KF|-0.2139|3.2719|15.08|18.00%|
|GSF BestBranch|-0.1128|0.9644|176.22|11.89%|
|GSF WeightedMean|0.6056|1.0218|283.90|11.45%|

Central68% half-width is(q84-q16)/2, not a Gaussian peak sigma. Central
quantile widths improve on this subset, but rare catastrophic positive pT
outliers substantially worsen RMS. Above100% absolute pT error: KF0.0093%,
BestBranch0.5097%, WeightedMean0.4351%. No tail clipping enters statistics.
FullMixtureMode is shown only in a separate17,300-first-published-track control
population; its native optimization status is1 for all those rows. The second
track's full five-dimensional mode cannot be reconstructed from marginal kappa.

## Recoil limitation and explicit user approval

The flat schema has full GSF directions only for the first published track.
It cannot provide an exact two-GSF-track recoil. The user explicitly chose
"KF/truth recoil plus a clearly labelled GSF-pT + KF-direction estimate" via
the clarification tool. All GSF recoil curves therefore use fitted pT with
CompleteTracks phi/tanLambda for BOTH tracks, with the approximation stated
in figure titles/legends/gallery/README. No fit is rerun to supply missing data.

Recoil uses nominal(240,0,0,0)GeV minus two bare-electron four-vectors, without
FSR recovery. Generator recoil retains ISR/FSR spread. Resolution residuals
are relative to the same generator recoil rather than to a fixed125GeV mass.
Exact KF/generator plots use17,121 reconstructed selected pairs, including
undefined KF masses in the denominator. GSF comparisons use14,870 common
complete pairs. Undefined recoil counts: KF4,BestBranch138,WeightedMean117.
These remain in histogram denominators; finite-mass quantiles have different
counts and must be read with those invalid fractions.

| Method | Recoil residual median GeV | Central68% half-width GeV | Finite masses |
| --- | --- | --- | --- |
|Exact KF|0.7320|5.4591|14866|
|BestBranch pT + KF directions (approx.)|0.2312|2.1183|14732|
|WeightedMean pT + KF directions (approx.)|-0.7114|2.1682|14753|

## Outputs and checks

Twelve PNG/PDF pairs cover publication coverage, common-track pT core/wide/
charge/angular/tail views, the separate first-track mode control, exact KF/truth
recoil and approximate GSF recoil wide/peak/residual views. Numerical data are
in tracks.csv/pairs.csv/summary.json; original job/event/track IDs are retained.
No Z-mass, pT or angle cut is applied beyond the documented validity/population
requirements. Formula self-checks,87-file identity/count checks, native scalar
closure and numerical summary assertions pass. Four figures were visually
inspected. A dedicated plot-gallery sub-agent handles full12-PNG coverage,
manifest organization, safe old-server handoff, port8000 launch and HTTP checks.
Server verification is recorded after its completion.

No new source/card/library, fit job, batch submission or production output is
changed. These completed-subset plots do not establish CEPC BH/GSF validation.

## Verified gallery launch

The dedicated plot-gallery agent validated12 PNGs exactly once across7 sections.
It safely stopped the old managed FSR/eBrem server PID1297841 and launched this
analysis on ailogin002.ihep.ac.cn,localhost8000,PID48142. Proxy-bypassed page and
pt_resolution_core.png HTTP checks both returned200, with image/png for the PNG.
The main agent reviewed the12-file manifest coverage and four figure images.
URL:http://127.0.0.1:8000/ on that host. For another machine:
`ssh -L 8000:127.0.0.1:8000 zhangcg@ailogin002.ihep.ac.cn`.
The gallery lifecycle changed only the managed plotting service, not production
jobs, data, cards or libraries.
