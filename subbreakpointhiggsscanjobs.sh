#!/bin/bash
# Silicon location scan of the first 10,000 existing Higgs events; no sim/rec rerun.
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
export SAMPLE_REGION=higgs
export INPUT_TUPLEPATH=${INPUT_TUPLEPATH:-higgs_full_20261030}
export OUTPUT_TUPLEPATH=${OUTPUT_TUPLEPATH:-higgs_silicon_scan_20261103}
export STAGES=breakpoint
export NEVT=200 SEED_FIRST=1 SEED_LAST=50
export SUBMIT_GROUP=${SUBMIT_GROUP:-higgs}
export BP_INTERVAL_SELECTION_MODE=HitScan BP_INTERVALS=
export BP_HIT_SCAN_MIN_NLL_GAIN=0 BP_FREE_LOSS_FIT=1 BP_TRUTH_OVERRIDE=0
export BP_BEAM_SPOT_CONSTRAINT=1 BP_CHARGED_ECAL_MEASUREMENT=1 BP_NEUTRAL_ECAL_MEASUREMENT=0
export BP_CHARGED_ECAL_ENERGY_MODE=ElectronPhoton BP_EXPERIMENTAL_DELAYED_NEUTRAL=0
export BP_ECAL_CONSTRAINT_INPUT=PreReclustering BP_CHARGED_ECAL_INPUT_CLUSTERS=
export BP_DIFFUSE_AUGMENTED_RTS=0 BP_VERBOSE=0 BP_SELECTED=
export BP_ORDINARY_BACKWARD_FILTER=0 BP_FREE_LOSS_BACKWARD_FILTER=0 BP_TRUTH_BACKWARD_FILTER=0
export BP_MEAN_LOG_LOSS=0 BP_SIGMA_LOG_LOSS=0.001
export BP_FREE_LOSS_MAX_LOG_LOSS=1 BP_FREE_LOSS_MAX_CALLS=180 BP_FREE_LOSS_TOLERANCE=0.001
# Preparing is dry by default; submit-prepared is an explicit submission request.
if [ "${1:-}" = submit-prepared ]; then
    export DRY_RUN=${DRY_RUN:-0}
    exec "${script_dir}/subbreakpointjobs.sh" submit "${OUTPUT_TUPLEPATH}"
fi
export DRY_RUN=${DRY_RUN:-1}
exec "${script_dir}/subbreakpointjobs.sh" "$@"
