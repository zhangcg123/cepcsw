# Two additional ECAL collection panels: |deltaTheta|<=100 and200mrad

User requested TWO additional panels, preserving the existing5/25/50mrad
absolute-limit figures. Phi absolute limit200mrad throughout. New panels have
same order FSR-tagged neutral, eBrem-tagged neutral, Neutral inclusive, Electron
charged, Total inclusive; all5 stacks have bold total response above them.
No full-theta-width annotation. New100/200absolute limits correspond to internal
total widths200/400, never confused with prior25/50absolute-limit steering.

## Identical definitions and controls

All100000 current REC_ECALTRUTH events, same198081 truth-covered tracks and
99988 covered events. Same349 missing electron-ECAL-entry exclusions. Photon
truth=BIRTH energy including conversions/absorptions/lost photons; electron
truth=first ECAL total energy. Frozen neutral55.757GeV cut and charged upper2%
76.59788360595702GeV cut reused EXACTLY, no retuning or changing denominators.
Green=exact selected main-electron MC index; other descendants/origins/unknown
black Others; no internal black percentage callout. Tagged neutral sets overlap.

Five denominators in displayed order: E_F,E_B,E_F+E_B,E_e,E_e+E_F+E_B.
ALL eligible neutral-cluster energy, including green/black, goes into Neutral
inclusive. No MC-origin filter or contribution weighting of its full numerator.
Native MC links only decompose components. Charged and neutral sets disjoint;
Total inclusive adds their energy before normalization. Ratios of sums, zero
response stays in truth D. Stacks can exceed100% with cross-origin energy;
bar denominators differ, so plotted heights should not be added. FSR truth
covers the generator-tagged sample, not an independent completeness claim.

## Reader and validation

Reuse the original native reader for the wider collection. Its only generalized
helper line replaces hardcoded largest eligible mask100total by max(WIDTHS).
The original20/50/100total steering has max100, hence unchanged behavior. Prior
reader bytes are saved in base_reader_before_extension.py.txt. No shared PFA,
simulation, tracking, GSF/BP source or run card changed. Full native REC reads
supply new window clusters/MC weights under exactly the original truth cohort.
Internal controls10/20/50/100total plus new200/400total are evaluated together.

All500 source path/size/mtime identities match before/after/current. Every common
truth sum, retained track and photon count agrees with the original. Raw charged
response agrees with the original raw result; its existing trim is then reused.
Previous3 published controls and original20total control match at relative1e-12;
previous3 are retained exactly in final summary. Components add to energy,
neutral+charged=total, nested windows verified. Existing3 figures not regenerated
or overwritten. Only2 new PNG/PDFs are published; no ROOT output or reconstruction.

## Whole-stack responses

| displayed theta absolute limit [mrad] | FSR-tagged neutral | eBrem-tagged neutral | Neutral inclusive | Charged | Total inclusive |
|---:|---:|---:|---:|---:|---:|
| 100 | 90.20% | 46.51% | 42.53% | 102.25% | 92.62% |
| 200 | 92.65% | 46.85% | 44.36% | 102.25% | 92.91% |

These are energy-collection diagnostics, not recoil or calibration validation.

Artifacts: summary_before_frozen_charged_trim.json, summary/validation JSON,
bar_values.csv, native jobs/* truth/cluster rows, plot/source hashes. Analysis
code, tables, PNG/PDFs and gallery manifest remain uncommitted. Only2 new plots
are appended to the same energy section; all existing plots remain unchanged.

## Gallery verification

{
  "verified_at_utc": "2026-11-13T14:52:39.425007+00:00",
  "host": "ailogin002.ihep.ac.cn",
  "url": "http://127.0.0.1:8000/",
  "pid": 443262,
  "served_directory": "/aifs/user/data/zhangcg/gsfdev/CEPCSW/TrackingPerformanceStudies/bp_matched_recoil_pt_20261110",
  "manifest": {
    "png_count": 56,
    "section_count": 14,
    "every_png_matched_once": true
  },
  "gallery_page": {
    "http_status": 200,
    "extended_section_present": true,
    "new_values_present": true
  },
  "new_panels": {
    "direct_pre_ecal_truth_collection_dtheta100.png": {
      "http_status": 200,
      "source_sha256": "d9085db05206c11b976e3a089913aba7be9b007bb6ea1bfa27357ce9d5645424",
      "local_sha256": "d9085db05206c11b976e3a089913aba7be9b007bb6ea1bfa27357ce9d5645424",
      "served_sha256": "d9085db05206c11b976e3a089913aba7be9b007bb6ea1bfa27357ce9d5645424",
      "hashes_match": true
    },
    "direct_pre_ecal_truth_collection_dtheta200.png": {
      "http_status": 200,
      "source_sha256": "7a20e6aa6b9735bdf9704d8bb86566b55aef541ff4c03ac337b46ca526d07401",
      "local_sha256": "7a20e6aa6b9735bdf9704d8bb86566b55aef541ff4c03ac337b46ca526d07401",
      "served_sha256": "7a20e6aa6b9735bdf9704d8bb86566b55aef541ff4c03ac337b46ca526d07401",
      "hashes_match": true
    }
  },
  "preexisting_gallery_images": {
    "count": 62,
    "all_sha256_match_before_gallery": true,
    "sha256": {
      "recoil_muon_electron_baseline_kf_10ab.png": "ba6fafe03c8e54b055cd71729bdff651fbf6853f259ad45150951aafa0fae14b",
      "recoil_muon_electron_baseline_kf_gsf_10ab.png": "0993c11d87731269d9d8e045d5a0c1e411289d71e935a18a5da60f05633657b5",
      "recoil_electron_kf_gsf_ebrem_lt1pct_10ab.png": "55b42a407c78fbe259cf32838cc9f3a48e12a48df4814d26e979025fca832d21",
      "recoil_muon_kf_electron_kf_gsf_neutral_recovery_10ab.png": "2c1edbec091fdb5f1d255e27d88a78cb4df3ce4556edd8fba4428128ec979017",
      "direct_pre_ecal_neutral_window_energy_budget.png": "9a2f08371f7d6c2cb02584fac895c266bedca42498ba968bb2ae10a7e7d719fd",
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
      "direct_pre_ecal_truth_collection_width100.png": "32cd39e567984750fbae0fef26bffe188541764c0f80866f60f70e1ca1d4d4e4",
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
      "direct_pre_ecal_truth_collection_width50.png": "9731f043a67ba7d4ff475278615ed267aae02cc38eb66f7c0021b02d60901956",
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

All2 new panels visually reviewed. Main independently checked all62 prior
image hashes unchanged, both new copies byte-identical and56PNG/14section
exact-once coverage. Same ailogin002 PID443262/localhost8000, no restart.
Complete outgoing3-panel status is saved in
2026-11-13-agents-before-extended-ecal-windows.md. Global status/laws/commands
remain byte-identical. No compiled source/install/run-card or batch changes.
