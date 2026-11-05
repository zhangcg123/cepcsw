#!/bin/bash
# Inclusive eeHX: first 200 events of files 0001-0500 = 100,000 events.
script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd) || exit 1
eehx_stdhep_pattern='/cefs/higgs/zhangkl/stdhep/E240/higgs/E240.Pe1e1h_X.e0.p0.whizard195/e1e1h_X.e0.p0.{seed:04d}.stdhep'
export STDHEP_INPUT_PATTERN=${STDHEP_INPUT_PATTERN:-$eehx_stdhep_pattern}
export SAMPLE_REGION=higgs PARTICLES=e-
export NEVT=${NEVT:-200} SEED_FIRST=${SEED_FIRST:-1} SEED_LAST=${SEED_LAST:-500}
export STAGES=sim,trk,calodigi,rec,breakpoint
export SUBMIT_GROUP=${SUBMIT_GROUP:-cms}
export OUTPUT_TUPLEPATH=${OUTPUT_TUPLEPATH:-higgs_eeHX_full_20261105}
# Preserve the full reconstructed event; refit/scan only the production pair.
export BP_SELECT_TRUTH_PRODUCTION_ELECTRONS=${BP_SELECT_TRUTH_PRODUCTION_ELECTRONS:-1}
if [ "${1:-prepare}" = submit-prepared ]; then
    shift
    if [ "$#" -ne 0 ]; then echo 'Usage: ./subbreakpointhiggsjobs.sh [prepare|submit-prepared]' >&2; exit 2; fi
    set -- submit "${OUTPUT_TUPLEPATH}"
    export DRY_RUN=${DRY_RUN:-0}
else
    export DRY_RUN=${DRY_RUN:-1}
fi
exec "${script_dir}/subbreakpointjobs.sh" "$@"
