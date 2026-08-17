# Flat tuple paired ECAL-track output

Date: 2026-08-17

## Request and implementation

The user requested that the flat ROOT tuple contain both the ordinary
tracker-only GSF track and the paired ECAL-constrained GSF track.  The change is
inside `Reconstruction/RecGsfTracking` and does not alter either tracking
algorithm or EDM collection.

`RecGsfFlatTuple` keeps every existing `gsf_*` branch unchanged and adds:

- `ecal_gsf_available` and `ecal_gsf_changed`;
- `ecal_gsf_pT`, `ecal_gsf_p`, `ecal_gsf_eta`, `ecal_gsf_theta`,
  `ecal_gsf_phi`, `ecal_gsf_d0`, `ecal_gsf_z0`, `ecal_gsf_omega`, and
  `ecal_gsf_tanl`;
- `ecal_gsf_chi2`, `ecal_gsf_ndf`, `ecal_gsf_nhits`, and `ecal_gsf_type`;
- `res_pT_ecal_gsf` alongside the existing `res_pT_gsf` and
  `res_pT_lcio`.

`ecal_gsf_available` is one only when `GSFTracksEcalConstrained` contains a
track for that event.  `ecal_gsf_changed` is one when its AtIP parameters or
fit quality differ from the ordinary `GSFTracks` result.  The paired
collection is looked up optionally through the event store, so default-off
jobs do not fail when the collection is absent.  Missing constrained fields
and their residual are zeroed.

No constrained hit-vector copy is written.  The paired EDM track deliberately
copies the ordinary GSF tracker hits, so `gsf_hit_*` is the common, non-
duplicated hit information.  `ecal_gsf_nhits` still records the paired track's
hit count.

The common track-filling helper is now called for every event, including empty
collections.  This also prevents ordinary LCIO/GSF scalar fields from retaining
the preceding event's values when a selected-event run produces an empty row.

This change adds no `RecGsfTracking` Gaudi property and leaves its audited
40-property surface unchanged.  `DumpGsfTrks/gsf.py.bk` needs no new steering;
its existing `RecGsfFlatTuple` instance gains the branches automatically.

## Validation

The separate `RecGsfFlatTuple` CMake target built and installed successfully in
the EL9/LCG 105 environment.

An enabled seed-11 smoke run processed 42 input rows with only entries 1 and 41
selected for GSF fitting:

| entry | ordinary pT (GeV) | available | changed | constrained pT (GeV) | ordinary residual | constrained residual |
|---:|---:|---:|---:|---:|---:|---:|
| 1 | 15.4492506 | 1 | 0 | 15.4492506 | -0.1037839 | -0.1037839 |
| 2, skipped control | 0 | 0 | 0 | 0 | -1 | 0 |
| 41 | 22.5056713 | 1 | 1 | 11.7097195 | +0.9265555 | +0.00238845 |

This verifies exact ordinary/constrained equality for an unchanged event,
correct paired separation for the known seed 11 entry 41 branch change, and
zero/reset behavior across intervening empty rows.

A separate three-event default-off run completed normally.  Its ordinary
`gsf_pT` values were 1.8987576, 10.5601043, and 41.2693577 GeV; all three rows
had `ecal_gsf_available=0`, `ecal_gsf_changed=0`, zero constrained pT, and zero
constrained residual.  Thus the new optional lookup is backward-compatible
with the frozen default-off run card.

All smoke cards, ROOT files, and logs stayed under `/tmp`.
