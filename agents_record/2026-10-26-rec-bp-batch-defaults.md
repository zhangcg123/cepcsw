# SIM-reuse rec and breakpoint batch defaults

User asked to put the previously proposed three overrides into subbreakpointjobs.sh.
Defaults now: STAGES=trk,calodigi,rec,breakpoint;
OUTPUT_TUPLEPATH=rec_bp_barrel_20261026;
BP_CHARGED_ECAL_ENERGY_MODE=ElectronPhoton.
Environment overrides remain supported. Input sim_large_barrel_20261018,
seeds1..500, NEVT200, beam/charged/neutral111, sigma_b=.001 and group cms unchanged.
The new output path was absent in the readiness check; existing output refusal
is unchanged. No job or campaign cards prepared/submitted, no files deleted.

Updated the wrapper's preparation test to expect the requested stage chain and
frozen ElectronPhoton setting. Bash syntax passes; all 44 batch unit tests pass.
No C++ changes or rebuild/install needed. Standalone-card/compiled mode remains
ElectronOnly. Current status synchronized, full outgoing AGENTS archived in
2026-10-26-agents-before-rec-bp-batch-defaults.md; global instructions unchanged.
