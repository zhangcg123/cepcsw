# Retired side material helper outputs

Date: 2026-08-22

## Decision

The maintained workflow no longer generates either of the historical material
helper outputs:

- the side `gsf_material_steps*.root` file produced by
  `GsfMaterialStepRecorderAnaElemTool`;
- the component-call-level runtime CSV formerly steered by
  `MaterialBHAuditCSV`.

Their current role is replaced by simulation-event provenance in
`GsfG4MaterialSteps` and `GsfSimTrackerHitG4StepLinks`, followed by the
default-on passive `GSFTruthMaterialIntervals`/
`GSFTruthMaterialRecordStatus` output and the final flat tuple's 50
`truth_material_*` branches. These records remain diagnostic and never steer
the GSF.

## Removed implementation and steering

- Removed `GsfMaterialStepRecorderAnaElemTool.{h,cpp}` and its registration
  from `Simulation/DetSimAna/CMakeLists.txt`.
- Removed the standalone
  `Reconstruction/RecGsfTracking/options/run_gsf_material_step_recorder_test.py`
  card.
- Removed the `MaterialBHAuditCSV` Gaudi property, stream lifecycle, candidate
  and child-row structures, and seed/forward/reverse writer calls from
  `RecGsfTracking`.
- Removed `GSF_MATERIAL_BH_AUDIT_CSV` steering from the reverse template and
  the per-sample audit filename from `DumpGsfTrks/gsf.py.bk`.
- Synchronized the authoritative option reference and workflow README. The
  supported `RecGsfTracking` property inventory is now 45, and the maintained
  card explicitly steers all 45.

Removing the optional returned-mixture sink from the three splitter call sites
does not alter the mixture calculation: the splitter argument already defaults
to null and was used only to serialize the retired child rows.

## Compatibility boundary

The `TruthBHLossSource="CSV"` and `TruthBHLossSource="G4StepTuple"` readers
remain available for reproducing old oracle controls. They consume historical
inputs but generate no helper output. Existing ROOT/CSV files, analysis
scripts, historical records, and the retired implementation in Git history are
preserved. Generated job cards that assign `MaterialBHAuditCSV` are stale and
must be regenerated from the maintained templates.

## Verification

- `DetSimAna` and `RecGsfTracking` built and installed successfully in the
  EL9/LCG 105 development build.
- The rebuilt `libDetSimAna.so` and regenerated component/configuration
  registries expose neither the material-step-recorder component nor
  `MaterialBHAuditCSV`.
- A one-event run through the active reverse template completed with the
  production BH model and no helper ROOT/CSV output. The final EDM contained
  status `1` and 231 `GSFTruthMaterialIntervals`; the final flat tuple contained
  the matching 231 `truth_material_*` entries.
- The option audit found 45 unique header properties, all documented and
  explicitly steered. Both maintained Python cards parse and the focused diff
  check is clean.

This cleanup changes recording interfaces only. It is not a material/BH
physics result and does not change the current ordered branch-local diagnostic.
