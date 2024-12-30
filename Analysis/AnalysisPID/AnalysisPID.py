import os, sys
from Gaudi.Configuration import *

########### k4DataSvc ####################
from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc", input="track.root")
##########################################

########## CEPCSWData ################# 
cepcswdatatop ="/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/data/latest"
#######################################


########## Podio Input ###################
from Configurables import PodioInput
inp = PodioInput("InputReader")
inp.collections = [ "CompleteTracks", 
                    "CompleteTracksParticleAssociation",
                    "RecTofCollection",
                    "DndxTracks" ]
##########################################

from Configurables import AnalysisPIDAlg
anaPID = AnalysisPIDAlg("AnalysisPIDAlg")
anaPID.OutputFile = "./pid.root"

##############################################################################
# POD I/O
##############################################################################


########################################

from Configurables import ApplicationMgr
ApplicationMgr( 
    TopAlg=[inp, anaPID ],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[podioevent],
    #OutputLevel=DEBUG
)
