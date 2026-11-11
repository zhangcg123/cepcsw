# Muon and inclusive electron recoil at10ab^-1 (2026-11-11)

The user requested a new inclusive120--130GeV recoil plot comparing muon recoil
and electron GSF+neutrals, normalized to10ab^-1; then added BP100+neutrals and
an electron Baseline KF filled area. Final figure has gray filled electron KF,
red muon KF, orange electron GSF BestBranch and blue electron BP100, with all
three electron estimators receiving the same nearby neutral sum for shared keys.
Bins are0.1GeV, y-axis expected events/bin, no peak fits or backgrounds.

## Normalization: all generated events before any selection

The user's explicit clarification requires all events, not events left inside
the window. Fixed per-event weights use N_generated before reconstruction,
fit, neutral-availability, finite-mass or display-window cuts:

| Channel | Cross section [fb] | N_generated | Weight at10000fb^-1 |
| --- | ---: | ---: | ---: |
| Muon | 6.77 | 50,000 | 1.354 |
| Electron, every estimator | 7.04 | 99,200 | 0.7096774193548387 |

Every bin is raw count times that weight; selected/finite/window counts never
set the denominator. No unit-area or window renormalization. Before selection
these correspond to67,700 and70,400 expected production events. Table2 of the
[CEPC paper](https://hepnp.ihep.ac.cn/article/doi/10.1088/1674-1137/abb4d8)
was independently checked: these are eeH/mumuH production rates at240GeV,
not rates multiplied by SM H invisible branching. They include the lepton
channel, so no extra Z branching factor is applied. Rates are reference
numbers, not exact recovered production-header weights. The numerical
formulas/denominators are printed on the plot after the user's clarification;
all histogram counts remain unchanged.

Electron source inventory:496 matched completed GSF sources x200=99,200,
matching the frozen REC/BP truth/neutral sources. GSF55/421 failed; completed
40/429 are outside this matched source inventory. Muon inventory is the
previous complete250x200=50,000 campaign. Raw muon REC directory/files are
missing(all250 inventory entries), but cached48,636 recoil pairs, NPZ,
summary and generator-source inventory remain available. They are sufficient
for this plot; no native muon reconstruction or recovery is rerun.

## Electron event selection and recovery

Start from all85,327 inclusive valid GSF/KF opposite-charge production-electron
pairs, with no BP100/BP110 success or FSR/eBrem gate. Reuse72,784 earlier
neutral sums, and extract saved neutral observations for12,543 extra pairs
from496 BP files with frozen identity/track-MC checks. All observations are
available; zero events are rejected for missing collection/reference. All
85,327 native GSF four-vectors reproduce previous bare masses. Neutral addition
occurs in29,335 GSF events.

Electron KF and GSF use the same85,327 keys and exact same neutral sum. BP100
uses its77,817 successful-fit subset inside that GSF pool;7,510 unavailable
BP100 pairs are excluded without fallback or yield renormalization. GSF and
muon curves stay unchanged when adding BP100. Adding gray KF also leaves all
three lines unchanged. Thus this is not a four-way same-selected-event shape
comparison. Every electron method still uses the same99,200 generated-event
normalization, so fit exclusions remain in the efficiency/yield.

Each estimator keeps its own fitted directions on both legs. BP100 remains
truth-selected location, fitted loss, tracker+beam, ECALoff; this assistance is
labelled. Neutral addition uses the same saved pre-reclustering charge-zero,
trackless PFO ECAL-cluster union in10mrad theta/200mrad phi half-width windows
around original CompleteTracks AtCalorimeter positions. Treat each cluster as
(E,E*r/|r|) from nominal IP, count once per pair, never add charged-cluster
energy. Material-radiation double counting and unrelated activity remain
possible; no truth label selects which cluster to recover.

## Generator distinction and limits

Current electron source is genuinely inclusive
E240.Pe1e1h_X.e0.p0.whizard195/e1e1h_X.e0.p0.0001.stdhep. The retained muon
source is E240.Pe2e2h_invi.e0.p0.whizard195: Higgs decays forced invisible.
The muon is an inclusive-production-normalized tracking reference with Higgs
factor1, not a physical SM invisible yield or validated inclusive-decay
muon-efficiency prediction. This corrects any attempt to reuse the older
forced-invisible-electron proxy wording for the present eeHX source. Selections
and decay environments differ; no matched-efficiency/background forecast or
algorithm/physics validation is claimed. No generator recoil is plotted.

## Final raw and expected counts in120--130GeV

| Curve | Selected pairs | Raw in window | Expected at10ab^-1 | Undefined |
| --- | ---: | ---: | ---: | ---: |
| Muon Baseline KF | 48,636 | 36,918 | 49,986.972 | 11 |
| Electron Baseline KF + neutrals, filled | 85,327 | 48,834 | 34,656.387 | 1,013 |
| Electron GSF BestBranch + neutrals | 85,327 | 51,850 | 36,796.774 | 2,039 |
| Electron BP100 + neutrals | 77,817 | 47,366 | 33,614.581 | 1,598 |

Undefined and outside-window masses do not change N_generated. Five muon
pairs with nonpositive recoil energy already lie outside the displayed window;
consistent physical-energy rejection changes no displayed count.

## Validation and artifacts

Muon CSV/NPZ arrays agree exactly, unique keys and250-source generated counts
close. All new ROOT identities, saved selected-track MC links, pre-ECAL PFO
neutral ownership, cluster energies/positions, masslessness and deduplication
gates pass through the previously verified neutral reader. Both BP100 and KF
match all72,784 overlapping old neutral-added recoil values. Independent
raw/weighted bins, total luminosity closure and source hashes pass. Existing
muon/GSF/BP100 histograms remain unchanged by adding gray KF; all four stay
unchanged when numerical normalization denominators are added to the caption.
Final layout is visually inspected.

Artifacts remain uncommitted in
TrackingPerformanceStudies/muon_gsf_neutral_recoil_10ab_20261111/: analyze.py,
verify.py, README, snapshot.json, analysis/validation logs,
additional_neutral_observations.csv, additional_added_clusters.csv,
electron_{gsf,bp100,kf}_neutral_recoil.csv, histogram.csv, summary.json,
validation.json, preserved_gallery_sha256.json, PLOTS_READY and one PNG/PDF.
The historical stem recoil_muon_kf_electron_gsf_neutrals_10ab now includes all
four curves. All44 earlierPNG/PDFs remain unchanged. The personal gallery agent
verified this figure first,23PNG across9sections exactly once, live explicit
pre-selection normalization/sample caveats, and the served final PNG SHA256
matching disk. Parent manifest review and all44 old/new plot hashes pass.
Server ailogin002 PID2122174, http://127.0.0.1:8000/.

No fitter/PFA/writer implementation, runtime library, original card, branch or
batch state changes. Source tuples remain read-only. Outgoing project status
is preserved in [the snapshot](2026-11-11-agents-before-muon-gsf-10ab.md).
Previous neutral geometry/QA/plots remain in
[the neutral-addition record](2026-11-11-nearby-neutral-recoil.md).
