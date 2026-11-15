# Recoil spectra for photon conversions before ECAL

User request: show recoil mass for events with a converted photon. Completed
read-only analysis of the current REC conversion cohort and current theta25 BP
recoil products. No simulation, PFA, refit, source, card, build, installation,
default or production submission change. Existing gallery plots are preserved.

## Sources and conversion definition

All500 current `REC_ECALTRUTH_500jobs_200evt_20261112` files,100000inputs.
Re-read the thin original-photon/G4 branches using the preceding conversion
study's `fate()` definition: subtype14, killed status2, before the original
photon's first ECAL entry. FSR has a status1 generator link to a selected Z→ee
electron; eBrem is a direct selected-electron G4 birth, creator3, no later than
its parent's ECAL-entry step. No photon birth-energy cut, cluster MC weights,
purity, dominance, resolved-neutral requirement or angular truth matching.
Conversions inside ECAL are outside the saved record and are not counted.

All500 per-job photon partitions and individual birth-E≥1GeV conversion counts
reproduce the preceding study exactly. Converted originals total7549FSR and
17106eBrem (≥1GeV:3820 and8394). Complete event membership and individual
converted-photon owner/identity/energy rows are saved, rather than selecting
the earlier study's representative examples. Selected-track cohort remains
198081covered tracks;349selected tracks without electron ECAL entry are absent.
The full-input positive tags are19623events with either type,7234with FSR,
and13752with eBrem. These are event counts, not the preceding photon counts.
A zero tag on a missing-truth electron does not establish no conversion.

Reuse `bp_h0f_theta25_recoil_20261114/events.csv`, built from native current
`BP_H0F_THETA25_NOSCAN_500jobs_200evt_20261113` flat files and exactly these REC
files. Check all REC/BP sizes/mtimes, source agreement between the two readers,
recoil/reference CSV hashes, and exact MC/input-track identities. No deleted
old BP tuple or old fitted endpoint is read, and no new fit is performed.

## Common sample, methods and normalization

Of98437complete classified pairs,98093have both selected-electron conversion
truth coverage;344pairs are excluded. This is not the349missing-track count.
The common plotted-method gate leaves75071pairs before conversion selection:
valid opposite-charge Baseline KF, neutral references, known selected truth,
and both ordinary/floating BP110 endpoints. No H0/H0F/GSF success gate. Every
electron curve in each panel has identical pre-window event keys.

- Baseline KF + neutrals: measured pre-reclustering trackless-neutral union,
  theta/phi half-windows25/200mrad, counted once across both legs. No energy,
  purity or isolation trim; outliers are retained as in the current fits.
- BP110: native ordinary fitted electron, truth-selected eligible material
  interval or native valid-null fallback; no location scan or extra neutral p4.
- BP110 + floating FSR: its own fitted electron and its own collinear IP photon
  p4 exactly once; no extra measured neutral p4.
- Muon Baseline KF: unchanged inclusive separate-channel reference,48636pairs
  and44236finite window entries. It is not conversion-selected.

Three square,120–140GeV,0.1GeV-bin figures: any FSR OR eBrem conversion; FSR
conversion; eBrem conversion. Each event enters once per plot, irrespective of
converted-photon multiplicity. FSR/eBrem groups overlap by801matched events.
Other unconverted photons and radiation on the other electron are allowed.
Each curve independently normalizes to its own finite120–140GeV count;
legends show raw window N. No luminosity scaling, generator recoil or new peak
fits. Gray/purple/gold plus line styles distinguish the electron curves; the
inclusive muon reference is black dashed. Bold AGORA simulation is outside axes.

| Conversion selection | Truth-covered pairs | Matched pairs | KF+neutrals window N | BP110 window N | BP110+FSR window N |
|---|---:|---:|---:|---:|---:|
| Any FSR OR eBrem |19414|12874|9853|9451|10269|
| FSR |7160|4965|3665|3429|3870|
| eBrem |13607|8710|6720|6529|6978|

Matched union closes:4965+8710−801=12874. Conversion-pair counts differ from
the full-input tags because complete-pair/truth and fit gates are applied.
Endpoint/baseline losses are6540any,2195FSR and4897eBrem pairs; these plots
condition on survivors and are not fit-efficiency measurements.

For all12874matched any-conversion events, fractions in120–130GeV are49.15%
KF+neutrals,52.24%BP110 and64.15%BP110+FSR. BP110→BP110+FSR decreases the
above140GeV fraction23.44%→12.67% but increases below120GeV3.10%→7.53%.
These use the entire matched pre-window denominator, including undefined masses
outside the numerator; they are not the independent unit-window plot values.
This demonstrates high-side recovery with a low-side penalty in this sample,
not a causal conversion penalty, calibrated tracking resolution or physics
validation. There is no conversion-veto control or independent held-out study.

## Verification and artifacts

All500 source identities checked before/after reads; native collection IDs,
G4 track identity, generator ownership, birth timing/creator and endpoint fate
checks pass. Photon partitions exactly match previous per-job counts. Every
event tag equals its converted-photon rows; original photon identities are
unique. Both selected MC/input-track identities match the current recoil cache.
OR/AND union, raw window/underflow/overflow/undefined partitions and unit-area
normalizations close. Muon window histogram is unchanged. Python compile and
assert-based checks pass; all three final renders inspected. Native physical
cluster purity is neither used nor claimed. No component dump/refit required
because no implementation or fit changed.

Uncommitted artifacts:
`TrackingPerformanceStudies/converted_photon_recoil_20261115/` contains read.py,
plot.py, README,500job JSONs, converted_photons.csv, conversion_events.csv,
events.csv, histogram.csv, read_summary.json, summary.json, logs and3PNG/PDF
pairs. The three figures are copied into the existing
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110/` gallery as
`recoil_converted_{any,fsr,ebrem}_shapes_120_140.png` (and PDF).
Gallery publication: appended one section, preserving all21old root sections,
all88old PNG hashes and the child manifest. Root coverage validates91PNGs in
22sections; the12-display child validates5sections. All6new PNG/PDF copies
match the reviewed source hashes. Gallery and all3PNG HTTP responses return200;
downloaded PNG hashes equal the study summary. The existing healthy server was
retained; actual PID640260 serves `TrackingPerformanceStudies` on ailogin002.
Earlier PID3994352/root-relative URL records were stale when rechecked.
Use `http://127.0.0.1:8000/bp_matched_recoil_pt_20261110/`; the event displays
remain in its `neutral_event_displays/` child. No PDF HTTP response is claimed
because the PNG gallery does not serve PDFs.

Outgoing AGENTS retained exactly in
`2026-11-15-agents-before-converted-photon-recoil.md`; previous conversion
definition/count details remain in `2026-11-15-photon-conversion-fractions.md`.
