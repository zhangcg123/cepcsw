# Current shared-sigma free-loss population comparison

## Fixed plan, before results

User requested more events to establish whether free-loss fitting is better.
No source, maintained card, installed library, shared KF/GSF code, batch
submission or remote change is part of this study. Starting checkpoint e2b4f3d.
The current installed shared-SigmaLogLoss implementation is used, not the
retired fixed-b optimizer or a diffuse/no-prior augmented KF.

All 100 events in each existing tracker file for seeds21--40 are included:
2,000 input events. Selection is by seed range, not previous residual or fit
quality. Two fixed blocks21--30 and31--40 will be reported separately.
These seeds are separate from the latest selected-event smoke test, but are
not claimed never to have appeared in historical development studies.

Input directory: gsf_doublebhoff_freshseed_diagnostic.
Card: Reconstruction/RecBreakpoint/options/run_breakpoint.py, unchanged.
Defaults: LocalMarginal, Truth largest-absolute-loss interval selection,
MeanLogLoss=0, SigmaLogLoss=.001, Forward SeedScale=1, BackwardSeedScale=100,
FirstMiddleLast, MS on, deterministic Eloss off, free-loss on, truth-centred on.
All three pairs are generated in the same current-code execution.
Only I/O/event count and diagnostic verbosity are set by the test driver.

Library SHA256:
8689b40e378ce8886e7e0784de983a17c0d943be2b3d014939106d0a73482b0d.
The driver records/checks card/library checksums and input sizes/timestamps.
Jobs run locally, three concurrent processes with numerical-library threads=1.
Runtime evidence, raw output and scripts:
/tmp/recbreakpoint-population-20260930.TbPv8E/.
Analysis tables/plots:
TrackingPerformanceStudies/breakpoint_shared_sigma_population_20260930/.
ROOT/log/plot/table/script artifacts remain uncommitted.

## Definitions and safeguards

Residual = 100*(pT_reco/pT_truth - 1), in percent.
Compare free-loss RTS and backward against both the baseline CompleteTracks KF
and their respective zero-centred Gaussian breakpoint counterparts.
Truth-centred endpoints are a reference, not used to tune the optimizer.
All ordinary/free comparisons are same-code paired results.

Use the existing topology-clear catalogue; verify event identity against the
same sample's passive GSF truth and baseline KF values. Truth categories use
cumulative G4 loss over matched tracker intervals, not the largest selected
interval alone: no loss, (0,1%), >=1%; additionally split1--10% and>=10%.
The fitter still treats only its single selected largest absolute-loss interval.

Primary paired sample requires one input track, topology clear, valid primary
association and finite ordinary/free endpoints. Optimizer KF fallbacks remain
included. Do not drop them by requiring optimization success. Do not require
the truth-centred pair to succeed when forming ordinary/free comparisons.
Primary failures/missing outputs and secondary activity are counted separately;
report candidate-normalized within-window rates as an efficiency cross-check.
Secondary events never enter the main physics comparison.

Predeclared metrics: median, width68=(q84-q16)/2, RMS about zero, within +/-1%,
tails outside +/-5% and +/-10%, paired improvement/worsening and new tails.
Intervals use 2,000 paired seed-cluster bootstrap repetitions. Fixed seed-block
agreement provides an additional stability check. Plotted-window overflow is
not discarded when calculating metrics or normalizing histograms.

## Status

Study running. Results and failure accounting will be recorded here after all
20 jobs and the identity/selection checks complete. No performance conclusion
is drawn from partial output. The complete outgoing focus is preserved in
2026-09-30-agents-before-free-loss-population.md; section1/laws remain unchanged.
