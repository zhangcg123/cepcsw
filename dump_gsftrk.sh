#!/bin/bash

if [ "$#" -lt 6 ] || [ "$#" -gt 8 ]; then
    echo "Usage: $0 particle momentum_mag momentum_trn theta seed nevt [truth_bh_override] [gsf_only]" >&2
    exit 2
fi

particle=$1
momenta_mag=$2
momenta_trn=$3
theta=$4
seed=$5
nevt=$6
truth_bh_override=${7:-false}
case "${truth_bh_override,,}" in
    1|true|yes|on) truth_bh_override=true ;;
    0|false|no|off) truth_bh_override=false ;;
    *)
        echo "truth_bh_override must be true/false, yes/no, on/off, or 1/0" >&2
        exit 2
        ;;
esac
gsf_only=${8:-false}
case "${gsf_only,,}" in
    1|true|yes|on) gsf_only=true ;;
    0|false|no|off) gsf_only=false ;;
    *)
        echo "gsf_only must be true/false, yes/no, on/off, or 1/0" >&2
        exit 2
        ;;
esac

WORKDIR=${CEPCSW_GSFDEV_DIR:-/aifs/user/data/zhangcg/gsfdev/CEPCSW}
jobpath=${WORKDIR}/DumpGsfTrks
sample=${particle}-${momenta_trn}-${theta}-${seed}
if [ "${truth_bh_override}" = true ]; then
    truthsuffix=-truth-bh
else
    truthsuffix=-truth-bh-off
fi

# Common ROOT-output location for every processing stage.  Leave empty to
# write directly under WORKDIR; set to a relative path to use a subdirectory.
tuplepath=""
if [ -n "${tuplepath}" ]; then
    tupledir=${WORKDIR}/${tuplepath}
else
    tupledir=${WORKDIR}
fi
mkdir -p "${tupledir}"

simcard=${jobpath}/runsim-${sample}.py
trkcard=${jobpath}/runtrk-${sample}.py
gsfcard=${jobpath}/rungsf-${sample}${truthsuffix}.py

simname=sim-${sample}.root
trkname=trk-${sample}.root
simfile=${tupledir}/${simname}
trkfile=${tupledir}/${trkname}

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

if [ "${gsf_only}" = false ]; then
    # Stage 1: simulation
    cp "${jobpath}/sim.py.bk" "${simcard}"
    sed -i "s#tuplepath = \"\"#tuplepath = \"${tuplepath}\"#g" "${simcard}"
    sed -i "s/particlename = 'mu-'/particlename = '${particle}'/g" "${simcard}"
    sed -i "s/inputseed = 12340/inputseed = ${seed}/g" "${simcard}"
    sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" "${simcard}"
    sed -i "s#sim_v01.root#${simname}#g" "${simcard}"
    ./run.sh "${simcard}"

    # Stage 2: tracker digitization and reconstruction
    cp "${jobpath}/trk.py.bk" "${trkcard}"
    sed -i "s#tuplepath = \"\"#tuplepath = \"${tuplepath}\"#g" "${trkcard}"
    sed -i "s/inputseed = 12340/inputseed = ${seed}/g" "${trkcard}"
    sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" "${trkcard}"
    sed -i "s#sim_v01.root#${simname}#g" "${trkcard}"
    sed -i "s#rec_v01.root#${trkname}#g" "${trkcard}"
    ./run.sh "${trkcard}"
else
    echo "GSF-only control: reusing ${trkfile}"
fi

# Stage 3: GSF track refit directly from tracking output
if [ ! -s "${trkfile}" ]; then
    echo "Missing tracker GSF input: ${trkfile}" >&2
    exit 1
fi
cp "${jobpath}/gsf.py.bk" "${gsfcard}"
sed -i "s#tuplepath = \"\"#tuplepath = \"${tuplepath}\"#g" "${gsfcard}"
sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" "${gsfcard}"
sed -i "s/inputseed = 12340/inputseed = ${seed}/g" "${gsfcard}"
sed -i "s/particle = 12340/particle = '${particle}'/g" "${gsfcard}"
sed -i "s#trk_v01.root#${trkname}#g" "${gsfcard}"
if [ "${truth_bh_override}" = true ]; then
    truth_bh_python=True
else
    truth_bh_python=False
fi
sed -i \
    "s/^gsf.TruthBHLossOverride = .*/gsf.TruthBHLossOverride = ${truth_bh_python}/" \
    "${gsfcard}"
./run.sh "${gsfcard}"
