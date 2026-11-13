# Revised ECAL collection panels: |deltaTheta|<=5/25/50 mrad

User requested theta TOTAL widths10/50/100mrad, no full-width label on the plot,
neutral-inclusive BEFORE charged, and an efficiency above EVERY stack.
Displayed labels are only |deltaTheta|<=5/25/50mrad and |deltaPhi|<=200mrad.
Order: FSR-tagged neutral, eBrem-tagged neutral, Neutral inclusive, Electron
charged, Total inclusive. All5 bold top values give the WHOLE stack response;
inside component percentages stay as before. Others black has no internal
percentage label. All targets keep the agreed truth denominators.

## Unchanged data definitions

Inherit the prior truth-normalized analysis at
TrackingPerformanceStudies/ecal_truth_collection_widths_20261113/.
All100000 current REC events;198081 common truth-covered tracks, no fit gate.
349 selected tracks lacking an electron ECAL entry remain excluded. Photon
truth=BIRTH energy including conversions/absorptions/uncollected photons;
electron truth=first ECAL total energy. Same neutral55.75722694396976GeV cap,
same global charged upper2% cut76.59788360595702GeV and same truth denominators.
No new percentile, truth classification, cohort selection or reconstruction.

Five bar denominators: E_F, E_B, E_F+E_B, E_e, E_e+E_F+E_B in the NEW order.
Neutral-inclusive numerator is the FULL unweighted energy of ALL eligible
in-window neutrals, including main-electron and Others. Native MC labels only
split the stack. Tagged FSR/eBrem neutral sets may overlap. Green remains exact
selected main-electron MC index; other descendants/origins/unknown in Others.
Ratios of sums, not per-track ratios. Different bar denominators prevent adding
plotted heights. Total-inclusive adds disjoint charged+neutral energies first.
Whole stacks can exceed100% because of cross-origin contributions. Generator-
tagged truth does not independently quantify untagged physical FSR completeness.

## New narrow-window calculation and checks

Only the new5mrad theta-half-window requires a recount. Reuse saved cluster
energies/native MC fractions and exact truth-covered track indices, reading
current REC cluster positions and first Baseline KF AtCalorimeter references.
All500 source path/size/mtime identities match before/after reading. Reproduce
ALL61965 previously eligible neutral clusters' three old masks at half-widths
10/25/50mrad, then select |deltaTheta|<=5mrad in the same phi200mrad union.
This new window is a strict subset of the previous10mrad half-window.
Cache both electron-centered angular offsets for all61965 candidates to support
later angular comparisons without reopening REC. Count each cluster once.

The retained25/50mrad-half-window results are copied and asserted EXACTLY equal
to the prior50/100mrad-total results, including every numerator, component,
count and denominator. Charged response is unchanged and identical across all
three panels. All components sum to energy and all new narrow-window responses
are bounded by the previous10mrad-half-window response. No ROOT files generated.

## Whole-stack responses

| displayed theta absolute limit [mrad] | FSR-tagged neutral | eBrem-tagged neutral | Neutral inclusive | Charged | Total inclusive |
|---:|---:|---:|---:|---:|---:|
| 5 | 74.10% | 41.64% | 36.65% | 102.25% | 91.67% |
| 25 | 83.20% | 44.94% | 39.88% | 102.25% | 92.19% |
| 50 | 87.07% | 45.85% | 41.15% | 102.25% | 92.39% |

These are total response fractions including cross-origin energy; the target
FSR/eBrem signal fractions are their orange/blue segments, not the full stack.
No recoil-performance or calibration validation is claimed.

Artifacts: summary/validation JSON, bar_values.csv, jobs/*_angles.csv,
plot_files.json and analysis_code_hashes.json. Prior three source images remain
untouched in the base analysis; previous gallery copies are archived before
replacement. Gallery source20-name alias now displays the new5mrad-half-window
panel. The50/100 filenames keep their identities with new bar order/top labels.
The original4 older archived budgets and every unrelated plot are preserved.
All analysis code/data/plots remain uncommitted under project rules.

## Verified gallery publication

The first panel keeps direct_pre_ecal_neutral_window_energy_budget.png, now
showing |deltaTheta|<=5mrad;50/100 filename panels show25/50mrad absolute limits.
All3 previous gallery copies are archived under prior_gallery/ in this artifact.
Prior source analysis images and all unrelated PNG/PDFs remain unchanged.

```json
{
  "verified_at_utc": "2026-11-13T14:42:46.338767+00:00",
  "host": "ailogin002.ihep.ac.cn",
  "url": "http://127.0.0.1:8000/",
  "pid": 443262,
  "served_directory": "/aifs/user/data/zhangcg/gsfdev/CEPCSW/TrackingPerformanceStudies/bp_matched_recoil_pt_20261110",
  "gallery_page": {
    "http_status": 200,
    "revised_section_present": true,
    "whole_stack_values_present": true
  },
  "manifest": {
    "png_count": 54,
    "section_count": 14,
    "every_png_matched_once": true
  },
  "revised_panels": {
    "direct_pre_ecal_neutral_window_energy_budget.png": {
      "source_file": "direct_pre_ecal_truth_collection_width10.png",
      "http_status": 200,
      "source_sha256": "9a2f08371f7d6c2cb02584fac895c266bedca42498ba968bb2ae10a7e7d719fd",
      "local_sha256": "9a2f08371f7d6c2cb02584fac895c266bedca42498ba968bb2ae10a7e7d719fd",
      "served_sha256": "9a2f08371f7d6c2cb02584fac895c266bedca42498ba968bb2ae10a7e7d719fd",
      "match": true
    },
    "direct_pre_ecal_truth_collection_width50.png": {
      "source_file": "direct_pre_ecal_truth_collection_width50.png",
      "http_status": 200,
      "source_sha256": "9731f043a67ba7d4ff475278615ed267aae02cc38eb66f7c0021b02d60901956",
      "local_sha256": "9731f043a67ba7d4ff475278615ed267aae02cc38eb66f7c0021b02d60901956",
      "served_sha256": "9731f043a67ba7d4ff475278615ed267aae02cc38eb66f7c0021b02d60901956",
      "match": true
    },
    "direct_pre_ecal_truth_collection_width100.png": {
      "source_file": "direct_pre_ecal_truth_collection_width100.png",
      "http_status": 200,
      "source_sha256": "32cd39e567984750fbae0fef26bffe188541764c0f80866f60f70e1ca1d4d4e4",
      "local_sha256": "32cd39e567984750fbae0fef26bffe188541764c0f80866f60f70e1ca1d4d4e4",
      "served_sha256": "32cd39e567984750fbae0fef26bffe188541764c0f80866f60f70e1ca1d4d4e4",
      "match": true
    }
  },
  "replaced_gallery_copies": {
    "count": 3,
    "archive_directory": "/aifs/user/data/zhangcg/gsfdev/CEPCSW/TrackingPerformanceStudies/ecal_truth_collection_widths_10_50_100_20261113/prior_gallery",
    "sha256": {
      "direct_pre_ecal_neutral_window_energy_budget.png": "937a9655e134a2c77361a76486f35f2bb9f98b897d4e3afcbd6995e64969135b",
      "direct_pre_ecal_truth_collection_width50.png": "c1a2466377ac296d5a9e5140863ed243ddd267fa762ca61fc2704ba165998b8c",
      "direct_pre_ecal_truth_collection_width100.png": "357868ca278a078cecbb52461b33dc8819c7a9a7b916ce5cd3179d11deb998a5"
    },
    "all_match_before_gallery": true
  },
  "unrelated_gallery_images": {
    "count": 59,
    "all_match_before_gallery": true,
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

No simulation, reconstruction, fit, source build/install, run-card or batch
change. Default reconstruction neutral window remains half10/phi200mrad.
The complete outgoing status/laws are retained in
2026-11-13-agents-before-bar-window10-reorder.md; all global status/laws/commands
remain byte-identical. H_F still installed1644ae8, no new physics validation.
