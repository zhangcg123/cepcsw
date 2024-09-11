import os, sys
from Gaudi.Configuration import *

############## GeomSvc #################
geometry_option = "TDR_o1_v01/TDR_o1_v01.xml"

if not os.getenv("DETCRDROOT"):
    print("Can't find the geometry. Please setup envvar DETCRDROOT." )
    sys.exit(-1)

geometry_path = os.path.join(os.getenv("DETCRDROOT"), "compact", geometry_option)
if not os.path.exists(geometry_path):
    print("Can't find the compact geometry file: %s"%geometry_path)
    sys.exit(-1)

from Configurables import GeomSvc
geomsvc = GeomSvc("GeomSvc")
geomsvc.compact = geometry_path
#######################################

########### k4DataSvc ####################
from Configurables import k4DataSvc
podioevent = k4DataSvc("EventDataSvc", input="CaloDigi_TDR_o1_v01_E240_nnHgg.root")
##########################################

########## CEPCSWData ################# 
cepcswdatatop ="/cvmfs/cepcsw.ihep.ac.cn/prototype/releases/data/latest"
#######################################

########## CrystalEcalEnergyCorrectionSvc ########
from Configurables import CrystalEcalEnergyCorrectionSvc
crystalecalcorr = CrystalEcalEnergyCorrectionSvc("CrystalEcalEnergyCorrectionSvc")
crystalecalcorr.CorrectionFile = os.path.join(cepcswdatatop, "CEPCSWData/offline-data/Service/CrystalEcalSvc/data/CrackRegionEnergyCorrection.root")
##################################################

########## Podio Input ###################
from Configurables import PodioInput
inp = PodioInput("InputReader")
inp.collections = [ "EcalBarrelCollection",
                    "EcalBarrelContributionCollection",  
                    "ECALBarrel", 
                    "ECALBarrelParticleAssoCol",
                    "HcalBarrelCollection", 
                    "HcalBarrelContributionCollection", 
                    "HCALBarrel",
                    "HCALBarrelParticleAssoCol",
                    "MCParticle", 
                    "CompleteTracks", 
                    "CompleteTracksParticleAssociation"]
##########################################

######### Reconstruction ################
from Configurables import CyberPFAlg
CyberPFAlg = CyberPFAlg("CyberPFAlg")
##----Global parameters----
CyberPFAlg.Seed = 1024
CyberPFAlg.BField = 3.
CyberPFAlg.Debug = 0
CyberPFAlg.SkipEvt = 0
CyberPFAlg.WriteAna = 1
CyberPFAlg.AnaFileName = "RecAnaTuple_TDR_o1_v01_E240_nnHgg.root"
CyberPFAlg.UseTruthTrack = 0
CyberPFAlg.EcalGlobalCalib = 1.05
CyberPFAlg.HcalGlobalCalib = 4.5
##----Readin collections----
CyberPFAlg.MCParticleCollection = "MCParticle"
CyberPFAlg.TrackCollections = ["CompleteTracks"]
CyberPFAlg.MCRecoTrackParticleAssociationCollection = "CompleteTracksParticleAssociation"
CyberPFAlg.ECalCaloHitCollections = ["ECALBarrel"]
CyberPFAlg.ECalReadOutNames = ["EcalBarrelCollection"]
CyberPFAlg.ECalMCPAssociationName = ["ECALBarrelParticleAssoCol"]
CyberPFAlg.HCalCaloHitCollections = ["HCALBarrel"]
CyberPFAlg.HCalReadOutNames = ["HcalBarrelCollection"]
CyberPFAlg.HCalMCPAssociationName = ["HCALBarrelParticleAssoCol"]

##--- Output collections ---
CyberPFAlg.OutputPFO = "outputPFO";

#----Algorithms----
CyberPFAlg.AlgList = ["GlobalClusteringAlg",      #1
                            "LocalMaxFindingAlg",       #2
                            "TrackMatchingAlg",         #3
                            "HoughClusteringAlg",       #4
                            "ConeClustering2DAlg",      #5
                            "AxisMergingAlg",           #6
                            "EnergySplittingAlg",       #9
                            "EnergyTimeMatchingAlg",    #11
                            "HcalClusteringAlg",        #12
                            "TruthClusteringAlg",       #15
                            "TrackClusterConnectingAlg",  #16
                            "PFOReclusteringAlg" ]  #17
CyberPFAlg.AlgParNames = [ ["InputECALBars","OutputECAL1DClusters","OutputECALHalfClusters"],#1
                                 ["OutputLocalMaxName"],#2
                                 ["ReadinLocalMaxName","OutputLongiClusName"],#3
                                 ["ReadinLocalMaxName","LeftLocalMaxName","OutputLongiClusName"],#4
                                 ["ReadinLocalMaxName", "OutputLongiClusName"], #5
                                 ["OutputAxisName"], #6
                                 ["ReadinAxisName", "OutputClusName", "OutputTowerName"],  #9
                                 ["ReadinHFClusterName", "ReadinTowerName","OutputClusterName"], #11
                                 ["InputHCALHits", "OutputHCALClusters"], #12
                                 ["DoECALClustering","DoHCALClustering","InputHCALHits","OutputHCALClusters"], #15
                                 ["ReadinECALClusterName", "ReadinHCALClusterName", "OutputCombPFO"],  #16
                                 ["ECALCalib", "HCALCalib", "MinAngleForNeuMerge"] ]#17
CyberPFAlg.AlgParTypes = [ ["string","string","string"],#1
                                 ["string"],#2
                                 ["string","string"],#3
                                 ["string","string","string"],#4
                                 ["string","string"], #5
                                 ["string"], #6
                                 ["string","string","string"],  #9
                                 ["string","string","string"], #11
                                 ["string", "string"], #12
                                 ["bool","bool","string","string"], #15
                                 ["string","string","string"],  #16
                                 ["double","double","double"] ]#17
CyberPFAlg.AlgParValues = [ ["BarCol","Cluster1DCol","HalfClusterCol"],#1
                                  ["AllLocalMax"],#2
                                  ["AllLocalMax","TrackAxis"],#3
                                  ["AllLocalMax","LeftLocalMax","HoughAxis"],#4
                                  ["LeftLocalMax","ConeAxis"], #5
                                  ["MergedAxis"], #6
                                  ["MergedAxis","ESHalfCluster","ESTower"],  #9
                                  ["ESHalfCluster","ESTower","EcalCluster"], #11
                                  ["HCALBarrel", "SimpleHCALCluster"], #12
                                  ["0","1","HCALBarrel","HCALCluster"], #15
                                  ["EcalCluster", "SimpleHCALCluster", "outputPFO"],  #16
                                  ["1.05","4.5","0.12"]  ]#17


##############################################################################
# POD I/O
##############################################################################
from Configurables import PodioOutput
out = PodioOutput("outputalg")
out.filename = "Rec_TDR_o1_v01_E240_nnHgg.root"
out.outputCommands = ["keep *"]


########################################

from Configurables import ApplicationMgr
ApplicationMgr( 
    TopAlg=[inp, CyberPFAlg, out ],
    EvtSel="NONE",
    EvtMax=3,
    ExtSvc=[podioevent, geomsvc],
    #OutputLevel=DEBUG
)
