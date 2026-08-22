# Global one-loss expanded event gate

Date: 2026-08-22
Branch: `test_new`
Frozen implementation: `8c0809a`
Recovery documentation base: `a0bba6d`

## Question and scope

This study tested the committed, separate
`RecGsfGlobalLossRefitter` on more events without changing source code,
maintained run cards, or ready workflows. The refitter reads
`CompleteTracks`, compares identity with all exactly-one-radiative-interval
histories after consuming every inward hit, and writes `GlobalLossTracks`.
It remains an unscheduled diagnostic instrument.

The study asked:

1. Does the event-3 improvement and event-4 identity protection generalize?
2. In topology-clear, recoverable transition-5--11 losses, does global
   all-hit evidence select the correct interval and retained fraction?
3. Does the method avoid creating new clean-track degradation and extreme
   tails?

Truth was used only to select and classify diagnostic events after the runs.
The refitter itself remained truth-blind.

## Frozen steering

All runs used the committed defaults, explicitly steered in temporary cards:

```text
ElectronHypothesis=true
BHModel=CEPC2GeV85StepConditioned
BHSplitThreshold=1e-4
MSOn=true
ElossOn=false
OuterSeedCovarianceScale=100
ProcessSigmaWindow=3
ProfileGridPoints=9
ProfileRefinementIterations=6
MinimumRetainedFraction=0.05
MinimumRadiativeLogBayesFactor=3
CandidateIntervalIndices=[]
VerboseDump=false
```

`SelectedEventIndices` and temporary input/output filenames were the only
per-run differences. No source file was edited. The retained GSF EDM files
were valid inputs because they preserve `CompleteTracks` and the tracker-hit
collections; the new algorithm reads those collections, not the stored
`GSFTracks`.

Temporary cards, outputs, and logs were written only under `/tmp`:

```text
/tmp/run_global_loss_all10_822751.py
/tmp/global_loss_all10_822751.root
/tmp/global_loss_all10_822751.log

/tmp/run_global_loss_panel35.py
/tmp/global_loss_panel35.root
/tmp/global_loss_panel35.log

/tmp/run_global_loss_mid19.py
/tmp/global_loss_mid19.root
/tmp/global_loss_mid19.log

/tmp/run_global_loss_mid21.py
/tmp/global_loss_mid21.root
/tmp/global_loss_mid21.log

/tmp/run_global_loss_mid28.py
/tmp/global_loss_mid28.root
/tmp/global_loss_mid28.log

/tmp/run_global_loss_mid66.py
/tmp/global_loss_mid66.root
/tmp/global_loss_mid66.log
```

Every job finalized successfully. The known ROOT PCM warnings remained
non-fatal.

## First expanded panel: 20 mixed events

The first run processed all ten entries of
`trk-e--20-85-822751.root`. The second used ten deliberately selected,
topology-clear entries from
`gsf_e-_reverse-truth-bh-off_35.root`: three hard-loss examples, two
light-loss examples, one degraded no-eBrem control, and four very good
no-eBrem controls.

Here `secondary` means `all_hit_n != lcio_hit_n`; these events are reported
separately and are not representative single-track optimization events.
Residuals are `100 * (pT / truth_pT - 1)`.

| File | Entry | Topology | Truth eBrem | LCIO residual | GSF residual | Global residual | Selected interval | Log BF |
|---|---:|---|---:|---:|---:|---:|---:|---:|
| 822751 | 0 | secondary | 0.275% | -1.023% | -0.963% | -0.978% | identity | -2.207 |
| 822751 | 1 | clear | 0.591% | +0.121% | +0.309% | +0.252% | identity | -2.304 |
| 822751 | 2 | secondary | 0 | +0.137% | +0.315% | +0.218% | identity | -2.228 |
| 822751 | 3 | secondary | 6.546% | -6.453% | -3.760% | -4.208% | 6, mode 2 | +4.273 |
| 822751 | 4 | clear | 0 | -0.163% | +2.148% | -0.154% | identity | +0.380 |
| 822751 | 5 | secondary | 0.696% | -0.733% | -0.628% | -0.808% | identity | -1.330 |
| 822751 | 6 | secondary | 0 | +0.098% | +0.244% | +0.151% | identity | -2.439 |
| 822751 | 7 | clear | 0.550% | -0.636% | -0.567% | -0.586% | identity | -1.920 |
| 822751 | 8 | clear | 0 | +0.054% | +0.145% | +0.056% | identity | -2.228 |
| 822751 | 9 | clear | 32.231% | -1.486% | -0.816% | -0.863% | identity | -2.331 |
| 35 | 6 | clear | 23.109% | -23.117% | -22.999% | -23.118% | identity | -1.814 |
| 35 | 7 | clear | 13.288% | +0.502% | +0.694% | +0.610% | identity | -2.383 |
| 35 | 19 | clear | 0 | +0.320% | +0.307% | +0.281% | identity | -2.430 |
| 35 | 45 | clear | 0 | -0.138% | -0.036% | -0.123% | identity | -2.325 |
| 35 | 48 | clear | 1.146% | -0.344% | -0.231% | -0.313% | identity | -2.438 |
| 35 | 53 | clear | 0 | -0.022% | -0.028% | -0.056% | identity | -2.241 |
| 35 | 59 | clear | 0.562% | -0.287% | +2.833% | -0.338% | identity | +0.416 |
| 35 | 61 | clear | 0 | -0.043% | +0.005% | -0.076% | identity | -2.213 |
| 35 | 70 | clear | 0 | -0.114% | +0.009% | -0.112% | identity | -2.739 |
| 35 | 90 | clear | 33.815% | -0.193% | -0.357% | -0.370% | identity | -2.485 |

Only the previously studied secondary-activity event 822751:3 crossed the
radiative gate. None of the 15 topology-clear entries selected radiation.

This initial absence was not immediately evidence that the global method could
not work. The hard topology-clear controls did not form the intended
information-rich mid-tracker set:

- 35:6 lost 23.1% at interval 1, an information-limited inner loss.
- 35:7 and 35:90 had their large losses at interval 231/230, near the outer
  end, while the identity IP refit was already accurate.
- 822751:9 had a 31.7% loss at internal-TPC interval 211 with
  `t/X0=4.31e-5`, below `BHSplitThreshold`; no truth-location radiative
  history existed in the current bank.
- 35:48 was another below-threshold internal-TPC loss.
- 35:59 had light losses at intervals 6 and 7. A false radiative alternative
  gained only +0.416 log evidence and the gate correctly retained identity,
  avoiding the existing GSF's +2.83% overshoot.

Selected-sample aggregate absolute residuals were:

| Population | N | LCIO mean/max | Existing GSF mean/max | Global mean/max |
|---|---:|---:|---:|---:|
| All first-panel events | 20 | 1.799% / 23.117% | 1.870% / 22.999% | 1.684% / 23.118% |
| Topology clear | 15 | 1.836% / 23.117% | 2.099% / 22.999% | 1.821% / 23.118% |
| No eBrem | 9 | 0.121% / 0.320% | 0.360% / 2.148% | 0.137% / 0.281% |
| Truth loss | 11 | 3.172% / 23.117% | 3.105% / 22.999% | 2.949% / 23.118% |

These are deliberately selected mechanism statistics. The no-eBrem row shows
that the radiative gate suppressed the sampled false GSF modes, but the
independent identity refit still broadened the selected clean mean relative
to `CompleteTracks` (0.137% versus 0.121%).

## Targeted topology-clear transition-5--11 gate

A read-only scan found 81 usable retained flat/EDM pairs and 226
topology-clear events containing at least one 2% or larger Geant4 loss in
intervals 5--11. Ten hard cases were selected across files 19, 21, 28, and 66.
The selection deliberately mixed catastrophic LCIO/GSF failures with cases
where the existing reverse GSF already recovered well. It is a mechanism
panel, not an unbiased validation population.

The dominant truth interval is the largest fractional loss in intervals
5--11. `Truth z` is its Geant4 retained fraction; `selected z` is the
global refitter's post-selection profile result.

| File | Entry | Truth interval | Truth loss | Truth z | LCIO residual | GSF residual | Global residual | Selected interval/mode | Selected z | Log BF |
|---|---:|---:|---:|---:|---:|---:|---:|---|---:|---:|
| 19 | 4 | 6 | 31.840% | 0.6816 | -31.981% | -1.556% | -1.517% | 6 / 4 | 0.6846 | +546.952 |
| 19 | 59 | 6 | 43.322% | 0.5668 | -42.791% | +4.366% | +4.569% | 5 / 4 | 0.5423 | +375.624 |
| 21 | 43 | 5 | 70.524% | 0.2948 | -70.979% | -16.000% | +13.385% | 4 / 4 | 0.2560 | +17.973 |
| 21 | 65 | 5 | 32.836% | 0.6716 | -33.576% | -26.010% | -25.968% | 5 / 3 | 0.8962 | +7.864 |
| 21 | 93 | 5 | 14.842% | 0.8516 | -15.503% | -15.243% | -15.349% | identity | 1.0000 | -1.631 |
| 28 | 62 | 7 | 31.818% | 0.6818 | -29.939% | -0.914% | -1.021% | 6 / 4 | 0.6920 | +388.311 |
| 66 | 22 | 6 | 22.950% | 0.7705 | -23.106% | -0.238% | -0.148% | 6 / 4 | 0.7606 | +287.342 |
| 66 | 29 | 7 | 21.019% | 0.7898 | -19.874% | +0.280% | +0.426% | 7 / 4 | 0.7755 | +611.928 |
| 66 | 66 | 5 | 79.225% | 0.2077 | -79.171% | -56.088% | +315.955% | 4 / 4 | 0.0500 | +113.309 |
| 66 | 86 | 6 | 36.977% | 0.6302 | -28.654% | +2.400% | +2.583% | 5 / 4 | 0.6883 | +97.369 |

The categorical outcome is:

```text
radiative history published:       9 / 10
exact truth interval selected:     4 / 10
adjacent wrong interval selected:  5 / 10
identity despite hard loss:        1 / 10
```

Aggregate selected-panel results were:

| Panel | N | LCIO mean abs / max / >3% | Existing GSF mean abs / max / >3% | Global mean abs / max / >3% |
|---|---:|---:|---:|---:|
| All | 10 | 37.557% / 79.171% / 10 | 12.310% / 56.088% / 5 | 38.092% / 315.955% / 5 |
| Excluding 66:66 tail | 9 | 32.934% / 70.979% / 9 | 7.445% / 26.010% / 4 | 7.218% / 25.968% / 4 |

The global method improved absolute residual over LCIO in 9/10 cases but
improved over the existing GSF in only 4/10. Excluding the catastrophic event
makes its selected-panel mean slightly better than GSF, but that exclusion is
not a valid performance claim. Tail safety dominates.

## Mechanism conclusions

The global all-hit concept is mechanically capable of using downstream
measurements to recover a large loss:

- 19:4 selects the exact interval and nearly exact retained fraction, moving
  from -31.98% to -1.52%.
- 66:22 also selects the exact interval and a close retained fraction, ending
  at -0.15%.
- 66:29 selects the exact interval and ends at +0.43%.

However, consuming all inward hits does not solve the branch decision:

1. **Loss location remains ambiguous.** Five of ten hard mid-tracker events
   select the immediately adjacent wrong interval.
2. **Correct location is insufficient.** In 21:65 the exact interval wins but
   `z=0.896` badly underestimates the truth loss (`z=0.672`), leaving a
   -25.97% residual.
3. **Hard losses can still look like identity.** Event 21:93 retains identity
   and remains -15.35%.
4. **The method can create a catastrophic new tail.** Event 66:66 has truth
   interval 5 and `z=0.2077`; the method selects interval 4 and profiles
   `z` to the configured lower boundary 0.05, publishing 169.23 GeV for a
   40.68 GeV truth track (+315.96%).
5. **The evidence gate cannot repair that tail.** The selected history in
   66:66 has marginalized log Bayes factor +113.3. Raising the clean gate by a
   modest amount would not reject it and would be tuning the wrong mechanism.
6. **The BH prior alone is not the only issue.** Both correct and wrong
   histories often select the hard-loss mode 4; one correct-interval failure
   selects mode 3. The all-hit likelihood and its covariance/interval timing
   must be decomposed before retuning the model.

This selected study does not validate the algorithm or justify integration.
It establishes that the global optimization proposal transfers the existing
local branch ambiguity into a discrete global-history ambiguity, with an
additional dangerous continuous-profile boundary.

## Ordered next diagnostic

Keep source and all defaults frozen initially. The primary diagnostic event is
now topology-clear file 66, entry 66, because it exposes the largest
truth-blind failure. Compare through the identical inward hit sequence:

```text
identity
truth-compatible interval 5 at z=0.207746
best profiled interval-5 history
selected interval 4, mode 4 at z=0.05
```

At every accepted measurement, record separately:

- `deltaChi2`;
- `logDetInnovation`;
- `-0.5 * (deltaChi2 + logDetInnovation)`;
- cumulative measurement log likelihood;
- discrete BH/history prior kept separate;
- residual and innovation covariance;
- surface/hit identity and detector region.

Locate the first hit where interval 4 overtakes truth-compatible interval 5.
Also inspect the profile-likelihood shape between `z=0.05` and the truth
region to determine why the published profile lands on the lower bound while
the marginalized interval/mode evidence is strongly positive.

Use these matched mechanism controls:

- 19:4: correct interval and correct retained fraction;
- 66:22: correct interval and close retained fraction;
- 21:43: adjacent interval with substantial over-correction;
- 21:65: correct interval but wrong retained fraction;
- 21:93: hard loss retained as identity;
- original 822751:3: adjacent interval control in the secondary-activity
  population, reported separately.

Classify the first divergence as state transport/residual, innovation
covariance/log determinant, discrete BH response/prior, interval placement, or
post-selection profiling. Do not add multi-loss histories, tune the Bayes
gate or retained-fraction floor, or integrate the refitter until this closure
is complete. After a same-code correction is predicted, apply focused verbose
checks, hard events 11/16/17, no-eBrem controls, and held-out population gates.
