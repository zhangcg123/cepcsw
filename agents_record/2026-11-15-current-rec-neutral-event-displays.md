# Current REC neutral event displays — 2026-11-15

User asks to recover earlier event displays and show a clearly separated neutral,
preferably with FSR and eBrem together, to inspect mixing. This task is read-only
analysis and presentation. No CEPCSW implementation, cards, build, installation,
defaults, production jobs or fits were changed or run.

## Historical and current sources

Earlier displays exist in
`TrackingPerformanceStudies/breakpoint_resolved_neutral_event_displays_20261103/`.
They used historical `higgs_full_20261030` job1 data, not the current REC
campaign. Their cluster/hit reader was reviewed for the native relation contract;
none of those old event payloads or plots is presented as a current event.

New artifacts are in `TrackingPerformanceStudies/neutral_event_displays_20261115/`.
The existing 55,391-neutral current-REC feature pool was searched for illustrative
single-FSR and single-eBrem dominated neutrals with a charged reference, and for
visibly separated FSR/eBrem-mixed neutrals. Two selected examples were freshly
read from the **existing current** files:

- `higgs_eeHX_ISRFSRON_REC_ECALTRUTH_500jobs_200evt_20261112/rec-higgs-369.root`,
  event index82, CompleteTracks index6, selected electron MC19/G4track6;
  source382576163bytes, mtime_ns1794423016000000000.
- Same campaign `rec-higgs-34.root`, event index52, CompleteTracks index8,
  selected electron MC13/G4track3. Its exact source identity is in `sources.json`.

All indices are zero-based. Only these two event entries were freshly read;
the pre-existing candidate search cache is from the 100k-event current campaign.
These are truth-selected illustrations, not population performance, a
reconstructed selector, or an optimization result. The older stable133-event
single-track control catalogue is not a selection or optimization population here.

## Event369/82/track6

| Pre-ECAL cluster | Measured GeV | Individual native photon/root fractions |
|---|---:|---|
| C5 matched charged | 12.35845089 | 46.3851% direct MC19; 53.4540% eBrem gamma150; 0.1609% eBrem gamma149 |
| N24 | 10.76643372 | 99.1657% eBrem gamma149; 0.8343% FSR gamma20 |
| N26 | 2.01147532 | 95.4909% FSR gamma20; 4.4854% eBrem gamma149; 0.0237% Others |
| N16 | 0.71867925 | 48.5032% FSR gamma20; 51.4968% eBrem gamma149 |

All three displayed neutrals lie inside the current25/200mrad theta/phi
half-window. They have534/274/482 linked hits respectively; C5 has607.
All three photons are independently verified to belong to this same selected
Z electron. Their saved birth/first-ECAL-entry energies are gamma20=2.33106136,
gamma149=10.80828762 and gamma150=6.33768463GeV.

Projected centres relative to the Baseline KF reference are N24=(214,12),
N26=(307,110), N16=(214,113)mm. N16 shares approximately one coordinate with
N24 and the other with N26. This observed pattern is compatible with projected
response sharing; it is not proof of a physical 3D overlap or a truth-map bug.
Actual3D cluster-centre distances to C5 are219.877/324.927/236.325mm for
N24/N26/N16. Do not identify these with the plotted ray-projected coordinates.

The same electron also owns FSR gamma16=48.821110GeV, whose original entry is
at the endcap outside this local barrel viewport. It is not gamma20 and does
not supply the displayed FSR fractions. The local2GeV FSR neutral is not an
accounting of the electron's full51GeV FSR birth sum.

## Event34/52/track8

C4=3.063657GeV is the matched charged cluster (87.3560% direct MC13,
12.6440% Others). FSR gamma14 has24.885633GeV birth/entry energy.
eBrem gamma129 has5.707401GeV birth energy and no original-photon ECAL entry:
its terminal saved process subtype14/status2 and directMC e-/e+ daughters
confirm conversion before ECAL. No entrance is invented for it.

N7=24.927002GeV is89.9798% gamma14FSR and9.9862% gamma129eBrem, with its
centre just outside the current phi boundary. N11=5.680412GeV is42.0632%FSR
and57.9085%eBrem, inside the collection window,333.866mm from C4 in3D.
Its inclusive isolation is I50=1 but I100=0.1833303. A larger nearby FSR-dominated
cluster exists just over50mrad away. N13=0.377167GeV is85.7954% Others and is
outside collection. Thus empty50mrad neighbourhood alone does not imply
single-photon native attribution, even far from the electron charged cluster.

## Native data and display contract

FSR photon identity uses generator-origin `GsfGeneratorFsrLinks` status1 for
selected owners. Material eBrem is matched by a unique exact MC/G4 birth
momentum/position tuple, parent selected-electron G4 ID, process subtype3, and
birth before the parent's first saved ECAL entry. Unique photon ancestry is
resolved with the existing individual-photon resolver; distinct roots remain
separate even when the radiation kind agrees. Ambiguous ancestry and non-root
contributions go to Others; direct selected MC-electron contribution is distinct.

Left panels show actual saved pre-reclustering3D hits. Colour is reconstructed
cluster ownership: blue matched charged, orange labelled neutrals, grey context.
No per-hit photon MC relation exists for these3D reconstructed hits. Stars are
independent saved original-photon first-ECAL entrances; electron truth entry is
blue x and Baseline KF ECAL reference black+. No angular truth matching or fit
is performed to manufacture a link or trajectory.

Both examples are barrel local views. All positions are projected along
nominal-IP rays to a cylinder at the selected track's ECAL-reference radius
(~1860mm), suppressing shower depth. x=r_ref*wrappedDeltaPhi,
y=r_ref*z/rho-z_track, inmm. This is a surface comparison projection, not exact
flat crystal coordinates or an actual propagation. The dashed rectangle is
|DeltaTheta|<=25mrad and |DeltaPhi|<=200mrad (confirmed in maintained BP card).
All finite positive linked hit energies are retained; only viewport crops the
drawing. Point area scales with sqrt(hit energy), saturating above0.5GeV for
display only; measured cluster energies/components are unchanged.

Right panels show normalized **native cluster MC-weight fractions**. Each
original tagged photon has its own segment; the denominator is the cluster's
total native weight, not the sum of radiation weights. Legend photon energies
are birth energies; C/N labels quote energy-corrected measured cluster energies.
Native U/V crystal-bar projection inheritance remains unvalidated as spatial3D
photon energy-deposit purity. The displays cannot establish the physical mixing
location; they make the geometry and existing attribution inspectable.

## Verification and artifacts

- `read.py`: current source size/mtime before/after, collection IDs, MC/hit index
  ranges, unique hit links within each cluster, fraction closure, exact measured
  target energies and fresh FSR/eBrem fractions versus current candidate cache.
- Inclusive isolation at50/100/300mrad matches the cache for all four target
  neutrals; every valid charged/neutral cluster is eligible, including electron
  clusters, only the candidate omitted.
- `verify.py`: independently compares8 individual photon fractions to the
  earlier per-photon cache; checks owners, source identities, conversion,
  window membership and projection inverse relation.
- `plot.py`: boundary/identity/azimuth wrapping/scale projection asserts,
  component-stack closure; both complete PNGs visually reviewed after label fixes.
- All three scripts compile. `sources.json`, fresh event JSONs,
  `verification.json`, `plot_summary.json`, logs and README retain exact evidence.

Analysis scripts/data/images/manifests remain uncommitted. Only this durable
record, the outgoing AGENTS snapshot, and updated current focus are committed.
Global status/laws/commands are preserved byte-for-byte before the focus heading.
The complete outgoing focus survives in
`2026-11-15-agents-before-neutral-event-displays.md`; its clean isolation maps,
white median profiles, bold definitions and earlier removal decisions remain.

Gallery publication evidence is in the study's `gallery_verification.json`.


## Verified gallery publication

The dedicated plot-gallery agent appended the final section
**Current REC event displays: separated and mixed neutrals** to the existing
`TrackingPerformanceStudies/bp_matched_recoil_pt_20261110` gallery. Both PNG/PDF
pairs were copied from the reviewed study outputs. All100 previous image hashes
and all19 previous section objects/order/descriptions remain identical. The
existing healthy server is retained on ailogin002, localhost:8000, PID3994352.
There are now76PNGs,20sections,28PDFs,104images; everyPNG belongs to exactlyone
section. Both new PNGs returnHTTP200 and their served hashes match their sources.
The renderer has no section anchor; the new group is last on the home page.
The outgoing manifest and hashes are in `gallery_before/`, outside the served
plot tree. Earlier isolation-map presentation/removals remain unchanged.
