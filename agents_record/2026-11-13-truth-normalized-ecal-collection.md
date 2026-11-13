# Truth-normalized pre-reclustering ECAL collection — 2026-11-13

User decisions: use corresponding truth energies, photon BIRTH energy including
conversions, theta TOTAL widths20/50/100mrad (=half-widths10/25/50), phi half-width
200mrad, and exclude the upper2% of charged-cluster energies.

## Sample and reconstruction inputs

Read all500 current REC_ECALTRUTH files x200events =100000events. Original
selected production-electron tracks198430, with no BP/GSF-fit success or recoil
window cut. All have unique G4 identity from complete silicon-hit provenance,
checked against saved generator FSR parentTrackID when present.349 selected
tracks have no saved first ECAL entry and are excluded from ALL numerators and
truth denominators; no zero-energy value is invented for missing truth. Keep
198081 tracks in99988 events, with12 input events contributing no covered track.
Same common cohort and truth denominators for all bars/windows.

Use native pre-reclustering ECAL cluster-to-MC weights. Charged clusters belong
to a unique nonzero-charge pre-PFO explicitly linked to a retained electron
CompleteTrack; no angular gate. Missing/ambiguous charged PFOs supply no charged
energy but retain their track truth denominator. Neutral clusters are trackless,
charge-zero pre-PFO clusters, collected in the union of retained electron first
Baseline KF AtCalorimeter windows at nominal IP. Count each cluster once even
if both windows contain it. Charged and neutral sets are disjoint.

Keep positive finite energy, nonempty hits and finite nonzero position. Native
MC labels follow uniquely generator-linked FSR and selected primary-electron
material-eBrem ancestry. Use only eBrem emitted before/at the electron's first
ECAL boundary step;22 later-emission records excluded from this upstream-radiation
study. Soft material photons without a saved unique MC birth still enter the
G4 truth denominator, while their reconstructed energy cannot gain a dedicated
eBrem MC label. No angular matching is used to label truth origins.

Orange=FSR, blue=material eBrem, green=EXACT retained final-electron MC index.
All non-main electron descendants, other origins and unknown merge into black
Others. No percentage labels on black. Cluster energy times normalized native
MC fraction is a response proxy, not exact deposited energy.

## Corresponding truth denominators and annotations

E_F = SUM saved FSR photon BIRTH energies owned by retained electron tracks,
without energy, ECAL-entry, angular, reconstruction or resolution selection.
Converted/absorbed/uncollected original photons remain in the denominator.
E_B = SUM recorded upstream material-eBrem photon BIRTH energies from the same
G4 primary-electron tracks. E_e = SUM original electron TOTAL energy at its
first saved ECAL entry, postKineticEnergy + electron mass.

| Bar | Reconstructed numerator | Truth denominator D |
|---|---|---|
| FSR-tagged neutral | Window neutrals with any positive selected FSR MC share | E_F |
| eBrem-tagged neutral | Window neutrals with any positive selected eBrem MC share | E_B |
| Electron charged | Clusters explicitly matched through pre-PFOs to retained tracks | E_e |
| Neutral inclusive | Every eligible neutral in the window union, once | E_F + E_B |
| Total inclusive | Charged + neutral-inclusive, no overlap | E_e + E_F + E_B |

Each coloured segment AND its printed percent is100*attributed reconstructed
energy / the bar's D. This replaces the old GeV/track ordinate and old per-bar
composition percentages. Ratios of sums over the common sample, not averages
of per-track ratios; zero-response tracks remain in D. Tagged neutral groups
can overlap. Denominators differ across bar types, so their plotted heights
must NOT be summed. Total-inclusive is formed in GeV before normalization.
Its charged and neutral pieces add exactly with their truth-energy weights.
A stack can exceed100% when it contains other-origin energy; it is not a pure
signal efficiency. In the first two bars, the orange/blue target segment is the
corresponding photon recovery efficiency. Main and cross-category segments show
contamination normalized to that same D. origin_recovery.csv provides genuine
per-origin charged/neutral/total recovery fractions with each origin's own D;
this avoids reading the small charged-bar orange segment as all-FSR efficiency.

## Tail treatment

Neutral cut stays FIXED at55.75722694396976GeV, previously the original50mrad
half-window pool's98th percentile; not recomputed for these windows/cohort.
Charged cut is one NEW global98th-percentile threshold from all valid charged
clusters in the common truth-covered sample: 76.597883605957GeV.
Raw charged maximum 5701.733887GeV; user explicitly chose trimming
upper2% instead of a240GeV cut or leaving it raw. Retain188856/192711 charged
clusters; remove3855 (2.000405%). The charged cut
also removes genuine high-energy measurements. ALL truth denominators stay
unchanged; removed response counts as reduced efficiency. The same retained
charged set is reused for all three theta widths.

## Results

Truth energy sums [GeV]:

```json
{
  "main": 8621667.236368723,
  "fsr": 424983.00449558924,
  "ebrem": 1233162.672157788
}
```

| theta TOTAL width [mrad] | FSR target recovery in neutrals | eBrem target recovery in neutrals | Charged stack response | Neutral-inclusive response | Total-inclusive response |
|---:|---:|---:|---:|---:|---:|
| 20 | 32.23% | 21.28% | 102.25% | 38.24% | 91.93% |
| 50 | 35.74% | 21.63% | 102.25% | 39.88% | 92.19% |
| 100 | 38.91% | 21.70% | 102.25% | 41.15% | 92.39% |

All5 bars use distinct appropriate denominators; stack responses include cross
origins. These are collection diagnostics, not a recoil-resolution validation.
FSR truth here means the uniquely generator-tagged selected-electron sample;
the fraction of untagged physical FSR remains undetermined, as in the ancestry
census. Photon birth normalization includes conversions within that tagged sample.

## Checks and artifacts

All500 source path/size/mtime identities match the unchanged native cache,
before/after reading and at final verification. All198430 selected tracks have
unique G4 IDs; truth entries define the common198081-track sample.61253 eligible
neutral-cluster native fractions and20/100-total-width masks reproduce the old
10/50-half-width native study on unchanged complete-truth events. Missing-truth
cohorts and22 post-entry emissions are explicit, not hidden old/new differences.

Unique cluster keys prevent double-counting. Every cluster's normalized weights
sumone; all energy components sum to reconstructed energy. Window masks and
neutral energy totals are nested; charged bars identical across widths. Saved
truth-track rows reproduce global truth sums. Recounted raw charged CSV energies
and components exactly reproduce untrimmed summaries, then the requested common
cut is applied once. Charged+neutral=total component by component. All source and
plot hashes recorded; new panels visually reviewed before publication.

summary_before_charged_trim.json retains the raw result. summary.json contains
the selected charged trim. validation.json, bar_values.csv, origin_recovery.csv,
excluded_charged_clusters.csv and jobs/* truth/cluster rows document everything.
All analysis scripts/data/PNGs/PDFs remain uncommitted under project rules.
No simulation, reconstruction, build/install, fit, batch or run-card change.
Old gallery energy-budget panels are archived unchanged before replacement.

## Publication and preserved status

The user-requested original direct_pre_ecal_neutral_window_energy_budget.png
URL now gives the20mrad-total truth-normalized five-bar figure.50/100mrad-total
companions follow. Four superseded absolute-energy gallery copies are archived
unchanged under the analysis artifact prior_energy_budget/; original analysis
source plots remain untouched. Earlier per-event neutral-energy control remains
unchanged and explicitly labelled as an earlier control. The QED diagram and
all unrelated gallery sections/images are preserved.

```json
{
  "verified_at_utc": "2026-11-13T14:09:24.270464+00:00",
  "host": "ailogin002.ihep.ac.cn",
  "url": "http://127.0.0.1:8000/",
  "pid": 443262,
  "served_directory": "/aifs/user/data/zhangcg/gsfdev/CEPCSW/TrackingPerformanceStudies/bp_matched_recoil_pt_20261110",
  "manifest": {
    "png_count": 54,
    "section_count": 14,
    "every_png_matched_once": true,
    "energy_section": "Truth-normalized charged and neutral ECAL energy collection"
  },
  "gallery_page": {
    "http_status": 200,
    "new_section_present": true,
    "new_panels_and_legacy_control_present": true,
    "retired_patterns_absent": true
  },
  "replacement_panels": {
    "direct_pre_ecal_neutral_window_energy_budget.png": {
      "source_file": "direct_pre_ecal_truth_collection_width20.png",
      "http_status": 200,
      "source_sha256": "a3972080dd6d6556f20e6e35cf5b1e98c8d07884812084c49988993d4742c321",
      "local_sha256": "a3972080dd6d6556f20e6e35cf5b1e98c8d07884812084c49988993d4742c321",
      "served_sha256": "a3972080dd6d6556f20e6e35cf5b1e98c8d07884812084c49988993d4742c321",
      "matches": true
    },
    "direct_pre_ecal_truth_collection_width50.png": {
      "source_file": "direct_pre_ecal_truth_collection_width50.png",
      "http_status": 200,
      "source_sha256": "8ba1f2b33c8e9329f45f0a8411eb4cba5ae5bd491900b92ff6f44a9e2b52368a",
      "local_sha256": "8ba1f2b33c8e9329f45f0a8411eb4cba5ae5bd491900b92ff6f44a9e2b52368a",
      "served_sha256": "8ba1f2b33c8e9329f45f0a8411eb4cba5ae5bd491900b92ff6f44a9e2b52368a",
      "matches": true
    },
    "direct_pre_ecal_truth_collection_width100.png": {
      "source_file": "direct_pre_ecal_truth_collection_width100.png",
      "http_status": 200,
      "source_sha256": "a41ecac122c7f533b0f18b049b55a3adbd8f7d9b208fda539fbc020ad46af22f",
      "local_sha256": "a41ecac122c7f533b0f18b049b55a3adbd8f7d9b208fda539fbc020ad46af22f",
      "served_sha256": "a41ecac122c7f533b0f18b049b55a3adbd8f7d9b208fda539fbc020ad46af22f",
      "matches": true
    }
  },
  "archived_superseded_panels": {
    "direct_pre_ecal_neutral_window_energy_budget.png": {
      "before_sha256": "d782ea48ca06ad0175f2d4a52e03e97daa5b07bc5ed6c90af7e2a2f5b3cb30cd",
      "archive_sha256": "d782ea48ca06ad0175f2d4a52e03e97daa5b07bc5ed6c90af7e2a2f5b3cb30cd",
      "matches": true
    },
    "direct_pre_ecal_neutral_window_energy_budget_main_electron.png": {
      "before_sha256": "e45e773afbffe45027dac447055e40d4d41d978cb653dd46b6f42168830572eb",
      "archive_sha256": "e45e773afbffe45027dac447055e40d4d41d978cb653dd46b6f42168830572eb",
      "matches": true
    },
    "direct_neutral_window_200_inclusive_components.png": {
      "before_sha256": "e3d4345b3a173e25657e19f2e4570c3c59356ea6278165aa66b9353c1b76076f",
      "archive_sha256": "e3d4345b3a173e25657e19f2e4570c3c59356ea6278165aa66b9353c1b76076f",
      "matches": true
    },
    "direct_neutral_window_200_energy_budget.png": {
      "before_sha256": "63d67cf92564afc58d9522bac6e4f80a604be5db43608eba995589841f10a872",
      "archive_sha256": "63d67cf92564afc58d9522bac6e4f80a604be5db43608eba995589841f10a872",
      "matches": true
    }
  },
  "unrelated_png_pdf_files": {
    "count": 59,
    "all_hashes_match_before_gallery_json": true,
    "sha256": {
      "recoil_muon_electron_baseline_kf_10ab.png": "ba6fafe03c8e54b055cd71729bdff651fbf6853f259ad45150951aafa0fae14b",
      "recoil_muon_electron_baseline_kf_gsf_10ab.png": "0993c11d87731269d9d8e045d5a0c1e411289d71e935a18a5da60f05633657b5",
      "recoil_electron_kf_gsf_ebrem_lt1pct_10ab.png": "55b42a407c78fbe259cf32838cc9f3a48e12a48df4814d26e979025fca832d21",
      "recoil_muon_kf_electron_kf_gsf_neutral_recovery_10ab.png": "2c1edbec091fdb5f1d255e27d88a78cb4df3ce4556edd8fba4428128ec979017",
      "recoil_gsf_muon_ebrem_only_shapes_120_140.png": "146c5829aafef575ffe5c612d18991563f288f69bf2880be33bf5c93fdaee619",
      "recoil_fsr_resolved_shapes_120_140.png": "eda001c174088145b0d685bf03749edd8cb0a7a71313fab74326513740ab26e9",
      "direct_reco_ecal_ebrem_centered_5k.png": "fa68766a8af83d3b0ae34585f166f56fd4ba029e16c5d249d26888d5579fdef0",
      "recoil_fsr_unresolved_shapes_120_140.png": "8044a60332b6ae6c3355ec0b987b94e83bc05d1100a2b4c711cb1d7f3cf61b41",
      "recoil_fsr_unresolved_bp110_floating_shapes_120_140.png": "1cd9bff40fa7ec55211bb1ea208c5f0a0f553d185a1e1f13328fa5623e276215",
      "recoil_bp_fsrfloat_neither_shapes_120_140.png": "0fb0bf8cf8b44956f2af9b900eb8fd3a38f5d60f771b3a82f0956febb78702ad",
      "recoil_bp_fsrfloat_fsr_only_resolved_shapes_120_140.png": "a3440accd38f69ad28ebf5b98de3fb588713d255d433fed208a408ae7b1e6b05",
      "recoil_bp_fsrfloat_both_resolved_shapes_120_140.png": "aa703ba24af2744ee4096b4a3689ee5d5c3393b8319af8022df3f3e4eeadbfd5",
      "bp110_fsr_scores_neither.png": "b867b011228b6aff0c1c2f18db941ade06b8e1def03599c4e2d6a90b1a3d8748",
      "bp110_fsr_scores_ebrem_only.png": "08e395016b85615f97b76bd0b0fb5f3bbdf8c734fe1694a2861b6eef94931b0a",
      "bp110_fsr_scores_both.png": "7f8dbe7d7bcde4f32ec4907890e408235ee54768bb8510f0d168fc6036cde8f0",
      "bp110_fsr_score_difference_categories.png": "e1b4f208a55f6632a08d357d1f8de562d6173f550586ebbd45a4ad4b745c6b19",
      "bp110_fsr_q_neither.png": "9385b6403931d16d09cf82b598cba32aae6bf7d6d34b31a6daad6c81d48bdba5",
      "bp110_fsr_q_ebrem_only.png": "3bf22b15f8b5d7f95b6c56aa255438872ef9d5aab5e617970ac899431a6af1e5",
      "bp110_fsr_q_both.png": "e758eb58f75138c101abb3c0abf104744e3af99ffce7cc2fa47059bb469d9008",
      "recoil_bp110_qselect_both_shapes_120_140.png": "3a412e4b9b71701a9c4bfbb8b027a3cae932aeb0903d12618a8b5ce04eee4a53",
      "z_ee_gamma_amplitudes.png": "94c6fb9feae0c9ab8a6bf36a16b905da904977bef95d8e8d3a78e4d42c795c70",
      "recoil_muon_electron_baseline_kf_gsf_10ab.pdf": "9b07069c4ee642c7304ef047d5f1dc3dc42dbb0ff78753a831a11adfa405a773",
      "direct_reco_ecal_fsr_centered_5k.png": "def05993e5e2d0d63d61350081cb86fc4fde00c5303ba4dcbf824a8714b93c57",
      "recoil_electron_kf_gsf_ebrem_ge1pct_10ab.pdf": "9dcd03c5d5a133ef494649a3fa1edc188c9eb3b49a73191318383952fe8aa18c",
      "photon_dominated_neutral_collection_window_full.pdf": "fd87a5707217c6e6d60eef8d2d18c6a14c3ba3ed12a2253169f5b30198c6580c",
      "recoil_fsr_resolved_bp110_floating_shapes_120_140.png": "d9bf2d54948b07c83b5bb545e686264a16b8d7fdd1229ead43a5b215b8559142",
      "recoil_bp_fsrfloat_ebrem_only_shapes_120_140.png": "47b967d5d35fccce811d26ffa3b75ce16b8b99ee203f621c649e9f692466bf79",
      "recoil_bp_fsrfloat_fsr_only_unresolved_shapes_120_140.png": "c51ace4bd35cfb8c40d0819ba8c97b9899f990b84a984ce0c8dc8a0f9a2f8f31",
      "recoil_bp_fsrfloat_both_unresolved_shapes_120_140.png": "4ed7ee9518591d95c7ae327bc7b98270c7860509a66e89425e1efbc50de33f2e",
      "bp110_fsr_scores_fsr_only.png": "73a17ffd3384d06397436deb50d17f77a67b62e5a1d1c51e3d188ed5f2a08ceb",
      "recoil_bp110_qselect_inclusive_10ab_120_140.png": "fcc701280a00a993a764fcbaa0890ea36f28b3b490475ab68c72ad903062ce1c",
      "recoil_electron_kf_gsf_inclusive_matched_10ab.pdf": "648dc57ada547adaee756fb6e06784d788b6de195937cca12eecf19143e46d1e",
      "recoil_no_fsr_bp110_vs_gsf_neutrals_shapes.png": "7bb7d193b498006dc968088c1f1d2e536af2c298c17b96bbc3418f0ba3d8c7e9",
      "direct_pre_ecal_collected_neutral_energy_per_event.png": "c103a154c57d2fe4e7c2322abd554964e460ea276cee049f2c4d10a8e973ff62",
      "direct_truth_ecal_ebrem_centered_5k.png": "c06fbc87373634047584acfa54d13519ace71d11246222b11dad7394b17cb857",
      "recoil_gsf_muon_fsr_only_shapes_120_140.png": "c2472e6c90dba8f83644b5ac4786d875da5dab884e4addf88755da429bdb92af",
      "recoil_bp_fsrfloat_inclusive_shapes_120_140.png": "6195d5d90e058279a227536bdf528d2a33bcc7c7e8b7c168094ca96dcbe4503d",
      "recoil_bp_fsrfloat_fsr_only_shapes_120_140.png": "a1a3fbba1605d942257437aee5a0a0206325b01c4ba219decb292e58350d7bb5",
      "recoil_bp110_qselect_ebrem_only_shapes_120_140.png": "065a0ab9d5ee74727843330b23e30933da202d02df6d934f7f576977043fb0aa",
      "direct_neutral_window_200_efficiency_comparison.png": "caca39120de42146133124a1fd227c45d5473cf6ddc8da933f01b615bb4a618e",
      "recoil_muon_electron_baseline_kf_10ab.pdf": "004374450f8680e919d87727e123314cdd967efbe272194c4689620550c50485",
      "event_fsr_ebrem_matrix.png": "a753cdc9f669846f12359115e748c2c2d98aeae3707909adf55ac0baa4c8fb77",
      "recoil_electron_kf_gsf_ebrem_lt1pct_10ab.pdf": "f7506972e9a7c72cfdc8f5e7b0008e88dcb4f21833c824c18537b25190328077",
      "recoil_electron_kf_gsf_ebrem_ge1pct_10ab.png": "9d1e7ee2d8cadbba947ec34578755c115a5c2b4e384acbffbb5380c9dc0d2a1c",
      "event_fsr_ebrem_summary_gt1gev_and.png": "a02fc628b1501673721e57705e7bc68cfd1be807fd132415a25585fa3ee9b6cd",
      "recoil_electron_kf_gsf_inclusive_matched_10ab.png": "5399bb90354a067da32b4fa4940163e3fdd2485518a2441d97fed3f0cb8f3037",
      "recoil_muon_kf_electron_kf_gsf_neutral_recovery_10ab.pdf": "addfa8f3c93c96227de9301a636cee5a06b4620751d561e7ed836207767d738e",
      "photon_dominated_neutral_collection_window_zoom.pdf": "cb1198c84866d941db44b5641519c32f25d59731baca5d56f78228dc349260ea",
      "direct_pre_ecal_neutral_window_absolute_efficiency.png": "45562c43ee4c0a589407c00d41f5e4bf37a4410fedbca8cec5d9e5a2612ab5dc",
      "recoil_gsf_muon_neither_shapes_120_140.png": "e1be411b538c090c9d78451bb4091eaee7143012c7a8b55190754231a281516b",
      "direct_truth_ecal_fsr_centered_5k.png": "354ce3c8e2d0a1c8d1d6097ea91d7187280c9fb44515fa238d1d6b51e68061b2",
      "recoil_gsf_muon_both_shapes_120_140.png": "9251858eccddf7bc9458bf2505ee6269f71a3a747316e1e9e19fd05c23c91c3b",
      "recoil_fsr_inclusive_bp110_floating_shapes_120_140.png": "e7868795bb7f3dddb2573469596d92e132f2de59c0bda9dafa37b5155064d9e6",
      "recoil_bp_fsrfloat_both_shapes_120_140.png": "d3def81a9bf4d0dc5d991caf9fa3a3bc329ee8cd9fdb30a8bea05b40a702662e",
      "recoil_muon_electron_baseline_kf_10ab_wide.png": "ee0c952847e488c3c023554a3bf75b602bcad4fc3d0d70000ea4e281c6811b27",
      "bp110_fsr_score_cut_efficiency_categories.png": "96ffd8ef8101be58bb27774220aa714431cd53c85ed126c1c5f60420bf89fe96",
      "bp110_fsr_q_fsr_only.png": "6b0c19236673adab6bb5743d73ab091acc0e825b36c68ed5a183520843162824",
      "recoil_bp110_qselect_neither_shapes_120_140.png": "d2d6eeeb96889e723c71bc9a3b25ac4a781ac5bc0f0dfd6192714ce197e04f0a",
      "recoil_bp110_qselect_fsr_only_shapes_120_140.png": "20c20255ce02b953a6fc4b3532fc7b00cffebfea437be7c0ee107bb93d0b5a08"
    }
  }
}
```

No codebase, installation, run card, fit or production job changed. H_F remains
installed1644ae8. Existing H0 production lacks H_F until another BP rerun; no
automatic four-way selection or physics validation. Complete outgoing status
is preserved in2026-11-13-agents-before-truth-normalized-ecal.md.

Follow-up definition confirmation: independently recounting FULL unweighted
energy of EVERY eligible in-window neutral cluster reproduces Neutral-inclusive.
No MC-origin tag or contribution weight filters that inclusive numerator; green
and black remain included. Numerators634130.5539021508/661187.0435199661/
682357.2100818031GeV divided by the SAME FSR+eBrem truth BIRTH denominator
1658145.6766533772GeV yield38.2433559868/39.8750877459/41.1518251797% for theta
TOTAL widths20/50/100. No numerical/cohort/cut change. Added bold whole-stack
percent above Neutral inclusive in all3 panels; existing colour labels remain.
Previous6 source PNG/PDFs preserved in before_neutral_total_label/; new source,
local and served hashes agree, all3PNG/pageHTTP200, same PID443262. Gallery
54PNGs/14sections/exact-once; all59 unrelated images and4 earlier archived
budgets unchanged. Separate evidence: neutral_inclusive_definition_check.json
and neutral_total_gallery_verification.json in the analysis artifact directory.
