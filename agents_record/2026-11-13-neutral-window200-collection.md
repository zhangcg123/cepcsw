# Neutral collection theta/phi 200/200 mrad comparison — 2026-11-13

User confirmed neutral-cluster collection efficiencies, purity and components,
with angular half-widths ±200mrad in both theta/phi. This is analysis-only:
no reconstruction, BP/GSF refit, card/default, build, installation or submission.
The default collector stays theta±10mrad /phi±200mrad. Prior plots preserved.

## Sample and definitions

All500 retained REC_ECALTRUTH files x200events =100000events, and198430 selected
production-electron tracks. No BP/GSF-success or recoil-window cut. Reuse native
MC-origin fractions from the all-angle neutral cache, reading current REC
positions, first Baseline KF AtCalorimeter references and native MC weights for
exact main-electron separation. Every input path/size/mtime matches the native
cache before/after each read and at final verification. Old REC_TPCFIX is absent
and never opened. No truth angular matching; native generator FSR links and
material-photon ancestry label the weights, reconstructed angles select clusters.

Neutral/trackless pre-reclustering ECAL clusters only, unchanged positive finite
energy, nonempty-hit and finite nonzero-position eligibility. Collect once per
event in the union of selected-electron windows. Wrapped phi remains±200mrad;
theta half-widths10,50,200mrad. Fixed common Ecluster<=55.75722694396976GeV cap,
originally the upper2% trim from the50mrad pool, not a newly retuned percentile
for each window. It need not exclude exactly2% of the200mrad population.

Energy acceptance = MC-attributed reconstructed neutral category energy inside
window / same eligible category energy at ALL angles. Count acceptance uses
>50% category MC-weight dominated clusters in numerator and all-angle denominator.
These are resolved-neutral response proxies, not generator-photon/deposit or
full FSR efficiencies. Merged charged clusters and missing photons are excluded.
All-collected energy purity divides category energy by ALL collected neutral
energy. Conditional tagged purity instead divides by total energy in clusters
with any positive category share; these tagged sets overlap.

Every budget uses all198430 selected tracks, including zero collection. Green
is only the exact selected final-electron MC index; non-main electron descendants,
other origins and unknown merge into black Others, with no black percentage text.
FSR/eBrem tagged bars use any positive weight, may overlap, and each divides by
the same FULL track denominator. No charged clusters included in any bar.

## Results

| theta half-width [mrad] | Collected neutrals | FSR energy acceptance | eBrem energy acceptance | FSR count acceptance | eBrem count acceptance | FSR+eBrem energy purity | Neutral GeV/track |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 10 | 48,634 | 49.85% | 88.37% | 35.89% | 82.33% | 63.10% | 3.2064 |
| 50 | 62,122 | 60.19% | 90.08% | 49.26% | 88.29% | 63.56% | 3.4499 |
| 200 | 91,675 | 67.46% | 90.27% | 61.47% | 89.24% | 61.76% | 3.7181 |

All-collected FSR energy share rises21.57%→24.20%→25.17%; eBrem falls41.54%→39.36%→36.59%; black Others rises0.50%→1.49%→5.53%. Conditional FSR-tagged purity is41.11%→44.66%→47.05%, and eBrem-tagged49.23%→47.47%→46.56%. Green main-electron shares36.40%→34.95%→32.71%. These denominators are distinct and should not be conflated.

The10-to200 increment per selected track [GeV] is:

```json
{
  "fsr": 0.2443217827564411,
  "ebrem": 0.02873690757799045,
  "main_electron": 0.049181831645442195,
  "others": 0.18948872051543394
}
```

Added-region FSR+eBrem energy purity53.36%. Wider theta captures more FSR,
with little extra material-eBrem energy and substantially more Others. This
does not establish recoil improvement or justify changing production defaults.

## Checks, plots and artifacts

Original10/50 masks match for all2522197 raw all-angle candidate rows; exact
aggregate counts, energies and category-dominated counts reproduce the earlier
100k study. The exact-main10mrad component reproduces its independent prior
native-weight split. All500 current source identities match; nesting and unique
cluster keys pass, including91675 unique eligible200mrad clusters. Budget sums,
nonnegative increments and full-track denominators pass. New plots visually
reviewed: square axes, large text, AGORA simulation above axes and transparent
grids. No new physics fits, luminosity scale or generator recoil plot.

Artifacts: TrackingPerformanceStudies/pre_ecal_window_200mrad_20261113/.
README, summary/validation JSON, CSV metrics/cluster keys, PNG/PDF hashes and
code fingerprints remain uncommitted analysis artifacts under project rules.
Three new PNGs join the collection sections of the existing gallery; all59
previous PNG/PDF hashes unchanged. Existing source plot images are preserved.
Gallery verification metadata is recorded below; source/local/served new image
hashes must agree and page/all3PNGs must returnHTTP200.

```json
{
  "verified_at_utc": "2026-11-13T12:56:55.465379+00:00",
  "host": "ailogin002.ihep.ac.cn",
  "url": "http://127.0.0.1:8000/",
  "pid": 443262,
  "served_directory": "/aifs/user/data/zhangcg/gsfdev/CEPCSW/TrackingPerformanceStudies/bp_matched_recoil_pt_20261110",
  "gallery_page": {
    "http_status": 200,
    "contains_all_new_plot_names": true
  },
  "manifest": {
    "png_count": 54,
    "section_count": 13,
    "every_png_matched_once": true
  },
  "preserved_previous_images": {
    "count": 59,
    "all_sha256_match_before_gallery_json": true
  },
  "new_plots": {
    "direct_neutral_window_200_efficiency_comparison.png": {
      "http_status": 200,
      "source_sha256": "caca39120de42146133124a1fd227c45d5473cf6ddc8da933f01b615bb4a618e",
      "local_sha256": "caca39120de42146133124a1fd227c45d5473cf6ddc8da933f01b615bb4a618e",
      "served_sha256": "caca39120de42146133124a1fd227c45d5473cf6ddc8da933f01b615bb4a618e"
    },
    "direct_neutral_window_200_energy_budget.png": {
      "http_status": 200,
      "source_sha256": "63d67cf92564afc58d9522bac6e4f80a604be5db43608eba995589841f10a872",
      "local_sha256": "63d67cf92564afc58d9522bac6e4f80a604be5db43608eba995589841f10a872",
      "served_sha256": "63d67cf92564afc58d9522bac6e4f80a604be5db43608eba995589841f10a872"
    },
    "direct_neutral_window_200_inclusive_components.png": {
      "http_status": 200,
      "source_sha256": "e3d4345b3a173e25657e19f2e4570c3c59356ea6278165aa66b9353c1b76076f",
      "local_sha256": "e3d4345b3a173e25657e19f2e4570c3c59356ea6278165aa66b9353c1b76076f",
      "served_sha256": "e3d4345b3a173e25657e19f2e4570c3c59356ea6278165aa66b9353c1b76076f"
    }
  }
}
```

The complete outgoing H_F status/laws are preserved in
2026-11-13-agents-before-neutral-window200.md; global status/laws/commands are
byte-identical. H_F remains installed as1644ae8 and has no population or four-way
selection validation. This analysis changes no fit or installed library.
