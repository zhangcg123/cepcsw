# Optional truth-assisted production-electron gate

## Authority and scope

The user requested a standalone configurable class inside RecBreakpoint to
select the two production-side electron tracks before expensive fits/scans.
The user confirmed no active jobs and authorized installation after validation.
No batch submission, remote/branch change, PFA/shared-KF/GSF change or new
generator run is authorized or performed for this task.

## Truth contract

The existing eeH generator record does not contain an explicit production-Z
parent. In all 200 events of `higgs_full_20261030/rec-higgs-1.root`, the two
stable generator electrons are outside Higgs ancestry and have no Z ancestor;
the recorded Z bosons belong to the Higgs decay. Thus an ancestor-PDG23 cut
would be wrong here. This selector identifies the unique production-side pair
for this eeH record, not a certified pure on-shell ZH subprocess (eeH can also
include Z-fusion). Check each new generator/process ancestry before enabling.

`TruthElectronSelector` requires a generator Higgs, then exactly one stable
generator e- and e+ without any Higgs ancestor. It follows parent links through
copy/shower nodes; it does not assume common MCParticleGen/MCParticle indices,
or hardcode Whizard indices/PDG94 paths. Extra non-Higgs stable electrons make
the truth pair ambiguous: no closest-Z guess. Invalid/cyclic ancestry fails
closed. Geant4 secondaries (`generatorStatus=0`) are not truth candidates.

`TrackTruthMatch` shares the existing flat-tupliser matching rule: sum positive
rounded association hit counts by MC object, require unique largest count.
Nonfinite/out-of-range weights are ignored rather than passed to integer
rounding. For duplicate tracks of a selected MC particle, choose most matched
hits, then highest matched-hit purity. Exact ties are not resolved arbitrarily.
One usable partner remains selected if the other is missing/tied.

## Integration and controls

`SelectTruthProductionElectrons=False` is the compiled/card/batch default.
`BP_SELECT_TRUTH_PRODUCTION_ELECTRONS=1` enables it; batch preparation validates
and freezes this boolean. The standard card and submission wrapper expose it.
No pre-existing campaign card or tuple is rewritten.

The class runs once per selected event, before heavy per-track fitting. The
original CompleteTracks input, IDs, input ordinals, hit/PFO/cluster links and
event environment remain intact. Every original track keeps its baseline,
matched truth and observation tuple row. Unselected rows have status0,
invalid output indices, NaN refitted endpoints and empty scan vectors; no
breakpoint fit, Minuit or scan is run for them. Accepted tracks use unchanged
fit/constraint/location logic. Disabled mode does not read MCParticle for this
gate and retains the previous workflow.

Both trees save `production_electron_selection_status`, `_error`,
`production_electron_mc_indices`, `production_electron_track_indices`,
`production_electron_candidate_counts` (vectors ordered e-,e+). Status0=off,
1=both tracks,2=one,-1=no generator Higgs,-2=nonunique/missing truth pair,
-3=neither reconstructed partner usable,-4=invalid ancestry/input.
The main-tree `production_electron_track_selected` is0=off,1=selected,-1=skipped.
The event-side status remains present even when no input track exists.

## Validation and deployment

Temporary evidence: `/tmp/bp-production-selector-36P9dF/`.

- All16 CTests pass, including the new ancestry/secondary/duplicate/tie/cyclic
  record fixtures and the existing shared-matching truth-ECAL tuple regression.
- All64 batch tests pass, including default-off, frozen enable and invalid-value
  rejection tests. Existing unrelated batch/helper edits were preserved.
- Direct selector-only audit of all200 REC events:198 select both partners,
  2 select one (events79/179);398 tracks selected out of433 inputs. Every event
  has exactly one non-Higgs stable generator electron of each sign. Events0,
  17,22 select original input ordinals(0,1),(1,0),(1,0), respectively.
- Direct old-library/OFF/ON reruns cover gun seed1 events11/16/17 and Higgs
  seed1 events0/17/22, with verbose component dumps, normal truth-location
  endpoints and additional110 scans. OFF preserves all9,812 previous
  branch-row values exactly (NaN-aware), including the side tree and scans.
  ON preserves all5,682 previous main branch-row values for the6 accepted
  Higgs tracks. The4 skipped rows (one Higgs extra plus3 no-Higgs gun controls)
  preserve baseline/truth identity, have status0/NaN refits and no scans.
  Event-side pre-existing data are unchanged in both modes.
- Package-only installation updates RecBreakpoint and its Gaudi metadata in
  the default105 runtime. Build/installed SHA256 both:
  `ca0f7ac186577d9a8123beda61bdd1ffdfe89acc0b55934eec839438afba534a`.
  Previous installed SHA256:
  `25fb531ed7774c8c3f9be6607935660d088557ac335ac79fa7d0365f64c872c9`.
  The previous library is retained as `previous_RecBreakpoint.so` in the
  temporary evidence directory. No global rebuild/install or runtime pin.
  A plain project-default `gaudirun.py` probe (no build-run path override)
  verifies the compiled default is OFF, enables the property through the card,
  and initializes/finalizes successfully. Existing frozen cards remain untouched.

No visible-Higgs population or CPU-speed/physics-performance claim is made.
The source gate is a truth-assisted study convenience, not real-data PID.

## Memory migration

The complete outgoing AGENTS is preserved in
`2026-11-05-agents-before-production-electron-selector.md`.
Global introduction, active laws/scope and essential commands are unchanged.
Outgoing Current focus (all subsections) maps to that snapshot and the existing
`2026-11-05-scan-null-same-ecal-observation.md` record. No history file or
directory is moved/deleted; a migration manifest is therefore not needed.
