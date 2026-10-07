# Breakpoint ECAL reconstruction workflow (2026-10-03)

## Outgoing AGENTS.md current focus, preserved verbatim

Active work remains RecBreakpoint on local `breakpoint`. A new independent
`DiffuseAugmentedRTS` experiment uses an exact flat-prior, persistent 6D
loss coordinate at one selected interval. Its compiled default is off, while
the maintained-card default is on. It publishes a separate endpoint/status
and never replaces
ordinary RTS, backward, free-loss, beam-guided or truth-centred results.
The package builds, numerical tests pass, and a 25-event barrel smoke has
12/12 selected-interval fits without fallback. A same-code on/off check found
ordinary RTS/backward endpoints bit-identical. Some hard-loss examples recover
IP momentum, but tiny-loss examples can worsen badly. This is mechanical and
selected-event evidence only, not physics validation. The exact method,
status meanings, gate and preserved outgoing focus are in
`agents_record/2026-10-03-exact-diffuse-breakpoint.md`.

Next: test categorized pT resolution, clean-track safety, failure and tail
rates, and fitted-loss calibration on independent barrel/endcap samples;
diagnose early-breakpoint tiny-loss failures before proposing a physical
loss-boundary or model-choice gate. Do not alter common KF/GSF workflows or
make this the production default from selected improvements. Current source
ROOT campaigns include `sim_large_barrel_20261001/` and `breakpoint_barrel/`;
regenerate tracker inputs for new studies rather than relying on temporary
smoke-test files.

## Diffuse population evidence preceding the workflow change

The 100-file `breakpoint_barrel/` batch contains 16,435 topology-clear
selected tracks in the maintained analysis. Diffuse inclusive width68 is
0.535% versus 0.354% for beam-guided free-loss and 1.079% for KF. In light
losses it is 0.913% versus 0.399% and 0.246%, respectively. Of 9,227 applied
diffuse fits, 3,315 have negative fitted log-loss. VXD L4→L5 light-loss fits
have median fitted b=-0.231 versus selected-interval truth b=0.000675.
This is a truth-selected location campaign, so the discrepancy is in loss
inference and/or state fitting, not reconstruction-only interval discovery.
Reproducible plots and diagnostics are under
`TrackingPerformanceStudies/breakpoint_diffuse_20261003/`.

## New data flow

The dedicated breakpoint batch worker now supports
`sim -> trk -> calodigi -> rec -> breakpoint` with any stage subset and
external predecessor inputs. The default is
`trk,calodigi,rec,breakpoint` from `sim_large_barrel_20261001/` into the new
`breakpoint_barrel_ecal/` directory; the previous batch is untouched.
Generated-only tracker/digitization/reconstruction cards carry simulated
calorimeter hits, CompleteTracks, tracker hits/associations, and embedded
Geant4 material-step and primary-bremsstrahlung-photon path provenance through
to rec. The shared templates and GSF workflow
are unchanged. Breakpoint reads the retained rec event, while its default
output remains only the flat tuple; ECAL is not yet part of the breakpoint
fit or flat schema.

The local dry-run prepared one seed without scheduler submission. A one-event
worker run completed trk, calodigi, rec, and breakpoint. The rec tree had one
event with `EcalCluster`, `CyberPFO`, `CompleteTracks`, digitized ECAL hits,
`GsfG4MaterialSteps`, and `GsfSimTrackerHitG4StepLinks`; the ECAL cluster had
44.1714 GeV. The breakpoint flat tuple had one successful row, and its
KF/RTS/free-loss/diffuse pT values for seed 1 event 0 matched the stored
pre-ECAL-input result exactly. The worker removed only its verified tracker
intermediate; it retained rec and calodigi. This establishes tuple hand-off,
not ECAL information quality or any eBrem recovery improvement.

A second full one-event smoke used seed 3 event 0, which contains primary
bremsstrahlung photons. The closed rec event retained three
`GsfG4BremsPhotons`, 1,606 `GsfG4BremsPhotonSteps`, 619 material steps, 233
SimTrackerHit-to-G4-step links, two `EcalCluster` objects, three `CyberPFO`
objects, and one `CompleteTracks` object. The breakpoint flat tuple completed
with one successful ordinary row; KF, RTS, free-loss and diffuse IP pT matched
the old tracker-input batch exactly for that same seed/event/track. The
photon, photon-step and material-step counts (3/1606/619) also matched the
original simulation event exactly.

Next, submit a fresh new-output campaign, then define a reproducible external
track-to-ECAL association and test whether cluster/shower variables resolve
the early-loss ambiguities without degrading clean and light-loss controls.
