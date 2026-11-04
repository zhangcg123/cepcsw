#!/bin/bash
# Scan all 200 events from all 249 existing Higgs REC files; no sim/rec rerun.
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
export SAMPLE_REGION=higgs
export INPUT_TUPLEPATH=${INPUT_TUPLEPATH:-higgs_full_20261030}
export OUTPUT_TUPLEPATH=${OUTPUT_TUPLEPATH:-higgs_location_scan_truth_20261104}
export STAGES=breakpoint
# rec-higgs-123.root is absent; 1..250 with that file skipped gives 249 jobs.
export NEVT=200 SEED_FIRST=1 SEED_LAST=250
export SUBMIT_GROUP=${SUBMIT_GROUP:-cms}
export BP_INTERVAL_SELECTION_MODE=Truth BP_INTERVALS=
export BP_LOCATION_SCAN_SETUPS=${BP_LOCATION_SCAN_SETUPS-110}
export BP_FREE_LOSS_FIT=${BP_FREE_LOSS_FIT:-1} BP_TRUTH_OVERRIDE=0
# Preserve truth-location endpoints; separately choose which approaches scan.
export BP_BEAM_SPOT_CONSTRAINT=${BP_BEAM_SPOT_CONSTRAINT:-1}
export BP_CHARGED_ECAL_MEASUREMENT=${BP_CHARGED_ECAL_MEASUREMENT:-1}
export BP_NEUTRAL_ECAL_MEASUREMENT=${BP_NEUTRAL_ECAL_MEASUREMENT:-0}
export BP_CHARGED_ECAL_ENERGY_MODE=${BP_CHARGED_ECAL_ENERGY_MODE:-ElectronPhoton}
export BP_EXPERIMENTAL_DELAYED_NEUTRAL=1
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
