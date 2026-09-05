# BH runtime model surface reduction

On 2026-09-06 the runtime `BetheHeitlerSplitter` model surface was deliberately
reduced to two selectors:

- `CEPCRuntimeCategoryAligned9Clear`, redefined as the new all-simulation
  global fit and made the compiled, active-template, and maintained-card
  default;
- `ActsAtlas`, retained as the non-CEPC control.

The redefined category-aligned selector contains ten total components: one
effective identity component for aggregate interval eBrem loss below 0.2%,
plus nine ordinary radiative Gaussians optimized over observations in the
hard likelihood range `0.2% <= loss < 100%`. The PDFs are not truncated at
those boundaries. The nine means and widths are shared across all eight t/X0
knots, while component priors are fitted independently at each knot. The
candidate used every compatible topology-clear barrel and endcap exact-hook
simulation interval available in the 2026-09-06 extraction. This is a compiled
research default, not physics validation.

The following former runtime selectors were retired rather than aliased:

- `CEPC2GeV85StepConditioned`
- `CEPC2GeV85StepConditioned6`
- `CEPCRuntimeGenericGrid5Clear`
- `CEPCRuntimeCategoryAligned5Clear`
- `CEPCRuntimeCategoryAligned15Clear`

Their designs, studies, and previous package state remain recoverable from Git
history and the dated records under `agents_record/`; callers using a retired
name now fail explicitly as an unknown BH model instead of receiving another
model silently.

The fit artifact and plots that supplied the new compiled table are under
`TrackingPerformanceStudies/bh_category_aligned_global9_radiative_all_sim_candidate_2026-09-06/`.

## Mechanical gate

The EL9/LCG-105 `RecGsfTracking` and `RecGsfFlatTuple` targets built and
installed successfully. A verbose same-code reverse run processed the first
18 entries of
`gsf_doublebhoff_freshseed_diagnostic/trk-e--2.0-85-1.root`, selecting
zero-based source events 11, 16, and 17 for component dumps. It observed all
ten compiled children (`g0` through `g9`) and wrote both EDM and flat-tuple
outputs. The corresponding one-based flat-tuple rows were 12, 17, and 18:

| source event | truth pT (GeV) | LCIO pT (GeV) | BestBranch pT (GeV) | FullMixtureMode pT (GeV) |
|---:|---:|---:|---:|---:|
| 11 | 40.73157 | 40.89545 | 40.89244 | 40.89529 |
| 16 | 37.89402 | 18.29283 | 18.28732 | 18.28732 |
| 17 | 18.79698 | 14.80667 | 18.74005 | 18.74018 |

An initialization-only run accepted `ActsAtlas`. The retired
`CEPC2GeV85StepConditioned` selector failed initialization explicitly with
`Unknown Bethe-Heitler model option`, as intended. JSON invariants passed for
eight knots, ten components per knot, and unit-normalized knot-local weights.
Shell/Python syntax checks and `git diff --check` also passed.

These checks establish packaging, steering, execution, and explicit selector
retirement only. They do not establish population performance or physics
validation; notably focused event 16 remains unrecovered.
