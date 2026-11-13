# FSR generator-parent assignment census — 2026-11-13

All500 current REC_ECALTRUTH files x200events =100000events. Read current saved
MCParticle generator histories and GsfGeneratorFsrLinks. Source path/size/mtime
matches the existing native cluster study before/after each read and again at
final validation. No simulation, reconstruction, build/install or plotting change.
No energy, ECAL-entry, collection-window, fit-success or recoil-mass cuts.

## Results and the denominator correction

All saved FSR origin records: 184,935.
Unique assignment: 184,935 (100.000000%).
Ambiguous assignment: 0 (0.000000%).
Missing emitter index: 0; missing final-electron index: 0.

For the SAME198430 reconstructed selected production-electron tracks used in
the neutral study: 180,123 FSR records, all 180,123 unique;
0 ambiguous. Their count exactly reproduces the old all500
native-link selected_fsr_roots total. All photon energies are included.

The previously described third case is NOT an independently identified FSR
population: no recognized electron-copy parent means no dedicated FSR record.
It is therefore impossible to measure its fraction of ALL physical FSR using
this tag alone. Zero entries of this kind in the FSR collection is structural,
not proof zero physical FSR photons are missed. Do not publish100/0/0 as a
physics completeness result. The measured statement is100% unique /0% ambiguous
among the saved records, with untagged physical-FSR fraction undetermined.

There are 2,880,667 stable generator photon records in total;
2,695,732 do not have our electron-FSR tag. They include many
ordinary decay/other-origin photons and must NOT be counted as missing FSR.
Direct-parent PDG111 accounts for 2,268,385 of them;
parent PDG22 accounts for 200,000. These are parent-record
patterns, not a complete physics ISR/FSR classification. The full parent-PDG
breakdown and broad-generator-record partition are saved in summary.json.

No untagged stable photon reaches a tagged outgoing-electron copy through a
photon-only copy chain in this audit: 0
(all electrons), 0 (selected electrons).
Selected-electron explicit PDG23 ancestor count is 0;
the saved history therefore cannot independently close the untagged category
with a Z-parent requirement. The selected production electrons come from the
sample/selection definition, not a mandatory explicit Z node in these records.

## Validation and interpretation

For EVERY event, independently replay the current GeneratorFsr.h rule over the
saved MC ancestry and require exact agreement of photon index, status, emitter
index and final-electron index with EVERY saved link; no duplicate photon links.
Synthetic unique/ambiguous/no-electron-parent cases pass. Stable generator
photons partition exactly into unique/ambiguous/untagged records. All500 input
identities and old selected-root totals agree. Photon-count statistics, not
event fractions. No angular inference and no new parent assignment introduced.

A unique generator-history emitter remains a generator convention; it does not
eliminate QED interference or prove a uniquely observable physical emitter.
This census is a bookkeeping/coverage check, not physics validation.

Artifacts: summary.json, fractions.csv, jobs/*.json, run.log, analysis code hashes.
Generated analysis files remain uncommitted under project rules. The gallery and
all production tuples/cards/installations are unchanged.

Current production H_F remains1644ae8; latest window200 study remains analysis-only.
The complete outgoing status is preserved in2026-11-13-agents-before-fsr-assignment-audit.md.
