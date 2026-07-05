#!/bin/bash

particle=$1
momenta_mag=$2
momenta_trn=$3
theta=$4
seed=$5
nevt=$6

cd /cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/

source setup.sh

jobpath=/cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/DumpGsfTrks/

momenta_low=${momenta_trn}
momenta_hig=${momenta_mag}

# ── Stage 1: Simulation ──
cp ${jobpath}/sim.py.bk ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/particlename = 'mu-'/particlename = '${particle}'/g" ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/inputseed = 12340/inputseed = ${seed}/g" ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/sim_v01.root/sim-${particle}-${momenta_low}-${theta}-${seed}.root/g" ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/momenta_low = 12340/momenta_low = ${momenta_hig}/g" ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/theta_low = 12340/theta_low = ${theta}/g" ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py
./run.sh ${jobpath}/runsim-${particle}-${momenta_low}-${theta}-${seed}.py

# ── Stage 2: Digitization + Tracking → CompleteTracks ──
cp ${jobpath}/trk.py.bk ${jobpath}/runtrk-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/sim_v01.root/sim-${particle}-${momenta_low}-${theta}-${seed}.root/g" ${jobpath}/runtrk-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/inputseed = 12340/inputseed = ${seed}/g" ${jobpath}/runtrk-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" ${jobpath}/runtrk-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/rec_v01.root/trk-${particle}-${momenta_low}-${theta}-${seed}.root/g" ${jobpath}/runtrk-${particle}-${momenta_low}-${theta}-${seed}.py
./run.sh ${jobpath}/runtrk-${particle}-${momenta_low}-${theta}-${seed}.py

# ── Stage 3: GSF refit ──
cp ${jobpath}/gsf.py.bk ${jobpath}/rungsf-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/trk_v01.root/trk-${particle}-${momenta_low}-${theta}-${seed}.root/g" ${jobpath}/rungsf-${particle}-${momenta_low}-${theta}-${seed}.py
#sed -i "s/evtmax = 12340/evtmax = ${nevt}/g" ${jobpath}/rungsf-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/gsf_v01.root/gsf-${particle}-${momenta_low}-${theta}-${seed}.root/g" ${jobpath}/rungsf-${particle}-${momenta_low}-${theta}-${seed}.py
sed -i "s/gsf_flat_v01.root/gsf_flat-${particle}-${momenta_low}-${theta}-${seed}.root/g" ${jobpath}/rungsf-${particle}-${momenta_low}-${theta}-${seed}.py
./run.sh ${jobpath}/rungsf-${particle}-${momenta_low}-${theta}-${seed}.py
