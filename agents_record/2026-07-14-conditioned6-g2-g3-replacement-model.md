# Six-component conditioned BH model replacing g2/g3

Date: 2026-07-14

## Request and interpretation

The user requested a new BH model based on the current conditioned model, with
three components replacing current g2 and g3. The implemented interpretation
keeps the current no-eBrem, 0--1%, and >20% components and replaces the old
1--5% plus 5--20% pair with three fixed truth strata: 1--5%, 5--10%, and
10--20%. The resulting model has six components and is named
`CEPC2GeV85StepConditioned6`. The current five-component model remains the
default and is unchanged.

## Extraction and artifact

The existing authoritative ten-file Geant4 transition sample was re-extracted
with `--loss-source ebrem --split-light-large`. It accepts 2,573,914 of
2,574,697 transitions, including 9,528 positive-eBrem rows, and excludes the
same 783 transitions beyond the 0.03-X0 range as the current model. It uses the
same eight t/X0 knots and interpolation rules.

Artifacts are under
`Reconstruction/RecGsfTracking/data/CEPC2GeV85StepConditioned6/`. The dense
artifact validation finds maximum weight-normalization error `5.55e-16` and
tail probability `1.57e-11` at `t/X0=1e-12`. At each knot, the weights of the
new 1--5%, 5--10%, and 10--20% modes sum to the weights of current g2+g3, so
the change redistributes support without changing total probability in the
replaced range.

The artifact fitter now accepts `--model-name` and `--output-stem`, allowing
parallel conditioned artifacts without overwriting or mislabelling the
default artifact.

## Implementation and validation

`BetheHeitlerSplitter` exposes the new model by exact string
`CEPC2GeV85StepConditioned6`. A templated conditioned interpolation path is
shared by the five- and six-component tables. The package builds and installs.

Comprehensive default-24 reverse event 284/1 validation:

- first material split: six children with modes g0--g5;
- retained hits: 232/232;
- reverse accepted/rejected: 5542/0;
- final reverse components: 24;
- truth/LCIO/new-model pT: 2.0004/1.9837/2.0247 GeV;
- current five-component baseline pT: 2.0275 GeV.

Log: `/tmp/gsf-conditioned6-284-1/seed-284.log`.

The available legacy `/tmp/gsf-match-tracks.root` yielded focused hard event
11 but did not contain requested events 16 and 17 before end-of-input. Event
11 completed with pT 1.9794 GeV (truth 2.0004, LCIO 1.7938), 22 final reverse
components, and 15 rejected reverse updates. The identical-input current-model
baseline gives pT 1.9830 GeV, 19 final components, and the same 15 rejected
updates. Thus the new model slightly weakens this event's recovery by 0.0036
GeV but does not introduce its rejection behavior. Logs:
`/tmp/gsf-conditioned6-hard-11-16-17.log` and
`/tmp/gsf-conditioned5-hard-11.log`.

No physics-performance conclusion is made. Next validation must use the full
19 overshoots, matched controls, ordinary light/clean/hard ladder, recover the
proper legacy 16/17 input, and only then proceed to full categories.

## First five-event overshoot comparison

Five deliberately stratified events from the user's exact MaxComponents=12,
transition-7--8, +0.5%--2% overshoot list were rerun with comprehensive dumps
under identical settings for the current and six-component models. Relative
to truth, old/new residuals are:

- 340/5: +1.8887% / +1.7467% (improves by 0.1420 point);
- 41/5: +1.4098% / +1.3568% (improves by 0.0530 point);
- 53/4: +1.8922% / +1.8957% (worsens by 0.0035 point);
- 26/9: +1.2038% / +1.3108% (worsens by 0.1070 point);
- 187/4: +1.1193% / +0.8294% (improves by 0.2899 point).

Thus 3/5 improve and 2/5 worsen, with mean absolute residual falling from
1.5028% to 1.4279%. LCIO remains closer to truth for low-loss 340/5, while
both GSF models substantially recover the large LCIO deficits in 26/9 and
187/4. Reverse rejection counts do not show a uniform regression: old/new are
5/6, 0/0, 0/0, 7/6, and 12/12. This small selected test is mixed and cannot
support adoption; it motivates the complete 50-event same-code comparison.

Table:
`TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/conditioned6_overshoot5_comparison.csv`.
Logs: `/tmp/gsf-old-overshoot5-valid/` and
`/tmp/gsf-new-overshoot5-valid/`.

## Complete overshoot-50 same-code A/B

All 50 events in the user's exact MaxComponents=12, transition-7--8,
+0.5%--2% overshoot table were subsequently rerun with both models using
AggregateWeight and comprehensive dumps. Both models completed 50/50 events
over the same 48 seed files.

The six-component model improves 28 events, worsens 21, and leaves one
unchanged. Despite that majority, its error distribution is not better:

- mean absolute residual: 1.0458% old, 1.0638% new;
- median absolute residual: 0.9291% old, 0.9256% new;
- residual RMS: 1.1348% old, 1.1976% new;
- events inside 1%: 28 old, 28 new;
- total reverse rejections: 261 old, 265 new.

Two newly enlarged positive tails dominate the mean/RMS regression. Seed
345/event 7 changes from +0.6729% to +2.6146%, and seed 102/event 4 changes
from +1.1587% to +2.9271%. The largest improvement is 320/4, +1.9702% to
+1.1373%, followed by 187/4, +1.1193% to +0.8294%.

Stratification is also mixed. Transition 8 improves on average (mean absolute
residual 0.9357% to 0.8958%), while transition 7 worsens (1.1475% to 1.2189%).
Losses below 1% improve from 1.3044% to 1.2195%, losses at least 5% improve
from 1.0614% to 0.9889%, but the dominant 1--5% group worsens from 0.8913% to
1.0000%.

This selected sample was constructed from old-model overshoots and therefore
is already favorable to finding replacements. Even so, the new model leaves
the 1% count unchanged and worsens mean error and RMS through new tails. It is
rejected as a replacement on this test and remains default-off. Full table and
machine-readable summary:

- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/conditioned6_overshoot50_comparison.csv`
- `TrackingPerformanceStudies/lcio_track_resolution_2p0_theta85/surveys/topology_clean_2026-07-13/conditioned6_overshoot50_summary.json`

Comprehensive logs are `/tmp/gsf-old-overshoot50-valid/` and
`/tmp/gsf-new-overshoot50-valid/`. The reproducible comparison parser is
`Reconstruction/RecGsfTracking/scripts/compare_bh_model_event_list.py`.
