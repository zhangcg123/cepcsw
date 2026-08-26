# CMS-like three-endpoint publication

Date: 2026-08-27

## Scope and superseded contract

Before this change, the experimental CMS-like backward workflow published one
generic `GSFTracks` result. That result was made by moment-matching the
surviving backward mixture at its innermost current surface and then moving
the resulting single Gaussian to the interaction point. `RecGsfFlatTuple`
therefore stored it in the generic `gsf_*` fields. CMS-like did not publish a
selected-component endpoint, a complete-mixture density mode, a mode status,
or final component records.

This change replaces that single-output contract with the same automatic,
row-aligned three-view endpoint interface already used by smoother and reverse
runs:

- `GSFTracksBestBranch` stores the component selected by
  `ReverseSelectionMode`, after that component is extrapolated to the IP.
- `GSFTracksWeightedMean` preserves the former CMS-like `GSFTracks` endpoint
  exactly, including its operation order: collapse at the innermost surface,
  then extrapolate the collapsed Gaussian to the IP.
- `GSFTracksFullMixtureMode` stores the maximum of the complete
  five-dimensional IP mixture density. Every positive-weight survivor is
  extrapolated independently before the existing mode finder is applied.
- `GSFFullMixtureModeStatus` retains the existing success/fallback contract.
  A failed optimization publishes the BestBranch state with a negative status.
- `GSFFinalMixtureComponent*` now records the CMS-like IP component set with
  source code `3`, normalized independently per output track.

New CMS-like files intentionally do not contain a generic `GSFTracks` result.
The flat tuple fills `bestbranch_gsf_*`, `weighted_gsf_*`, and
`fullmixture_gsf_*`; its generic `gsf_*` fields are zero. Historical CMS-like
files retain `GSFTracks` and `gsf_*` and must be interpreted as the historical
WeightedMean endpoint. CMS-like component-lineage vectors remain empty: this
change adds endpoint and final-mixture persistence, not a CMS-like lineage DAG.

No configurable property, allowed value, or compiled default changed.
`GSFOutputMode` remains a forward-only selector. `ReverseSelectionMode`
chooses only the CMS-like BestBranch and does not change WeightedMean or
FullMixtureMode.

## Mechanical validation

The focused EL9/LCG 105 build and install completed for `RecGsfTracking` and
`RecGsfFlatTuple`; only the pre-existing external-library compiler warnings
were emitted.

A same-input, same-code event-14 comparison at CMS error-rescaling 1 checked
all thirteen persisted endpoint scalars. The former generic CMS endpoint and
the new WeightedMean were exactly equal (`max_abs_difference=0`), including
`pT=37.258535346723988 GeV`, helix parameters, chi-square, NDF, hit count, and
type. The new endpoints were:

| Event | BestBranch pT [GeV] | WeightedMean pT [GeV] | FullMixtureMode pT [GeV] | Mode status | Components |
|---|---:|---:|---:|---:|---:|
| job 28, entry 14 | 35.6842787187 | 37.2585353467 | 35.0766256375 | 1 | 10 |

The required hard-loss gate used the same installed code, the archived
job-1 BH15/cutoff-1e-4/max-components-10 card, CMS error-rescaling 1, and
verbose component output:

| Entry | BestBranch pT [GeV] | WeightedMean pT [GeV] | FullMixtureMode pT [GeV] | Mode status | Component rows |
|---:|---:|---:|---:|---:|---:|
| 11 | 40.9254213146 | 41.1945863466 | 40.9037665669 | 1 | 10 |
| 16, output track 0 | 18.2956179302 | 18.2954378526 | 18.2955637712 | 1 | 2 |
| 16, output track 1 | 38.5660 (verbose) | 38.7273 (verbose) | 38.5746291 (verbose) | 1 | 10 |
| 17 | 18.8031346587 | 18.8927721142 | 18.8031446697 | 1 | 9 |

All 41 recorded component rows had source code `3`, valid IP states, and
weights summing to one independently for each output track. The generic
`gsf_pT` was zero and all three explicit availability flags were one in the
flat rows. Event 16 exercised row alignment with two published tracks.

These are mechanical and compatibility gates. They do not validate the
physics performance of CMS-like BestBranch or FullMixtureMode and do not
promote CMS-like over the reverse production candidate.
