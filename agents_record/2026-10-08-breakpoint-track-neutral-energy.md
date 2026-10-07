# Track-wise hit-supported neutral ECAL energy in the main tuple

The user requested that every main-tree track row save the hit-supported
neutral clusters selected for that particular track, rather than requiring
only an event-level side-tree lookup. This recording is unconditional for
selected input track rows, including ECAL refit Off, no breakpoint and failed
ordinary fits. The event-level neutral_pfos tree is retained.

## Implementation and schema

`collectNeutralLoss` is called once per track before the fit. Its selected
candidate is persisted and reused by an enabled ECAL refit; the selector,
window definitions, energy values and uncertainty formula are unchanged.
The window uses the input CompleteTracks AtCalorimeter reference point.
The defaults are |delta theta| <= 10 mrad and wrapped |delta phi| <= 200 mrad.
Candidates must belong to a neutral trackless PFO and EcalCluster, carry
calorimeter hits, and have valid positions and finite positive energies.
Duplicate cluster links are counted once within a track. Overlapping windows
on different tracks may select the same cluster; no new exclusive ownership
algorithm is introduced.

New branches in `breakpoint`, keyed by event_index and input_track_index:

- `collected_neutral_ecal_status`: 0 no input ECAL reference; 1 valid reference
  with no selected cluster; 2 selected clusters; -1 collection error.
- `collected_neutral_ecal_error`: exception message for status -1.
- `collected_neutral_ecal_cluster_count`: selected unique cluster count.
- `collected_neutral_ecal_cluster_indices`: indices into EcalCluster, joining
  the indices stored in the event-level neutral_pfos tree.
- `collected_neutral_ecal_cluster_energy`: aligned per-cluster energies, GeV.
- `collected_neutral_ecal_cluster_energy_error`: aligned per-cluster errors,
  GeV, using a*sqrt(E)+c*E with configured coefficients.
- `collected_neutral_ecal_energy`: summed selected energy, GeV.
- `collected_neutral_ecal_energy_error`: independent-cluster quadrature error,
  GeV.

No reference or no match gives empty vectors/count zero and zero energy/error;
status distinguishes the two. Errors give empty vectors/count zero and NaN
energy/error. Members reset per track, so a later track cannot retain another
track's selection. Existing absolute_neutral prior/cluster fields keep their
fit-specific semantics; no result or refit control is renamed here.

## Validation

A new native EDM unit test uses two tracks with different ECAL directions,
two clusters in one window and one in the other. It also checks synthetic
clusters, charged PFOs, track-associated PFOs, theta/phi rejection, duplicate
links, absent ECAL reference, empty window, energy sums and uncertainties.

Local event regression and validation artifacts are in
`/tmp/bp-track-neutral-fwebh4/`, compared with the preceding controller outputs
under `/tmp/bp-ecal-controller-0sAb5a/`. The rebuilt package passes all four
RecBreakpoint CTests and all 35 batch/card tests. Same-event checks cover six
tracks (seed 1 events 5, 11, 16, 17, 25 and seed 2 event 10) in each of the four
ECAL reference modes: 24 comparison rows. Every pre-existing main-tree field
and every event-level neutral_pfos field agrees with the preceding build.
The new collection fields agree across all four modes, including Off; vector
alignment, uniqueness, energy sums and quadrature errors pass. For example,
seed 1 event 5 records one cluster with 2.935391 GeV even with ECAL refitting
off, while events 11, 16 and 17 record valid empty windows (status 1).
The focused regression uses the historical 4% error configuration; the
native EDM unit test also checks the stochastic-plus-constant error formula
and distinct selections for multiple tracks.
The shared InstallArea is not updated by this change; no Condor jobs are run.
