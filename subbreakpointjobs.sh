#!/bin/bash
# Independent breakpoint campaign. Existing GSF scripts/cards are not modified.
# Set campaign constraints/loss sigma below; other fit settings in run_breakpoint.py.
export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:${PATH}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
export CEPCSW_BREAKPOINT_DIR=${CEPCSW_BREAKPOINT_DIR:-${script_dir}}
export NEVT=${NEVT:-200}
export SEED_FIRST=${SEED_FIRST:-1}
export SEED_LAST=${SEED_LAST:-500}
# Refit existing REC files; retain the previous flat tuples for comparison.
export INPUT_TUPLEPATH=${INPUT_TUPLEPATH:-sim_large_barrel_20261026}
export OUTPUT_TUPLEPATH=${OUTPUT_TUPLEPATH:-breakpoint_delayed_neutral_20261028}
export STAGES=${STAGES:-breakpoint}
# Full-chain jobs retain simulation geometry and PID models in one process.
export MEMORY_MB=${MEMORY_MB:-8000}
# hep_sub -g group, used for both new and already-prepared jobs.
export SUBMIT_GROUP=${SUBMIT_GROUP:-higgs}
export DRY_RUN=${DRY_RUN:-0}
# Prior sigma of b=-log(z), shared by ordinary, free-loss and truth-assisted fits.
# Must be finite and positive. Frozen into generated cards at preparation.
export BP_SIGMA_LOG_LOSS=${BP_SIGMA_LOG_LOSS:-0.001}
# Optional additional location scans; truth-location approaches remain enabled.
# Default110 scans only110; explicitly empty runs no scans. Use Truth mode.
export BP_LOCATION_SCAN_SETUPS=${BP_LOCATION_SCAN_SETUPS-110}
export BP_FREE_LOSS_FIT=${BP_FREE_LOSS_FIT:-1}
# Optional truth-assisted eeH production-pair gate; 0 preserves all input tracks.
export BP_SELECT_TRUTH_PRODUCTION_ELECTRONS=${BP_SELECT_TRUTH_PRODUCTION_ELECTRONS:-0}
# Default beam/charged/neutral = 110; enable the separate-neutral stage explicitly.
export BP_BEAM_SPOT_CONSTRAINT=${BP_BEAM_SPOT_CONSTRAINT:-1}
export BP_CHARGED_ECAL_MEASUREMENT=${BP_CHARGED_ECAL_MEASUREMENT:-1}
export BP_NEUTRAL_ECAL_MEASUREMENT=${BP_NEUTRAL_ECAL_MEASUREMENT:-0}
# Campaign-only: delayed neutral update for free-loss RTS when a neutral is available.
export BP_EXPERIMENTAL_DELAYED_NEUTRAL=${BP_EXPERIMENTAL_DELAYED_NEUTRAL:-1}
# 110: ElectronOnly uses charged energy; ElectronPhoton adds collected neutral energy.
export BP_CHARGED_ECAL_ENERGY_MODE=${BP_CHARGED_ECAL_ENERGY_MODE:-ElectronPhoton}
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
