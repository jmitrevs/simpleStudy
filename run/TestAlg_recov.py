
from AthenaCommon.Constants import *
from AthenaCommon.AppMgr import theApp
from AthenaCommon.AppMgr import ToolSvc
from AthenaCommon.AppMgr import ServiceMgr
import AthenaPoolCnvSvc.ReadAthenaPool
import popen2


#-------------------------------------------------------------------------
# Options to turn on/off certain algorithms
#-------------------------------------------------------------------------
if not 'doPhotonRecoveryAlg'       in dir():
	doPhotonRecoveryAlg        = True
if not 'doPhotonPostProcessingAlg' in dir():
	doPhotonPostProcessingAlg  = True
if not 'doReRunEgamma'             in dir():
	# True  : Run full conversion reconstruction+egamma from scratch.
	# False : Search conversion in the electron container.
	doReRunEgamma              = False 
if not 'doHiggsAnalysisUtils'      in dir():
	doHiggsAnalysisUtils       = False
if not 'doNewTopoClusters'         in dir():
	# For the NTupleDumper - leave at False
	doNewTopoClusters          = False


# # Particle Properties
# from PartPropSvc.PartPropSvcConf import PartPropSvc

# # the POOL converters
# include( "ParticleBuilderOptions/AOD_PoolCnv_jobOptions.py" )
# include( "ParticleBuilderOptions/McAOD_PoolCnv_jobOptions.py" )
# include( "EventAthenaPool/EventAthenaPool_joboptions.py" )

DetDescrVersion="ATLAS-GEO-02-01-00"

include( "RecExCond/AllDet_detDescr.py" )
# the global detflags
# from AthenaCommon.GlobalFlags import globalflags
# from AthenaCommon.DetFlags import DetFlags
# DetFlags.ID_setOn()
# DetFlags.Calo_setOn()
# DetFlags.Muon_setOn()
# # the global flags    
# globalflags.DetDescrVersion = 'ATLAS-CSC-02-01-00'
# from AtlasGeoModel import SetGeometryVersion
# from AtlasGeoModel import GeoModelInit

#--------------------------------------------------------------
# Define which samples should be read in
#--------------------------------------------------------------
import AthenaPoolCnvSvc.ReadAthenaPool
from glob import glob

#INPUT = ["/data/jmitrevs/mc09_valid.106384.PythiaH120gamgam.recon.AOD.e352_s462_s520_r729_tid075168_photonRecov.root"]

#INPUT = glob('/data/jmitrevs/AtlasOffline-rel_1/run_orig_huge/AOD.pool.root')
#INPUT = glob('/data/jmitrevs/AtlasOffline-rel_1/run_calovtx_huge/AOD.pool.root')
INPUT = glob('/data/jmitrevs/mc08.105410.GMSB1_jimmy_susy.merge.AOD.e352_s462_s520_r808_r838_tid095999/AOD.095999._000001.pool.root.1')
#INPUT = glob('/data/jmitrevs/mc08.105410.GMSB1_jimmy_susy.merge.AOD.e352_s462_s520_r808_r838_tid095999/AOD.095999._000001.recov.pool.root.1')

ServiceMgr.EventSelector.InputCollections = INPUT

ServiceMgr.EventSelector.BackNavigation = False 

from RecExConfig.RecFlags  import rec
rec.readAOD = False
rec.readESD = False
rec.readTAG = False
if ServiceMgr.EventSelector.InputCollections[0].rfind('TAG') != -1:
	rec.readTAG = True
elif ServiceMgr.EventSelector.InputCollections[0].rfind('ESD') != -1:
	rec.readESD = True
else: # should cover the AOD and DnPD case (n in [1,2])
	rec.readAOD = True

#--------------------------------------------------------------
# Load Trigger DecisionTool
#--------------------------------------------------------------
#include( "InsituTools/SetupTriggerDecisionTool.py" )

#--------------------------------------------------------------
# Load POOL support
#--------------------------------------------------------------
# include( "InsituTools/LoadPoolSupport.py" )
# svcMgr.EventSelector.RunNumber = 0

#--------------------------------------------------------------
# Define your Signal Selection Algorithm and Add Tools
#--------------------------------------------------------------
# Full job is a list of algorithms
from AthenaCommon.AlgSequence import AlgSequence
theJob = AlgSequence()


# Some basic container names, that are common to recovery/post-processing/DPD maker
#
PhotonClusterContainer            = "egClusterCollection"
TrackParticleCandidates           = "TrackParticleCandidate"
ConversionContainer               = "ConversionCandidate"
#
if rec.readESD:
	PhotonDetailContainer             = "egDetailContainer"
	McEventCollectionName             = "TruthEvent"
	ElectronContainer                 = "ElectronCollection"
	PhotonContainer                   = "PhotonCollection"
	CaloCells                         = "AllCalo"
	JetCollectionName                 = "Cone4H1TowerJets"
else:
	PhotonDetailContainer             = "egDetailAOD"
	McEventCollectionName             = "GEN_AOD"
	ElectronContainer                 = "ElectronAODCollection"
	PhotonContainer                   = "PhotonAODCollection"
	TrackParticleCandidates           = "TrackParticleCandidate"
	CaloCells                         = "AODCellContainer"
	JetCollectionName                 = "Cone4H1TowerAODJets"


# Recovery algorithm
if doPhotonRecoveryAlg:
	include('PhotonAnalysisAlgs/PhotonRecovery_jobOptions.py')
	
# Post Processing algorithm
if doPhotonPostProcessingAlg:
	include('PhotonAnalysisAlgs/PhotonPostProcessing_jobOptions.py')


# Add top algorithms to be run
from InsituAnalysis.InsituAnalysisConf import TestAlg
testAlg = TestAlg( "TestAlg" )   # 1 alg, named "HelloWorld"
from AthenaCommon.AppMgr import ToolSvc
testAlg.OutputLevel = DEBUG

# Add example to Reader
theJob += testAlg

#--------------------------------------------------------------
# Add Dictionary for writing out in PoolDPDs
#--------------------------------------------------------------
AthenaSealSvc = Service( "AthenaSealSvc" )
include( "AthenaSealSvc/AthenaSealSvc_joboptions.py" )
#include ( "InsituEvent/InsituEventDict_joboptions.py" )

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------
# Number of events to be processed (default is 10)
theApp.EvtMax 					= -1

ServiceMgr.MessageSvc.OutputLevel = WARNING

import MCTruthClassifier.MCTruthClassifierBase
print MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier

from InsituAnalysis.InsituAnalysisConf import TruthUtils
TruthUtils = TruthUtils(name                    = "TruthUtils",
                        MCTruthClassifierTool   = MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier.getFullName(),
                        TruthConeMatch          = .2,
                        OutputLevel = INFO)
ToolSvc += TruthUtils
print      TruthUtils


#==============================================================
#==============================================================
# setup TTree registration Service
# save ROOT histograms and Tuple
from GaudiSvc.GaudiSvcConf import THistSvc
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output = ["TestHistograms DATAFILE='TestHistograms.root' OPT='RECREATE'"]

