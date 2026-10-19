# Charged and neutral ECAL measurements: focused free-loss A/B

This is a local diagnostic of installed `breakpoint` commit `31d317d`, not a
population validation. The two inputs were
`sim_large_barrel_20261018/rec-barrel-2.root` and `rec-barrel-3.root` from the
same directory. From earlier flat tuples, we selected all events in these files
with a truth-matched track, one selected truth-owned breakpoint, at least one
charged ECAL cluster, and a hit-supported neutral cluster. The chosen event
indices for file 2 were
`1,10,12,28,65,74,78,104,123,126,131,148,166,167,170,199`; for file 3
they were
`1,22,24,30,34,44,59,84,94,99,113,124,133,140,162,182,185,186`.
All selected events were processed (200 events per input file), giving 43 rows,
of which 35 met the final track-level criterion.

Three same-code runs per file used the ordinary free-loss optimizer with:

- charged ECAL measurement ON, neutral OFF;
- charged OFF, neutral ON;
- both measurements ON.

`FreeLossFit=1`, `LossStateMode=Persistent6D`, `SigmaLogLoss=0.001`, and
`IntervalSelectionMode=Truth`. The beam objective, diffuse result and truth
loss override were OFF; all backward filters remained OFF. The neutral cluster
selection used the card's 10 mrad theta and 200 mrad phi windows and its
per-cluster energy-error formula. The truth interval selects location only;
no truth loss amount was fed to these fits.

Eligible rows required `truth_match_status=1`, truth purity at least 0.99,
one `selected_breakpoint_interval`, `collected_neutral_ecal_status=2`,
at least one charged cluster, and charged measurement status 2. This gives
31 primary-track rows (`input_track_index=0`) and four secondary-track control
rows. Four primary rows occur in multi-track events and are kept separate.
The conservative single-track primary cohort is 27 rows; combined neutral
measurement applies in 24 and fails with exact charged-only fallback in three.
Across all 31 primary rows, combined neutral applies in 28 and fails in three.
Neutral-only applies in 30/31.

Median absolute IP pT residual, `100*abs(pT_reco/pT_truth - 1)`:

| Cohort | N | KF | Charged only | Neutral only | Both |
|---|---:|---:|---:|---:|---:|
| Single-track primary, including fallback | 27 | 17.47% | 10.51% | 2.20% | 6.21% |
| Single-track primary, combined neutral applied | 24 | 16.17% | 6.67% | 1.67% | 4.97% |
| All primary rows, including multi-track events | 31 | 17.91% | 10.51% | 2.80% | 5.71% |
| Nonprimary control rows | 4 | 9.99% | 32.04% | 60.63% | 19.56% |

For the 27 single-track rows, the absolute error exceeds 10% in 19 KF,
14 charged-only, nine neutral-only and 12 combined results. Combined improves
absolute error over charged-only for 16/27, over neutral-only for 13/27, and
over KF for 23/27. Neutral-only has the narrowest median core here, but all
approaches have large tails; the mean absolute errors for neutral-only and
combined are 15.59% and 16.62% respectively. These counts are diagnostic,
not selection efficiencies for a general electron sample.

Three examples show why the two ECAL observations cannot yet be assumed to
combine beneficially. File 2/event 1 has selected truth loss 9.34 GeV and
neutral energy 8.71 GeV; the signed pT residual is -1.64% for neutral-only
but -23.75% for both. File 2/event 148 has truth loss 14.34 GeV and neutral
energy 13.41 GeV; neutral-only is -2.19% and both -31.76%. File 3/event 133
has truth loss 8.82 GeV and neutral energy 8.93 GeV; neutral-only is +0.18%
and both -18.93%. These values demonstrate degradation when the neutral
energy is close to the selected truth loss. They do not by themselves identify
whether charged-cluster calibration, correlation with the input track, or
linearization causes it.

The six local flat tuples and logs use `/tmp/bp_moreboth_f{2,3}_*_20261020`.
The final combined files are named `both_ok` and all six final jobs exited
successfully. Two exploratory combined runs had requested 205 events from
200-event files; their complete trees match the corrected 200-event reruns
field by field (515 fields, 21/22 rows). The corrected files are authoritative.
No source code, workflow card, batch job or project ROOT tuple was changed.
This sample uses truth-selected breakpoint locations, and neutral cluster
truth association and charged-cluster independence remain unverified.
