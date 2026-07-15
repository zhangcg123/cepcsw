# Topology-clean resurvey and first fresh decisive-hit odds

The historical representative indices were treated as stale and were not used
as the selection basis. A reproducible survey was built from the current
surface-owned category table after excluding every event with a non-primary
tracker SimHit. The executable definition is
`Reconstruction/RecGsfTracking/scripts/survey_topology_clean_gsf_outcomes.py`;
its complete event catalogue, cell summaries, and median-proximity candidates
are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/`.

The topology-clean populations are 2,032 no-eBrem, 2,132 light-eBrem, and 694
hard-eBrem. An ordinary optimization event additionally requires both LCIO and
GSF absolute residuals below 10%; 2,099 light events pass. Outcome definitions
are deterministic. For LCIO residual below -1%, the correction fraction is
`(GSF-LCIO)/(-LCIO)`. Final GSF within 1% is good recovery; more than 0.1
percentage-point absolute-error worsening is degradation; correction below
20% is missed, 20--80% is partial, 80--120% outside the 1% core is near
recovery, and above 120% is overshoot. Truth-like LCIO events are classified
separately.

Across all 2,132 topology-clean light events, the final classes are 1,478
truth-like-LCIO preserved, 68 truth-like-LCIO degraded, 291 good recoveries,
205 missed recoveries, 50 partial recoveries, 7 near recoveries, 20 overshoots,
10 degradations, and 3 other truth-like cases. These are final-state survey
labels, not claims about internal mechanism.

Five fresh ordinary representatives were rerun with complete component dumps.
At each decisive hit the Gaussian likelihood ratio uses the exact
`exp[-0.5*(deltaChi2+logDetS)]` quantity. Odds below are winner over the stated
truth-compatible alternative.

| event/class | decisive hit | prior odds | likelihood ratio | posterior odds | interpretation |
|---|---:|---:|---:|---:|---|
| 2/7 missed | 0 | 1,020.21 | 1.2089 | 1,233.33 | overwhelmingly prior-limited identity win |
| 234/4 good | 7 | 2.9298 | 3.0142 | 8.8310 | moderate prior and hit support for recovery |
| 309/6 partial | 4 | 4.1363 | 13.6032 | 56.267 | measurement reinforces under-correcting branch |
| 299/7 overshoot | 7 | 0.8856 | 62,727 | 55,553 | excessive branch starts behind; hit likelihood dominates |
| 463/7 low-loss false correction | 1 | 0.6158 | 2.0766 | 1.2787 | near-boundary inner-hit flip |

KL reduction is not the primary cause in these comparisons. For 2/7, the
truth-compatible competitor grows from 0.0005510 to 0.0007323 while identity
stays 0.747136. For 234/4, compared weights grow from 0.0095459/0.0035748 to
0.0124745/0.0042579. For 309/6 they grow from 0.13320/0.033779 to
0.139788/0.033795. For 299/7, KL favors the truth-compatible branch more:
0.010463/0.0089846 becomes 0.012079/0.013639 before the extreme likelihood
flip. The 463/7 decisive hit has no intervening reduction; priors are
0.258928/0.420507 for false/compatible branches.

The first comparison demonstrates that missed recovery cannot be repaired by
a generic measurement threshold, while the overshoot demonstrates that prior
reweighting alone cannot handle every false selection. Second representatives
in each populated mechanism class are still required before proposing a
change.

## Second representatives

Second fresh representatives reproduce the mechanism diversity:

| event/class | decisive hit | prior odds | likelihood ratio | posterior odds | interpretation |
|---|---:|---:|---:|---:|---|
| 166/6 missed | 0 | 123,511 | 2.3920 | 295,439 | even more strongly prior-limited than 2/7 |
| 399/8 good | 8 | 35.35 million | 5,234.7 | 185.0 billion | recovery branch has overwhelming prior and hit support over identity |
| 377/3 partial | 0 | 0.7349 | 1.9079 | 1.4020 | final hit reverses a truth-compatible lead to under-correction |
| 26/9 overshoot | 5 | 0.2823 | 6.2679 | 1.7693 | hit likelihood flips a branch that started behind |
| 340/5 low-loss false correction | 4 | 1.1858 | 1.4517 | 1.7215 | KL aggregation puts false branch ahead, but the unmerged branch would still win after the hit |

For 340/5 the false/identity raw pre-merge odds are 0.217941/0.252130 =
0.8644. Multiplying by the exact 1.4517 likelihood ratio still gives about
1.255, so unmerged selection cannot fix this event. In contrast, 463/7 has no
intervening reduction at its decisive hit and is purely a near-boundary
likelihood flip. The paired survey therefore does not support KL semantics as
the general light-tail mechanism.

## Rejected dominant-unmerged-lineage selection diagnostic

An opt-in `ReverseSelectionMode=DominantLineage` was implemented with unchanged
`AggregateWeight` default. Each component tracks the fraction of aggregate
weight carried by its strongest real pre-merge lineage. Splitting and
likelihood normalization preserve the fraction; moment merging sets it from
the maximum contributing lineage rather than their sum. The diagnostic changes
only final reverse-branch selection, not filtering, measurement updates, or
published aggregate weights.

Focused complete dumps retained all hits. It fixed 463/7 from +1.7468% to
-0.0705% but left 340/5, 2/7, 234/4, 309/6, 299/7, clean 62/9, and hard 1/3
essentially unchanged. Hard 1/3 remained at -0.4587%.

The full 2,132-event topology-clean light category rejects the candidate:

| metric | AggregateWeight | DominantLineage |
|---|---:|---:|
| median residual | -0.0969% | -0.1104% |
| central-68 half-width | 0.4357% | 0.4743% |
| inside 1% | 1,769 | 1,761 |
| inside 2% | 1,959 | 1,963 |
| inside 5% | 2,054 | 2,057 |
| beyond 10% | 33 | 31 |

There are 169 material changes: 59 improve and 104 worsen. In particular, 55
previously preserved truth-like events worsen, while only 16 existing
truth-like-LCIO degradations improve. Two >10% tails are fixed and none are
created, slightly improving RMS, but the core and 1% population worsen. The
candidate is rejected before no-eBrem/hard production validation. Exact
eventwise results are under
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/dominant_lineage_2026-07-13/`.

Surface-owned truth also shows that the dominant eBrem transition and decisive
measurement need not coincide. Examples include 234/4 transition 8 with
selection at hit 7, 399/8 transition 9 with selection at hit 8, 26/9 transition
7 with overshoot emerging at hit 5, and 463/7 only a tiny 0.146% transition at
69 while the false correction occurs at hit 1. Remaining work must distinguish
process prior/history from the inward measurement lever arm rather than treat
all inner flips as KL aggregation.
