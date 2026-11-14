# Photon pair conversion before ECAL — 2026-11-15

User asks how many eBrem and FSR photons convert to e-/e+ and their fractions.
All500 current REC files (100000 events) were read using saved photon birth and
compact original-photon endpoint truth. No simulation, reconstruction or fit ran.
No ECAL cluster weights or neutral-purity selection enters the count.

## Result

| Photon | All birth energies: converted / total | Fraction | Birth E >=1GeV: converted / total | Fraction |
|---|---:|---:|---:|---:|
|FSR|7,549 / 179,732|4.20%|3,820 / 39,267|9.73%|
|eBrem|17,106 / 338,509|5.05%|8,394 / 113,328|7.41%|

Fractions are photon counts, not event or track fractions. The denominator uses
198081 selected Z→ee electron tracks with unique G4 ownership and saved main-
electron ECAL entrance, covering99988 of the100000 read events.349 additional
selected tracks without that entrance truth are excluded, matching the cohort
of the maintained ECAL truth studies. FSR ownership is generator-based,
GsfGeneratorFsrLinks status1. eBrem is a saved direct G4 birth from the selected
electron, creator subtype3 and no later than its first ECAL-entry step.22
post-entry eBrem births are excluded. ISR and photons from other particles are
outside this population. No energy cut is applied in the all-energy column;
>=1GeV refers to each individual photon's birth energy, not summed track loss.

This counts conversion BEFORE the original photon first enters ECAL. The saved
original-photon history stops at first ECAL entry; it cannot measure conversion
inside ECAL or the whole calorimeter. A photon entering ECAL can convert later,
which is outside this count. Conversion products are not additional counted
photons in this original-photon denominator.

## Authoritative definition and checks

A saved original step before entry with processSubtype14 (fGammaConversion)
and trackStatus2 (fStopAndKill) is an e-/e+ conversion. Entry on that same record
would be kept as a separate boundary category. Current data have no such cases.
Installed Geant4 11.2.0 G4EmProcessSubType.hh and G4TrackStatus.hh confirm the
numeric definitions; conversion-to-muons is a different process subtype.
Edm4hepWriterAnaElemTool::recordGsfBremsPhotonStep stops recording at ECAL,
updates the compact last endpoint, and preserves original step process/status.
No indirect inference from missing photons or calorimeter contribution maps.

Every eligible photon has a classified endpoint: zero no-history, unfinished or
entry-boundary cases. The state partitions are exactly:

| Origin | Converted before ECAL | Original enters ECAL | Other termination | Total |
|---|---:|---:|---:|---:|
|FSR|7549|124123|48060|179732|
|eBrem|17106|319159|2244|338509|

All >=1GeV photons have only conversion or entry fates. All-energy other
terminations are mainly photoelectric process12:48041 FSR and2196 eBrem;
remaining processes are121 or world-boundary transportation91. They are not
silently counted as pair conversions. The diagnostic birth>=2m_e sample has
7549/111030 FSR conversions (6.7991%) and17106/320663 eBrem (5.3346%); this
minimum-energy diagnostic is not the >=1GeV column.

Independent MC audit of six converted photons (three FSR, three eBrem) confirms
direct saved e- and e+ daughters in every case. FSR root IDs come directly from
generator links; eBrem root IDs use unique exact saved birth momentum/position
matches. This is a focused cross-check of the process definition, not a full
MC-daughter persistence gate that could bias low-energy conversion counts.

Per-file source sizes/mtime_ns, collection/link index and G4-track consistency,
generator owner mapping, creator process and step ordering/uniqueness are checked.
The aggregate exactly recovers prior179732/338509 photon counts and198081 tracks,
and reproduces both prior birth-energy sums within the numeric tolerance. Terminal
process/status histograms independently recover all conversion counts. Partition
closure and zero unresolved-history assertions pass at every energy selection.
Self-checks cover conversion, entry, absorption, missing/unfinished endpoints,
step ordering and an entry/conversion boundary; both analysis scripts compile.

## Reproducibility and scope

Artifacts: TrackingPerformanceStudies/photon_conversion_fractions_20261115/.
`count.py` reads4 files concurrently with trivial uproot executors and per-job
source/code-hash caches. `jobs/*.json` retain exact sources, counts, endpoint
process histograms and representative cases; summary.json is the full500-file
aggregate, results.csv its compact table. verify_pairs.py and
pair_daughter_checks.json contain the independent six-pair checks. The1000-event
pilot is superseded. README/logs retain the method and execution evidence.

Sources are the existing
higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112 campaign. No historical
or deleted BP/REC tuples are used. No CEPC implementation, cards, build, install,
defaults, production jobs or gallery changes. Analysis scripts/data remain
uncommitted. Only this durable result record, AGENTS focus and its outgoing
snapshot are checkpointed. Global status/laws/commands remain identical;
outgoing focus is retained in
2026-11-15-agents-before-photon-conversion-fractions.md. The prior12 event displays
and gallery remain available unchanged at localhost8000/neutral_event_displays/.
