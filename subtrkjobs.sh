#!/bin/bash

# Batch submission controls. Edit these values here or override the scalar
# controls through the environment when starting a campaign.
export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:${PATH}
WORKDIR=${CEPCSW_GSFDEV_DIR:-/aifs/user/data/zhangcg/gsfdev/CEPCSW}

NEVT=${NEVT:-200}
SEED_FIRST=${SEED_FIRST:-1}
SEED_LAST=${SEED_LAST:-500}
TRUTH_BH_OVERRIDE=${TRUTH_BH_OVERRIDE:-false}
INPUT_TUPLEPATH=${INPUT_TUPLEPATH:-sim_large_20260823}
OUTPUT_TUPLEPATH=${OUTPUT_TUPLEPATH:-sim_large_endcap_20260905}
STAGES=${STAGES:-sim}
BH_MODEL=${BH_MODEL:-CEPCRuntimeCategoryAligned9Clear}
MEMORY_MB=${MEMORY_MB:-5000}

# Physics scan controls are deliberately maintained in this script.
particles=(e-)
thetas=(85)
transverse_momenta=(2.0)

usage_error() {
    echo "subtrkjobs.sh: $*" >&2
    exit 2
}

if ! [[ "${NEVT}" =~ ^[1-9][0-9]*$ ]]; then
    usage_error "NEVT must be a positive integer"
fi
if ! [[ "${SEED_FIRST}" =~ ^[0-9]+$ && "${SEED_LAST}" =~ ^[0-9]+$ ]]; then
    usage_error "SEED_FIRST and SEED_LAST must be non-negative integers"
fi
if (( SEED_FIRST > SEED_LAST )); then
    usage_error "SEED_FIRST must not exceed SEED_LAST"
fi
if ! [[ "${MEMORY_MB}" =~ ^[1-9][0-9]*$ ]]; then
    usage_error "MEMORY_MB must be a positive integer"
fi

case "${TRUTH_BH_OVERRIDE,,}" in
    1|true|yes|on)
        TRUTH_BH_OVERRIDE=true
        log_suffix=_truth-bh
        ;;
    0|false|no|off)
        TRUTH_BH_OVERRIDE=false
        log_suffix=_truth-bh-off
        ;;
    *)
        usage_error "TRUTH_BH_OVERRIDE must be true or false"
        ;;
esac

if [ -z "${INPUT_TUPLEPATH}" ] || [ -z "${OUTPUT_TUPLEPATH}" ]; then
    usage_error "INPUT_TUPLEPATH and OUTPUT_TUPLEPATH must both be non-empty"
fi
if [ "${INPUT_TUPLEPATH}" = "${OUTPUT_TUPLEPATH}" ]; then
    usage_error "INPUT_TUPLEPATH and OUTPUT_TUPLEPATH must differ"
fi

for path_name in INPUT_TUPLEPATH OUTPUT_TUPLEPATH; do
    path_value=${!path_name}
    case "${path_value}" in
        /*) usage_error "${path_name} must be relative to WORKDIR" ;;
    esac
    if [[ "/${path_value}/" == *"/../"* ]]; then
        usage_error "${path_name} must not contain a .. path component"
    fi
done

IFS=',' read -r -a stage_list <<< "${STAGES}"
if [ "${#stage_list[@]}" -eq 0 ]; then
    usage_error "STAGES must select at least one of sim, trk, or gsf"
fi
declare -A seen_stages=()
for stage in "${stage_list[@]}"; do
    case "${stage}" in
        sim|trk|gsf) ;;
        *) usage_error "unknown STAGES entry: ${stage}" ;;
    esac
    if [ -n "${seen_stages[${stage}]:-}" ]; then
        usage_error "duplicate STAGES entry: ${stage}"
    fi
    seen_stages[${stage}]=1
done

case "${BH_MODEL}" in
    CEPCRuntimeCategoryAligned9Clear|ActsAtlas) ;;
    *) usage_error "unknown BH_MODEL: ${BH_MODEL}" ;;
esac

if ! command -v hep_sub >/dev/null 2>&1; then
    usage_error "hep_sub is not available"
fi
if ! command -v bc >/dev/null 2>&1; then
    usage_error "bc is not available"
fi
if [ ! -x "${WORKDIR}/dump_gsftrk.sh" ]; then
    usage_error "worker is missing or not executable: ${WORKDIR}/dump_gsftrk.sh"
fi

log_dir=${WORKDIR}/${OUTPUT_TUPLEPATH}/outlog
if ! mkdir -p "${log_dir}"; then
    usage_error "cannot create log directory: ${log_dir}"
fi
if ! cd "${WORKDIR}"; then
    usage_error "cannot enter WORKDIR: ${WORKDIR}"
fi

echo "Submitting GSF campaign:"
echo "  stages=${STAGES}"
echo "  input=${INPUT_TUPLEPATH}"
echo "  output=${OUTPUT_TUPLEPATH}"
echo "  truth-BH=${TRUTH_BH_OVERRIDE}"
echo "  BH-model=${BH_MODEL}"
echo "  events/job=${NEVT}, seeds=${SEED_FIRST}-${SEED_LAST}"

submission_failures=0
for particle in "${particles[@]}"; do
    for ((seed=SEED_FIRST; seed<=SEED_LAST; seed++)); do
        for theta in "${thetas[@]}"; do
            for trans_mom in "${transverse_momenta[@]}"; do
                mom=$(echo "scale=8; ${trans_mom} / s(${theta} * 3.141592653589793 / 180)" | bc -l)
                submit=(
                    hep_sub dump_gsftrk.sh
                    -g cms
                    -mem "${MEMORY_MB}"
                    -o "${log_dir}/job_${particle}_${trans_mom}_${theta}_${seed}${log_suffix}.out"
                    -e "${log_dir}/job_${particle}_${trans_mom}_${theta}_${seed}${log_suffix}.err"
                    -argu
                    "${particle}"
                    "${mom}"
                    "${trans_mom}"
                    "${theta}"
                    "${seed}"
                    "${NEVT}"
                    "${TRUTH_BH_OVERRIDE}"
                    "${OUTPUT_TUPLEPATH}"
                    "${INPUT_TUPLEPATH}"
                    "${STAGES}"
                    "${BH_MODEL}"
                )
                if ! "${submit[@]}"; then
                    echo "Submission failed: particle=${particle} pT=${trans_mom} theta=${theta} seed=${seed}" >&2
                    submission_failures=$((submission_failures + 1))
                fi
            done
        done
    done
done

if (( submission_failures > 0 )); then
    echo "${submission_failures} job submission(s) failed" >&2
    exit 1
fi
