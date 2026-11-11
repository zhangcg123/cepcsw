# Inclusive bare Baseline KF recoil at 10 ab^-1

User requested a clean gallery with one inclusive120--130GeV recoil plot,
only muon and electron Baseline KF, normalized to10ab^-1, with no plot title
or long annotations. The figure has two curves, axis labels and a legend;
the luminosity appears only as the legend heading. Bin width0.1GeV.
Electron is gray filled; muon is red. No photons or neutral clusters are added.

Electron uses all498 completed BP-source caches,99600 generated inputs,
without requiring any BP or GSF fit to succeed. Selected production-electron
tracks must form a valid opposite-charge Baseline KF pair. Both states use
their own CompleteTracks pT/phi/tanLambda. No FSR/eBrem category cut is made.
This replaces the former85,327-pair GSF-valid electron cohort for this plot.
There are97,910 electron pairs,29 undefined recoil masses;42,631 in120--130GeV.
Within events represented in the track cache,1,549 lack an opposite-truth-PDG
partner and134 have same-sign Baseline KF curvature. Seven generated events
have no cached selected tracks. All99,600 inputs enter the normalization.

Muon reuses the previously verified cached48,636 Baseline KF pairs from
50,000 generated inputs. Nonpositive recoil energy remains undefined;
11 masses are undefined and36,918 are inside the displayed window.
No fit or ROOT read is performed for this figure.

Weights use generated inputs before ALL selections, never selected/window
counts: electron10000*7.04/99600=0.7068273092369478;
muon10000*6.77/50000=1.354. Expected window counts are30,132.755 electron
and49,986.972 muon. No efficiency or window/area renormalization occurs.
Cross sections are the same published reference values used in the previous
study, not exact production-header rates. Reference:
https://hepnp.ihep.ac.cn/article/doi/10.1088/1674-1137/abb4d8 (Table2).

Muon Higgs decays were forced invisible; electron eeHX has inclusive Higgs
decays. The muon normalization uses inclusive-production Higgs factor1 as a
tracking reference, not a physical SM invisible yield or validated
inclusive-decay efficiency prediction. Original muon REC files are missing;
only the validated cache is used. No backgrounds or peak fits are included.
These caveats stay in notes rather than long text on the graph.

Checks: source-cache SHA256/provenance, all-generated normalization closure,
unique selected event keys, pair-rejection accounting, independent histogram
in-window closure, exactly two legend curves and no title/text annotations.
The final layout was visually inspected. Files: analyze.py, analysis.log,
summary.json, electron_pairs.csv, histogram.csv, PLOTS_READY and
recoil_muon_electron_baseline_kf_10ab.png/.pdf. No native fit/card/library/job
changes. Existing served plots are archived outside the served directory,
with hashes, before this becomes the sole displayed figure. Analysis caches
and source-study originals remain available.

The outgoing AGENTS focus is preserved verbatim in
[the snapshot](2026-11-11-agents-before-kf-only-gallery.md).
All text before Current focus, including global laws and compile/run commands,
is byte-identical. No history directory is moved or renamed.

## Gallery replacement and square-grid follow-up

The plot-gallery agent moved all32 previous displayed PNGs,32 matching PDFs,
old gallery.json and README (66files) into
`.analysis_archive/gallery_before_kf_muon_electron_only_20261111T125855Z/`.
inventory.json records original sizes/SHA256s. Agent and parent independently
verified every archived file; no source CSV/script/cache/ROOT file was moved.
The served directory contains only the new PNG/PDF and a one-section manifest.

Before completion the user requested a square canvas and transparent grid.
The figure was rerendered7x7in/1260x1260pixels with grid alpha0.2 and line
width0.6, beneath data. No title or text annotations were added. Both curve
counts, generated-input weights and expected yields remain identical. Final
square layout inspected by parent. The archived preceding figures are untouched.

Earlier square-grid version SHA256 (superseded by the styling below): PNG
`1d0ebd96fc8664ef22473c93f7a29e1b13e3de832e87adbcec4c53271e58a2e5`;
PDF`e6817e8a37aa2bf79f1c85a468fd7a92f459481a4a12532eb422388e0f16214a`.
The existing ailogin002.ihep.ac.cn server PID2122174 serves the same directory
on http://127.0.0.1:8000/. Gallery has one PNG, one category, exact-once coverage.

Latest user styling moves10ab^-1 to the top-left inside the axes, removes
the legend luminosity heading, changes the muon reference to solid black,
and sets tight-layout padding to zero. Electron remains gray filled; square
1260x1260canvas and faint alpha0.2 grid remain. Only the short luminosity
label is annotated; there is no title or long plot text. All counts, weights
and expected yields are unchanged. Parent visually inspected the final plot.
Final PNG SHA256:
`ba7432622fe14457a0824bd0e8dfb1bb91020f92cea06eadd1545028d6ec350b`;
PDF`3a67c7b216a7316509017f2b060d98f4a88ed7daf27ea6e9b0fe877d7a1eb3fb`.

## DSCB fits and larger tick labels

User requested larger tick labels and fitting both peaks with DSCB. New
fit_dscb.py reuses the unchanged existing ROOT RooCrystalBall helper in
higgs_muon_recoil_reference_20261102/fit_crystal_ball.py. Both channels use
one Gaussian core sigma and independent tails, a123.5--128GeV fit window,
raw-count bin-integrated likelihood, three starts and Minuit2/migrad strategy2.
Every trial has status0/covariance quality3 with consistent minima; no fitted
parameter is near a configured bound. No new dependency or parallel DSCB
implementation is introduced. Model definition:
https://root.cern.ch/doc/master/classRooCrystalBall.html .

Muon mean125.07425+/-0.00318GeV, sigma0.27266+/-0.00361GeV,
Poisson deviance47.472/38ndf. Electron mean125.17035+/-0.00793GeV,
sigma0.29941+/-0.00644GeV, deviance45.134/38ndf. Errors are MC statistical.
Right-tail powers0.964muon/0.641electron are allowed by finite-range
normalization; there is no whole-line extrapolation. Core sigma is not a
full-spectrum RMS/sigma68 or pure detector resolution. Fit-range/systematic
variations have not been evaluated.

Underlying100 histogram bins, source caches, generated-input weights and
yields are unchanged. Each fit's integral over the fit window equals the
raw fit-window count times that channel's fixed10ab^-1 weight. Curves use
the0.1GeV bin width to overlay expected events/bin. Helper checks exact
bin-integral/yield closure. New curve CSV, summary and fit log preserve all
parameters, covariance matrices and trial diagnostics; dscb_fit_summary.json
is authoritative for current plot hashes while source summary.json remains
unchanged as pre-fit provenance.

Ticks and axis labels are14pt. Existing square1260canvas, top-left luminosity,
black solid muon reference, gray-filled electron, alpha0.2grid and zero outer
padding are preserved. Dashed red/blue curves show muon/electron fits; the
four-entry legend has no numerical/long annotations. Parent inspected layout.
Final PNG`45efbe4d03d041f07943ba8ed9b27ec64d86a201102a91919a497e193a588e9f`;
PDF`17117ad802ac686dab4ff51500a674e867c5dff9130da946990671884b00c928`.

Independent numeric integration of the700-point plotted curves reproduces
the fit-window expected yields within1.3e-7relative (tolerance2e-5); both
saved covariance matrices have positive eigenvalues. All input histogram,
source-summary and maintained-fitter hashes remain unchanged; output PNG
dimensions remain1260x1260. No fit-range or binning-variation claim is made.

## Fit-summary labels and larger typography

The user removed the two DSCB legend entries while retaining the dashed fit
curves, and requested sigma and window yields printed on the graph. Legend
now contains only electron/muon Baseline KF. Compact notes show muon core
sigma0.273+/-0.004GeV andN(120--130GeV)=49,987; electron0.299+/-0.006GeV
andN=30,133. These are rounded actual histogram expected yields at10ab^-1,
not raw MC counts or fitted123.5--128GeV normalization yields.

Latest steering enlarges axis titles to18pt,ticks16pt,legend14pt,luminosity
16pt and fit notes14pt. Each channel note uses three short lines to avoid
the peak. Square canvas, faint grid, zero outer padding, top-left luminosity,
black solid muon reference and gray-filled electron remain. No title added.
Parent inspected the final layout. Repeated rendering reproduces exactly
the same fitted means/sigmas/errors and histogram yields; input/fitter hashes
and covariance/convergence gates remain unchanged.
Final PNG`de183863491b20f27ddafce482edb5fed36c9317a67a81448b28c4ccf1db2d39`;
PDF`0b4b7e3f873612c555eace28bf3c6842844c3ab3fc06f6b4436e3cf98cd4c96b`.
