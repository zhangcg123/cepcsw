# Population matrix versus current recoil counts

User asks why the population matrices do not agree with the H0F/theta25 recoil
section. Read-only comparison of existing row caches on2026-11-16 confirms
identical98437(job,event)keys, zero>1GeVcategory disagreements, and zero
differences in the maximum per-leg truth birth-energy sums (relative1e-10,
absolute1e-6GeV). Direct flags also reproduce from the current per-leg energies.
No native reconstruction/fit or plot rerun and no gallery change.

Three distinctions explain the displayed counts:

1. The newly added unthresholded matrix uses recorded presence; the recoil
   categories use strict>1GeV per-electron sums. Only the>1GeVmatrix has the
   same category definition as the recoil panels. Both event flags may occur
   on different selected electron legs.
2. Matrix denominator98437 is before fit success/reconstructed-charge cuts.
   Category recoil shapes require opposite reconstructed baseline charge,
   usable neutral references, known truth and all4BP hypotheses valid on both
   legs, leaving75068common pairs. The135baseline-charge rejections leave98302
   baseline pairs first. Missing fits are not relabeled as another category.
3. Each recoil legend N counts that curve's own finite120–140GeV entries.
   Every category curve then independently normalizes to unit window area;
   its height does not represent the global category fraction.

| Category | >1GeVmatrix | Common recoil pairs | BP110+FSR in120–140 |
|---|---:|---:|---:|
|Neither|21602|17948|16859|
|eBrem only|43739|31798|27651|
|FSR only|11517|9673|7941|
|Both AND|21579|15649|12217|
|Total|98437|75068|64668|

Other methods have different window counts on those same75068pre-window keys:
KF+neutrals62328,H0=54766,H0F=64648,Hbeta=59793,HbetaF=64668. This reflects
different reconstructed masses/undefined recoils, not a category disagreement.

The inclusive10ab^-1figure uses each method's own valid pair sample, not the
all4common gate: baseline98302pairs, HbetaF75511pairs. Their own120–140raw
counts65793/65003 are weighted by0.704fromall100000generated electron inputs.
Consequently its yields do not equal raw matrix counts or sums of category
shape legends. The repeated muon curve is an inclusive separate-channel
reference, not an electron-category population.

Audit artifacts are uncommitted in
`TrackingPerformanceStudies/recoil_population_count_audit_20261116/`:
counts.csv and summary.json. Inputs are the matrix events_gt1gev.csv and the
current bp_h0f_theta25_recoil_20261114/events.csv. Unique-key/energy/category,
same-denominator and common/window count closure assertions pass. An initial
inline command had a Python whitespace typo, corrected before any audit output
was written; it was not a data or plotting error.

For a direct visual population comparison, a separately labeled common-fit
matrix should use75068events; retain the user's raw pre-fit matrices. No such
new figure or presentation edit is made by this diagnostic question alone.
