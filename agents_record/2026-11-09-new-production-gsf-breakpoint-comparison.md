# New-production-only GSF / breakpoint comparison (2026-11-09)

## User scope

The user clarified that the current task concerns only the newly reproduced
normal-ISR/FSR REC production. Historical code-regression fixtures must stay
separate from physics comparisons unless explicitly requested. Labels must
identify the actual campaign, source job and event; storage-format names are
not physics sample definitions.

## Frozen input and runs

One completed new-production file was used for every run:
`higgs_eeHX_ISRFSRON_REC_500jobs_200evt_20261109/rec-higgs-4.root`
(306,045,565 bytes, 200 source events, completed-job marker present).
The selected source events are **0,1,11,16,17**, with two generator/association-
selected production-electron tracks per event. Source job is always **4**.
No historical REC fixture is read by any card in this comparison.

GSF uses the private selector implementation checkpoint `799c0f6` and private
install. Its maintained reverse settings are preserved: fresh inward seed,
both BH directions on, MaxComponents=10, CEPC BH model, look-ahead depth 2,
beam on, ECAL off. Verbose output is off and OutputLevel is WARNING.

Breakpoint uses the installed library read-only and maintained defaults:
Truth interval selection plus additional LocationScanSetups=[110], fitted
loss-prior centre with sigma=0.001, beam/charged ECAL on, separate neutral
measurement off, PreReclustering/ElectronPhoton energy, FirstMiddleLast seed,
and separate ordinary/diffuse/G4-centred-prior diagnostics. Optional backward
continuations remain off. No extra HitScanAll suite was introduced here.
Both methods use the same selected pair and original input ordinals.

All six native jobs completed: one GSF job reads through event 17 and fits
only the five selected entries; five breakpoint jobs each fit their one
selected entry. Gaudi loop length and baseline/control tuple rows are not
additional fitted physics events. Every input size/mtime check passed.
No source, build or install change was made for these comparison runs;
production and private runtime fingerprints remain unchanged.

## Results and limitations

All **10 selected tracks** have GSF endpoints. Breakpoint's ordinary/truth-
location workflow produces **7 outputs**. The remaining three are blocked by
strict embedded-truth checks in this new REC file:

- event 1, input track 23: accepted hit 85 is 7.78976 mm from its exact hook,
  exceeding the configured 5 mm guard;
- event 1, input track 30: accepted hit 9 has incomplete provenance, status 28;
- event 17, input track 22: accepted hit 9 has incomplete provenance, status 30.

These failures are preserved and explicitly marked; no guard was loosened.
The maintained Truth workflow also skips its later optional scan on these
rows. Analysis-level calorimeter fallback remains separately identified.
Numerical GSF and breakpoint results vary across the selected examples; this
cohort does not establish population efficiency, recoil resolution or tails.

Current artifacts:
`/aifs/user/data/zhangcg/gsfdev/CEPCSW/build.gsf-selection-dev/validation/new-production-comparison-sf3tovpa`

- `comparison.html`: new-production-only tables, with job/event/track labels;
- `tracks.csv`, `recoil.csv`: all extracted endpoint/mass values;
- `manifest.json`, `runtime-fingerprints.json`, `validation.json`: one-input
  scope, frozen cards, completed runs, and unchanged runtime verification;
- `raw-breakpoint.json`, `scan-details.json`, `gsf-states.json`: raw diagnostic
  values and saved IP states used in the comparisons.

The analysis reuses the existing strict non-null scan ranking and calorimeter
routing functions. Helper source hashes are saved in summary.json. Recoil
uses each method's own saved IP direction at sqrt(s)=240 GeV. Generator-pair
recoil uses stable generator electrons, retaining radiation effects.

Both latest-comparison pointers now reference this new-production-only report.
The previous mixed regression report is retained as historical, with a banner
linking to the current report. Its numerical data are not reused here.
The complete preceding AGENTS.md is preserved in
`2026-11-09-agents-before-new-production-comparison.md`; global instructions
and active project laws are retained byte-for-byte in the current file.
