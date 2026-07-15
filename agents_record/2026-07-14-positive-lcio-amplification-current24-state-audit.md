# Current-24 audit of the 30 positive-LCIO amplification IDs

Date: 2026-07-14

## Scope and reproducibility

All 30 durable IDs from
`positive_lcio_amplified_30_candidates.csv` were rerun from `tuples285` with
the installed `MaxComponents=24`, `AggregateWeight` default and
`VerboseDump`, `VerboseSplitDump`, and `ComponentDebugDump` enabled. The 28
seed jobs terminated successfully and produced 30 finite reverse IP outputs.
The runs recorded 157,298 accepted and 99 rejected component updates; there
was no missing output-track or covariance-failure marker. These component
rejections are baseline observations, not evidence that a new change is safe.

Disposable ROOT files and logs are under
`/tmp/gsf-positive-lcio-amplified-30-verbose24`. Durable outputs are:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/positive_lcio_amplified_30_current24_diagnostics.csv`
- `Reconstruction/RecGsfTracking/scripts/analyze_positive_lcio_amplification.py`

The analyzer records the current truth/LCIO/GSF state, stored/current drift,
final identity and selected states, selected process signature, the first
pre-KL radiative-over-identity divergence, exact prior and innovation odds,
and the local KL weight change. A measurement divergence is the first inward
hit where the largest radiative pre-KL posterior score exceeds the identity
score. If the reverse seed is already radiative-dominant, it is explicitly
labelled `reverse_seed`. The exact likelihood uses
`exp[-0.5*(Delta dchi2 + Delta logDetS)]`. This definition avoids attributing
post-update KL-created component IDs to the measurement update itself.

## Stored/current drift

The durable ID set was preserved. Only 21/30 events still satisfy the original
current amplification threshold of at least +0.25 percentage point. Nine
drift below it:

```text
319/9  92/6  71/6  285/8  163/5  167/0  207/3  84/5  452/1
```

Eight are no-eBrem and one is light-eBrem. Their current amplification ranges
from -0.0355 to +0.0120 percentage point. The largest GSF-residual drift is
2.271 percentage points. Twenty-nine events differ from the stored result by
more than 0.001 percentage point, so same-code reruns are mandatory.

The stored-amplification split is especially informative. Of 13 IDs stored
below +1 point, the current median amplification is only +0.005 point and
eight publish identity. For the 17 stored at or above +1 point, the current
median remains +2.315 points. The low-amplification IDs are therefore a useful
drift/control stratum, not a stable optimization target.

## State mechanisms

The no-eBrem stratum has 20 events: 15 first diverge at a measurement and five
are already radiative-dominant in the reverse seed. Its current median
amplification is +0.457 point. The light-eBrem stratum has 10 events: seven
measurement divergences and three reverse-seed divergences, with median
amplification +1.844 points.

Across all 30, the median radiative-to-identity prior odds at the recorded
divergence are 0.0643, the median exact innovation likelihood ratio is 1.494,
and the median posterior odds are 2.689. These medians summarize an extremely
heterogeneous population and must not be read as a single mechanism. Large
likelihood ratios and effectively overwhelming inherited reverse-seed odds
both occur. At every locally auditable decisive reduction, radiative and
identity KL weight amplification is exactly 1.0; KL is not creating the
decisive crossing.

Nine current identity publications have no selected radiative signature. The
remaining selected branches use radiative surfaces 5--10, concentrated at
surface 8 (8 events), 6 (7), 5 (5), and 7 (4), with one each at 9 and 10.
This agrees with the previous 19-overshoot/18-control audit: persistent
amplification is organized by a discrete radiative surface/mode choice in the
informative 5--11 region, not by global process-core probability or recurring
KL deletion. The audit does not identify a calibrated replacement selection
rule; no physics or selection setting was changed.

## Decision and resume point

Steps 1--3 of the positive-LCIO audit are complete. Keep the 30-ID table intact
and use `current_amplification_pct >= 0.25` only as an analysis flag, not as a
replacement ID selection. The next bounded task is to compare the 21 persistent
events, especially the 17 large stored amplifications, directly with the
existing 19 overshoots and 18 matched controls at the selected surface/mode.
Seek a physically interpretable discriminator that preserves identity-like
clean events and the demonstrated hard recovery. Do not tune the rejected
noisy-OR floor, KL reduction, global process prior/covariance, dominant-lineage
publication, or an ad hoc measurement-evidence threshold.

Any proposed change must begin the existing validation ladder: 19 overshoots,
18 controls, five ordinary light representatives, clean 62/9, hard 1/3, then
11/16/17 and the full clean/light/hard populations before transfer controls.

