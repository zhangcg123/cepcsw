import os, sys
from Gaudi.Configuration import *

########### k4DataSvc ####################
from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc", input="Rec_TDR_o1_v01_E240_nnHgg.root")
##########################################

########## CEPCSWData ################# 
cepcswdatatop ="/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/data/latest"
#######################################

########## Podio Input ###################
from Configurables import PodioInput
inp = PodioInput("InputReader")
inp.collections = [ "PandoraPFOs" ]
##########################################

from Configurables import JetClustering
jetclustering = JetClustering("JetClustering")
jetclustering.nJets = 2
jetclustering.R = 0.6
jetclustering.OutputFile = "Rec_JetClustering_TDR_o1_v01_E240_nnHgg.root"

########################################

from Configurables import ApplicationMgr
ApplicationMgr( 
    TopAlg=[inp, jetclustering ],
    EvtSel="NONE",
    EvtMax=100,
    ExtSvc=[podioevent],
    #OutputLevel=DEBUG
)