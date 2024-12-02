import os, sys
from Gaudi.Configuration import *

########### k4DataSvc ####################
from Configurables import k4DataSvc
#podioevent = k4DataSvc("EventDataSvc", input="./FullSim_Samples/nnHgg/Rec_TDR_o1_v01_E240_nnh_gg_test_003.root")
podioevent = k4DataSvc("EventDataSvc", input="/publicfs/cms/user/wangzebing/CEPC/CEPCSW_tdr24.9.1/CEPCSW/FullSim_samples/Rec_TDR_o1_v01_E240_nnh_gg.root")
##########################################

########## CEPCSWData ################# 
cepcswdatatop ="/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/data/latest"
#######################################


########## Podio Input ###################
from Configurables import PodioInput
inp = PodioInput("InputReader")
inp.collections = [ "CyberPFO", "MCParticle" ]
##########################################



from Configurables import GenMatch
genmatch = GenMatch("GenMatch")
genmatch.nJets = 2
genmatch.R = 0.6
genmatch.OutputFile = "./RecJets_TDR_o1_v01_E240_nnh_gg.root"
#genmatch.OutputFile = "./FullSim_samples/RecJets_TDR_o1_v01_E240_nnh_gg_CalHits.root"

##############################################################################
# POD I/O
##############################################################################


########################################

from Configurables import ApplicationMgr
ApplicationMgr( 
    TopAlg=[inp, genmatch ],
    EvtSel="NONE",
    EvtMax=3,
    ExtSvc=[podioevent],
    #OutputLevel=DEBUG
)
