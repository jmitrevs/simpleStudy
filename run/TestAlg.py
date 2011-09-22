#--------------------------------------------------------------
# Templated Parameters
#--------------------------------------------------------------

from glob import glob
#InputList = glob('/data3/jmitrevs/mc09_7TeV.106050.PythiaZee_1Lepton.merge.AOD.e468_s765_s767_r1250_r1260_tid129729_00/AOD.129729._000001.pool.root.1')
#InputList = glob('/data3/jmitrevs/data10_7TeV.00155073.physics_L1Calo.merge.DESDM_EGAMMA.r1299_p161_p165_tid143766_00/*')
#InputList = glob('/data3/jmitrevs/mc09_7TeV.114007.SPS8_110_jimmy_susy.merge.AOD.e530_s765_s767_r1302_r1306_tid140030_00/AOD*')
#InputList = glob('../../run/ESD.pool.root')
#InputList = glob('../../run_*/copy_AOD*.pool.root')
#InputList = glob('../../run/AOD.pool.root')
#InputList = glob('/data/jmitrevs/AtlasProduction-15.8.0.2/run/ESD.pool.root')
#InputList = glob('/data3/jmitrevs/AtlasOffline-rel_3/run/ESD.pool.root')
#InputList = glob('/data3/jmitrevs/mc09_7TeV.108085.PythiaPhotonJet_Unbinned7.recon.ESD.e534_s765_s767_r1305_tid137093_00/ESD.137093._000011.pool.root.1')
#InputList = glob('/data3/jmitrevs/dataskims/ee_merge/*periodI*/*.pool.root*')
#InputList = glob('../../run_lb358/myESD.pool.root')
#InputList = glob('/data3/jmitrevs/mcskims/mc10_7TeV.118441.Pythia_GGM_Bino600_300.merge.AOD.e640_s933_s946_r1831_r1700_tid222610_00/AOD.222610._000001.pool.root.1')
InputList = glob('/data3/jmitrevs/data11_7TeV.00178109.physics_Egamma.merge.AOD.f351_m765/*AOD*')

from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
athenaCommonFlags.FilesInput = InputList
#athenaCommonFlags.SkipEvents=2
athenaCommonFlags.EvtMax=-1
#athenaCommonFlags.EvtMax=200


# # use closest DB replica
# from AthenaCommon.AppMgr import ServiceMgr
# from PoolSvc.PoolSvcConf import PoolSvc
# ServiceMgr+=PoolSvc(SortReplicas=True)
# from DBReplicaSvc.DBReplicaSvcConf import DBReplicaSvc
# ServiceMgr+=DBReplicaSvc(UseCOOLSQLite=False)


from RecExConfig.RecFlags import rec

rec.doTrigger.set_Value_and_Lock(False)

#--------------------------------------------------------------
# ANALYSIS
#--------------------------------------------------------------

rec.UserAlgs.set_Value_and_Lock("TestAlg_simple.py")
#UserExecsList = ["ToolSvc.PhotonProcessingTool.PhotonisEMKey = 'PhotonTight'","ToolSvc.PhotonProcessingTool.ElectronisEMKey = 'ElectronTight'"]
#UserExecsList = ["ToolSvc.PhotonProcessingTool.excludeCrackRegion = False"]
#UserExecsList = ["NtupleDumper.SUSY_ProductionVeto = True","NtupleDumper.SUSY_ProductionTypeAccepted = 2","NtupleDumper.isMC = True"]
#UserExecsList = ["PhotonTrace.FillSPHist = True"]
#rec.UserExecs.set_Value_and_Lock(UserExecsList)

#--------------------------------------------------------------
# General Configuration
#--------------------------------------------------------------

OutputLevel = INFO

rec.readRDO.set_Value_and_Lock(False)
rec.readESD.set_Value_and_Lock(False)
rec.readAOD.set_Value_and_Lock(True)

rec.doAOD.set_Value_and_Lock(False)
rec.doAODCaloCells.set_Value_and_Lock(False)
rec.doCBNT.set_Value_and_Lock(False)
rec.doESD.set_Value_and_Lock(False)
rec.doHist.set_Value_and_Lock(False)
rec.doWriteAOD.set_Value_and_Lock(False)
rec.doWriteESD.set_Value_and_Lock(False)
rec.doWriteTAG.set_Value_and_Lock(False)
rec.doFileMetaData.set_Value_and_Lock(False)
rec.doJiveXML.set_Value_and_Lock(False)

rec.doPerfMon.set_Value_and_Lock(False)

#### jOs from Thijs for spacepoints from ESDs:
#from AthenaCommon.BeamFlags import jobproperties
#from InDetRecExample.InDetJobProperties import InDetFlags
#jobproperties.InDetJobProperties.Enabled.set_Value_and_Lock(True)
#InDetFlags.preProcessing.set_Value_and_Lock(True)
#InDetFlags.doSpacePointFormation.set_Value_and_Lock(True)

# main jobOption - must always be included
include ("RecExCommon/RecExCommon_topOptions.py")
