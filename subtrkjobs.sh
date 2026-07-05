#!/bin/bash
export PATH=/cvmfs/common.ihep.ac.cn/software/hepjob/bin:$PATH

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
				hep_sub dump_gsftrk.sh -mem 8000 -g cms -o /cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}.out -e /cefs/higgs/zhangcg/cepc/28Jun2026/CEPCSW/outlog/trk_${particle}_${trans_mom}_${theta}_${seed}.err -argu ${particle} ${mom} ${trans_mom} ${theta} ${seed} ${NEVT}
			done
		done
	done
done
