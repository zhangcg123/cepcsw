# Current BP110 floating-FSR: resolved/unresolved FSR-only and Both — 2026-11-13

## User request and unchanged parent observables

User requested the resolved/unresolved study in the current section, and chose
**FSR-only and Both categories separately** in the clarification reply. Four
NEW shape figures extend the current full500-job BP section. The inclusive
three-curve10ab^-1plot and every parent category/other figure remain unchanged.
All four electron curves within each subgroup use identical pre-window keys
and EXACT parent recoil masses, with the same inclusive muon reference.

Parent common-category sample has67950pairs. The studied categories contain
9008FSR-only and13864Both pairs. Their existing all-method endpoint-validity
selection remains; this is not an unbiased full-production efficiency study.
No new fitting,reconstruction,PFA,simulation,library/install/card or submission.

## Resolution definition and trustworthy current provenance

Resolved means at least one usable separately reconstructed neutral,trackless
pre-reclustering ECAL cluster ANYWHERE has ANYpositive saved native MC-weight
contribution uniquely descending from generator-linked FSR belonging to either
selected production electron. The cache uses status1GeneratorFsrLinks roots
and MC-parent lineage, native PFO/cluster ownership, nonempty hits, finite
positive cluster energy and finite nonzero position. No angular truth matching,
purity/dominance/energy cap or extra photon-energy cut defines the tag.

Unresolved means no such usable FSR-contributing neutral is identified. This
includes merged,unreconstructed,invalid,unlinked or ambiguous cases; it does
NOT establish all FSR merged. Resolved requires >=1contribution, not all photons
or the hardest photon. A tiny soft FSR share can tag a mixed neutral while
hard FSR remains merged. Resolved outside the nominal collection window is
still resolved. Parent categories retain either-leg summed birthFSR>1GeV,
and respectively no material-eBrem>1GeV or Both=FSR AND eBrem,possibly on
different legs. Soft radiation is allowed under either threshold definition.

Reuse UNTRIMMED all-angle native-weight rows from
pre_ecal_absolute_efficiency_20261112/jobs. All500cache source paths/sizes/mtimes
match EXACT REC_ECALTRUTH inputs to the current BP files. The selected
track→MC map matches both production-electron rows for every target event.
Current BP/source identities are unchanged. Raw cache rows precede the older
energy study's upper2%trim;2312clusters across100000input events exceed that
old55.757GeV cap and remain eligible here, matching the established no-cap
resolution definition. The cap is applied only in the older energy aggregate,
not in these raw cluster caches or the new resolution tag.

Independently reread source1's200events from saved REC with the maintained
native-link reader and its selection self-checks. All5019neutral cluster rows
and399selected-track rows reproduce their cached CSVs BYTE-FOR-BYTE. No full
software reconstruction is run. Cache code hash matches its recorded original.
No label is transferred across different PFA replays, so old579mismatch
exclusions are NOT needed. All22872target events classify; no unknown group.
8434overlapping previously verified old FSR-only labels also agree.

## Counts and performance observations

| Category/group | Common events | KF+N120–140 | GSF+N120–140 | Ordinary110120–140 | Floating110120–140 | Ordinary110120–130 | Floating110120–130 |
|---|---:|---:|---:|---:|---:|---:|---:|
| fsr_only_resolved | 7004 | 5161 | 5153 | 3988 | 5394 | 1803 | 3813 |
| fsr_only_unresolved | 2004 | 1673 | 1671 | 1750 | 1881 | 1163 | 1611 |
| both_resolved | 11097 | 7502 | 7478 | 6411 | 8408 | 3107 | 5992 |
| both_unresolved | 2767 | 2079 | 2199 | 2352 | 2463 | 1566 | 2131 |

Resolved7004+unresolved2004=9008FSR-only;11097+2767=13864Both. Every raw mass
histogram partitions its parent category bin-by-bin for all four methods.
Muon reference remains44236finite120–140entries in every plot,unit-normalized.
All new curves independently normalize by their OWN finite window count;
legendN is raw, not luminosity-weighted. Display120–140GeV,0.1GeVbins,square,
large text,grid,AGORA simulation above axes,no title/longfooter. Muon dashed
black;KF+neutrals green,GSF+neutrals gold,ordinary110red,floating110blue solid.
No extra neutral addition to either BP; matching electron+FSR p4 used once.

After floating FSR,120–130contains54.4%of resolved vs80.4%of unresolvedFSR-only
events, and54.0%vs77.0%inBoth. These use full selected subgroup denominators,
not just the displayed window. Unresolved groups have a more populated central
region after fitting; resolved groups retain larger high-mass tails. This is
not a controlled causal or calibrated resolution comparison: FSR energy and
topologies differ, tags can represent partial/soft resolution, all-method fit
validity can bias samples, and material/FSR/contamination ambiguities remain.
Low-mass tails also remain. No physics-validation claim or new DSCB fit.

## Artifacts, memory and gallery

Artifacts: TrackingPerformanceStudies/bp_fsrfloat_resolved_recoil_20261113/.
Includes analyze.py,run.log,events.csv,histogram.csv,summary.json,native_check.json,
native_check/jobs/,code/source hashes and four PNG/PDF pairs. All four original
renderings were visually inspected; shortened subgroup labels were then
rerendered from unchanged event records to avoid covering the muon peak.
Representative resolved/unresolved final layouts were rechecked.

The exact outgoing AGENTS is preserved in
[the snapshot](2026-11-13-agents-before-current-bp-resolved-study.md). Global
status,laws and compile instructions remain verbatim. Current focus now records
this extension while retaining inclusive normalization/default option status.
No history directory is moved; no migration manifest is required.
[Parent category study](2026-11-13-full-bp-floating-recoil-shapes.md).
[Inclusive absolute normalization](2026-11-13-inclusive-bp-floating-recoil-10ab.md).
Published into the existing full500BP category section on ailogin002.ihep.ac.cn,
localhost8000,managedPID443262,no restart. The resolved/unresolved pairs sit
immediately after their respective parent category PNGs. Gallery36PNGs/
11sections/exact-once;all40pre-existing PNG/PDF hashes unchanged. Page and all
four new PNGs returnHTTP200 with served/source SHA256agreement. The main agent
independently checked all four copied image hashes and the nine-entry current
BP section pattern list. Complete publication was delegated to the personal
plot-gallery skill agent. Analysis/images/manifests remain uncommitted.
