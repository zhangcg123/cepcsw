# Independent GSF neutral recovery with theta half-window 25 mrad

The user clarified that GSF and BP are separate functions and requested the
same collection window for GSF. The earlier documentation of plotting-script
reuse of BP IDs did not implement independent GSF recovery. This change does.

Workspace analysis implementation:
`Reconstruction/RecGsfTracking/scripts/recover_neutral_photons.py`.
It reads GSF's saved pre-ECAL snapshot and native BestBranch per-track endpoints.
The corresponding REC supplies CompleteTracks ECAL reference points, absent
from existing GSF flat files. No BP tuple, collector IDs, fit or score is read.
Theta/phi half-window defaults are 25/200 mrad; CLI overrides are supported.
Neutral selection follows zero-charge trackless PFO links, positive finite
energy, nonempty hits and valid positions. Absolute theta and wrapped phi
differences must be within the half-windows. A cluster shared between selected
legs is added once to the event. No truth classification, energy cap or purity
selection is introduced. Photon p4 uses energy and the direction from nominal
IP to the measured cluster position. JSON preserves IDs by track and union,
photon p4, bare/recovered recoil masses at 240 GeV and input size/mtime records.
Existing outputs are refused and changing input identities fail the run.

The reader checks event count/numbering and all five first-CompleteTrack
Baseline KF parameters against REC to catch misaligned inputs. A REC replay
preserving tracking may supply references even when PFA clusters differ:
measurements ALWAYS come from the GSF snapshot. Requiring identical REC/GSF
ECAL snapshots was tried and rejected as unnecessarily strict for this split;
it rejected event 2 of source 1 because its replayed PFA differs. No cluster
label/energy is transferred from REC. The baseline check is an event-alignment
check, not a complete provenance hash of every selected original track.
Missing production legs/ECAL references and invalid GSF pairs are explicit
unavailable statuses, rather than substituted KF fits.

Validation on all 200 entries of GSF_MATCHBP source 1 with current
REC_ECALTRUTH source 1: 178 recovered pairs, 21 unavailable GSF pairs, 1 missing
production leg. Negative selector placeholder at event 130 remains unavailable.
Every first-Baseline-KF state matches REC. The theta-10 control selects 96
unique event-clusters, theta-25 selects 104; every control set is a subset of
its theta-25 set, energies are nondecreasing, and all bare GSF masses/statuses
remain unchanged. Four focused tests cover window defaults/override, phi wrap,
shared-cluster deduplication, charged/no-hit exclusion, malformed inputs,
empty collection, photon p4 and recoil arithmetic. No BP input is used.

Artifacts: `TrackingPerformanceStudies/gsf_neutral_window25_20261113/`, including
both 200-entry JSONs, test log, implementation hash and validation summary.
The Python analysis/helper and its focused test remain uncommitted analysis
artifacts under the project policy. READMEs and AGENTS record the new entry point.
No C++, fit property/card, default-off ECAL re-ranking prototype, tuple schema,
build/install, job submission or gallery plot changed. Existing plotting
scripts still use their historical BP-derived IDs; new plots must explicitly
use this independent output. These are mechanical checks, not performance
validation or evidence of better recoil resolution.
