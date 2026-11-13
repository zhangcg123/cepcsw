# Floating collected FSR energy at the beam boundary

Promoted to the project source/default installation by user request on 2026-11-12.
This is a research model, not a validated production correction. The option name
retains `Experimental` for compatibility. A fitted post-FSR electron and a
massless collinear photon are exported separately; no pre-FSR TrackState exists.

## Corresponding families

The property `ExperimentalBeamSpotFsrLoss` and card/batch switch `BP_BEAM_SPOT_FSR_LOSS` default ON. Set the switch to `0` to disable. Newly prepared cards freeze the setting; existing frozen campaigns are unchanged. When enabled it books:

| Setup | New FSR subfamily prefix |
|---|---|
|000|free_loss_fsr_|
|100|beam_spot_free_loss_fsr_|
|110|charged_ecal_free_loss_fsr_|
|111|neutral_ecal_free_loss_fsr_|

The ordinary control families retain their original rts_* outputs. Use the electron_p4 in the FSR subfamily with the photon from that SAME subfamily. Adding the new photon to the ordinary control electron can double-count/misassign energy because those fits are different.

A separate `null_110_*` reference is always attempted for each processed track,
independently of scans and this FSR switch. It is the genuine 5D no-breakpoint
beam+ECAL fit, reusing the scan-null C+N routing. Its scalar status and one-entry
vectors distinguish success, missing observations and fit failure; use
fit_status=2 and actual_constraint_mask=3. Its state/score is available even if
later truth-location matching fails. See the README for the full suffix/stride
contract. This adds the H0 reference alongside the ordinary and FSR-floating
110 alternatives; it introduces no automatic hypothesis decision.

### Independent FSR-only hypothesis H_F

With the same FSR switch ON, every processed/selected track also attempts
`fsr_only_110_*`, before truth-hook matching. This is a fresh native 5D
tracker/beam fit with one nonnegative additive FSR energy, **no material
breakpoint**, no loss prior or prior-center optimization, and no location scan.
It runs even when Truth selects a material interval for the other families.
It uses the existing zero-material FSR fitter and the same local conditioning,
closure gates and collinear photon convention; no pre-FSR state is introduced.

Its 19 suffixes are the same as the 110 FSR family listed below. Status2 and
constraint_mask3 mean a fitted H_F; status-2 has an explicit error and no
fallback endpoint. Status0 means not attempted. The four additional fields
`ecal_observed_energy`, `ecal_observed_error`, `resolved_ecal_sum_applied` and
`measurement_dimensions` record the actual scalar observation and score size.
Missing beam/charged observations fail explicitly. With the switch OFF these
branches are absent, just like the other FSR families. H0 stays independent of
the switch. Existing files need a BP rerun to acquire H_F.

H_F follows H0's configured observations: ElectronPhoton with a collected neutral
uses C+N and the quadrature uncertainty; ElectronOnly uses C. There is one
total-energy observation, no separate neutral update and no extra neutral p4
to add to the fitted electron+photon. Existing material-FSR110 always includes
collected N, even in explicit ElectronOnly mode, so that nondefault setting
does not give comparable four-model observations when N exists.

The four hypotheses are H0=no material/no FSR, H_F=no material/floating FSR,
H_beta=material/no FSR and H_betaF=material/floating FSR. No automatic selection
or penalty is added here. Compare scores only for successful fits with matching
observations; raw likelihood preference alone is not a calibrated decision rule.

For example, 110 fields are charged_ecal_free_loss_fsr_energy, _px, _py, _pz, _energy_variance, _status, _direction_mode, _constraint_mask, _active_boundary_mask, _primary_ecal_energy, _neutral_ecal_energy, _electron_ip_parameters, _electron_p4, _joint_ip_energy_covariance, _joint_p4_covariance, _nonlinear_closure, _material_closure, _nll2 and _error.

energy/px/py/pz are GeV and define one effective massless recovered photon. electron_p4 has order [E,px,py,pz] in GeV and the electron mass .00051099895GeV. electron_ip_parameters retains ONLY the fitted post-FSR electron AtIP parameters in EDM order [D0,phi,omega,Z0,tanLambda]. There is no corrected/pre-FSR electron state.

joint_p4_covariance is an8x8 row-major GeV² matrix with order [electron_E,electron_px,electron_py,electron_pz,photon_E,photon_px,photon_py,photon_pz]. It includes electron/photon correlations. If S=[I4 I4], the dressed-system covariance is S C S^T; do not add the two marginal covariance blocks alone.

joint_ip_energy_covariance preserves the existing complete6x6 parameter covariance in KalTest order [drho,phi0,kappa,dz,tanLambda,F], including position/FSR-energy correlations. It is conditional on the optimized material-prior center, as before. The p4 covariance is its first-order Jacobian transformation, not a calibrated full nonlinear posterior.

Status0=unavailable/not attempted,1=tracker-only electron retained with FSR undetermined,2=fitted,3=retained110 when no separate neutral observation,−2=failed.000/100 retain status1 where their ordinary endpoint exists:electron_ip_parameters is available but photon scalars are NaN and electron_p4/joint covariance vectors are empty. They introduce no ECAL measurement. For fitted F=0, photon p4 and its conditional covariance are zero. direction_mode1 means photon direction assumed collinear with the fitted electron at IP;0 means undetermined. It is not a measured neutral-cluster direction or a truth direction.

Constraint-mask bits remain beam1,primaryECAL2,separateNeutral4. Active-boundary bits denote profiled FSR components fixed at zero. For110,primary_ecal_energy is the full collected excess and neutral_ecal_energy=0;for111 they are the charged and neutral excess contributions and sum to energy.

## Recoil use

Read the selected electron rows. For each leg and family:

```python
prefix = "charged_ecal_free_loss_fsr_"
electron = row[prefix + "electron_p4"]
photon = [row[prefix + key] for key in ("energy", "px", "py", "pz")]
dressed = [a + b for a, b in zip(electron, photon)]
```

Use both dressed legs in the recoil calculation. Add recovered FSR only once. The previous beam_fsr_* vector schema is removed in new files; old files are preserved and are not rewritten. Existing plots remain frozen until explicitly regenerated.

## Model and restrictions

For each material-loss prior-center trial, run the native tracker/beam KF/RTS
without ECAL, then profile nonnegative additive FSR amplitudes at its smoothed
ECAL reference. There is no BH or Gaussian FSR prior. For 110 the single scalar
observation is the sum of charged and collected neutral energies:

```
C + N = p_ECAL * exp(b_material) + F
```

With no selected material breakpoint it is `p_ECAL + F`. The total includes a
collected neutral even for the null material hypothesis. This differs from the
ordinary null-material 110 control, which uses charged energy alone. 111, when
`BP_NEUTRAL_ECAL_MEASUREMENT=1`, profiles separate contributions:

```
C = p_ECAL + F_C
N = p_ECAL * expm1(b_material) + F_N
F = F_C + F_N
```

Condition every trajectory marginal using the original RTS cross covariances
and native KalTest scalar measurement updates; do not count hits twice. A free
positive 110 FSR amplitude absorbs the energy residual without an ECAL mean
update at that material trial. At a zero boundary, deferred Gaussian conditioning
can update the electron. The existing outer material prior-center optimization
and its bounds remain. The covariance is conditional on that optimized center.

Supported: Truth/Manual, RTS, nominal zero beam pivot, zero/one material
breakpoint. `HitScanAll` requires FSR OFF. Additional `LocationScanSetups` keep
their ordinary control meaning and do not produce FSR scan families. 111 stays
disabled by default, with status -2 in its FSR family; enabling it without a
usable neutral retains 110 with status 3. 000/100 introduce no ECAL constraint.

No full iterative relinearization is implemented. Reject local ECAL energy
closure beyond max(0.005 GeV, 1e-4 times observed energy), and material jump
closure beyond max(0.01 GeV, 1e-4 times upstream momentum). Passing these gates
is not proof of a calibrated nonlinear posterior.

The fitted FSR energy is effective collected calorimeter excess. Contamination
can imitate FSR; uncollected photons cannot be recovered. Material/FSR sharing
can be degenerate, especially for early loss or missing dominant truth intervals.
The FSR-only sample showed improved peak population but also a larger low-mass
tail. Both-category controls showed improved dressed-system momentum while the
post-FSR electron alone could worsen. No inclusive physics-validation claim is
made. See the [promotion evidence](../../../agents_record/2026-11-12-floating-fsr-default-promotion.md).

## Verification

`RecBreakpointFsrJointTest` independently checks Gaussian profiling/conditioning,
active boundaries and finite-difference joint-p4 covariance (46 cases).
`RecBreakpointFsrFamilyTupleTest` checks ROOT persistence and per-row reset.
Batch tests check compiled/card/helper default agreement and explicit disabling.
Focused same-code OFF/ON Higgs checks use source 1 events 3,15,112 and source 2
event 135, with verbose state/covariance output. The historical particle-gun
hard-loss events 11,16,17 are unavailable and have not been substituted by
unrelated Higgs event indices.

Run focused compiled tests through the build environment so they load the built
plugin rather than an older installed copy:

```bash
source setup.sh
cmake --build build.105.0.0.x86_64-el9-gcc11-opt --target RecBreakpointFsrJointTest RecBreakpointFsrFamilyTupleTest -j4
build.105.0.0.x86_64-el9-gcc11-opt/run ctest --test-dir build.105.0.0.x86_64-el9-gcc11-opt -R '^RecBreakpointFsr(Joint|FamilyTuple)$' --output-on-failure
```
