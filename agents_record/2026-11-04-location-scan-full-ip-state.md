# Full native IP state for every scan entry

The user requested complete IP parameters for the location scan, because the
old pT-only scan output cannot supply momentum directions for recoil mass.
This is passive recording under HitScanAll, not a new fitter or selection.
The silicon plus exact TPC-row0 candidate list and all four setups are unchanged.

## Output contract

Every saved `(point,setup)` entry, including the null and truth-only reference,
now has these additional `location_scan_*` branches:

| Suffix | Values per entry | Meaning |
|---|---:|---|
| ip_parameters | 5 | Native EDM `[D0,phi,omega,Z0,tanLambda]`; mm, rad, 1/mm, mm, dimensionless |
| ip_covariance | 21 | Native EDM packed lower triangle, row-major, for `[D0,phi,omega,Z0,tanLambda,time]` |
| ip_reference_point | 3 | Native reference `[x,y,z]` in mm |
| ip_time | 1 | Native time value; the fitter does not estimate timing |
| ip_location | 1 | Native TrackState location; AtIP=1 on the tested fits |

For entry j, the first three blocks start at5*j,21*j,3*j. The leading15
covariance values are the fitted five-dimensional helix covariance; the
remaining time entries are preserved from the native state, not a timing fit
or the augmented loss parameter b. With saved pT, momentum is
`px=pT*cos(phi), py=pT*sin(phi), pz=pT*tanLambda`, in GeV.

The state is copied from the SAME accepted `pair.rts.ip` as that entry's saved
NLL/pT. Inherited observations copy their accepted source state at the same
point. An unsuccessful fit has NaN floating blocks at the same fixed strides
and location=-1. Reset clears every IP vector between track rows. Do not use
the inactive ordinary/free endpoint families in scan-only rows.

`LocationScanTuple.cpp` holds the existing booker and the new serialization
function, separated only to permit a standalone ROOT round-trip test without
running an optimizer. There is no new KF/RTS implementation, extrapolation,
loss map, likelihood, optimizer setting, location ranking or steering option.
Legacy ranked HitScan and its outputs are unchanged.

## Verification

- Full build succeeded. Fifteen CTests pass, including the new ROOT test of
  ordering/strides, all21 covariance elements, reference/time/location,
  inherited copies, failed-state NaNs and reset/ROOT-address stability.
- Fifty-seven batch/helper tests in the current working tree pass. The scan
  verifier now requires all five new branches; it cannot silently validate an
  old pT-only scan tuple as the new schema. Existing unrelated edits in the
  batch helper/tests were preserved and are excluded from this checkpoint.
- Fresh staged Truth runs on events1:11,16,17 reproduce all2970 previous
  branch-row values exactly, NaN-aware; the added scan-IP fields remain empty
  in these non-scan rows.
- Fresh staged scan1:5 records40 setup entries. All933 previous track-row
  fields match the earlier TPC-row0 scan exactly. New arrays have200 helix,
  840 covariance and120 reference values, plus40 times and40 locations.
- All40 states reproduce their saved pT and pT error from omega/covariance.
  Cartesian momentum reconstruction is finite and transverse magnitude closes.
  The inherited111 null has exactly the same complete state as its110 source.
- At the independent fixed truth location h3, every000/100/110/111 native IP
  parameter/covariance block matches the corresponding earlier Manual refit
  exactly. No truth energy is supplied to the scan. No population/recoil-mass
  improvement claim follows from these serialization checks.

Artifacts: `/tmp/bp-scan-full-ip-D6wllG`, including staged library, scan5.root,
truth.root, validation.py/json/log and deployment
probe. Existing focused references are in `/tmp/bp-scan-tpc-row0-7RNVnE` and
`/tmp/bp-scan-only-9xx5YM`. Temporary artifacts may expire; source and tests are
the maintained implementation. Pre-existing build clock-skew/external-header
warnings remain; no shared-code workaround was added.

## Deployment and limits

The user confirmed no active jobs and authorized installation after validation.
Only the RecBreakpoint package library, configurable and discovery metadata
are installed; no shared KF, PFA, GSF, sim/rec products or batch controls change.
The default-environment EvtMax0 probe terminates successfully, writes an
isolated /tmp output and books all five new IP branches. It is a deployment
check, not physics validation. Installed and tested library hashes match.
Tested library SHA256:
`ccb10088f63a2945d42b2fae071174cda392c21a026d0ec888db9d44fb572e48`.
Previous installed library:
`5b72f16565ba033168defbda1f2eb2f8cb379cafc4fa85861cbc8183d43fab1f`.

Old scan files cannot recover the missing directions; rerun only the breakpoint
stage, retaining rec inputs and preserving old output files. No batch jobs are
submitted by this change. Full IP state enables recoil calculation for an
offline chosen scan entry; the separate per-site charged-ECAL pull needed for
the historical2-sigma analysis fallback is still not added by this request.

The existing current-focus notes are only augmented with this schema/record
reference. No focus, law, scope, command, evidence or historical detail is
removed; the two-section structure and all previous records remain intact.
