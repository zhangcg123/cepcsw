# Per-track Q distributions for the two available BP110 hypotheses

The user requested Q distributions in each radiation category. Define
Q = minimized normalized -2lnL(H_beta) - minimized normalized -2lnL(H_betaF),
using the saved ordinary free-loss BP110 and floating-FSR BP110 scores.
Positive Q favors floating FSR. The new H0 reference is not part of these
figures; its separate normal-flat campaign is prepared, not submitted here.

Four individual category plots and an updated overlay are in
`TrackingPerformanceStudies/bp_fsr_q_categories_20261113/`, with source script,
README, histogram CSV, input/plot hashes and summary. Data reuse the checked
per-track score cache. All500 native BP source file identities are unchanged;
unique keys, score subtraction and own-track truth category labels pass.

The126828 comparable tracks comprise Neither65985, eBrem-only41209,
FSR-only11856 and Both7778. Categories use THIS track's summed photon birth
energies strictly>1GeV; Both is AND on the same track. Sub-threshold radiation
is allowed. The original67950-event common valid-fit/GSF selection is inherited;
these are not all production tracks. Independent score/observation gates apply
to each track, with no recoil-window cut or partner-score veto.

All finite positive and negative Q tails are retained. Four category figures
and their overlay use identical symlog x axes and variable-width bins, with
one central[-0.1,0.1] bin for tiny differences. Histograms are independently
normalized using full category counts and show fraction per bin, not density.
All four histogram counts close and their fractions sum to one. A rerender
is idempotent; all input/PNG hashes pass. All five figures were visually reviewed.

The gallery previously had42PNGs/12sections. Four category PNGs are added and
the existing score-difference overlay is updated to label Q explicitly;
49other pre-existing PNG/PDF files stay unchanged. Raw-score, efficiency and
recoil plots remain. The gallery manifest places these in the existing score
section; verified count46PNGs/12sections with exact-once coverage. The existing
ailogin002 PID443262 server remains on localhost8000 with no restart. The page
and all five Q PNGs return HTTP200 with served/local/source hashes identical.

No refits, builds, installation changes, new production submission, theoretical
chi-square reference, significance calibration or selection threshold. Negative
Q outliers and the approximate-likelihood limitations remain unresolved.
