# Inclusive BP110 + floating FSR: absolute10ab^-1 normalization — 2026-11-13

The user requested real normalization for the inclusive plot and only the muon,
electron baseline and floating-FSR BP curve. The initial BP100 label was
explicitly clarified by the user to **BP110 + floating FSR**. BP100 keeps FSR
undetermined; no tracking configuration or implementation was changed.

Only the inclusive plot is updated. It now contains bare muon Baseline KF
(black solid), bare electron Baseline KF(gray filled) and BP110 + floating FSR
(blue solid). There is no GSF or ordinaryBP curve. Electron Baseline KF has no
neutral addition. BP uses its matching electron+FSR p4 once, as before.
Display120–140GeV/0.1GeVbins,square,large fonts,transparent grid,AGORA simulation
above axes and10ab^-1inside top-left. Legend window yields are EXPECTED counts,
not raw MC entries. No new fit, reconstruction, build/install or submission.

## Absolute weights and actual input denominator

Weight=L*sigma/N_generated BEFORE pair,fit,category or mass-window cuts.
All500electron manifests sum to100000inputs; the maintained muon reference
contains50000generated inputs. Use L=10000fb^-1, sigma_eeH=7.04fb and
sigma_mumuH=6.77fb. This gives electron weight0.704 and muon weight1.354.
The reference values were freshly checked in
[Table2 of the CEPC study](https://hepnp.ihep.ac.cn/article/doi/10.1088/1674-1137/abb4d8).
No extra Z/H branching multiplier, window normalization or fit-success
renormalization is applied. There is no radiation-category cut in this plot.

The previous common-method GSF/ordinary-BP gates are removed for the inclusive
absolute-yield comparison. Bare electron KF uses98302valid opposite-charge
production pairs. Floating110 uses all75389ofitsownvalid pairs; unavailable
fits remain inefficiency relative to100000inputs. Missing fits are not replaced
by KF. Muon uses48636reconstructed pairs from50000inputs. Electron methods
therefore have different selected populations with the SAME input weight.
This differs intentionally from the matched four-category shape diagnostics.

| Curve | Generated inputs | Selected pairs | Raw120–140 | Expected120–140 |
|---|---:|---:|---:|---:|
| Muon Baseline KF | 50000 | 48636 | 44236 | 59895.54400000001 |
| Electron Baseline KF | 100000 | 98302 | 65793 | 46318.272 |
| BP110 + floating FSR | 100000 | 75389 | 64648 | 45512.191999999995 |

## Reproduction, preservation and limitations

Artifacts remain in TrackingPerformanceStudies/bp_fsrfloat_recoil_categories_20261113/.
The added uncommitted inclusive_10ab.py reads the existing current event cache
plus validated native baseline/MC caches, avoiding any new ROOT/refit work.
It validates exact selected-track/MC joins, generated denominator closure,
no success/window renormalization, exactly three plotted curves, input hashes
and unchanged hashes for all eight category PNG/PDF files. The new inclusive
figure was visually reviewed. Current data: inclusive_10ab_summary.json,
inclusive_10ab_histogram.csv and inclusive_10ab_events.csv. Parent metadata
scopes the inclusive yield plot separately from four category shape plots.

Old inclusive shape PNG/PDF and metadata are preserved outside the served
gallery under before_inclusive_yield_normalization/. The stable inclusive
filename keeps its historical _shapes_ suffix. For full reproduction, run
analyze.py followed by inclusive_10ab.py. The four category figures, their
67950common pairs and their unit-area normalization remain unchanged.

These are established published reference cross sections, not recovered exact
generator-header cross sections. Muon Higgs decays are forced invisible and
retain the prior inclusive-tracking normalization convention, not an SM
invisible signal yield. Reconstruction/truth-selection efficiencies differ
between channels. Floating FSR has its known material/FSR ambiguity and
low-mass tails; no pure detector-resolution or physics-validation claim.

The outgoing AGENTS is preserved exactly in
[the snapshot](2026-11-13-agents-before-inclusive-10ab.md). Global laws and
commands remain unchanged. Current focus is corrected to distinguish the new
inclusive yield display from unchanged category shapes. No history directory
is moved and no migration manifest is required.
Gallery verification completed on ailogin002.ihep.ac.cn at localhost8000,
unchangedPID2122174,no restart. Only the inclusivePNG and final section
description changed. All other30PNGs and8PDFs match the pre-update hashes;
the four category figures are preserved.31PNGs/11sections/exact-once remain.
Page and image returnHTTP200;source,destination and HTTP-served SHA256 equal
6195d5d90e058279a227536bdf528d2a33bcc7c7e8b7c168094ca96dcbe4503d.
The main agent independently checked source/destination hashes after reviewing
the image. The complete publication workflow was delegated to the same
plot-gallery agent; no generic directory-listing server was started.
