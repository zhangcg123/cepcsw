#!/bin/bash
export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:$PATH
WORKDIR=${CEPCSW_GSFDEV_DIR:-/aifs/user/data/zhangcg/gsfdev/CEPCSW}

NEVT=100
TRUTH_BH_OVERRIDE=${TRUTH_BH_OVERRIDE:-false}
case "${TRUTH_BH_OVERRIDE,,}" in
	1|true|yes|on) LOG_SUFFIX=_truth-bh ;;
	*) LOG_SUFFIX= ;;
esac

for particle in e-
do
	for seed in {0..10} #500} #500}
	do
		for theta in 85 #135
		do
			for trans_mom in 2.0 #,1.0,2.0}
			do
				mom=$(echo "scale=3; $trans_mom / (s($theta * 3.1415926 / 180))" | bc -l)
				hep_sub dump_gsftrk.sh -g higgs -o ${WORKDIR}/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}${LOG_SUFFIX}.out -e ${WORKDIR}/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}${LOG_SUFFIX}.err -argu ${particle} ${mom} ${trans_mom} ${theta} ${seed} ${NEVT} ${TRUTH_BH_OVERRIDE}
			done
		done
	done
done
