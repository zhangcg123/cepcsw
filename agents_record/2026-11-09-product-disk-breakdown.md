# Product disk breakdown after compact truth

The user questioned the approximately 1% whole-product size reduction and
asked what consumes disk. This is a read-only audit: no collections, files,
merge settings, simulation/reconstruction code or output-retention settings
were changed. No build/install or jobs were required.

## Paired product measurements

Same two generator events, same installed code, full-history versus compact
truth. Sizes are actual file lengths, not uncompressed event-data estimates.
All MB/GB below are decimal.

| Product | Full bytes | Compact bytes | Reduction |
|---|---:|---:|---:|
| SIM | 27,718,198 | 27,287,596 | 1.5535% |
| Tracker | 28,407,109 | 27,976,751 | 1.5150% |
| Calorimeter digitization | 28,633,740 | 28,202,917 | 1.5046% |
| REC | 29,213,196 | 28,782,618 | 1.4739% |
| Default breakpoint flat | 2,622,189 | 2,622,203 | effectively unchanged (+14 bytes) |

The compressed Gsf-truth leaf payload changes from 754,750 to 324,583 bytes,
a 57.0% reduction in that small block, saving 430,167 bytes. Full SIM truth is
only 2.723% of the original SIM file, so this gives a 1.55% whole-file saving.
REC additionally contains reconstruction data, giving 1.47%. The flat tuple
does not contain the discarded original G4 trajectories; it retains the same
fit diagnostics and photon checkpoints, so a size reduction there is not
expected. Small byte differences include compression/order/metadata effects.

## Existing 200-event production REC

Representative source:
`higgs_eeHX_100jobs_20261106/rec-higgs-1.root`, 200 events,
3,638,815,083 bytes (3.639 GB).

Compressed event-basket bytes were summed over leaf branches, without decoding
events. PODIO relation/vector-member branches were attributed to their owning
collection, avoiding parent/subbranch double counting. The remaining 4,246,592
bytes are file/tree/schema/configuration and related overhead, not assigned
to event collections.

| Content | Compressed bytes | Share of complete file |
|---|---:|---:|
| ECAL simulated hits + contributions | 3,042,151,036 | 83.603% |
| HCAL simulated hits + contributions | 260,614,366 | 7.162% |
| All Gsf truth collections | 56,579,528 | 1.555% |
| All remaining content and overhead | 279,470,153 | 7.680% |

Largest individual collections:

| Collection | Compressed MB | File share |
|---|---:|---:|
| EcalBarrelCollection | 1,267.008 | 34.82% |
| EcalBarrelContributionCollection | 1,165.827 | 32.04% |
| EcalEndcapsCollection | 318.960 | 8.77% |
| EcalEndcapsContributionCollection | 290.357 | 7.98% |
| HcalBarrelCollection | 105.880 | 2.91% |
| HcalBarrelContributionCollection | 95.341 | 2.62% |
| TPCCollection | 64.388 | 1.77% |
| TPCTrackerHits | 60.085 | 1.65% |
| GsfG4MaterialSteps | 36.576 | 1.01% |
| GsfG4BremsPhotonSteps | 18.310 | 0.50% |

Thus about 90.8% of this REC file is raw calorimeter simulation. Even deleting
ALL Gsf truth would save only about 1.55% in this existing file. This is a size
upper bound, not a proposal to remove required truth. It is an existing sample
without the newly added explicit FSR collections; the paired two-event test
above compares equal new truth content in both full and compact modes.

## Why the calorimeter block is large

The maintained full-detector simulation card and these test cards configure
`CalNamesMergeDisable` for EcalBarrel, EcalEndcap, HcalBarrel and HcalEndcap.
`CalorimeterSensDetTool::createSD` passes `is_merge_enabled=false` to
CaloSensitiveDetector. Its `ProcessHits` creates a new hit for each depositing
Geant4 step instead of looking up and merging the cell-ID hit. The EDM writer
then persists both the SimCalorimeterHit and its contribution record, including
energy, time, position and truth relation information. These records describe
the entire calorimeter shower, not just the selected electron/photon tracks.

In the two-event test, ECAL barrel has 355,471 and 193,518 simulated-hit records,
and exactly the same numbers of contribution records. Endcap has 4,501 and
159,481 hit/contribution pairs. Energy and x/y/z position fields are among the
largest baskets, with energy/position information appearing in both containers.
This accounts for the bulk size; it is not an unexplained ROOT metadata cost.

The SIM, tracker, calodigi and REC cards use `out.outputCommands=["keep *"]`.
Consequently raw calorimeter simulation is retained in downstream REC. Changing
merge behavior changes the simulated-hit position/aggregation contract and
could affect digitization/reconstruction; no such change is part of this audit.
Output retention requires separate dependency review, including future truth
association requirements. No automatic collection pruning was performed.

Relevant implementation: `Simulation/DetSimSD/src/CalorimeterSensDetTool.cpp`,
`Simulation/DetSimSD/src/CaloSensitiveDetector.cpp`, and
`Simulation/DetSimAna/src/Edm4hepWriterAnaElemTool.cpp`.

## Existing production flat tuple

`higgs_eeHX_100jobs_20261106/breakpoint_flat-higgs-1.root`:
268,461,477 bytes, 5,585 breakpoint rows and 200 event rows.

- Covariance branches: 176,125,291 compressed bytes, **65.605%** of the file.
- Within that total, 27 forward-predicted/filtered/smoothed covariance branches
  occupy 150,871,396 bytes, **56.199%**.
- Nine `persistent_*` branches occupy 28,955,950 bytes, **10.786%**; this group
  overlaps covariance/transport totals and must NOT be added to them.
- Transport branches occupy 5,865,761 bytes, **2.185%**.
- The entire neutral_pfos event tree is only 522,332 compressed bytes.

Several beam/charged-ECAL ordinary/free-loss per-hit covariance branches each
occupy about 9.3 MB. These are multiple diagnostic fit families, not a claim
that their numerical arrays are identical. Compacting simulation truth does
not remove these downstream fit diagnostics.

## Current campaign file totals

A one-level stat scan of regular ROOT files (not a physics-event validation)
found the following at audit time. Partial/incomplete products, if present,
still consume bytes and are included. No frozen study inventory was changed.

| Directory | REC | Flat | Other ROOT intermediates | Total ROOT |
|---|---:|---:|---:|---:|
| higgs_eeHX_100jobs_20261106 | 354.858 GB (96 files) | 21.577 GB (96) | 19.853 GB | 396.288 GB |
| higgs_eeHX_ISRFSROFF_parent_20261107_50x100 | 90.838 GB (49) | 5.945 GB (49) | 2.876 GB | 99.660 GB |

Together these two directories hold approximately **495.948 GB of ROOT files**,
of which **445.696 GB are REC**. These are current file-size observations, not
an assertion that every file is complete/validated, nor a complete workspace
or user-quota inventory. Other directories, logs, build products and snapshots
were not exhaustively scanned.

The worker's current cleanup machinery removes job-owned SIM/TRK/calodigi only
after downstream verification; it retains external inputs. The present totals
show that remaining intermediates are much smaller than the REC population.
The dominant storage choices are raw calorimeter retention in REC and detailed
per-hit fit diagnostics in the flat tuple. None was changed by this work.

## Reproduction artifacts and limits

Header-only analysis scripts/reports under `/tmp/truth-recorder-20261109/clean/`:
`disk_breakdown.py`, `disk-breakdown.json`, `flat-disk-breakdown.json`.
File sizes also came directly from stat. Branch compression metrics sum stored
ROOT baskets and do not estimate filesystem-level compression/deduplication.
The collection percentages are from one representative 200-event radiation-on
file plus the paired two-event tests; no population compression-ratio claim is
made. No data were deleted or rewritten, and no policy was changed.

## Consumer trace: are raw calorimeter hits used?

Follow-up question: whether the simulated ECAL/HCAL hits provide information or
start any manipulation. Source inspection confirms the following boundaries:

1. Digitization reads raw `Ecal{Barrel,Endcaps}Collection` and
   `Hcal{Barrel,Endcaps}Collection` and their contributions. EcalDigiAlg first
   merges step hits into bars in memory, retaining step energy, position, time
   and MC labels. The contribution loop uses step position/time for two-ended
   bar response and accumulates MC energy weights. It writes ECALBarrel and
   ECALEndcaps digitized hits and direct reconstructed-hit-to-MC associations.
   HcalDigiAlg similarly merges raw steps, calculates the configured response,
   time and MC energy fractions, and writes HCALBarrel/HCALEndcaps. The inspected
   card enables realistic ECAL digitization; realistic HCAL digitization is off,
   but energy/time/MC-weight construction still consumes raw contributions.
2. CyberPFA reads digitized ECAL/HCAL hits and the direct hit-to-MC associations,
   plus reconstructed tracks. CaloHitsCreator creates local calorimeter hits/
   bars and attaches the supplied MC weights. Clustering, track-cluster matching
   and PFO creation then use these objects. The card also runs truth HCAL
   clustering into a separate collection; this likewise uses derived MC weights,
   not direct access to the original raw SimCalorimeterHit collections.
3. FinalPIDAlg reads CyberPFO, TOF, Dndx and muon tracker-hit inputs; its current
   interface has no raw simulated ECAL/HCAL-hit input. RecBreakpoint similarly
   has no raw simulated calorimeter-hit/contribution handle. Its default ECAL
   measurements come from CyberPFOPreReclustering/EcalClusterPreReclustering
   charged and phi-window-collected neutral clusters. FinalPFO is an explicit
   alternative. Electron/photon truth checkpoints are separate Gsf collections.
   The existing FSR/eBrem plots do not perform calorimeter-shower deposit
   attribution from these raw contribution records.

Consequently these raw hits are essential BEFORE digitization and for rerunning
or validating it. Their unconditional retention in final REC is not a direct
requirement of the current breakpoint fitting path. They also retain useful
shower-level truth information for future studies. Derived MC associations
label deposited energy by retained MC particles; the simulation's ancestor/
primary policy and secondary-recording threshold must be respected before
claiming exact attribution to an original FSR/eBrem photon.

At the time of this consumer trace, filtered-REC pruning was untested. The
subsequent user-authorized implementation and exact regression results are in
[the pruning record](2026-11-09-rec-calorimeter-output-pruning.md). Digitized hits,
cluster constituents and the direct MC associations consumed by PFA must remain;
any retained relation targeting a removed collection needs consistent handling.
This trace is not a request or permission to alter merging, prune collections,
or investigate the excluded many-to-many association topic.

Source anchors: `Digitization/DigiCalo/src/EcalDigiAlg.cpp` (MergeHits and
execute contribution loop), `HcalDigiAlg.cpp` (response/time/MC-weight loop),
`Reconstruction/RecPFACyber/src/Tools/CaloHitsCreator.cpp`,
`Reconstruction/ParticleID/src/FinalPIDAlg.h`, and
`Reconstruction/RecBreakpoint/options/run_breakpoint.py`.
