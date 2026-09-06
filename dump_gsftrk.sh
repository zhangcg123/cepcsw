#!/bin/bash

if [ "$#" -lt 10 ] || [ "$#" -gt 11 ]; then
    echo "Usage: $0 particle momentum_mag momentum_trn theta seed nevt truth_bh_override output_tuplepath input_tuplepath stages [bh_model]" >&2
    exit 2
fi

particle=$1
momenta_mag=$2
momenta_trn=$3
theta=$4
seed=$5
nevt=$6
truth_bh_override=$7
output_tuplepath=$8
input_tuplepath=$9
stages=${10}
bh_model=${11:-CEPCRuntimeCategoryAligned9Clear}

run_sim=false
run_trk=false
run_gsf=false
case ",${stages}," in *,sim,*) run_sim=true ;; esac
case ",${stages}," in *,trk,*) run_trk=true ;; esac
case ",${stages}," in *,gsf,*) run_gsf=true ;; esac

WORKDIR=${CEPCSW_GSFDEV_DIR:-/aifs/user/data/zhangcg/gsfdev/CEPCSW}
jobpath=${WORKDIR}/DumpGsfTrks
sample=${particle}-${momenta_trn}-${theta}-${seed}
if [ "${truth_bh_override}" = true ]; then
    truthsuffix=-truth-bh
    truth_bh_python=True
else
    truthsuffix=-truth-bh-off
    truth_bh_python=False
fi

# Newly executed stages always write under output_tuplepath. A selected stage
# consumes its predecessor from there when that predecessor also runs;
# otherwise it reads the required existing input from input_tuplepath.
output_tupledir=${WORKDIR}/${output_tuplepath}
mkdir -p "${output_tupledir}"
runcard_dir=${output_tupledir}/runcards
mkdir -p "${runcard_dir}"

simcard=${runcard_dir}/runsim-${sample}.py
trkcard=${runcard_dir}/runtrk-${sample}.py

simname=sim-${sample}.root
trkname=trk-${sample}.root
if [ "${run_sim}" = true ]; then
    simfile=${output_tupledir}/${simname}
else
    simfile=${WORKDIR}/${input_tuplepath}/${simname}
fi
if [ "${run_trk}" = true ]; then
    trkfile=${output_tupledir}/${trkname}
else
    trkfile=${WORKDIR}/${input_tuplepath}/${trkname}
fi

# These cards are single-event-loop jobs. Avoid BLAS/OpenMP thread expansion
# exhausting the batch/account process limit.
job_threads=${CEPCSW_JOB_THREADS:-1}
export OPENBLAS_NUM_THREADS=${job_threads}
export OMP_NUM_THREADS=${job_threads}
export MKL_NUM_THREADS=${job_threads}
export BLIS_NUM_THREADS=${job_threads}
export NUMEXPR_NUM_THREADS=${job_threads}

cd "${WORKDIR}"
source setup.sh
set -u

if [ "${run_sim}" = true ]; then
    if [ -e "${simfile}" ]; then
        echo "Refusing to overwrite existing simulation output: ${simfile}" >&2
        exit 1
    fi
    cp "${jobpath}/sim.py.bk" "${simcard}"
    sed -i "s#tuplepath = \"\"#tuplepath = \"${output_tuplepath}\"#g" "${simcard}"
    sed -i "s/particlename = 'mu-'/particlename = '${particle}'/g" "${simcard}"
    sed -i "s/inputseed = 12340/inputseed = ${seed}/g" "${simcard}"
    sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" "${simcard}"
    sed -i "s#sim_v01.root#${simname}#g" "${simcard}"
    ./run.sh "${simcard}"
    sim_status=$?
    if [ "${sim_status}" -ne 0 ] || [ ! -s "${simfile}" ]; then
        echo "Simulation failed or produced no output: ${simfile}" >&2
        exit 1
    fi
fi

if [ "${run_trk}" = true ]; then
    if [ ! -s "${simfile}" ]; then
        echo "Missing simulation input: ${simfile}" >&2
        exit 1
    fi
    echo "Reading simulation input: ${simfile}"
    if [ -e "${trkfile}" ]; then
        echo "Refusing to overwrite existing tracker output: ${trkfile}" >&2
        exit 1
    fi
    cp "${jobpath}/trk.py.bk" "${trkcard}"
    sed -i "s#tuplepath = \"\"#tuplepath = \"${output_tuplepath}\"#g" "${trkcard}"
    sed -i "s/inputseed = 12340/inputseed = ${seed}/g" "${trkcard}"
    sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" "${trkcard}"
    sed -i "s#sim_v01.root#${simfile}#g" "${trkcard}"
    sed -i "s#rec_v01.root#${trkname}#g" "${trkcard}"
    sed -i "s#Digi_MUON.root#Digi_MUON-${sample}.root#g" "${trkcard}"
    ./run.sh "${trkcard}"
    trk_status=$?
    if [ "${trk_status}" -ne 0 ] || [ ! -s "${trkfile}" ]; then
        echo "Tracking failed or produced no output: ${trkfile}" >&2
        exit 1
    fi
fi

if [ "${run_gsf}" = true ]; then
    if [ ! -s "${trkfile}" ]; then
        echo "Missing tracker GSF input: ${trkfile}" >&2
        exit 1
    fi
    gsfcard=${runcard_dir}/rungsf-${sample}${truthsuffix}.py
    cp "${jobpath}/gsf.py.bk" "${gsfcard}"
    sed -i "s#tuplepath = \"\"#tuplepath = \"${output_tuplepath}\"#g" "${gsfcard}"
    sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" "${gsfcard}"
    sed -i "s/inputseed = 12340/inputseed = ${seed}/g" "${gsfcard}"
    sed -i "s/particle = 12340/particle = '${particle}'/g" "${gsfcard}"
    sed -i "s#trk_v01.root#${trkfile}#g" "${gsfcard}"
    sed -i \
        "s/^gsf.TruthBHLossOverride = .*/gsf.TruthBHLossOverride = ${truth_bh_python}/" \
        "${gsfcard}"
    if [ -n "${bh_model}" ]; then
        sed -i "s/^bh_model = .*/bh_model = \"${bh_model}\"/" "${gsfcard}"
    fi
    ./run.sh "${gsfcard}"
    gsf_status=$?
    if [ "${gsf_status}" -ne 0 ]; then
        echo "GSF pass ${truthsuffix} failed" >&2
        exit 1
    fi

    # The maintained GSF card writes only its flat tuple. Confirm that durable
    # result before removing a tracker tuple produced by this same job. Never
    # delete a shared tracker tuple supplied by input_tuplepath to a gsf-only
    # job.
    gsf_method=$(sed -n 's/^method = "\([^"]*\)"/\1/p' "${gsfcard}" | head -n 1)
    gsf_flat_output=${output_tupledir}/gsf_flat_${particle}_${gsf_method}${truthsuffix}_${seed}.root
    if [ -z "${gsf_method}" ] || [ ! -s "${gsf_flat_output}" ]; then
        echo "GSF completed but its flat output is incomplete; retaining tracker input: ${trkfile}" >&2
        exit 1
    fi
    if [ "${run_trk}" = true ]; then
        if ! rm -- "${trkfile}"; then
            echo "Failed to remove intermediate tracker tuple: ${trkfile}" >&2
            exit 1
        fi
        echo "Removed intermediate tracker tuple after verified GSF outputs: ${trkfile}"
    else
        echo "Retaining external tracker input used by gsf-only stage: ${trkfile}"
    fi
fi
