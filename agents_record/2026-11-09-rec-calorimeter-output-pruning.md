# Omit raw calorimeter simulation from REC; preserve full breakpoint inputs

## User request and authority

The user explicitly asked to preserve the information used by the complete
breakpoint method and its ECAL constraint while cancelling simulated calorimeter
hits in REC. After the required new idle-job query, the user answered **No jobs
are active**. Local process inspection also found no CEPCSW processes; the
batch queue command was unavailable. Existing library installations are not
changed. Only dedicated breakpoint batch output policy/tests/docs are edited.
No PFA/PID/digitization/KF/fit/optimizer/truth-ownership code is modified, no
branches/remotes or Hive migration change, and no batch jobs are submitted.
The complete outgoing AGENTS is preserved in
[the snapshot](2026-11-09-agents-before-rec-calo-pruning.md), with its global
section retained byte-for-byte. Existing unrelated user edits are preserved.

## What the ECAL constraint actually uses

The default `EcalConstraintInput=PreReclustering` reads
`CyberPFOPreReclustering` and `EcalClusterPreReclustering`, with their reconstructed
hit constituents and exact PFO-to-track links. FinalPFO is an explicit alternate
source and its data are retained too. The charged energy follows the PFO owning
that exact CompleteTracks object; ambiguity and alternate-source hit containment
checks are unchanged. Neutral clusters require charge-zero/no-track PFOs,
nonempty reconstructed hits and the configured theta/phi window around the
track's AtCalorimeter reference point. Cluster positions and the full track
state therefore matter, not only a saved total energy.

For a resolved ElectronPhoton110 observation with one breakpoint:

    E_measured = E_charged + sum(E_collected_neutral)
    sigma_E^2 = sigma_charged^2 + sigma_neutral^2
    E_predicted = p_ECAL * exp(b), b = log(p_before / p_after)

Each cluster's error remains the configured `a*sqrt(E)+c*E`; cluster variances
are summed. The implementation performs the same scalar native KF observation
at the ECAL face, using residual E_measured-E_predicted and innovation variance
H*P*H^T+sigma_E^2. RTS propagates the state/covariance information back through
the tracker, and the normalized likelihood contributes to the free-loss
objective. ElectronOnly, unresolved/no-breakpoint routing, missing-measurement
fallbacks, 111 and existing bounds/acceptance are unchanged. FSR energy is not
subtracted by this storage change.

Source: `ChargedEnergyCandidate.cpp`, `NeutralLossCandidate.cpp`,
`ChargedEnergyMeasurement.h`, `BreakpointFitter.cpp`, `RecBreakpoint.cpp`.

## Exact output policy

Only these twelve collections are omitted AFTER PFA/PID, when writing REC:

- EcalBarrelCollection, EcalBarrelContributionCollection;
- EcalEndcapsCollection, EcalEndcapsContributionCollection;
- HcalBarrelCollection, HcalBarrelContributionCollection;
- HcalEndcapsCollection, HcalEndcapsContributionCollection;
- ECALBarrelAssoCol, ECALEndcapsAssoCol, HCALBarrelAssoCol, HCALEndcapsAssoCol.

The last four are digitized-hit-to-SimHit associations, whose targets are being
omitted. No broad wildcard is used. Keep everything else, particularly:

- ECALBarrel/ECALEndcaps/HCALBarrel/HCALEndcaps and direct
  `*ParticleAssoCol` hit-to-MC associations;
- every reconstructed PFA calorimeter-hit/cluster collection, both final and
  pre-reclustering PFOs, PID data and constituent/track links;
- CompleteTracks and states, tracker hits/associations/simulated tracker hits;
- MCParticleGen/MCParticle, Gsf material/checkpoint/span/hit-step records and
  eBrem/FSR photons/steps/origins.

`batch_breakpoint.py` freezes explicit `keep *` plus twelve `drop <name>`
commands into newly prepared REC cards, records `rec_drop_collections` in their
job manifests, and checks those omissions during output verification. Legacy
frozen cards/manifests and external REC inputs retain their existing semantics;
absence of the manifest field does not reject older full files. Shared card
templates, SIM/TRK/calodigi production and existing intermediate cleanup are
unchanged. Existing production REC files are NOT rewritten or deleted.

The active working-tree campaign cleanup can remove job-owned SIM/TRK/calodigi
after downstream verification. With this output policy, a completed job may
therefore retain no raw calorimeter-deposit copy. This is the user's chosen
breakpoint-oriented output content; a separate raw SIM/calodigi input is needed
for future shower-deposit studies/re-digitization. No change to cleanup or any
extra automatic deletion is introduced here.

## Verification artifacts and completed checks

Artifacts: `/tmp/rec-calo-pruning-20261109/`. Original source files remain intact.
Native PODIO filtering copies event parameters and other existing metadata
categories, and refuses overwriting outputs. A reference audit inspects every
retained reference and finds zero valid references targeting removed collections.
Existing unavailable relation slots remain unchanged, not repaired or counted
as new dangling links.

| Fixture | Events | Original bytes | Filtered bytes | Reduction |
|---|---:|---:|---:|---:|
| Newly simulated compact REC | 2 | 28,782,618 | 2,473,055 | 91.4078% |
| Frozen compact REC fixture | 18 | 316,954,118 | 22,036,050 | 93.0476% |

The native typed-buffer comparator checks every retained data member, vector
member, ID/type, relation and event parameter (including fixed-array types that
uproot cannot decode). All 76 retained collections in the two-event fixture
and all 73 in the 18-event fixture compare exactly. These contain 126,720 and
1,455,047 objects respectively. Reference audits inspect 159,562 / 1,822,400
reference slots, with zero references into the removed collections.

Default two-event breakpoint rerun with verbose dumps, production-electron
selection and default 110 scans on the filtered fixture reproduces ALL 953
track branches across 40 rows and ALL 114 event branches across two rows,
including NaN locations/statuses, cluster indices, measurements, b, NLL,
minimizer/fit statuses, states and covariances. This is exact fixed-REC
preservation, independent of the existing PFA ordering/PID repeatability issue.

A real Gaudi PodioInput -> PodioOutput copy using the SAME generated drop policy
writes two events to `rec-policy.root` (2,471,272 bytes). Its 76 retained
collections also compare exactly. File metadata/configuration explains the
small size difference from the native copy. The EventLoopMgr's warning about
no external EventSelector is benign with EvtSel=NONE; the resulting events tree
has two entries and all checks passed.

Two new policy/verification tests, all 65 existing batch-planning tests and both
compact-input-card tests pass. No C++ algorithm/header changes or installation
are needed. The direct and native output-policy gates are completed below.

## Final direct A/B and native-PFA gates

The focused events 0,11,16,17 were rerun DIRECTLY with the same current installed
code, same defaults/110 scan settings, production-electron selection and verbose
dumps on the full and filtered forms of the 18-event fixture. Both jobs exited
successfully. All **953 breakpoint branches across 89 rows** and all **114 event
branches across four rows** agree exactly, including NaN positions/statuses,
energy measurements, cluster IDs, fitted b, objective/scan results, means and
covariances. No t/X0 tolerance was needed. This satisfies the focused hard-loss
regression gate without rerunning or changing the upstream reconstruction.
Reports: `focused-flat-comparison.json`, `focused-control.log`,
`focused-filtered.log` under the artifact directory.

The real Gaudi copy-policy file was also processed with the default complete
breakpoint fit. All 953/114 branches across 40/two rows agree with the original
unpruned two-event REC. This exercises the actual KeepDropSwitch commands, not
only the native frame-copy helper.

Finally, native CyberPFA + FinalPID were rerun from the IDENTICAL two-event
`/tmp/truth-recorder-20261109/clean/compact/calodigi.root`, same seed/options,
with only the outputCommands changed. Output:
`/tmp/rec-calo-pruning-20261109/rec-native-policy.root`, **2,473,038 bytes**,
versus original 28,782,618 bytes. The file has two events and omits exactly the
intended twelve collections. All 76 retained collections (126,720 objects),
including their serialized data, IDs, all vector members/relations and event
parameters, compare EXACTLY to the original REC. The native reference audit
again finds zero valid retained references into removed collections; the same
658 unavailable slots remain. All three PFO collection families additionally
match their complete physical content and links on both events. The earlier
baseline PID/order variation did not occur in this particular pair; it was
not fixed or normalized away.

Default RecBreakpoint on that newly produced native-policy REC also reproduces
ALL 1,067 branches (40 track rows, two event rows) exactly. Logs include verbose
component dumps. The existing invalid-provenance track remains invalid in
both cases; no NaN/failure is counted as a successful fit.

Additional reports:

- `policy-retained-comparison.json`, `policy-flat-comparison.json`;
- `native-retained-comparison.json`, `native-reference-audit.json`;
- `native-pfo-comparison.json`, `native-policy-flat-comparison.json`;
- `copy-policy.log`, `native-rec-policy.log`, `native-policy-fit.log`;
- `policy-tests.log` (two tests), `batch-tests.log` (65 tests),
  `truth-card-tests.log` (two tests).

The two new policy tests also pass against the selectively staged HEAD-based
batch module, so they do not depend on unrelated dirty campaign changes.
Core implementation/tests/README checkpoint: **c37cea1**. Only this task's
batch-helper hunks were staged; pre-existing Higgs/cleanup/expected-event
changes remain in the working tree. Shared templates and all C++/header files
are unchanged by this task. Analysis scripts, temporary cards, native checking
tools, ROOT files and logs are not committed. No remote push/branch operation,
job submission, bulk existing-file conversion or data deletion occurred.

These are two-event native checks and four stored-event fit checks, not
population physics validation. The ECAL measurement, including any FSR already
inside its shower energy, remains unchanged. The delivered change is output
retention after reconstruction, not a new calorimeter model or breakpoint fit.
