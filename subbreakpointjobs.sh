#!/bin/bash
# Independent breakpoint campaign. Existing GSF scripts/cards are not modified.
# Set the shared loss sigma below; edit other fit settings in run_breakpoint.py.
export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:${PATH}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
export CEPCSW_BREAKPOINT_DIR=${CEPCSW_BREAKPOINT_DIR:-${script_dir}}
export NEVT=${NEVT:-200}
export SEED_FIRST=${SEED_FIRST:-1}
export SEED_LAST=${SEED_LAST:-100}
export INPUT_TUPLEPATH=${INPUT_TUPLEPATH:-sim_large_barrel_20261001}
export OUTPUT_TUPLEPATH=${OUTPUT_TUPLEPATH:-breakpoint_barrel}
# Preserve the reconstructed ECAL/PFO event tuple for later external eBrem
# studies. Existing simulation is reused unless sim is explicitly selected.
export STAGES=${STAGES:-breakpoint}
export MEMORY_MB=${MEMORY_MB:-5000}
export DRY_RUN=${DRY_RUN:-0}
# Prior sigma of b=-log(z), shared by ordinary, free-loss and truth-assisted fits.
# Must be finite and positive. Frozen into generated cards at preparation.
export BP_SIGMA_LOG_LOSS=${BP_SIGMA_LOG_LOSS:-0.001}
# Freeze the default-on parallel beam-guided free-loss objective in each job.
# The base free-loss outputs remain independent and unchanged.
export BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE=${BP_FREE_LOSS_BEAM_SPOT_OBJECTIVE:-1}
# New simulation files use sim-barrel-<seed>.root. SAMPLE_REGION controls that
# filename convention, and also the gun theta range if STAGES includes sim.
# Set SAMPLE_REGION='' to prepare an older momentum/theta-named campaign;
# THETAS and TRANSVERSE_MOMENTA are used only in that legacy mode.
export SAMPLE_REGION=${SAMPLE_REGION-barrel}
export PARTICLES=${PARTICLES:-e-}
export THETAS=${THETAS:-85}
export TRANSVERSE_MOMENTA=${TRANSVERSE_MOMENTA:-2.0}
if [ "$#" -eq 0 ]; then set -- prepare; fi
exec python3 "${CEPCSW_BREAKPOINT_DIR}/Reconstruction/RecBreakpoint/options/batch_breakpoint.py" "$@"
