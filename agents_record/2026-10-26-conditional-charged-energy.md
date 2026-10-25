# Conditional charged energy and the no-breakpoint limit

User requested ElectronPhoton switch to electron-only when a separate usable
neutral is found, and charged energy must not be skipped solely because no
breakpoint was selected. Implementation is built/tested, NOT INSTALLED.
No PFA changes, shared KF changes, batch submissions or installation in this turn.

## Contract

- ElectronOnly remains the compiled/card default and always means electron energy.
- ElectronPhoton now requests track-wise routing: one selected breakpoint and
  no collected hit-supported neutral -> electron+photon sum; collected neutral
  or no selected breakpoint -> electron-only. The existing KF ECAL-entry window
  (theta +/-10 mrad, phi +/-200 mrad) defines collection, not any neutral elsewhere.
- Collection steers routing independently of NeutralEcalMeasurement. With the
  neutral switch OFF, a resolved photon is not mistakenly added to the charged
  energy prediction. With it ON and a breakpoint, apply the separate neutral
  measurement. No breakpoint means no neutral loss update; retain earlier stage.
- ElectronPhoton still requires ElossOn=False for the unresolved sum model.
  The global sum+neutral ban is removed in Gaudi and batch preparation. The
  core fitter still rejects an actually unresolved sum plus a neutral measurement,
  so accidental duplicate photon observations cannot be passed directly to it.
- charged_ecal_energy_mode retains requested steering. New main-tree string
  charged_ecal_effective_energy_mode records the resolved interpretation; it does
  not imply fit success. Use constraint statuses/masks for actual application.
- Missing/ambiguous charged observation or failed update still retains the
  preceding accepted result. No-breakpoint does not manufacture a cluster.
- All stages start from the original seed/prior with cumulative observations.
  Existing ordinary/free, truth, diffuse and backward families remain independent.

## Shared machinery

The native KalmanAdapter::predictCalorimeter now has a 5D overload, with the
6D overload lifting its same native F/Q and preserving b/correlations. The
shared terminal charged update uses native ScalarMeasurementSite/Filter and
one RTS edge for either dimension. A no-breakpoint fit stays 5D throughout:
ECAL prediction -> electron-energy update -> RTS to last tracker hit -> ordinary
RTS inward, then beam endpoint or native IP propagation as configured. No fake
loss variable, singular zero-variance coordinate or artificial b prior is used.
The updated ECAL-face state is published. The captured likelihood adds the
conditional energy observation once. No-breakpoint free output copies the
constrained ordinary result, because there is no b for Minuit to optimize.

## Checks / artifacts

Workspace /tmp/bp-charged-routing.FMJ30l contains run.py, verbose logs and ROOT
outputs: auto, electron, sum10, neutral_off, empty, empty_no_beam, off.
Inputs are /tmp/bp-pre-sum-regression.Csbg1Z/rec.root (seed1, original entries
0..97 with pre-PFO exports). Before this implementation, all 206 existing final
PFA fields were checked exact against production rec for those 98 entries.

- 20 original events (0..19), 22 track rows: events 2 and 13 have two tracks
  and are controls, not a topology-clean optimization population.
- Automatic 111 routing: 15 successful no-breakpoint charged updates; one
  resolved breakpoint (event5) applies beam+charged+neutral (mask7); five
  unresolved breakpoints apply beam+charged (mask3). Remaining event12 has no
  charged ECAL candidate and retains beam only. All applicable updates succeed.
- Requested ElectronPhoton with resolved neutral/no breakpoint exactly matches
  explicit ElectronOnly for all existing fit fields (requested-mode string aside).
- Frozen previous electron-only breakpoint rows (3,5,13track0,16) reproduce all
  old fields exactly. Baseline/reference families reproduce for the other rows.
- Earlier ten unresolved sum events 38,41,48,54,60,69,78,79,89,97: charged ordinary
  and free endpoints, covariances and diagnostics reproduce exactly, including
  all unconstrained families. Neutral ON without a candidate copies the charged
  stage; source remains pre-reclustering. Beam OFF as in their original run.
- Neutral OFF tests entries0,5,12: charged routing/results unchanged from ON.
- Manual empty breakpoint list at entries0,5, beam ON and OFF: successful 5D
  ECAL update, finite full likelihood and smoothed score, one extra measurement
  dimension, terminal curvature variance contracts, no extra b optimization,
  free/ordinary endpoints identical. Beam case forward tracker states unchanged.
- Constraint-OFF run on all20 entries reproduces unconstrained endpoint families
  of the ON runs exactly. No truth/diffuse endpoint drift.
- All 11 C++ tests pass, including new routing truth table and native 5D/6D
  electron-update equivalence with independent loss, and invalid 5D sum rejection.
- 43/44 batch tests pass. Only pre-existing wrapper-default test fails because
  user subbreakpointjobs.sh uses trk,calodigi,rec, test expects breakpoint. User
  script is unchanged. New sum+neutral frozen-config test passes.

Recheck with:

```
python3 Reconstruction/RecBreakpoint/tests/check_charged_routing.py \
  /tmp/bp-charged-routing.FMJ30l /tmp/bp-pre-pfo.Ra9UAA/pre.root \
  /tmp/bp-pre-sum-regression.Csbg1Z/pre.root
```

The passive complete smoothed score is finite, not asserted numerically identical
to the forward innovation sum: manual empty-list event0/5 differ by about
+0.007/-0.013 with beam and +0.054/-0.199 without beam. Existing score evaluation
machinery is unchanged; this is not a new likelihood/linearization-closure claim.
No population performance or endcap validation. ECAL errors/cluster ownership
remain experimental. Only routing and the no-breakpoint charged path are gated.

## Deployment / memory

This update plus the pre-PFO consumer and PFA snapshot exporter remain uninstalled.
Use the build/run wrapper only for these local tests. New default input requires
regenerated rec with pre-PFO snapshots; it does not work on old rec automatically.
Diffuse constraints/wrapper remain explicitly deferred. No defaults for energy
mode, constraint switches or user batch paths/stages/group changed.
Outgoing AGENTS is preserved in 2026-10-26-agents-before-conditional-charged-energy.md;
only its current focus is replaced, with global laws unchanged.
