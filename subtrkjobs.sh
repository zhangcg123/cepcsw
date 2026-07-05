#!/bin/bash
export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:$PATH
WORKDIR=${CEPCSW_GSFDEV_DIR:-/aifs/user/data/zhangcg/gsfdev/CEPCSW}

NEVT=1000

for particle in mu-
do
	for seed in {1..2}
	do
		for theta in 85 #135
		do
			for trans_mom in {0.5,1.0,2.0}
			do
				mom=$(echo "scale=3; $trans_mom / (s($theta * 3.1415926 / 180))" | bc -l)
				hep_sub dump_gsftrk.sh -mem 8000 -g cms -o ${WORKDIR}/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}.out -e ${WORKDIR}/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}.err -argu ${particle} ${mom} ${trans_mom} ${theta} ${seed} ${NEVT}
			done
		done
	done
done
