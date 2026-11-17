# BP ECAL+HCAL response audit — 2026-11-18

## Finding

The user notices the new paired-production recoil peak moving left. A direct
read-only check confirms positive combined-calorimeter response bias on clean
selected electrons. The present `corrected pre-ECAL + 4 * pre-HCAL` recipe is
not established as an unbiased electron/photon measurement. This is evidence
against treating its shifted peak as a resolution improvement, not proof of a
particular calibration error or of duplicated deposits.

No native source, run card, installed library, running job or production output
was changed. The active production retains both calorimeter results for study.

## Direct completed-file check

Read the first 25 completed jobs in the frozen snapshot at
`TrackingPerformanceStudies/bp_ecal_hcal_recoil_20261118/snapshot.json`:
5,000 input events and 9,934 selected-track rows. Analysis and detailed results
are uncommitted `closure_audit.py`, `closure_audit_5k.json` and
`closure_audit_5k_tracks.json` in that study directory.

Every selected row passes assertions that each calibrated HCAL energy equals
four times the saved raw cluster energy, the per-cluster sum equals its saved
aggregate, and the combined charged/neutral observations equal their ECAL
part plus exactly one HCAL part. Cluster IDs are unique within each sum and
disjoint between charged and neutral sums. No HCAL cluster ID is shared
between the selected legs in these 5,000 events. These checks do not establish
that distinct reconstructed clusters cannot share underlying deposits.

Clean response subset: selected production electron, unique track-MC match,
valid charged ECAL measurement, finite positive ECAL energy, MC energy >10GeV,
no tagged FSR birth energy, and summed primary-electron eBrem birth energy
<0.1GeV. There are 1,633 tracks; no tail trim is applied. The denominator is
the matched electron MC initial energy, before tracker-material losses. Its
eBrem photon energy is therefore not added to that denominator.

| Median measured minus truth energy | ECAL only | ECAL+HCAL |
|---|---:|---:|
|Charged observation|−0.138GeV|+0.162GeV|
|Charged + collected neutral observation|−0.130GeV|+0.171GeV|
|Same sum, relative residual|−0.268%|+0.345%|

Of these tracks, 1,521 also have successful 110+FSR endpoints in both modes
and belong to the baseline-valid plotted event pairs. Their median fitted
electron-plus-FSR energy bias is +0.020GeV for ECAL-only and +0.196GeV for
ECAL+HCAL. The median paired fitted-energy increase is +0.158GeV. This is a
track-level diagnostic; it is not a recoil peak fit or resolution claim.

## Source interpretation and limits

`TrackClusterConnectingAlg::EcalChFragAbsorption` applies the ECAL response
and crack correction before the snapshot. `CreatePFOSnapshot` saves ECAL
`getLongiE()` and HCAL `getHitsE()`; neutral PFO assembly separately uses the
configured HCAL factor four. BP reads the cluster energies, not that already
assembled PFO energy, and applies its HCAL factor once. No hidden second
factor or PFO-plus-cluster double sum was found.

The ECAL correction's training target has not been established by this audit;
one cannot yet conclude that it specifically compensates the same HCAL leakage.
HCAL associations are geometric, not proven selected-electron leakage. Joint
response calibration, contamination and ECAL/HCAL correlations remain open.
The nonnegative floating-FSR parameter can absorb a positive calorimeter
surplus and include it in the visible recoil system even on truth-clean tracks.
Larger assigned uncertainty alone does not establish unbiased response.

The earlier all-leg-success paired-event check (56,106 events) already finds
median visible-energy increase +0.539GeV and recoil-mass change −0.678GeV.
These are paired medians, not fitted peak shifts. Together the checks localize
the concern to the measurement/response model rather than a demonstrated
plotting or duplicate-summation error. Do not tune HCAL scale from recoil-peak
position alone or replace defaults during the running campaign.
