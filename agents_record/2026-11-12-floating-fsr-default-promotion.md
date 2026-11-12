# Floating FSR promoted to the default source and installation — 2026-11-12

## Authorization and scope

After confirming that FSR is an option for 110/111, the user explicitly asked:
“then, prompt it as current default, codebase and installation”. This promotes
the private photon-output revision, superseding its temporary-only restriction.
No branch switch, simulation, tracking/PFA algorithm edit, job submission or
plot regeneration is part of this change. The current branch is
`breakpoint-delayed-neutral`; the already-authorized remote publication target
is `origin/breakpoint`. Unrelated working-tree changes are preserved.

The outgoing AGENTS focus is preserved in
[the exact snapshot](2026-11-12-agents-before-floating-fsr-promotion.md).
Only the current-focus heading is replaced; all global laws remain unchanged.

## Promoted implementation

The reviewed private revision was `/tmp/bp-fsr-photon-bqxy3jkj` with plugin
SHA256 `b1dcac7c3806e701861dcba1d37ad59b2fab7c3918ad0a60f09922a7ab10c65c`.
Register its fitter in the package CMake and its two focused tests. Set
`ExperimentalBeamSpotFsrLoss=true` in the compiled configurable and
`BP_BEAM_SPOT_FSR_LOSS=1` in the maintained run card and batch helper. Explicit
0 disables it. Batch manifests freeze/validate the option and reject
HitScanAll unless FSR is off. The compiled HitScanAll guard is retained.
The experimental property name is preserved to avoid an unnecessary rename.

Export matching post-FSR electron and massless collinear photon four-vectors
plus their full joint covariance in four corresponding families. 000/100 have
FSR undetermined and receive no ECAL measurement; 110 fits combined collected
ECAL energy; optional 111 separates charged/neutral observations. Original
ordinary endpoint/control branches remain separate. No pre-FSR TrackState or
old `beam_fsr_*` tuple-vector schema is introduced in the maintained version.
The immutable old production runtime/cards and existing ROOT files are untouched.

The authoritative equations, status codes, covariance ordering, recoil use and
restrictions are in [the package contract](../Reconstruction/RecBreakpoint/docs/beamspot-fsr-loss.md).

## Prior verification and physics limits

The private revision passed 46 independent Gaussian/kinematic covariance tests,
ROOT family serialization/reset tests, and four focused native Higgs events
(source1:3,15,112; source2:135), eight selected tracks and 16 fitted family views.
All 917 ordinary main-tree and 126 side-tree branches were exact OFF/ON.
Electron+photon recoil agreed with the previous artificial-state representation
within 3.85e-6 GeV. The 78 reference source/install fingerprints and previous
private runtime were unchanged before this promotion.

This is implementation promotion, not population physics validation. In the
previous FSR-only campaign, 8,930 common valid pairs gave 120–140 GeV peak-window
counts 5,681→7,209 and 120–130 counts 2,937→5,372, but below-120 tails grew
31→171. 482 pair failures were excluded from all electron-method comparisons;
9,458 parent events do not mean 9,458 successful floating fits. Cluster
contamination can mimic FSR (a no-FSR control gained false 7.828 GeV); unresolved
or uncollected photons remain missing. Early material loss can trade against
FSR, and missing dominant intervals remain a limitation. In the ten Both(AND)
event control, dressed pT RMS improved 13.18→8.32% while the fitted post-FSR
electron pT RMS worsened 12.09→17.09%. No inclusive improvement claim follows.

Historical particle-gun hard-loss inputs 11,16,17 are absent. The focused Higgs
events are stated explicitly and do not substitute unrelated indices for them.

## Promotion verification

Package and both test targets built successfully. Through the build runtime,
both CTests passed: 46 independent numerical cases and ROOT serialization/reset.
All 65 batch-planning tests passed, including compiled/card/helper default
agreement, explicit 0/true/malformed values and unsupported HitScanAll rejection.

Only the RecBreakpoint package was installed using its generated
`cmake_install.cmake`: plugin, Python configurable, components and confdb.
No current user process mapped the reference InstallArea before replacement;
all 496 known private-campaign native processes had completed. The batch queue
client was unavailable, so this is a host process-map/known-campaign check,
not a claim of a global scheduler audit. Five previous installed files were
copied to `/tmp/bp-fsr-promotion-20261112/previous-install` before replacement.

Installed default `ExperimentalBeamSpotFsrLoss=True` was verified through
`Configurables`. Installed and built plugin SHA256 both equal
`692aa4bf63bc0fdecf8455934c18a9257e4eb1536b416efe19db3893ed142372`.
The previous installed library was
`0433a66e2abb74c28b4f8cb678d015123ae8d84cbab5c933f6e1af333649f88d`.

Four fresh native processes exited 0 and loaded ONLY that installed plugin:
source1 events3,15,112 and source2 event135 with optional111 enabled, plus
source1 event3 with the ordinary default110 and an explicit FSR-off control.
No FSR environment override was supplied to the three enabled runs. They
cover four unique events/eight unique selected tracks, with extra repeated
controls. Comprehensive verbose state/covariance dumps were retained.

The installed optional111 runs match the validated private photon-output
revision EXACTLY in all 993 main-tree branches (78 source1 rows,17 source2 rows)
and all 126 side-tree branches (3+1 event rows). The maintained default110 versus
explicit-off check matches EXACTLY in all 917 ordinary main branches (32 rows)
and 126 side branches (1 row). There are exactly 76 additional FSR branches.
Both selected default110 tracks have 000/100 status1 with NaN FSR energy and
empty p4, successful110 status2 and disabled111 status-2. FSR110/000/100 output
is independent of whether111 is enabled. Retired pre-FSR-state branches are
absent. These checks distinguish the new families from unchanged controls.

No production submission or plot regeneration occurred. Git staging contains
only this promotion; earlier unrelated batch/card/test changes remain unstaged.
Generated logs, output ROOT files, cards, rollback copies and machine-readable
audits stay at `/tmp/bp-fsr-promotion-20261112`. The saved AGENTS snapshot has
379 lines; its global-law prefix is byte-identical to the new AGENTS. The sole
replaced heading, Current focus, is preserved exactly in the snapshot. No history
directory was moved, so no migration manifest was needed.
