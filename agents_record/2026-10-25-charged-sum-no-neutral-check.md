# Additional no-neutral sum-energy constraint check — 2026-10-25

User requested more events with truth eBrem >1% and no hit-supported neutral
ECAL cluster. Ran paired local OFF/ON refits, not Condor, using the unchanged
installed RecBreakpoint module from c29e4d6. No project source, maintained run-card, installation,
batch campaign or gallery change. No existing tuple was overwritten/deleted.

## Selection and steering

Input: rec_large_barrel_20261025/rec-barrel-1.root. Zero-based entries
38,41,48,54,60,69,78,79,89,97, all additional to the preceding constraint tests.
These events appeared in earlier timing displays; this is not an independent
held-out population. No selection on fit quality, residual, or ON/OFF improvement.

Candidates came from the earlier primary-row CSV but were independently checked
against the CURRENT rec event: exactly one CompleteTracks track, primary
generator electron at MC index 0, matching truth pT, and no secondary tracker
sim-hit MC index >0 in any of the six tracker collections. New fit rows also
all have truth_match_status=1, truth_mc_index=0 and input_track_index=0.

Truth loss is the same compound positive primary G4 eBrem-step momentum-loss
fraction: 1-product(1-min(1,delta_p/p_pre)), parentID=0, matching primary G4
track, abs(PDG)=11, processSubtype=3 and positive momentumLoss. Recomputed
fractions agree with the prior table. Every selected fraction is strictly >1%.

No-neutral is event-wide, not window-only: all trackless charge-zero CyberPFOPID
objects are checked for final EcalCluster links with nonempty direct hits.
All selected events have zero such clusters. Synthetic no-hit neutrals are
allowed. Candidate event 64 was excluded because it has a real neutral cluster.
An eligible truth-layer-owned breakpoint is required and its actual index in
the new fits agrees with the candidate table.

Both runs: installed runtime via source setup.sh; shared default fitter and
ordinary/free/truth prior sigma=.001; free loss enabled; all backward filters
off; beam off; separate neutral constraint off; material ElossOn=False. ON uses
ChargedEcalInputClusters=TrkMergedECAL and ChargedEcalEnergyMode=ElectronPhoton.
OFF differs only in ChargedEcalMeasurement=False. Truth supplies the breakpoint
location, not the fitted loss amount or the cluster energy. Verbose dumps on.

## Results

Both jobs exited successfully. All 10 merged source selections and all 10
ordinary/free charged stages applied successfully. Exact OFF/ON comparison
passes for 163 unconstrained/reference-family branches, including their states
and diagnostics. Thus the baseline does not drift when the option is enabled.

All table entries except loss are pT residuals in percent:
100*(pT_reco-pT_truth)/pT_truth. Ordinary means Gaussian-prior augmented RTS;
free means its outer prior-center optimization. The measured cluster is not changed.

| Event | Loss % | KF | Ordinary OFF | Ordinary ON | Free OFF | Free ON |
|---:|---:|---:|---:|---:|---:|---:|
| 38 | 2.667 | -2.686 | -2.696 | -2.584 | -2.696 | -0.445 |
| 41 | 40.247 | +0.117 | +0.130 | -0.324 | +0.205 | -0.324 |
| 48 | 4.733 | -4.284 | -4.247 | -3.998 | -0.827 | -0.697 |
| 54 | 3.417 | -3.250 | -3.255 | -3.014 | -0.946 | -0.810 |
| 60 | 2.667 | -0.615 | -0.679 | -0.644 | -0.103 | -0.133 |
| 69 | 4.349 | -4.155 | -4.161 | -4.028 | -2.381 | -2.326 |
| 78 | 35.946 | -1.009 | -0.967 | -0.892 | +0.580 | +0.525 |
| 79 | 6.988 | -6.944 | -6.914 | -6.535 | +22.170 | +2.900 |
| 89 | 2.469 | -2.050 | -2.034 | -1.937 | -1.343 | -1.063 |
| 97 | 2.071 | -1.843 | -1.829 | -1.713 | +1.700 | +0.959 |

Free ON is closer to truth than Free OFF in 8/10, worse in 2/10 (41,60).
It is closer than baseline KF in 9/10, worse in 1/10 (41).
Median absolute residual: free 1.144314% -> .753361%; KF 2.367861%.
RMS: free 7.149044% -> 1.327089%; KF 3.325932%. The RMS change is strongly
influenced by the event-79 outlier and is not a population resolution claim.
Ordinary improves in 9/10, with median absolute residual 2.364699% -> 2.260207%.
The larger gain in this small cohort is from sum-constrained free loss.

No-real-neutral does not prove that every emitted photon is contained in the
selected charged shower. Remaining biases/under-recovery and the two worsened
events require separate investigation. No broad physics validation is claimed.

## Reproduction artifacts

All analysis helpers, selection JSON, OFF/ON cards, verbose logs, ROOT outputs
and exact-value results.csv are in /tmp/bp-sum-no-neutral.l87iV3/:
choose_events.py, selection.json, run.py, off.root, on.root, off.log, on.log,
summarize.py, results.csv. The temporary selection script was renamed from
select.py to avoid shadowing Python's standard-library select module.
These generated artifacts are not committed and /tmp is not durable storage.
The source/input selection, steering and results above are the durable record.
