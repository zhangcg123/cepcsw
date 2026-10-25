# Breakpoint ECAL constraints from pre-reclustering PFOs

## User request / scope

User asked to switch charged and neutral constraint inputs to the newly saved
pre-reclustering PFOs, make the source configurable, and default to pre-reclustering.
No installation was requested in this turn. Only RecBreakpoint source, tests,
maintained card/batch helper, README and status records changed. No further PFA
source change, fit algebra change or batch submission. User edits to
subbreakpointjobs.sh and interval-identification documentation remain untouched.

## Controls and contract

```
fit.EcalConstraintInput = "PreReclustering"   # compiled/card default
fit.ChargedEcalInputClusters = ""            # linked ECAL collection
```

PreReclustering resolves to CyberPFOPreReclustering and
EcalClusterPreReclustering. FinalPFO resolves to CyberPFOPID and EcalCluster.
The one source selector controls both observations. It also controls passive
neutral collection when its measurement gate is off. Empty charged override
means direct selected-PFO cluster links; an explicit alternate source still
uses the existing unique constituent-hit matcher. Previous maintained steering
is FinalPFO + ChargedEcalInputClusters=TrkMergedECAL; historical final-energy
steering is FinalPFO + empty override (or explicit EcalCluster).

Default charged selection follows the selected PFO's original track link and
uses only its linked ECAL clusters. Neutral selection follows trackless,
zero-charge PFOs from the same source, excludes HCAL and empty-hit clusters,
retains the input KF ECAL-reference window (10 mrad theta / 200 mrad phi), and
uses unchanged error .011*sqrt(E) + .004*E GeV per cluster, quadrature for sums.
This does not require final PID and does not add neutral PFO energy or HCAL.

New track-row fields: ecal_constraint_input, ecal_constraint_input_pfos,
ecal_constraint_input_clusters. Existing charged_ecal_input_clusters records
the resolved override. Collected neutral indices now refer to the configured
ECAL collection; with the default they are NOT indices in the final neutral_pfos
diagnostic tree. That side tree and charged_pfo_* main-tree diagnostics retain
their final-CyberPFOPID meaning. Selected neutral energies/errors remain directly
stored per track. Snapshot object IDs are different from final object IDs.

The maintained card loads the pre-PFO ECAL/HCAL graph and checks its ROOT branches
before starting PodioInput. Missing source is an error, never silent fallback.
An early test without this preflight exposed the existing PodioInput missing-
collection segmentation fault; guarded rerun exits with a clear ValueError
before event processing. The C++ consumer also reports unavailable data if
retrieval throws. Existing empty collections/no eligible candidates retain
the preceding accepted constraint stage, as before.

Batch helper freezes BP_ECAL_CONSTRAINT_INPUT, explicitly defaulted to
PreReclustering, and BP_CHARGED_ECAL_INPUT_CLUSTERS. New rec verification requires
pre-PFO/ECAL/HCAL snapshot collections before deleting owned calodigi intermediates
when the frozen selector requests them. Older manifests lacking the selector
retain their old rec verification behavior. No submit-script stages, paths or
group were modified. Old prepared cards that omit the selector inherit the
new compiled default after installation; reprepare explicitly for legacy input
or use the new card for regenerated rec, rather than editing frozen checksums.

## Validation

Workspace: /tmp/bp-pre-pfo.Ra9UAA/.
Input: /tmp/pfa-pre-reclustering.xY0wma/neutral-full.root (seed-1 events 0–19,
new pre-PFO snapshot, otherwise unchanged final-PFA output).
Selected original zero-based events 0,3,5,12,13,16,17: 8 track rows, including
two CompleteTracks at event 13. Beam/charged/neutral=111, ElectronOnly,
SigmaLogLoss=.001, Persistent6D, free-loss on, all backward filters off,
truth layer-owned selection, verbose dumps on.

Three local runs:

- reference.root: installed old runtime and frozen old maintained card,
  explicit TrkMergedECAL charged source.
- legacy.root: rebuilt consumer, FinalPFO and explicit TrkMergedECAL source.
- pre.root: rebuilt consumer, new default PreReclustering and empty override.

Results:

- All 851 prior track branches reproduce exactly in legacy mode.
- New default physics outputs match legacy mode in this same-observation
  fixture. Only source strings/charged cluster indices differ (5 fields).
- All 57 final-PFO neutral_pfos side-tree fields unchanged.
- Seven charged selections read snapshot energies directly; event 12 has no
  charged ECAL selection. Neutral observations occur at events 5 and 12:
  2.9353911876678467 and 17.253450393676758 GeV, respectively.
- Event 5 applies the charged and neutral additions; event 12 has no eligible
  applied neutral fit despite a collected neutral. Other no-neutral rows use
  the existing prior-stage fallback. Collection status is not fit success.
- All 11 C++ tests pass, including direct pre-PFO ownership without the
  alternate-hit-match ambiguity and rejection of HCAL links by both collectors.
- 42/43 Python batch tests pass. The sole existing failure is
  test_submission_wrapper_rec_default_and_full_chain_override: user-edited
  subbreakpointjobs.sh sets trk,calodigi,rec; test expects breakpoint. This
  unrelated user setting is preserved. New selector freeze/validation and
  missing-pre-PFO cleanup guards pass.
- Missing pre-PFO input rerun exits cleanly with explicit remediation;
  valid-input gaudirun --dry-run preflight succeeds.

Durable fixture checker:

```
python3 Reconstruction/RecBreakpoint/tests/check_ecal_sources.py \
  /tmp/bp-pre-pfo.Ra9UAA/reference.root \
  /tmp/bp-pre-pfo.Ra9UAA/legacy.root \
  /tmp/bp-pre-pfo.Ra9UAA/pre.root \
  /tmp/pfa-pre-reclustering.xY0wma/neutral-full.root
```

The exact equality of pre/legacy physics is fixture-specific because selected
energies happen to agree. It is not a claim that all pre/final associations are
identical or that performance improves. This is a source-routing regression,
not a new population validation.

## Deployment / unchanged limitations

Built RecBreakpoint library SHA256:
40daa5b79c831ee0c84616652fbd26a1521239a5ae4ae73b04acc032fa9a97c2

Unchanged installed libraries:

```
RecBreakpoint: 2d868baf1b3f2280d43a727ad2e7839608d17d5a1ac97b3bbb0bd73659d34a69
CrystalCaloRec: 686747f7273dcc1c07d10be98ca121f43875edf2aef8858f81bf794c7ae78254
```

Both the prior PFA exporter and this consumer need coordinated installation
before batch use of the default. Rec files then need the saved pre-PFO graph;
old production rec files do not have it. No installation/batch happened here.

ElectronPhoton still requires separate neutral OFF; no automatic resolved/
unresolved sum routing was implemented. Charged/neutral additions still require
one selected breakpoint. All solver, unconstrained, truth/diffuse and backward
workflows remain as before. No endcap/population physics validation is claimed.
Current status replaces the outgoing focus, preserved verbatim in
2026-10-26-agents-before-pre-pfo-constraint-input.md; global laws are unchanged.
