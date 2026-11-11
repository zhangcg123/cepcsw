# KF/GSF recoil split by material eBrem at1% (2026-11-11)


`split_ebrem.py` adds two electron-only diagnostic plots, preserving BOTH
inclusive figures and their PNG/PDF hashes. Each diagnostic compares KF
and bare GSF on identical event keys, without a BP-success or FSR cut and
without neutral addition. The85,327 common two-leg-valid pairs split into
24,647 no+light and60,680 hard events. The category is independently checked
using each track's cached material-photon birth-energy sum divided by its
post-FSR generator electron energy: both legs<1%(includingzero), versus
at least one leg>=1%. Exactly1% belongs to hard. Muon has no electron-eBrem
category and remains only in the inclusive plots.

Both categories use the existing generated-input weight10000*7.04/99600;
there is no category, successful-fit or mass-window renormalization. The
diagnostic KF curve uses the GSF-valid common sample, while the original
inclusive KF plot retains all97,910 valid KF pairs. FSR is retained.

| Category | Raw window KF/GSF | Expected window KF/GSF | Core sigma KF/GSF [GeV] |
| --- | ---: | ---: | ---: |
| Both legs<1%, including0 | 16,392/16,252 | 11,586.313/11,487.357 | 0.28975/0.29955 |
| At least one leg>=1% | 23,192/29,667 | 16,392.739/20,969.446 | 0.31686/0.42193 |

Hard-eBrem window recovery improves27.92%, while its fitted core broadens.
The inclusive peak sigma therefore does not summarize GSF's recovery of
the radiative population. No+light yields decrease0.85%. This is a cached
selected-sample diagnostic, not held-out algorithm validation or a pure
detector-resolution measurement. Undefined KF/GSF masses are0/27 no+light
and26/726 hard; these remain in full denominators.

Same maintained DSCB fit123.5--128GeV, display120--130GeV/0.1GeV bins.
All12 fit trials have status0/covariance quality3, no boundary parameters.
Poisson deviance/38ndf is55.45/75.49(no+light KF/GSF) and39.34/41.02(hard).
The no+light GSF deviance indicates imperfect DSCB shape description;
core-width errors are statistical and fit-range/model systematics are not
evaluated. Notes are placed left of the peak in these diagnostic plots to
avoid overlap. AGORA header outside, large fonts and square layout remain.

New stems: recoil_electron_kf_gsf_ebrem_lt1pct_10ab and
recoil_electron_kf_gsf_ebrem_ge1pct_10ab, PNG/PDF. Source hashes, complete
fit diagnostics and event counts are in ebrem_split_summary.json; log is
ebrem_split.log, marker EBREM_SPLIT_READY. Source-category/identity checks,
event uniqueness, exhaustive/disjoint split, histogram closure and previous
inclusive plot hashes pass. No event ROOT or tracking refit is performed.

## Direct event migrations

window_transitions.json counts transitions on the exact same event keys.
Hard:6,922 KF-above130 events enter the GSF120--130 window, plus30 from
below120 and7 from undefined. Of23,192 KF in-window events,22,708 stay;
269 move below120,185 above130 and30 become undefined. Net gain6,475.
No+light has100 enter and240 leave, net loss140. These exact transitions
show the source of the window-yield gain without claiming a narrower core.

The full outgoing AGENTS is preserved in
[the snapshot](2026-11-11-agents-before-ebrem-split-recoil.md).
All global introduction/law/scope/compile text before Current focus is
byte-identical; no history directory is renamed or moved.

## Final QA and gallery

Parent verified all12 starts have status0/covariance quality3 and positive
definite saved covariance matrices; exact fixed-weight yields and source
hashes pass. Allfour original inclusive PNG/PDFs remain byte-identical.
Both new PNGs are1260square and layouts were visually inspected. The
dedicated plot-gallery agent added a first category section (low then hard),
retaining the original inclusive section and KF-only/GSF order. Agent and
parent verify4PNGs/2sections, exact-once coverage and served/source hash
closure. Homepage and both new PNGs return HTTP200, image/png for images.
Existing ailogin002.ihep.ac.cn PID2122174 remains on localhost8000; prior
66-file archive is unchanged and hash-valid.

Low PNG`61948240bc49c3710e826a9758721d9c7368f6edefa3e1d0826d6a7a7be9005b`;
PDF`6e82a64459eb218358f3b25c8b522853a1f26e163e65a7c281288c3d902fcf56`.
Hard PNG`cd5e36e3ed199fd64ee8a12d6e60d1c51b3d27145e5a4aaff93bdbe846f64ccf`;
PDF`7b2d4cfdd432fa641a3a5b2ce1df8dedc4e84874ba9eba20eecd4f3eeda86ef4`.

## Add matched inclusive KF/GSF without muons to the category section

User requested one inclusive KF/GSF plot in the by-eBrem section, without
muons. New recoil_electron_kf_gsf_inclusive_matched_10ab PNG/PDF uses the
same85,327common event union. Its counts equal the sums of both categories:
KF39,584raw/27,979.052expected; GSF45,919raw/32,456.803expected (+16.00%).
Undefined26/753 are retained; same weight10000*7.04/99600 and no FSR/BP
gate, neutral addition, category/window renormalization or tracking refit.

Core DSCB sigma0.29675+/-0.00720GeV KF and0.34136+/-0.00846GeV GSF.
Same123.5--128GeV fits; selected fits status0/covariance quality3 with no
boundary parameters. Two KF starts give consistent minima; the third fails
its Hessian(status302/cov0) and is rejected by the existing helper. All3
GSF starts converge. Poisson deviance/38ndf47.23KF/47.90GSF. No fit-range
or model systematic variation is evaluated.

split_ebrem.py --inclusive-only adds only this figure, preserving all8
preceding PNG/PDF artifacts byte-for-byte. New report is
ebrem_inclusive_matched_summary.json; log matched_inclusive.log; marker
EBREM_INCLUSIVE_READY. Input identity, exact categories, fixed weights and
prior plot hashes pass. Parent inspected final layout. Same square canvas,
large fonts, outside AGORA header, grid, luminosity and sigma/yield notes.
The new plot precedes low/hard in section1; section2's original two inclusive
muon/electron variants remain untouched. Gallery now5PNGs/2sections.
PNG`e25f955e35a91677820bd5882e4c50ea61fb9f19603a6a5695a344b074c5089c`;
PDF`abc4b2728e82536fa93f6af9094d4ec727910cb8b87422e27e63148ea31e6638`.
