# Charged-ECAL energy measurement in free-loss RTS (2026-10-19)

Historical first version. Its last-tracker-state update was replaced by the
explicit ECAL-face state and terminal RTS edge recorded in
`2026-10-19-explicit-charged-ecal-face-rts.md`.

## Outgoing focus, preserved verbatim

Active work on local `breakpoint` is the optional ECAL-guided free-loss fit.
`FreeLossEcal` is installed and default-on in the maintained breakpoint card
(compiled default off). It adds accepted hit-supported
neutral energy in a parallel Persistent6D scalar free-loss search for one
selected breakpoint; it does not add an ECAL Kalman hit. Tracker-only
`free_loss_*` endpoints remain in the same tuple, while `free_loss_ecal_*`
holds the ECAL-guided pair or a tracker-only copy when ineligible. The on/off
no-neutral fallback and one focused ECAL recovery reproduce the isolated prototype.
Selected neutral misassociations can cause severe momentum overshoots, so the
default-on study steering is not a physics-validation claim. All backward
filters remain default-off. Mechanics and
tests are in `agents_record/2026-10-19-free-loss-ecal-promotion.md`.

Next: evaluate association quality and held-out categorized performance before
considering broader use. The prior layer-owned identification focus, including
its pending tuple regeneration and seed-separated training, is preserved in
that record and `agents_record/2026-10-18-identification-layer-owned-truth.md`.

## New implementation and gate

`FreeLossChargedEcalMeasurement` is compiled and card default-off. For one
charged CyberPFOPID sharing the input track, the code sums linked,
hit-supported `EcalCluster` energy with provisional per-cluster resolution.
The ordinary free-loss Persistent6D trial propagates its outermost tracker
state to the ECAL face, performs a scalar energy Kalman update there (no
cluster position residual), then RTS-smooths inward. Its Minuit objective is
the tracker marginal likelihood plus the conditional ECAL-energy likelihood.
The neutral-ECAL branch and ordinary RTS are unchanged; inactive parallel
copy branches follow the base free-loss endpoint as before. The flat tuple
records candidate status, measured energy/error, prediction, innovation
variance and chi2. Source/configuration details are in the package README.

Same-input local tests on `sim_large_barrel_20261018/rec-barrel-1.root`:

- Events 3, 5, 25 each had one linked charged ECAL cluster. OFF after the
  change reproduced **all 593 pre-existing branches exactly** against the
  pre-change installed run, with NaN pairs treated as equal. This covered
  their free-loss pT values 15.1533159536, 21.9499186147, 49.8542038001
  GeV.
- ON converged for all three. Their free-loss RTS pT values were
  15.0955505016, 22.0676743748, 49.8453330732 GeV and ECAL innovation
  chi2 values were 3.063, 0.930, 0.600. These are execution tests, not
  evidence of population improvement.
- Event 122, track 1 had no linked charged ECAL cluster. Its ON result exactly
  matched the same-code OFF free-loss pT (22.1987394488 GeV), optimized prior
  center and normalized likelihood.
- Build and six configured `RecBreakpoint` CTests passed. The focused
  batch-control test passed. The full Python batch suite had one unrelated
  existing wrapper expectation failure: its default-stage case requested an
  absent external `rec-barrel-12.root` and expected success.
- The final package-only installed plugin hash matched the built plugin;
  one ON event ran successfully through the installed `gaudirun.py` runtime.
  Installed OFF event 3 again matched all 593 old branches.

No charged-ECAL resolution or independence calibration has been performed.
CyberPFA may have used the input track momentum when preparing the charged
cluster. The native ECAL extrapolation supplies only a covariance, not its
cross covariance with the last tracker state; the first version estimates a
nonnegative propagation-noise remainder. Keep this study option OFF until
those limitations and held-out clean/hard-loss behavior are assessed.
