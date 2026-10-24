#!/bin/bash
# Independent breakpoint campaign. Existing GSF scripts/cards are not modified.
# Set campaign constraints/loss sigma below; other fit settings in run_breakpoint.py.
export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:${PATH}
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
export CEPCSW_BREAKPOINT_DIR=${CEPCSW_BREAKPOINT_DIR:-${script_dir}}
export NEVT=${NEVT:-200}
export SEED_FIRST=${SEED_FIRST:-1}
export SEED_LAST=${SEED_LAST:-500}
export INPUT_TUPLEPATH=${INPUT_TUPLEPATH:-sim_large_barrel_20261018}
export OUTPUT_TUPLEPATH=${OUTPUT_TUPLEPATH:-bp_flat_barrel_20261018}
# Breakpoint-only by default, using existing REC inputs. For existing SIM,
# use STAGES=trk,calodigi,rec,breakpoint; prepend sim for a complete new chain.
# Retain SIM/REC/flat outputs; remove owned TRK/calodigi only after validation.
export STAGES=${STAGES:-breakpoint}
export MEMORY_MB=${MEMORY_MB:-5000}
export DRY_RUN=${DRY_RUN:-0}
# Prior sigma of b=-log(z), shared by ordinary, free-loss and truth-assisted fits.
# Must be finite and positive. Frozen into generated cards at preparation.
export BP_SIGMA_LOG_LOSS=${BP_SIGMA_LOG_LOSS:-0.001}
# This batch campaign defaults to cumulative beam/charged/neutral = 111.
export BP_BEAM_SPOT_CONSTRAINT=${BP_BEAM_SPOT_CONSTRAINT:-1}
export BP_CHARGED_ECAL_MEASUREMENT=${BP_CHARGED_ECAL_MEASUREMENT:-1}
export BP_NEUTRAL_ECAL_MEASUREMENT=${BP_NEUTRAL_ECAL_MEASUREMENT:-1}
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
