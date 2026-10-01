# Outgoing RecBreakpoint focus before tuple cleanup

Exact pre-edit `AGENTS.md` current-focus section, retained for provenance:

## 2. Current focus

Active work is RecBreakpoint on local breakpoint. The user now requires the
free-loss pair to use the SAME SigmaLogLoss as ordinary and truth-centred
fits. Every Minuit trial and final refit retains that positive Gaussian width.
Minuit optimizes the loss PRIOR CENTER mu; the shared KF/RTS/backward fitter
then updates b and its variance from measurements. SigmaLogLoss is the prior
sigma, not the fitted posterior sigma or Minuit's error on mu. The latter is
not added to track covariance. No new configurable property or separate
filter was introduced. Free fitting still supports one LocalMarginal interval.
Its objective remains complete RTS chi2 + log det S_all + M log(2*pi), now
including the loss-prior uncertainty in the captured birth process covariance.

Both numerical tests and 28 card/batch tests pass (29 explicit properties).
Ten direct before/after rows reproduce all ordinary/truth fields and 18,310
verbose state/covariance records exactly. Four native Gaussian refits at the
optimized centers reproduce the free pair exactly, including 4,650 verbose
records. Widths 0.001 and 0.05, off/empty copies and unsupported-mode KF
fallback were checked. 12:17 is a separate secondary-activity control.
This is a mechanical gate, not a physics-performance claim. The user confirmed
no active jobs and explicitly authorized installation after validation;
only RecBreakpoint was installed. No remote operation was performed.
Contract, schema migration, numerical examples and deployment evidence:
agents_record/2026-09-30-free-loss-shared-sigma.md.

The primary Unconstrained/Fixed workflows and LossPriorMode remain removed;
Gaussian LocalMarginal and Persistent6D remain. No reference iterations,
shared KF/GSF changes, or modification of existing outputs/prepared cards.
Historical tuples without free_loss_treatment="PriorCenter" retain the old
fixed-b free-fit meaning. Current tuples distinguish the optimized prior center,
configured sigma and fitted loss/variance. Existing cards inherit the installed
algorithm change while retaining their frozen SigmaLogLoss; regenerate cards
that still assign the retired LossPriorMode property.

Completed fixed 2,000-event seeds21--40 study: 1,677 clean paired tracks,
34 interval-association failures among 1,711 single-track candidates, and
237 secondary-activity events reported separately. One optimizer KF fallback
is included. Free-loss narrows inclusive width68 (RTS .341% vs KF .861%) but
worsens RMS (12.563% vs 6.759%) and creates eight >100% positive outliers.
It improves >=1% loss central recovery but significantly damages (0,1%) loss
tracks (within +/-1% falls from 97.0% to 82.9%). Both fixed seed blocks agree;
seed-cluster bootstrap supports these category and RMS differences. It is not
an overall safe replacement. Current Gaussian shared-width free-loss is not
the proposed no-prior augmented KF. No source/card/runtime or remote changed.
Next: discuss these findings; inspect extreme/light-loss failures before any
new objective/prior/default changes. No such changes are authorized yet.
Full counts, numerical results and provenance: agents_record/2026-09-30-free-loss-population.md;
outgoing status: agents_record/2026-09-30-agents-before-free-loss-population.md.
Historical no-prior response discrepancies remain unresolved in
agents_record/2026-09-13-unconstrained-breakpoint-loss.md.
The outgoing focus and full retired fixed-b contract are archived under
agents_record/2026-09-30-agents-before-free-loss-shared-sigma.md and
agents_record/2026-09-30-retired-fixed-free-loss-contract.md. Maintained formulas
and workflow are in Reconstruction/RecBreakpoint/docs/free-loss-fit.md and
docs/smoothed-objective.md under that package.
