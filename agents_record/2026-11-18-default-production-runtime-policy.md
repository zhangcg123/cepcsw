# Default production runtime and independent collection — 2026-11-18

The user requires all campaigns to use the project's current default
installation. The previous Nov11 GSF worker explicitly prepended
build.gsf-ecal-tuple-dev/install/lib and its Python directory; thus its flat
writer came from that private development install. This is historical evidence,
not the intended policy for new production. Its deleted campaign's small runtime
provenance remains in the new campaign's previous-campaign-provenance.json.

The new GSF_BPNEUTRALS worker sources only workspace setup.sh, adds no private
LD_LIBRARY_PATH/PYTHONPATH overrides, and freezes the actual default InstallArea
GSF/flat/truth libraries and generated configurables. No implementation or
installation change is needed for this clarification. A direct loader/config
resolution check is saved as default-runtime-check.json/log in the prepared
campaign. The new default flat writer was already tested on the native focused
events in the preceding implementation record.

The user also clarifies that GSF and BP should follow the same collection logic,
not copy results from one another. The implementation already does this:
identical collector functions are separately compiled/executed in the GSF flat
writer on its own REC/PFO/cluster inputs. Its worker never opens a BP flat file;
BP manifest/output paths are provenance metadata only. Native BP fields are read
only by offline regression verification. The GSF algorithm never receives the
collected calorimeter observations as fit constraints.

AGENTS now records the user-authorized production-runtime law explicitly; this
is an intentional addition to its active laws, with all prior laws retained.
The current focus and authoritative GSF README explicitly state independent
collection. Production exceptions require explicit user authorization; an old
private writer used solely as an isolated regression control is not a campaign.
