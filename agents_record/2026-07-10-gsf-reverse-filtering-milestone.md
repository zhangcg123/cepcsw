# GSF reverse-filtering milestone

Date: 2026-07-10

This record preserves completion of the focused tracking-workflow roadmap:
navigation auditing, surface-boundary ownership, reverse multi-component
filtering, and interaction-point output.

## Navigation and boundary semantics

Every matched hit now carries its original association order, KalTest surface
index, sorting policy, cell ID, and position. Focused verbose audits report
surface repetition, input/surface monotonicity, consecutive-step direction
reversals, and minimum step cosine.

For events 11, 16, and 17, the original `CompleteTracks` order equals the
radius order, no surface repeats, no geometric direction reversal occurs, and
all consecutive-step cosines are positive. Event 11 is not monotonic in KalTest
surface index (`5932 -> 5334` along the physical hit path), proving that layer
index is not a universal navigation key. The focused reverse pass therefore
uses the audited forward hit sequence in reverse. This validates the selected
topology; it does not establish general navigation for curling, displaced, or
repeated-crossing tracks.

The final forward measurement surface owns no outgoing process transition:
its filtered mixture is preserved as the reverse-pass starting mixture. This
avoids applying material that no forward propagation consumes.

## Reverse workflow

When `ReverseFiltering=True`, the algorithm:

```text
filtered final-surface mixture
  -> reverse traversal of preceding measurements
  -> exact MarlinTrk prediction/update at each surface
  -> full det(S)-normalized posterior likelihood
  -> direction-reversed process convolution
  -> low-weight cutoff and current-surface KL reduction
  -> innermost continuation mixture
  -> geometric continuation to the IP
  -> publish highest-weight reverse branch with its own chi2/NDF
```

The existing BH mixture is reused. For reverse traversal, retained momentum
fraction `z` maps `p_current` to `p_previous = p_current/z`, equivalently
`kappa_previous = z*kappa_current`; its covariance transformation follows the
direction-aware inverse-q/p form. The published track state and fit-quality
metadata both come from the selected reverse component.

`ReverseFiltering` remains opt-in with default `false` while material/BH physics
and broader topology coverage remain unvalidated.

## Focused validation

All three focused reverse runs retain all 234 measurements, reject no reverse
updates, and produce finite IP states:

| event | truth pT [GeV] | LCIO pT [GeV] | forward GSF pT [GeV] | reverse GSF pT [GeV] | best weight |
|---:|---:|---:|---:|---:|---:|
| 11 | 2.0004 | 1.7934 | 1.7933 | 1.9785 | 0.3843 |
| 16 | 2.0004 | 1.8118 | 1.8118 | 1.9970 | 0.5926 |
| 17 | 2.0004 | 1.5790 | 1.5789 | 2.2591 | 0.5923 |

Event 11 performs 1,442 accepted reverse updates with four splits and four
reductions; event 16 performs 996 accepted updates with three splits and three
reductions; event 17 performs 2,340 accepted updates with three splits and
three reductions. Each has zero reverse rejection.

The reverse IP state is closer to generator pT than LCIO in all three focused
events. Event 17 overshoots truth, but the user explicitly decided that this is
not an immediate blocker. These results validate completion of the focused
tracking-workflow milestone, not the current material estimate, BH model, broad
physics performance, or arbitrary-track navigation.

## Remaining project work

The tracking workflow TODOs through focused reverse-IP validation are complete.
The full project roadmap is not complete. Remaining work is:

1. compute component-local, incidence-path-corrected `t/X0` with explicit CEPC
   surface material ownership;
2. fit and validate a CEPC step-`t/X0`-conditioned BH mixture from primary
   electron tracker-volume Geant4 eBrem truth;
3. tune the retained component count toward 4-5 with cutoff/current-surface KL
   reduction and reassess whether component age is necessary;
4. validate navigation beyond the focused monotonic topology before claiming
   general tracking support;
5. only then run broad GSF-versus-LCIO studies and consider production use.
