# Where the resolved FSR neutrals miss the50/200mrad window

Full100000-event cached totals: missed FSR-weighted neutral energy is
109586.981GeV. Of it108516.292GeV (99.02%) belongs to FSR-dominated clusters,
not merely small FSR contamination of unrelated clusters. Missed dominated
clusters:14061. Keep the existing Ecluster<=55.75722694396976GeV cut.

Truth geometry study uses representative seeds1,22,43,63,84 x200events =1000.
The20largest missed cluster proxy energies from the full sample are examined
separately as biased stress cases, never pooled into representative statistics.
No simulation, PFA or fit rerun. Read only current saved REC truth/cluster data.

Use cached direct MC fractions and selected-track IDs from the completed study.
Follow MC parents to uniquely owning saved generator FSR roots. A cluster with
one root contributing >50% of its TOTAL MC weight has a definite photon/electron
pair. Use GsfGeneratorFsrLinks photon relation -> GsfG4FsrPhotons -> saved photon
last/ECAL step. Map its parentTrackID to the electron's exact first-ECAL step.
No nearest-position truth match. Original photons converting before ECAL have
no original-photon entrance, and are reported separately.

In1000events there are268FSR-category-dominated eligible neutrals,134collected
and134missed by BOTH selected-electron windows. All134misses have one dominating
photon root;10collected clusters contain multiple FSR roots with none >50%.
The plots therefore show258single-photon-dominated neutrals:124inside/134missed.
Generator panel compares photon MC momentum to the FINAL stable owning electron
MC momentum, not the virtual pre-radiation emitter. ECAL panel compares cluster
position to owning Baseline KF AtCalorimeter reference. Collection is the union
over both selected electrons; in this plotted sample none is recovered only
by the other electron. Full view shows all258points; zoom labels out-of-view
counts. Marker size follows saved cluster energy. MC weights are response proxies.

Misses relative to the owning electron:55theta-only,16phi-only,63both. Of134,
123already fail the same rectangular limits using generator directions; these
carry88.80%of the missed sample FSR-weighted energy. This is a statement about
angular-component limits relative to the final electron, not automatically a
3D opening-angle threshold.11start within these limits but miss at ECAL;
transport/bending and conversions contribute to that subset.

120misses have original photon ECAL entry and truth electron ECAL entry.115of
120remain outside the window with BOTH true entrances. Only3would be collected
by substituting the photon truth entry for reconstructed cluster position;
only2by substituting the truth electron entrance for KF reference. Median
absolute cluster-photon offsets are1.29mrad(theta),1.21mrad(phi); KF-electron
truth offsets0.041/0.404mrad. Reconstruction errors are not the main explanation
in this sample.14remaining originals stop at processSubtype14, gammaConversion,
confirmed against local Geant4 G4EmProcessSubType.hh; their descendants can
still make reconstructed neutral clusters.

Generator ancestry examples are in generator_ancestry_examples.json. Seed63,
event142: photonMC14 and final electronMC13 both have electron parentMC11;
its saved virtual mass36.506GeV, photonE38.176GeV, final electronp14.255GeV,
3D opening1.797rad (103degrees). The electron-copy ancestry ends at non-electron
PDG94. Other checked wide-angle examples similarly have25-46GeV virtual
outgoing-electron copies. Thus the checked wide pairs are present in generator
history, not produced by cluster positions or a direction-based FSR tag.

Separate bending example seed349/event86: photon55.34GeV, electron5.28GeV;
birth phi difference7.71mrad, truth ECAL phi difference304.50mrad. The photon
is well reconstructed; the electron propagation after radiation separates it
beyond the phi window. Generator FSR need not remain close to the FINAL tracked
electron. A resolved-neutral sample excludes FSR merged into charged clusters,
so it must not be used to claim that most generator FSR is wide-angle.

Artifacts: full_sample_missing_summary.json (all100k), trace_summary.json and
mechanism_summary.json (representative/stress populations separated), per-source
traceJSON, source identities, ancestry examples and code/plot hashes. Both plots
are full/zoom generator-versus-ECAL offsets; no source algorithm was modified.

## Project state and validation

This study answers the user's concern that the window seems large. It does not
change collection cuts, FSR identification, native tracking/PFA, installed
libraries, cards, production job state or input data. Full-sample acceptances
remain60.186%FSRenergy/49.264%FSRdominated clusters at50mrad. The detailed
mechanism fractions above come from1000representative events, with20largest
misses separate; do not relabel them as100000-event mechanism statistics.

Saved in-window flags reproduce the original exact angular union for every
traced target. Dominating roots have valid direct generator/G4 relations, and
all25source file identities match the all-angle campaign cache. Generated plots
were visually inspected and output hashes verified. The original plotting
assertion incorrectly assumed every FSR-category-dominated cluster had one
photon root above50%; corrected to the validated258single-photon subset. No
scientific data was changed to satisfy it.

Outgoing complete AGENTS is preserved in
2026-11-12-agents-before-missed-fsr-diagnosis.md. Only current focus changes;
active scope/laws remain intact. No history-directory migration. The
plot-gallery skill delegates new figure publication to one dedicated agent;
previous plots are retained. Analysis scripts/artifacts remain uncommitted.

## Gallery publication

Dedicated plot-gallery agent added full/zoom PNGs as the first section. Gallery
validates14PNGs/6sections, each PNG covered once. Existing ailogin002 managed
localhost8000 server PID2122174 updated live without restart. Page and both
new imagesHTTP200; source hashes match copies, independently rechecked by main.
All preceding figures/sections preserved. Manifest and plot copies uncommitted.
