from glob import glob

# Set up the reading of a file:
#FNAME = glob("/afs/cern.ch/work/k/krasznaa/public/xAOD/devval_rel_4/*ESD*")
FNAME = "../../run8/ESD.pool.root"
include( "AthenaPython/iread_file.py" )

#--------------------------------------------------------------
# Define your Signal Selection Algorithm and Add Tools
#--------------------------------------------------------------

# Full job is a list of algorithms
from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

# from PhotonAnalysisUtils.LowPtJetFinder import LowPtJetFinder
# mygetter2 = LowPtJetFinder() # will insert the alg into the AlgSequence()
# print mygetter2

# from PhotonAnalysisUtils.PhotonAnalysisUtilsConf import PAUcaloIsolationTool
# mycaloisolationtool = PAUcaloIsolationTool()
# mycaloisolationtool.DoAreaCorrections = True
# ToolSvc += mycaloisolationtool

import MCTruthClassifier.MCTruthClassifierBase
print MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier

MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier.TrackParticleContainerName  = "GSFTrackParticleCandidate"
MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier.TrackParticleTruthCollection  = "GSFTrackParticleTruthCollection"

import PyCintex
PyCintex.loadDictionary('ElectronPhotonSelectorToolsDict')
from ROOT import egammaPID

from ElectronPhotonSelectorTools.ConfiguredAsgElectronIsEMSelectors import ConfiguredAsgElectronIsEMSelector
electronSelector = ConfiguredAsgElectronIsEMSelector("myIsEmSelector", egammaPID.ElectronIDMediumPP,
                                                     OutputLevel = DEBUG)
ToolSvc += electronSelector

from ElectronPhotonSelectorTools.ConfiguredAsgPhotonIsEMSelectors import ConfiguredAsgPhotonIsEMSelector
photonSelector = ConfiguredAsgPhotonIsEMSelector("myPhotonSelector", egammaPID.PhotonIDTightAR,
                                                 OutputLevel = DEBUG)
ToolSvc += photonSelector

# Add top algorithms to be run
from simpleStudy.simpleStudyConf import TestAlg
testAlg = TestAlg(name = "TestAlg",
                  ElectronSelector = electronSelector,
                  PhotonSelector = photonSelector,
                  MCTruthClassifier = MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier,
                  DoTruth = False,
                  DoElectrons = True,
                  DoPhotons = False
                  )   # 1 alg, named "HelloWorld"
from AthenaCommon.AppMgr import ToolSvc
testAlg.OutputLevel = DEBUG

# Add example to Reader
topSequence += testAlg

#--------------------------------------------------------------
# Event related parameters
#--------------------------------------------------------------

ServiceMgr.MessageSvc.OutputLevel = WARNING


# from simpleStudy.simpleStudyConf import TruthUtils
# TruthUtils = TruthUtils(name                    = "TruthUtils",
#                         MCTruthClassifierTool   = MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier.getFullName(),
#                         TruthConeMatch          = .2,
#                         OutputLevel = DEBUG)
# ToolSvc += TruthUtils
# print      TruthUtils

#==============================================================
#==============================================================
# setup TTree registration Service
# save ROOT histograms and Tuple
from GaudiSvc.GaudiSvcConf import THistSvc
ServiceMgr += THistSvc()
ServiceMgr.THistSvc.Output = ["TestHistograms DATAFILE='TestHistograms.root' OPT='RECREATE'"]

