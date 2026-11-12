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
