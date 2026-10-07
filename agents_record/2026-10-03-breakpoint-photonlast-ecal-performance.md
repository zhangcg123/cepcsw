# Breakpoint photon-last-step ECAL performance (2026-10-03)

## Outgoing focus preserved before replacement

Active work remained RecBreakpoint on local `breakpoint`, diagnosing its
early-breakpoint loss ambiguity with external ECAL information. Diffuse RTS
underperformed beam-guided free loss in the 100-file barrel comparison,
especially for light loss and VXD intervals; no method was physics-validated.
Prior ECAL workflow and population evidence were preserved in
`agents_record/2026-10-03-breakpoint-ecal-workflow.md`.

The validated RecBreakpoint plugin was installed. It read retained REC events
and wrote track-matched MC/PFO fields plus a once-per-event `neutral_pfos`
tree. That tree retained each primary eBrem photon's emission truth, first
ECAL entry if recorded, and full last recorded Geant4 step even without ECAL
entry. It did not associate photons to reconstructed neutral PFOs. The fit
itself was unchanged; focused installed-runtime and batch-helper checks
passed. Schema, old-tuple incompatibility, and smoke evidence were in
`agents_record/2026-10-03-breakpoint-matched-pfo-flat.md` and the package
README.

The outgoing next step was to rerun only breakpoint from retained REC into a
new flat output directory and test whether charged/neutral ECAL observables
helped identify early loss while preserving clean/light-loss tracks. The old
flats were to be kept for comparison. Neutral-cluster truth provenance still
required a validated bridge from CyberPFO output clusters to original
calorimeter hits; synthetic hit IDs and event-level photon arrays were not a
truth match. Successful I/O was not physics validation.

## Completed analysis and limits

The user reran the breakpoint stage into `breakpoint_barrel/`. All 72 job
manifests completed. The new flat files contain the photon last-step schema.
The analysis in `TrackingPerformanceStudies/breakpoint_photonlast_20261003/`
reproduces the established 11,820 selected track keys and exact old truth pT;
no selected rows were excluded. It saves `performance_rows.csv`, summary
JSON, regenerated tracking pT plots, and charged/charged-plus-neutral ECAL
energy residual plots. The old comparison plots remain under
`TrackingPerformanceStudies/breakpoint_current72_pfo_20261003/`; the old flat
ROOT files were replaced by the new campaign, contrary to the prior plan to
keep them in a distinct output directory.

Charged energy is the unique set of ECAL clusters on charged PFO(s) linked to
the selected CompleteTracks object. Charged-plus-neutral adds every neutral
PFO's ECAL cluster in that event, deduplicated by EcalCluster index. There
were zero duplicate or overlapping cluster references in this cohort. The
truth denominator is the track-matched generator primary electron's energy.
This inclusive neutral sum is not an eBrem-photon-to-cluster association.
Histograms are normalized to the full selected category, and width68 is
computed on the full residual population.
Seed-1 events 0, 1, and 3 were independently spot-checked against REC
`EcalCluster` indices and energies with exact agreement.

| Category | N | Charged width68 (%) | Charged+neutral width68 (%) | Within ±5%, charged | Within ±5%, charged+neutral |
|---|---:|---:|---:|---:|---:|
| Inclusive | 11,820 | 2.988 | 2.067 | 9,623 | 9,521 |
| No eBrem | 4,460 | 1.186 | 1.631 | 4,073 | 3,641 |
| eBrem <1% | 3,869 | 1.208 | 1.715 | 3,522 | 3,160 |
| eBrem ≥1% | 3,491 | 7.384 | 3.917 | 2,028 | 2,720 |

Neutral addition recovers hard-loss energy on average but degrades clean and
light-loss distributions, and inclusive ±5% containment falls by 102 tracks
despite a narrower width68. The result motivates selective association, not
an unconditional energy sum or a claim of tracking improvement. Cluster
energy remains passive and did not steer RecBreakpoint.
