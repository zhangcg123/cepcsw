# Track-matched truth and CyberPFO PID flat schema (2026-10-03)

## Outgoing focus before photon last-step deployment

Active work remained RecBreakpoint on local `breakpoint`, diagnosing its
early-breakpoint loss ambiguity with external ECAL information. Diffuse RTS
underperformed beam-guided free loss in the 100-file barrel comparison,
especially for light loss and VXD intervals; no method was physics-validated.
The prior ECAL workflow and population evidence were preserved in
`agents_record/2026-10-03-breakpoint-ecal-workflow.md`.

The dedicated worker read retained REC events with `CompleteTracks`,
`CyberPFOPID`, ECAL clusters, track/MC associations, and Geant4 provenance.
The new breakpoint flat schema gave each track row its own matched-MC
`truth_pt`, hit-count matching purity, charged-PFO PID and ECAL clusters, plus
a separate once-per-event `neutral_pfos` tree. The fit was unchanged. Focused
build-library tests preserved KF/RTS/free-loss endpoints; the installed
runtime had not yet been updated for this schema. Exact fields, old-tuple
incompatibility, and smoke gates were recorded here and in the package README.

The outgoing next step was to deploy the plugin when batch activity permitted,
regenerate breakpoint flats from retained REC, then measure whether
charged/neutral ECAL observables discriminated early loss while preserving
clean/light-loss tracks. Neutral-cluster truth provenance still required a
separate validated bridge from CyberPFO's output clusters to original
calorimeter hits; synthetic hit IDs were not a truth match. Successful I/O
was not physics validation.

## Outgoing AGENTS.md focus, preserved before replacement

Active work remains RecBreakpoint on local `breakpoint`, now preparing external
ECAL information to diagnose its early-breakpoint loss ambiguity. The 100-file
barrel comparison found diffuse RTS worse than beam-guided free loss overall,
especially for light loss and VXD intervals, despite truth-selected breakpoint
locations. Diffuse fitted b is often negative or far from matched truth; the
method is not physics-validated. The detailed result and preserved outgoing
focus are in `agents_record/2026-10-03-breakpoint-ecal-workflow.md`.

The dedicated breakpoint worker now supports the default chain
`trk -> calodigi -> rec -> breakpoint` from retained simulation inputs, with
optional sim and stage subsets. Breakpoint reads the retained rec event, which
contains ECAL clusters/PFOs, CompleteTracks, and G4 tracker-step provenance.
A dedicated generated-card pass-through also retains primary eBrem-photon
paths for later ECAL association studies.
A one-event end-to-end smoke passed and reproduced the prior KF/RTS/free-loss/
diffuse pT values. Existing GSF cards/source and older batch outputs were not
changed. ECAL variables do not yet steer RecBreakpoint or enter its flat tuple.

Next: run a fresh output campaign, establish a track-to-ECAL association and
test whether ECAL observables help discriminate early loss while preserving
clean/light-loss tracks. Do not treat successful I/O or selected-event gains
as physics validation. The exact workflow and smoke gate are in the dated
record above; the user-facing controls are in
`Reconstruction/RecBreakpoint/README.md`.

## Correction and implementation

The former `truth_pt` was calculated once from the sole generator-status
electron and repeated on every `CompleteTracks` track row. This did not
represent a per-track truth association and must not be used as such in
historical flat tuples. `TruthDiagnostics` was removed. The new `truth_pt`
comes from the unique highest-weight MC particle in
`CompleteTracksParticleAssociation` for the row's exact track. The association
producer sets weight to a count of reconstructed hits assigned to an MC
particle. The tuple records track-hit count, best-MC hit count, total
truth-linked hit count, their best/track purity ratio, status 0/1/2 for
no/unique/tied best association, and matched MC index/PDG/generator
status/energy, momentum x/y/z, charge, vertex/endpoint coordinates, and
parent MC indices. No match or tie means NaN `truth_pt`.

The per-track tree records `CyberPFOPID` entries that link to the same
`CompleteTracks` object, all their PID hypothesis likelihoods, and their
ECAL-only cluster energies/positions. Exact charged-PFO and charged-ECAL-
cluster counts are independent; neither is capped at 3. A second `neutral_pfos`
tree in the same file has one row per selected event and stores each neutral
PFO once, with its ECAL-cluster energies and positions. Its `event_index`
joins to track rows. These are passive reconstructed associations, not an
ECAL-photon truth match. Existing original ECAL-hit truth associations do not
directly join to CyberPFO's synthetic calibrated output hits, whose object IDs
and cell-ID encoding differ.

Only `Reconstruction/RecBreakpoint` and its dedicated card/batch helper were
changed. The 2026-10-03 local build of `RecBreakpoint` passed. A build-library
smoke on rec-barrel-8 event80 found one track with 233/233 hit purity, one
charged PFO/ECAL cluster, and two neutral PFOs/ECAL clusters in the companion
tree. The PFO IDs, PID hypotheses and energies agreed with the input REC
collections. A verbose same-code check on events11,16,17,22 found old/new KF,
RTS and free-loss pT identical for all five track rows. Event22 has two
tracks: a 226-hit primary track and a 7-hit fragment; both are associated
with the same primary MC electron with purity1. Thus purity alone does not
establish track completeness; hit counts and topology cuts remain essential.
The final helper revision rebuilt and passed a repeat event80 smoke: stored
`truth_pt` equals hypot of stored MC momentum components within float input
precision, and the neutral companion row remains present. The 33 dedicated
batch-helper unit tests passed. This is an I/O and
non-regression gate, not physics validation. The built plugin was used for
these tests; installed runtime deployment was not part of this gate.

A multi-MC test on rec-barrel-1 event23 produced four track rows and one
neutral-event row. Track0 matched MC index3 (electron daughter) at 119/119
hits, track1 matched MC index4 (positron daughter) at 224/224, and track2
matched MC index0 (primary electron) at 222 best hits out of 225 linked/total,
giving purity 0.9866667. Track3 is a seven-hit primary-electron fragment at
7/7. This confirms that `truth_pt` can differ by track and that the best-MC
purity reflects competition rather than blindly repeating the gun pT. No
missing-truth-link example appeared in the checked seed1/seed8 REC scans.

## Primary eBrem-photon snapshot in the neutral event row

The `neutral_pfos` event row now also stores one entry per recorded
primary-electron eBrem photon. Parallel arrays preserve the photon collection
index, Geant4 track ID, parent track/step IDs, emission position (mm), and
emission four-momentum `(E, px, py, pz)` (GeV). The first linked photon step
whose post-point enters ECAL provides the ECAL-entry position and four-momentum,
with its step number and `ecal_entry_status=1`. Photons without a recorded
entry remain in the arrays with status 0, step number -1, and NaN entry values.
This is a truth path snapshot, not a photon-to-neutral-PFO association.

The built plugin passed a one-event check on rec-barrel-3 event0: all three
photons' birth and ECAL-entry four-vectors matched the REC collections. A
second check on event37 found three photons, including one with no ECAL-entry
step; its status was 0 and its entry values were NaN. No tracking fit logic
was changed by this tuple extension.

The event row was subsequently extended with the highest-numbered recorded
Geant4 step for each photon, including those without an ECAL entry. The flat
snapshot retains its process subtype, statuses, step endpoints, four-momenta,
times, length, and energy deposit. Missing steps are flagged separately while
birth truth stays populated. In rec-barrel-3's 200 events, six of 208 photons
had no ECAL-entry step: three ended with subtype 12 (photoelectric), three
with subtype 14 (conversion), all with killed-track status and zero post-energy.
This is a sample diagnostic, not a universal classification of non-entry.
The focused event37 check reproduced the subtype-12, killed-track, zero-energy
last step in the new flat tree while preserving its finite birth energy and
NaN ECAL-entry energy. The RecBreakpoint build and all 33 batch-helper tests
passed, and the installed library was checked byte-for-byte against the
validated build; the installed-runtime smoke reproduced the last-step fields.
Regenerate only the `breakpoint` stage from retained REC files into a new
output directory as documented in the package README. No existing flat or REC
files were overwritten.
