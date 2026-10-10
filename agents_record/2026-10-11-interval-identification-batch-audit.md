# Barrel baseline interval-identification batch audit (2026-10-11)

## Outgoing focus preserved

The previous `AGENTS.md` current focus established a separate
`RecBreakpointIdentification` dataset producer under RecBreakpoint. It reads
existing REC, records hits and geometry, baseline forward, native-smoothed and
independently seeded backward diagnostics, and truth labels only after the
reconstructed features are complete. It does not fit a breakpoint, apply ECAL
information or steer from truth. The standard first/middle/last hit prefit uses
both track ends in both directions, so the filter outputs are not strictly
independent evidence. The 18-event/20-track build-tree gate passed truth-on/off
feature isolation and production-style CompleteTracks pT reproduction; it was
a mechanical gate, not physics validation. Native timing is a sixth KF state,
not a loss parameter. The first version orders hits by barrel radius. The
installed runtime and recovery algorithms were unchanged. Original evidence:
`2026-10-10-interval-identification.md` and
`2026-10-10-identification-material-and-first20.md`.

The outgoing focus stated reconstructed t/X0 was missing. That is superseded:
schema 2 added independent forward/reverse DD4hep chord t/X0 and coverage
statuses, validated on the initial 20 tracks. Explicit surface axes remain
unrecorded. The outgoing focus also prohibited running a production batch as
part of the initial development; the user subsequently launched this dedicated
identification batch. Its REC input and production tracking remain untouched.

## Batch and denominator

`breakpoint_interval_barrel_20261010/` has 72 completed job manifests and 72
output ROOT files, sourced from 72 nonempty `rec-barrel-SEED.root` files under
`sim_large_barrel_20261001/` with 200 input events requested per file.
There are 14,398 events with at least one output track and 15,395 track rows.
Two of the 14,400 requested event slots have no track row. Each completed job
verified its tree and key branches. The batch was not a held-out physics test.

Primary analysis cohort: 13,182 rows with exactly one reconstructed track in
their event, >=100 hits, unique successful truth match (`truth_status=1`) and
truth-match purity >=0.9. Multi-track events, short tracks and invalid truth
remain separate controls. Overall 14,681/15,395 rows have successful truth;
714 fail provenance matching, most often a hit without a provenance-bearing
SimTrackerHit. This cohort is **not** the previously certified topology-clear
catalogue; use the exact selection above when comparing numbers.

For each interval, fractional loss is its summed primary-electron Geant4
momentum loss divided by the associated generator-particle momentum magnitude.
All quantities below concern an in-span truth interval. Primary radiation
before/after the measured hit span is not silently relabelled an in-span loss.
In the selected cohort, 1,686 tracks have at least one such out-of-span step;
1,135 have >=0.2% summed out-of-span loss. Among the 5,172 tracks with zero
in-span loss, 435 have >=0.2% out-of-span loss; these are not clean no-radiation
controls.

## Loss multiplicity and label limits

| Minimum loss per interval | Tracks with >=1 interval | Exactly 1 | Exactly 2 | >=3 |
|---|---:|---:|---:|---:|
| 0.2% | 5,349 | 4,244 | 961 | 144 |
| 1% | 3,958 | 3,422 | 492 | 44 |
| 5% | 2,339 | 2,194 | 141 | 4 |

At a 1% threshold, a method forced to output at most two true intervals cannot
achieve 99% **exact whole-track correctness** even with an oracle: 44/3,958
positive tracks (1.11%) have three or more qualifying intervals. A two-loss
cap only crosses 99% for this sample at the 5% threshold (4/2,339, 0.17%).
Separate event detection, interval recall, exact location and +/-1 interval
location; report all denominator choices.

For the 4,539 >=1% intervals, 299 (6.6%) have a truth boundary-step ambiguity
flag. Keep this cohort visible when defining exact versus neighbour-tolerant
scoring. The algorithm assigned an owner, but the flag warns that the emission
occurred on a hook boundary.

## Direct feature ranking

For interval i -> i+1, forward local chi2 is from the update at hit i+1 and
backward local chi2 is at hit i. The curvature separation is
`abs(B_predicted_kappa[i]-F_updated_kappa[i]) / sqrt(P_B[kappa,kappa]+P_F[kappa,kappa])`;
it neglects cross-covariance and is a diagnostic score, not a calibrated
probability. Material score is reconstructed `interval_chord_tx0`.

For >=1% loss intervals, exact inclusion among the highest-scoring candidates
on that track is:

| Candidate score | Top 5 exact | Top 20 exact |
|---|---:|---:|
| max(forward, backward local chi2) | 6.9% | 18.4% |
| curvature separation | 3.4% | 7.6% |
| reconstructed t/X0 | 74.7% | 86.3% |

The chi2 audit excludes discarded-site code 5 with DBL_MAX-like sentinel but
retains finite chi2-cut code 6 scores. The original unmasked simple ranking
gave the same broad conclusion but must not be used for a trained feature.
With +/-2 interval tolerance, the top 20 max-chi2 score reaches 49.4% and
the top 20 t/X0 score reaches 88.9%; neither is 99% exact identification.
Selecting five material intervals on every one of the 13,182 tracks would
publish 65,910 candidates, including only 3,389 of the 4,539 >=1% true
intervals: 5.14% candidate precision. Selecting twenty on every track gives
3,916 true among 263,640 candidates: 1.49% precision. These figures show why
t/X0 is a proposal prior, not the final eBrem decision.
At 99% sensitivity for detecting whether a track contains any >=1% loss,
a threshold on its maximum forward local chi2 falsely flags 98.1% of
no-in-span-loss controls with <0.2% out-of-span loss. The backward maximum and
maximum simple curvature separation likewise falsely flag about 98%.
This does not establish a limit on a future whole-track method; it rejects
these simple maximum rules.

The 4,539 >=1% labelled intervals are distributed as 1,097 VXD, 1,686 ITK
and 1,756 TPC. Of the TPC set, 611 (13.5% of all labelled intervals) are
internal TPC intervals with hit index below 220. Material top-20 exact recall
is 100% in the VXD and ITK subsets, but only 64.5% for TPC. Its apparent
strength is dominated by a few thick boundaries; it misses many internal TPC
losses. The first 20 events were too small to show that clearly.

KF rejection is a useful but incomplete signal. Among selected tracks, 742
of 3,958 with >=1% loss have at least one forward/backward rejected hit,
versus 25 of 9,224 below 1% loss. MarlinTrk status 6 denotes chi2-cut
rejection and status 5 denotes general site discard; both must be read as
statuses, not as numeric local chi2. Only 18.7% of >=1% tracks exhibit a
rejection, so this is a high-confidence special case, not the main finder.

For true >=1% intervals with valid material values (4,536/4,539), reconstructed
chord t/X0 versus truth interval t/X0 has median absolute relative difference
0.114% and 90th percentile 2.52%. This supports using the reconstructed
geometry feature, with the established caveat that it is not the exact curved
KF path. Missing/invalid paths have explicit status/NaN.

## Revised identification strategy

1. Define the output contract first: no loss, one, two and a rare three-or-more
   category; a declared minimum loss fraction; exact and +/-1 interval metrics;
   boundary-ambiguous and out-of-span controls. Protect the 714 truth-invalid
   rows from being treated as negatives.
2. Use reconstructed t/X0 and detector boundaries as a candidate prior, but
   scan **all internal TPC intervals** with a separate hit-sequence test.
   Restricting to the thickest intervals caps recall below the requested 99%.
3. Develop a physics change-point score under RecBreakpoint from the hit series:
   independent inner and outer segment fits seeded only from their own hits,
   with a nonnegative momentum-loss parameter profiled at each candidate
   interval. Compare the full-hit likelihood to no-loss and penalize extra
   breakpoints. Current forward/backward seeds reuse both ends and cannot be
   treated as independent likelihood factors.
4. Use KF rejection flags as reliable auxiliary features. Do not train on
   invalid numeric chi2 sentinels. A learned sequence proposal can be compared
   after the physical whole-track baseline; evaluate whether it adds recall in
   internal TPC rather than merely memorizing material peaks.
5. Gate in order: candidate recall >=99% on held-out seeds for an explicit
   loss threshold, then final exact-location precision and whole-track
   correctness, then endcap/angle/momentum and topology controls. No such gate
   is passed by the present batch.

Audit scripts and intermediate JSON are in `/tmp/bpid_population_audit.py`,
`/tmp/bpid_rejection_audit.py`, `/tmp/bpid_corrected_score.py`, and
`/tmp/bpid_population_audit.json` (local scratch, not durable project inputs).
The denominators and rankings above are the durable summary.
