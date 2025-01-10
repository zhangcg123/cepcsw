import os, sys
from Gaudi.Configuration import *

########### k4DataSvc ####################
from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc", input="Rec_TDR_o1_v01.root")
##########################################

########## CEPCSWData ################# 
cepcswdatatop ="/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/data/latest"
#######################################


########## Podio Input ###################
from Configurables import PodioInput
inp = PodioInput("InputReader")
inp.collections = [ "CyberPFO", "CyberPFOPID", "MCParticle" ]
##########################################



from Configurables import GenMatch
genmatch = GenMatch("GenMatch")
genmatch.InputPFOs = "CyberPFOPID"
genmatch.nJets = 2
genmatch.R = 0.6
genmatch.OutputFile = "Jets_TDR_o1_v01.root"
#genmatch.OutputFile = "./FullSim_samples/RecJets_TDR_o1_v01_E240_nnh_gg_CalHits.root"

##############################################################################
# POD I/O
##############################################################################


########################################

from Configurables import ApplicationMgr
ApplicationMgr( 
    TopAlg=[inp, genmatch ],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[podioevent],
    #OutputLevel=DEBUG
)
