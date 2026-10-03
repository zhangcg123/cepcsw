# Candidate CyberPFA phi-boundary track/ECAL association failure

Status: **diagnosed, not fixed**. This is evidence for a likely common cause of
the 53 hit-supported neutral ECAL clusters in the no-eBrem control sample whose
selected electron tracks have no associated charged ECAL cluster. The user
explicitly requested a record only; no reconstruction source or run card was
changed. A wrapped-phi same-code A/B rerun has not been performed, so the
population-level causal claim remains provisional.

## Sample and neutral-cluster inventory

The sample is the 72-file barrel breakpoint/REC run under
`breakpoint_barrel/breakpoint_flat-barrel-*.root` and
`sim_large_barrel_20261001/rec-barrel-*.root`, restricted to the 11,820
topology-clear selected tracks used in
`TrackingPerformanceStudies/breakpoint_photonlast_20261003/performance_rows.csv`.
The direct-hit/virtual audit is in
`TrackingPerformanceStudies/breakpoint_neutral_geometry_20261004/`.

| Truth-loss category | Neutral ECAL clusters | Zero-direct-hit virtual split | Direct-hit cluster |
| --- | ---: | ---: | ---: |
| No eBrem | 1,157 | 1,100 | 57 |
| eBrem <1% | 1,124 | 1,065 | 59 |
| eBrem >=1% | 3,017 | 1,955 | 1,062 |
| Total | 5,298 | 4,120 | 1,178 |

The virtual objects have a one-child-core, one synthetic cell-ID `-99` hit
signature and no direct ECAL hits. Their position is constructed from the
parent charged PFO's calorimeter clusters, so close direction is not
independent photon evidence. They all carry reconstructed PFO label 130.
Direct-hit clusters are a separate population; their reconstructed labels
(22 or 130) are **not** truth particle IDs. Of the 57 no-eBrem direct-hit
clusters, 47 are labelled 22 and 10 labelled 130. For 53/57, the selected
track's charged PFO has no ECAL cluster; the neutral shower carries nearly
the primary electron energy (median neutral-cluster/truth-energy ratio
0.998). The other four do have a charged ECAL cluster.

## Evidence for the phi-boundary failure

- All 53 no-eBrem unassociated tracks have 230--235 linked tracker hits,
  with outermost hit radius 1807.4--1810.3 mm. A 53-track ECAL-matched
  same-seed, nearby-truth-energy control has 231--235 hits and the same
  outer-radius range. The tracks do not stop early in the tracker.
- All 53 have an ECAL-extrapolated track state. The opening angle from that
  state to the hit-supported neutral cluster centroid is 0.84--4.47 mrad
  (median 1.95 mrad). Centroid proximity alone does not prove a valid
  bar-level match because the points have different shower depths.
- The ECAL-state reference point is in the `x<0, y<0` quadrant for **all
  53**. Its azimuth lies 0.7--84.7 mrad above `-pi`. Among all 4,460
  no-eBrem tracks, 57 lie in that negative-side 85-mrad wedge: 53 have no
  charged ECAL cluster and four do. None of the 4,403 outside the wedge has
  this missing-charged-ECAL outcome. This sharp one-sided concentration is
  not explained by track length or a broad track/shower separation.
- `TrackMatchingAlg::CreateTrackAxis` compares barrel-U azimuth with
  `fabs(extrapo_point.Phi() - local_max.Phi())`, without periodic wrapping
  (`Reconstruction/RecPFACyber/src/Algorithm/TrackMatchingAlg.cpp`). A track
  just below `-pi` and nearby U bar with center at `+pi` therefore appear
  almost `2*pi` apart. The barrel-V branch uses a spatial distance rather
  than this raw azimuth subtraction.
- The 3D cluster receives a track only if the same track pointer is present
  in **both** U and V longitudinal clusters
  (`EnergyTimeMatchingAlg::XYClusterMatchingL0`, near line 1098).
  An absent U link therefore leaves a nearby shower uncharged. The subsequent
  `TrackClusterConnectingAlg::EcalChFragAbsorption` can rescue an unassociated
  ECAL fragment only when its energy is below 2 GeV, among other conditions;
  all 53 showers exceed 10 GeV and cannot be rescued by that branch.

Focused reproduction: rerunning REC on seed 1 through event 12 with
`CyberPFAlg.WriteAna=True` reproduced the original 17.253 GeV neutral ECAL
cluster (electron truth energy 17.371 GeV) and no charged ECAL cluster.
The diagnostic `TrackAxis` tree has a V track axis but **no U track axis**;
the 3D cluster has `typeU=100`, `typeV=10100`, and `nTrk=0`. For one U local
maximum and extrapolated ECAL track point, the radial gap is 5.94 mm and
`|delta z|` is 0.001 mm. The raw azimuth difference is 6.2570 rad, whereas
the wrapped difference is 0.02616 rad, below that bar's 0.07757-rad
half-length matching limit. Thus the unwrapped azimuth comparison directly
rejects a geometrically acceptable U candidate in this event.

Interpretation: this is a **high-confidence candidate** common mechanism for
the 53 no-eBrem full-energy showers labelled neutral. The detailed U/V
intermediate state has been checked directly for seed 1/event 12 only; the
other 52 share the highly specific azimuth-boundary and final-association
signature. A temporary wrapped-phi A/B rerun, with unchanged upstream inputs
and event-by-event PFO comparison, is the remaining confirmation gate.
Do not silently treat these 53 clusters as eBrem photons, and do not change
CyberPFA as part of the current record-only task.
