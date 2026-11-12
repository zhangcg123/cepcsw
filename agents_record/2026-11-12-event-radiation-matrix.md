# Event radiation fractions: FSR/eBrem, 2026-11-12

User requested a matrix of FSR only,eBrem only,neither,both. Event-level true
radiation flags from either selected recoil-production electron; Both can be
on different legs. No fit-success, baseline-charge, recoil-window, material-loss
fraction, extra photon-energy or neutral-cluster collection requirement.
FSR generator-linked; eBrem positive recorded G4 material-photon energy for
selected electron G4 owner. Recorded radiation definitions, not an assertion
of arbitrarily soft unrecorded physical radiation absence.

All500BP inputs/100000events: original498-source validated track CSV plus
later-completed40/429 read with the same refresh_completed_bp.read_job helper.
Extra397+396rows bring total198430selected tracks. No reconstruction/PFA/fit run.
Original source/cache unchanged.98437events have exactly two opposite-PDG
selected production electrons and known flags;1556incomplete pairs and7without
selected tracks excluded. No unknown radiation among complete pairs.

Counts/fractions over98437events:
Neither1113/1.1306724098%;eBrem only14635/14.8673771041%;
FSR only5946/6.0404116338%;Both76743/77.9615388523%.
Allcells sum98437/100%; generated-population closure98437+1556+7=100000.
No fit-validity filter. Denominator differs from current11750FSR-free matched
recoil subset; matrix includes the whole classifiable production sample.

Square2x2heatmap,rowsFSRabsent/present,columns eBremabsent/present; each cell
category/count/percentage. AGORA simulation header,N98437, no luminosity.
Artifacts:TrackingPerformanceStudies/event_radiation_matrix_20261112/:analyze.py,
extra_tracks.csv,extra_sources.json,events.csv,summary.json,code/plot hash manifests,
run.log,PNG/PDF. Allsource IDs checked at extra extraction; future matrix redraw
uses frozen caches without requiring old REC files to remain. Unique keys,
selected pair/knownflag gates, full coverage/population closure/fraction closure,
source hashes/plot hashes pass; layout viewed. Gallery published16PNGs/8sections/
exact-once, all15prior images unchanged. New matrix section immediately before
no-FSR recoil shape comparison. Page/newPNGHTTP200,sourcehashmatches; main
reviewed copiedhash/categorycoverage. Same ailogin002 PID2122174 localhost8000,
no restart. gallery_verification.json preserves checks.

## Companion: per-electron radiation energy strictly >1GeV

User requested a second matrix beside the original with FSR/eBrem loss>1GeV.
Same98437event keys/denominator, no fit/endpoint/recoil/neutral gates added.
For each selected electron sum saved photon BIRTH energies separately by type;
event type present if EITHER per-electron sum>1GeV(maxlegsum>1). No cross-leg
energy summing or individual-photon threshold. Exactly1GeV fails. Neither now
permits radiation below/equal1GeV. Definition stated before computation.

Counts Neither21602(21.9450003556%),eBrem only43739(44.4334955352%),
FSR only11517(11.6998689517%),Both21579(21.9216351575%). Sum98437/100%.
New source files threshold_1gev.py,events_gt1gev.csv,summary_gt1gev.json,
run_gt1gev.log,code/plot hash manifests,event_fsr_ebrem_matrix_gt1gev.png/pdf
in original event_radiation_matrix_20261112 study directory. Original matrix
unchanged; shared renderer refactor independently reproduces original PNG hash
exactly. Source code/metadata before refactor preserved in before_threshold_companion/.
Allthreshold/finite-nonnegative-energy/key/subset/closure/hash gates pass; new
layout viewed. Cache-only; no ROOTevent, reconstruction or fit run. Companion
published immediately after original in same section:17PNGs/8sections/exact-once,
all16prior images unchanged, page/bothmatrixPNGHTTP200/newsourcehashmatch. Main
checked copiedhashes, category coverage and matrix adjacency. Same ailogin002
PID2122174 localhost8000, no restart. gallery_verification_gt1gev.json records
checks in source-study directory.

## User clarification: combined threshold entry is OR

Combined=fOR e. FSR-only=fANDNOTe; eBrem-only=eANDNOTf, with unchanged per-leg
truth-energy threshold event flags. Only counts remain11517/43739,Neither21602.
InclusiveOR76835=78.0549996444%of98437. It contains11517+43739+21579(AND), so the
four displayed entries are NON-EXCLUSIVE and must not sum to100%. Neither+OR
sum100%. Both operators and display overlap are stated explicitly.

New companion labelled Either(OR) with a note "Includes both only groups".
Removed presence-axis labels because lower-right now cannot be interpreted as
FSR-present AND eBrem-present; it is a2x2summary of requested quantities. Original
unthresholded matrix unchanged; earlier ANDthreshold source/data preserved.
New source or_companion.py,summary_gt1gev_or.json,PNG/PDF stem
event_fsr_ebrem_summary_gt1gev_or,run/plot/code hash manifests. All input event
keys/threshold flags checked; only groups unchanged, union/complement closure
pass, layout viewed. Cache-only, no reconstruction or fit. ORsummary published beside original:
17PNGs/8sections/exact-once, other16images unchanged; old threshold-AND PNG
absent from active gallery. Page/bothactivePNGHTTP200,newsourcehashmatch.
Prior servedANDPNG+sectionJSON archived in before_or_summary/gallery/. Main
reviewed coverage, hashes, pattern order, absent old preview and nonexclusive
definition. Same ailogin002 PID2122174 localhost8000,no restart. Verification:
gallery_verification_gt1gev_or.json in source-study directory.

## Latest user change: last block is Both[AND]

User explicitly reversed combined operator to AND. Last block nowfANDe,
21579/98437=21.92163516%,labelBoth(AND). Other three entries unchanged:
Neither21602/21.95%,eBrem-only43739/44.43%,FSR-only11517/11.70%. These four
categories are mutually exclusive and sum98437/100%. Removed OR overlap
caption. Threshold flags/cuts/denominator unchanged; either leg can supply
each type's per-electron-summed truth energy strictly>1GeV. No requirement
that both types be on the same electron.

New and_companion.py,summary_gt1gev_and.json,event_fsr_ebrem_summary_gt1gev_and
PNG/PDF and run/code/plot manifests. Prior OR source retained for history.
Cached flags/energies, unchanged only groups, original AND identity, population/
percentage closure, hashes pass; layout viewed. No native run/refit. Gallery
verified17PNGs/8sections/exact-once; other16images unchanged, prior ORpreview
absent, page/bothactivePNGHTTP200/newsourcehashmatch. Main checked sourcehash,
coverage and adjacency. Prior ORservedPNG+sectionJSON in before_and_summary/gallery/.
Same ailogin002 PID2122174 localhost8000, no restart. Verification artifact:
gallery_verification_gt1gev_and.json in source-study directory.
