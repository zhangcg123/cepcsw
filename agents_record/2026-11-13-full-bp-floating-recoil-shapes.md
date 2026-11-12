# Full floating-FSR BP production: inclusive and radiation-category recoil — 2026-11-13

## Request and source clarification

The user initially called the newly completed files GSF. Local inspection found
only the existing Nov11 GSF campaign; the user explicitly chose
`BP_FSRFLOAT_NOSCAN_500jobs_200evt_20261112` in the clarification reply.
The new plots therefore read the new BP production, with the existing GSF
BestBranch as a reference. No new GSF production is claimed.

All500 BP jobs have completion markers and200event-side entries each, covering
100000raw inputs. Every frozen card/checksum, source/output identity and relevant
control is verified. Truth selects a single eligible material-eBrem location or
null; location scans and111 areOFF. Floating FSR isON. No reconstruction,
source/build/install/card/batch change or new submission is part of this task.

Artifacts: `TrackingPerformanceStudies/bp_fsrfloat_recoil_categories_20261113/`.
The uncommitted analysis contains analyze.py, snapshot.json, focused_check.json,
events.csv, histogram.csv, summary.json, plot hashes, README and five PNG/PDF
pairs. Inputs remain unchanged. A focused200-event extraction is checked before
all500files are read. All five rendered figures were visually reviewed.

## Matching, truth and neutral recovery

The same67950pre-window event keys appear in all FOUR electron curves. They
have both selected production electrons, usable opposite-charge Baseline KF,
valid GSF BestBranch, same-run ordinary BP110 and floating110 endpoints, plus
usable neutral observations. Each electron uses its OWN fitted direction.
Floating110 forms the sum of its matching post-FSR electron p4 and recovered
photon p4. There is no pre-FSR electron TrackState and NO extra neutral p4 is
added to either BP curve. Photon p4 is checked massless/collinear as modeled.

GSF is the maintained existing reference.496cached sources are reused only
after matching their current original native file identities/card hashes;
previously omitted completed sources40/429 are read natively, making498sources.
Failed GSF sources55/421 supply no reference endpoint. Current REC replay
manifests point to the exact original GSF REC path/size/mtime. All current
selected track/MC IDs, KF pT, generator pT/energy and material/FSR birth-energy
sums agree with the native baseline/truth caches. No scalar collection-0
endpoint or borrowed KF direction is used for GSF.

KF/GSF receive the SAME freshly computed neutral union from CURRENT BP files,
not historical sums from the older PFA replay. Use the saved collector IDs,
10mradtheta/200mradphi halfwidth about Baseline KF ECAL references, zero-charge
trackless hit-supported pre-reclustering clusters, and native PFO/cluster links.
Construct massless(E,E*r/|r|)at nominalIP and add the union once. Cluster IDs,
energies, hit support and native ownership are checked. No angular truth
matching, purity cut, energy-tail trim or extra PFA replay is introduced.

Truth categories are unchanged: sum birth energies per type and selected
production-electron leg; a type is present if EITHER leg sum strictly>1GeV.
Neither=!FSR&&!eBrem; eBrem-only=eBrem&&!FSR; FSR-only=FSR&&!eBrem;
Both=FSR&&eBrem, possibly on different legs. Softer radiation is allowed.
FSR ownership comes from saved generator-origin links, material-eBrem ownership
from selected-electron G4-parent IDs. All98437fresh classified event keys agree
exactly with the prior >1GeV radiation matrix, before endpoint/window cuts.

## Count bridge and figures

100000raw events minus1563incomplete selected pairs yields98437classified pairs.
Then subtract135same-Baseline-KF-charge pairs,12619unavailableGSFpairs,
12607unavailableordinaryBP110pairs and5126unavailablefloating110pairs, in that
order, to obtain67950common pairs. No neutral-observation failure remains at
that stage. The5126floating failures are excluded from ALL electron curves.
These are sequential exclusions; fsr_track_failures in summary.json separately
counts failed selected tracks among all classified pairs, including earlier gates.

Five square figures: Inclusive, Neither, eBrem-only, FSR-only and Both(AND).
120–140GeV,0.1GeVbins, each curve independently normalized by its OWN finite
window count. Legend N is that raw window count, not a weighted yield.
Muon is the SAME inclusive bare Baseline KF reference,44236window entries,
black dashed; electron curves are solid: KF+neutrals green, GSF+neutrals gold,
ordinary BP110 red, floating110 blue. No fits, luminosity scaling or generator
recoil curve. AGORA simulation is bold outside the axes, large text, grid,
no plot title or long footer. Category histograms partition inclusive bins
exactly for every method; every displayed shape sums to1.

| Category | Common events | KF+N window | GSF+N window | BP110 window | Floating110 window |
|---|---:|---:|---:|---:|---:|
| inclusive | 67950 | 56949 | 56076 | 55351 | 59024 |
| neither | 16768 | 15977 | 15885 | 16060 | 15768 |
| ebrem_only | 28310 | 24557 | 23690 | 24790 | 25110 |
| fsr_only | 9008 | 6834 | 6824 | 5738 | 7275 |
| both | 13864 | 9581 | 9677 | 8763 | 10871 |

## Interpretation and limits

In the identical common sample, ordinary→floating110 counts in120–130GeV
change42410→49742inclusive,2966→5424FSR-only and4673→8123Both. These demonstrate
peak-region population recovery conditional on successful fits, not pure
resolution improvement or unbiased full-production efficiency. Neither peak
counts change14274→14104; its below120tail grows128→532. Inclusive below120
grows1095→3189. Material/FSR degeneracy, contamination and clean-track
preservation remain open; promotion was implementation promotion, not physics
validation. GSF+neutrals can double-count material radiation; all spectra also
include physical ISR/radiation. Historical hard-loss particle-gun inputs11/16/17
remain unavailable. No new physics-validation claim is made.

## Memory and publication

The exact outgoing AGENTS is preserved in
[the snapshot](2026-11-13-agents-before-full-bp-floating-recoil.md).
Mapping: Introduction/global status/laws/compile instructions retained verbatim;
Current focus preserved in that snapshot and replaced by this completed study.
No history directory was moved; no migration manifest is necessary.
The default FSR source/install contract remains in
[the promotion record](2026-11-12-floating-fsr-default-promotion.md).
Gallery publication completed on ailogin002.ihep.ac.cn using the existing
managed server,PID2122174,http://127.0.0.1:8000/,no restart. Five new PNGs are
copied into bp_matched_recoil_pt_20261110 and one section appended. The gallery
now has31PNGs/11sections,all matched exactly once. All34pre-existing PNG/PDF
hashes remain unchanged;all five new copies match their study sources. The
page and new inclusivePNG returnHTTP200. The main agent reviewed all five
source figures and independently checked all five copied hashes and the count
bridge. Personal plot-gallery skill was delegated to the dedicated gallery
agent, as required; gallery/analysis artifacts remain uncommitted.
