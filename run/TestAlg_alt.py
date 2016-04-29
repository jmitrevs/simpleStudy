from glob import glob

# Set up the reading of a file:
#FNAME = glob("/afs/cern.ch/work/k/krasznaa/public/xAOD/devval_rel_4/*ESD*")
#FNAME = "../../run/myAOD.pool.root"
#FNAME = glob("/afs/cern.ch/user/j/jmitrevs/workdir/data12_8TeV.00205113.physics_JetTauEtmiss.merge.AOD.r5723_p1751_p1793*/*AOD*root*")
#FNAME = "../../Reconstruction/RecExample/RecExCommon/run/ESD.pool.root"
#FNAME = "../../Reconstruction/RecExample/RecExCommon/run_old/ESD.pool.root"
#FNAME = "/afs/cern.ch/atlas/project/rig/referencefiles/QTests-Run2/RDO-run2.Nov5.2014-500events.pool.root"
#FNAME = "/afs/cern.ch/atlas/project/rig/referencefiles/QTests-Run2/RDO-run2.Nov27.2014-500events.pool.root"
#FNAME = "/afs/cern.ch/atlas/offline/ReleaseData/v18/testfile/mc12_8TeV.105200.McAtNloJimmy_CT10_ttbar_LeptonFilter.digit.RDO.e1513_s1499_s1504_d700_10evt.pool.root"
#FNAME = glob("/afs/cern.ch/user/j/jmitrevs/workdir/mc12_8TeV.105200.McAtNloJimmy_CT10_ttbar_LeptonFilter.recon.ESD.*/*root*")
#FNAME = glob("/afs/cern.ch/user/j/jmitrevs/workdir/input/mc12_8TeV.105200.McAtNloJimmy_CT10_ttbar_LeptonFilter.merge.AOD.*/*root*")
#FNAME = "/afs/cern.ch/atlas/project/rig/referencefiles/MC/valid1.110401.PowhegPythia_P2012_ttbar_nonallhad.recon.RDO.e3099_s2081_r6112_tid04860198_00/RDO.04860198._000028.pool.root.1"
#FNAME = "/afs/cern.ch/user/j/jmitrevs/workdir/input/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.merge.DAOD_TRUTH1.e3657_p2324_tid05304593_00/DAOD_TRUTH1.05304593._000060.pool.root.1"
#FNAME = "/afs/cern.ch/user/j/jmitrevs/workdir/input/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.merge.AOD.e3698_s2608_s2183_r6630_r6264_tid05444660_00/AOD.05444660._000447.pool.root.1"
#FNAME = glob("/home/jmitrevs/workdir/testinput/mc15_13TeV.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.merge.AOD.e3698_s2608_s2183_r6630_r6264*/*")
FNAME = "../run/AOD.pool.root"

include( "AthenaPython/iread_file.py" )

from RecExConfig.RecFlags import rec
#rec.doApplyAODFix.set_Value_and_Lock(True)

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

# import MCTruthClassifier.MCTruthClassifierBase
# print MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier

# MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier.TrackParticleContainerName  = "GSFTrackParticleCandidate"
# MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier.TrackParticleTruthCollection  = "GSFTrackParticleTruthCollection"

# import PyCintex
# PyCintex.loadDictionary('ElectronPhotonSelectorToolsDict')
# from ROOT import egammaPID

# from ElectronPhotonSelectorTools.ConfiguredAsgElectronIsEMSelectors import ConfiguredAsgElectronIsEMSelector
# electronSelector = ConfiguredAsgElectronIsEMSelector("myIsEmSelector", egammaPID.ElectronIDMediumPP,
#                                                      OutputLevel = DEBUG)
# ToolSvc += electronSelector

# from ElectronPhotonSelectorTools.ConfiguredAsgPhotonIsEMSelectors import ConfiguredAsgPhotonIsEMSelector
# photonSelector = ConfiguredAsgPhotonIsEMSelector("myPhotonSelector", egammaPID.PhotonIDTight,
#                                                  OutputLevel = DEBUG)
# ToolSvc += photonSelector

# Add top algorithms to be run
from simpleStudy.simpleStudyConf import TestAlg
testAlg = TestAlg(name = "TestAlg",
                  ElectronSelector = None,
                  PhotonSelector = None,
                  # MCTruthClassifier = MCTruthClassifier.MCTruthClassifierBase.MCTruthClassifier,
                  DoTruth = False,
                  DoElectrons = False,
                  DoPhotons = True,
#                  McEventCollectionContainerName = "GEN_AOD"
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

if not hasattr(ServiceMgr, theApp.EventLoop):
   ServiceMgr += getattr(CfgMgr, theApp.EventLoop)() 
evtloop = getattr(ServiceMgr, theApp.EventLoop)
evtloop.EventPrintoutInterval = 1

theApp.EvtMax=10
 
