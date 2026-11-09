# ACTS BH audit — 2026-11-10

Local CEPCSW HEAD: `6e8ac8091cf2bb137c2dc627b255215643b91f43`.
Upstream ACTS main pinned for this audit:
`04770a4bce017d7f109f68514070ea7a2fd7132e`.

Official source was read over HTTPS into memory. No ACTS checkout or source
files were downloaded into CEPCSW. This temporary directory contains our own
audit script, numeric reference coefficients and results only.

## Mixture implementation

Compared upstream `Core/src/TrackFitting/BetheHeitlerApprox.cpp` and
`Core/include/Acts/TrackFitting/BetheHeitlerApprox.hpp` with local
`Reconstruction/RecGsfTracking/src/BetheHeitlerSplitter.cpp`.

- All 108 polynomial coefficients agree exactly as doubles.
- Highest-order-first Horner evaluation, inverse-logit weight/mean,
  exponential variance, and normalized weights agree.
- Both defaults use one six-component coefficient table from 0 to 0.2 X0.
  ACTS general documentation describing distinct low/high ranges does not
  describe this particular default factory.
- Both use identity below 0.0001 X0 and a single Gaussian below 0.002 X0.
  Local exponential moment formulas are algebraically equivalent to ACTS's
  power formulas.
- Both mixture functions return the 0.2-X0 response above their upper range.
  ACTS separately reports invalid thickness through its validity method,
  counter and debug log; the local splitter has no equivalent range check.

Loaded the actually installed `libRecGsfTracking` and called
`BetheHeitlerSplitter("ActsAtlas").mixture(t)` at 10,022 finite nonnegative
thicknesses, including immediate floating-point neighbors of each regime
boundary, a uniform grid through 0.3 X0 and the measured endcap examples.
Compared every returned weight, mean and variance to a separately evaluated
reference using the official coefficients. Maximum absolute differences:
weight 0; mean 0; variance 1.1102230246251565e-16. The six-component regime
agrees exactly. Maximum weight normalization error is 2.220446049250313e-16.
The minimum component mean over this scan is 0.4003130633771537, so the local
0.01 mean floor is inactive for the default ACTS mixture on this domain.

## Track application is a local adaptation

Compared upstream `Core/src/TrackFitting/GsfUtils.cpp::applyBetheHeitler`
against local `applyMixture`.

For component mean m, variance v and incoming curvature k:

| Operation | CEPCSW | ACTS |
| --- | --- | --- |
| Forward mean curvature | k/m | equivalent q/p divided by m |
| Reverse mean curvature | k*m | equivalent q/p multiplied by m |
| Forward added curvature variance | k^2*v/m^4 | equivalent k^2*v/m^2 |
| Reverse added curvature variance | k^2*v | equivalent k^2*v |
| Existing covariance | curvature row/column scaled by deterministic Jacobian | BH helper copies existing covariance and adds q/p variance |

Curvature/q-over-p equivalence here assumes unchanged direction and the
corresponding unit/field conversion. It is a formula comparison, not a full
ACTS fit execution. The local rule is the documented first-order Jacobian
approximation, introduced in the August development; the discrepancy alone
does not establish that it is a bug. Full fitted uncertainties and measurement
weights cannot be described as identical to ACTS. Upstream also skips tiny
means and applies its component weight cutoff before creating children.

## Approximation limitations inherited from the default

These numbers use untruncated Gaussian-mixture moments, matching the model's
representation. Ideal BH mean is exp(-t), not a Geant4 validation target proved
for CEPC.

| Actual t/X0 | Mixture mean retained fraction | Ideal BH mean |
| --- | --- | --- |
| 0.001 | 0.999000499833375 | 0.999000499833375 |
| 0.002 | 0.9804202553475206 | 0.9980019986673331 |
| 0.01 | 0.9761162404382974 | 0.9900498337491681 |
| 0.1 | 0.9045745520865143 | 0.9048374180359595 |
| 0.17374897676 | 0.8403041787061671 | 0.8405078581851333 |
| 0.2 | 0.8192874609172365 | 0.8187307530779818 |
| 0.27206206592 | 0.8192874609172365 | 0.761806977371586 |

There is a real default-model discontinuity at 0.002 X0: the mean changes
from approximately 0.998002 just below the boundary to 0.980420 at it. This
exists in the official default and is not a local transcription error.
The 0.272062-X0 endcap interval is clipped and outside the polynomial range.

No source, steering, library, installation or REC files were changed. No ACTS
full fit or new CEPC physics population validation was performed.
