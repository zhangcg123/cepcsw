# Passive BP-matched neutral collection in GSF flat tuples — 2026-11-18

## User intent and authorization

The user requests rerunning GSF flat tuples with the same reconstructed neutral
collection as BP, even though downstream GSF recovery and BP constraints use
those measurements differently. They confirm no other CEPCSW jobs are active,
explicitly authorize committing the required RecGsfTracking CMake registration,
and request deleting the current large GSF directory before the rerun.

Only RecGsfTracking's flat writer/helper/test/build registration, its README,
the maintained gsf.py.bk flat properties, workflow README and project memory
change. Existing unrelated dirty GSF fitter steering/options are preserved and
excluded from this implementation commit. GsfAlgorithm.h/.cpp, BP, original
PFA/tracking/simulation, shared fitting libraries and physics are unchanged.

## Implementation and contract

CollectedNeutralTuple compiles the exact existing BP NeutralLossCandidate and
ChargedEnergyCandidate implementations into the flat-writer module and reuses
its HcalEnergyCandidate helper. It does not link or run the BP algorithm.
Targets are sorted original CompleteTracks indices from the production-electron
selection, including partial one-leg selections and failed GSF fits. The
successful-publication-only GSFTrackInputIndices list is deliberately not the
collection target list. Selector-off targets all CompleteTracks.

Centre: first input BaselineKF AtCalorimeter referencePoint. Compare IP-to-point
theta/phi against IP-to-cluster-position theta/phi; wrapped phi, inclusive
half-windows25/200mrad. Use pre-reclustering zero-charge, trackless PFOs with
positive finite, hit-supported ECAL clusters; deduplicate ECAL identity per
track. HCAL follows selected PFO links, applies scale4 once, deduplicates HCAL
identity and excludes that track's charged-observation HCAL according to BP's
default enabled charged association. No independent HCAL window/HCAL-only-PFO
selection, truth matching, purity selection or study energy cap.

36 new branches retain per-target input indices, KF reference/status, collection
status/error/count, PFO/ECAL/HCAL IDs, individual/aggregate ECAL and HCAL energies
and uncertainties, HCAL raw/scaled energies, selected HCAL positions and owner
edges, and the combined observation. Seven effective settings are scalars.
The complete pre_ecal snapshot and all309 previous fields remain. Every nested
row is keyed by neutral_collection_track_input_index. Shared observations may
occur in both track records; event-level recovery must deduplicate the union.
No neutral p4 is automatically added to GSF and no fit is constrained.

Seven new RecGsfFlatTuple properties: theta/phi windows25/200mrad, ECAL error
coefficients.011/.004, HCAL scale4 and error coefficients.30/.07. Resolution
terms add linearly within each cluster; independent variances add across
clusters/detectors. Windows are finite0<window<3141mrad, matching BP steering.
Scale is finite positive; errors finite nonnegative with a positive pair sum.
Validation happens before opening the output. Both READMEs and explicit
maintained flat steering are synchronized. A dedicated option agent audits all
43unchanged GSF fitter properties and all10flat-writer properties.

No full ECAL/PFO snapshot: availability0, per-target status0, NaN energies.
Partial snapshot/unavailable relations retain the prior hard-failure behavior.
No KF reference: status0; valid empty selection:1; nonempty:2; neutral exception:
-1 with text/NaN energy. Missing HCAL preserves ECAL but makes HCAL/combined
energies NaN and availability0. Invalid selected HCAL energies fail the writer,
as in BP. Charged-association failures/ambiguities follow BP's exclusion rules
and are explicitly recorded. Legacy Python recovery remains ECAL-only and does
not automatically consume the new native fields.

## Verification and installation

Build succeeds. Four related CTests pass: new collection/serialization test,
GSF truth selector and BP neutral/charged candidate tests. The new test covers
separate/overlapping track windows, first-reference ownership, no/invalid
reference, missing snapshots/HCAL, empty selections, nonempty-hit eligibility,
charged/tracked exclusions, duplicate ECAL/HCAL ownership, charged-HCAL
exclusion, HCAL-only exclusion, uncertainties, nested ROOT persistence and
event clearing, invalid indices/energies/settings including3141mrad.

Exact native old/new runs use source1currentREC, EvtMax18 and fitted events
0/3/11/16/17. New run includes full verbose component dumps, covering the required
hard-loss events11/16/17. Old control uses the previous private production flat
writer (SHA256c26c2618652d6b8ca7fba4696055701dc0c298a422c9ad2219b8f94612a29678),
not the stale previous default writer. All309old branches across18entries match
exactly (5562branch-row comparisons, including vector fields). Ten selected
tracks match160BP observation-field values exactly: selected IDs, energies,
per-cluster/aggregate errors, HCAL contributions and combined observations.
There are real nonzero HCAL observations as well as empty windows. Event0/track13
has no GSF endpoint yet keeps its collected neutrals. Both original KF reference
positions match BP exactly. Old/new native runs exit0. This is mechanical
regression evidence, not a physics-resolution or photon-calibration claim.

The new worker verifier accepts all18native rows/10targets and rejects four
deliberate corruptions (wrong window, missing target, wrong sum, duplicate HCAL).
Its actual PyROOT check is recorded alongside the analysis results. An optional
probe omitting the GSF producer and normal services fails before event0 with
both old and new writers; it is excluded from validation and is not the prepared
workflow. No standalone REC-to-flat-only support or unrelated crash fix is added.

Only default libRecGsfFlatTuple.so, its generated Python configurable and
module-specific .components/.confdb are refreshed. Installed flat-library SHA256:
e554b8382e79ac26e463cde523eaf056c8985e0043846ad0002ae62f3421280c.
GSF fitter remains6a37fc59ba7b069ceb1dfe8bb0d8f26cb90e843d806af6888268abd57c4eecad;
BP remains7e833d9868c72638f3b591e1c73be16f17064f7000d98ae0b40f40196c727fe6;
truth libraries also hash unchanged. Old/default-control artifacts, full logs,
ROOT files and JSON proofs remain uncommitted under
TrackingPerformanceStudies/gsf_bp_neutral_collection_20261118/.

## Replacement batch and disk

Prepared/unsubmitted:
higgs_eeHX_ISRFSRON_GSF_BPNEUTRALS_500jobs_200evt_20261118/.
500jobs x200currentREC events =100,000inputs,4000MB,mid,cms. Every input path,
size/mtime and event range matches the completed paired BP campaign. Previous
GSF fit steering is retained; only the input REC production and passive flat
settings change. Cards, runtime hashes, shell/Python syntax and all500submission
dry-run commands pass. Workers verify input identities,200rows, endpoints and
the36new collection fields before completing. No jobs are submitted here.

The user-authorized old GSF directory removal frees816,433,629,925bytes. It had
498completed jobs plus historical partial failures55(72entries) and421(145),
not500verified completions. Old job records, runtime hashes and failure tails
are preserved in the replacement's previous-campaign-provenance.json. Old ROOT
files/logs/cards are removed. CurrentREC/BP files and studies remain. Filesystem
reports525GBused/1.5TBavailable of2TB after deletion. Full new output is expected
to fit; monitor production usage and fit failures. Earlier partial failures are
not attributed to this passive collector change.

Submit from the normal working batch-login shell:

```bash
bash /aifs/user/data/zhangcg/gsfdev/CEPCSW/higgs_eeHX_ISRFSRON_GSF_BPNEUTRALS_500jobs_200evt_20261118/submit.sh --submit
```

The local queue client still lacks htcondor. Generated cards, batch helpers,
ROOT/log/JSON artifacts remain uncommitted. The explicit user exception covers
this change's required RecGsfTracking/CMakeLists.txt registration. The complete
outgoing AGENTS is preserved in2026-11-18-agents-before-gsf-neutral-collection.md;
its global status/laws/commands prefix is retained byte-for-byte in live AGENTS.
