# Default-KL native smoother kappa scan (2026-07-13)

## Requested constraint

The user required a smoother covariance scan on events with poor reverse-filter
performance while preserving the current default filter and reducer settings.
The earlier temporary TopN scan was explicitly rejected and is not evidence for
this study.

`RetainedLineageSmoothing` cannot run with default KL reduction because a
moment-merged component no longer represents one discrete process lineage. A
separate default-off diagnostic, `NativeTrackSmoothing`, was therefore added.
It invokes KalTest `SmoothAll()` on the components produced by the unchanged
default forward filter and KL reducer. No production default changed. Focused
runs set only:

- `ReverseFiltering=False`, because smoothing and reverse refitting are
  alternative backward workflows;
- `NativeTrackSmoothing=True`;
- the scanned `KappaSeedCov` value.

All BH, material, component-count, cutoff, and KL reduction settings remained
at their current defaults. The native smoother is a diagnostic on the
representative KalTest history retained through KL moment merging; it is not a
statistically exact cross-lineage Gaussian-sum smoother.

## Events and results

The topology-clean light-eBrem misses were 433/6 (dominant loss transition 0),
302/9 (transition 4), and 342/8 (transition 3). The scan covered the same
curvature-variance range that previously helped a different TopN hard-loss
event.

| KappaSeedCov | 433/6 pT [GeV] | 302/9 pT [GeV] | 342/8 pT [GeV] |
|---:|---:|---:|---:|
| 1e-7 | 1.922566 | 1.912560 | 1.899755 |
| 1e-5 | 1.922566 | 1.912560 | 1.899755 |
| 1e-4 | 1.922566 | 1.912560 | 1.899755 |
| 3e-4 | 1.922566 | 1.912560 | 1.899755 |
| 1e-3 | 1.922566 | 1.912560 | 1.899755 |
| 1e-2 | 1.922566 | 1.912560 | 1.899755 |

Truth pT is 2.000359 GeV. The nominal LCIO pT values are approximately 1.9220,
1.9127, and 1.9034 GeV. Every one of the 18 jobs terminated successfully; the
focused tracks retained 232, 233, and 233 hits. The verbose nominal 302/9 run
accepted all 2,321 component updates and produced finite IP parameters.

## Conclusion

Under the requested current default KL setup, native smoothing is insensitive
to seed curvature variance over five orders of magnitude and does not recover
these inward light-eBrem events. The old covariance-dependent recovery table
used retained-lineage RTS plus TopN on hard event 1/3 and is not transferable
to this setup. No default should be changed from this result.

Focused outputs are under `/tmp/gsf-native-kl-covscan-inner`; the comprehensive
nominal log is `/tmp/gsf-native-kl-302-9.log`.
