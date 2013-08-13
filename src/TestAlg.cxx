///////////////////////////////////////////////////////////////////
// TestAlg.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

//#include <TDirectory.h>
//#include <TFile.h>
//#include <TROOT.h>

#include "EventInfo/EventInfo.h"
#include "EventInfo/EventID.h"
#include "EventInfo/EventType.h"

#include "TH1.h"
#include "McParticleEvent/TruthParticle.h"
#include "McParticleEvent/TruthParticleContainer.h"

#include "egammaEvent/ElectronContainer.h"
#include "egammaEvent/Electron.h"
#include "egammaEvent/PhotonContainer.h"
#include "egammaEvent/Photon.h"

#include "egammaEvent/EMConvert.h"
#include "egammaEvent/EMShower.h"
#include "egammaEvent/EMTrackMatch.h"
#include "egammaEvent/EMErrorDetail.h"
#include "egammaEvent/egammaParamDefs.h"

#include "egammaInterfaces/EMAmbiguityToolDefs.h"
#include "egammaUtils/EMType.h"

#include "MissingETEvent/MissingET.h"

#include "Particle/TrackParticleContainer.h"
#include "ParticleTruth/TrackParticleTruthCollection.h"
#include "GeneratorObjects/McEventCollection.h"

#include "TrkTrackLink/ITrackLink.h"
#include "TrkParticleBase/LinkToTrackParticleBase.h"
#include "TrkParticleBase/TrackParticleBaseCollection.h"

#include "VxVertex/VxTrackAtVertex.h"
#include "VxVertex/ExtendedVxCandidate.h"
#include "InDetSecVxFinderTool/InDetJetFitterUtils.h"
#include "TrkNeutralParameters/MeasuredNeutralPerigee.h"

#include "AnalysisUtils/AnalysisMisc.h"

//#include "simpleStudy/TruthUtils.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"

//#include "ElectronPhotonSelectorTools/IAthElectronIsEMSelector.h"
//#include "ElectronPhotonSelectorTools/IAthPhotonIsEMSelector.h"
#include "PATCore/IAthSelectorTool.h"

#include "simpleStudy/TestAlg.h"

// #include "egammaInterfaces/IEMBremsstrahlungBuilder.h"
// #include "egammaInterfaces/IEMFourMomBuilder.h"

#include "VxVertex/ExtendedVxCandidate.h"
#include "FourMomUtils/P4Helpers.h"

//#include "PhotonAnalysisUtils/IPAUcaloIsolationTool.h"

#include <gsl/gsl_math.h>

using CLHEP::GeV;

//================ Constructor =================================================

TestAlg::TestAlg(const std::string& name, 
		 ISvcLocator* pSvcLocator) : 
  AthAlgorithm(name,pSvcLocator)
  // m_TruthUtils("TruthUtils/TruthUtils")
{

  declareProperty("HistFileName", m_histFileName = "TestHistograms");
  //  declareProperty("TruthUtils", m_TruthUtils);
  //  declareProperty("PAUcaloIsolationTool", m_PAUcaloIsolationTool);
  declareProperty("MCTruthClassifier", m_MCTruthClassifier);
  declareProperty("ElectronSelector", m_electronSelector);
  declareProperty("PhotonSelector", m_photonSelector);

  declareProperty("DoTruth", m_doTruth = false);

  /** Electron selection */
  declareProperty("ElectronContainerName", m_ElectronContainerName = "ElectronAODCollection");
  declareProperty("ElectronPt",   m_electronPt=10*GeV);
  declareProperty("ElectronEta",  m_electronEta=3.2);
  declareProperty("ElectronIsEMFlag", m_electronIsEMFlag="Medium");
  declareProperty("ElectronIsEM", m_electronIsEM=0);

  declareProperty("TruthElectronPtMin", m_truthElectronPtMin = 5*GeV);

  declareProperty("PhotonContainerName", m_PhotonContainerName = "PhotonAODCollection");
  declareProperty("PhotonPt",   m_photonPt=10*GeV);
  declareProperty("PhotonEta",  m_photonEta=3.2);
  declareProperty("PhotonIsEMFlag", m_photonIsEMFlag="Tight");
  declareProperty("PhotonIsEM", m_photonIsEM=0);

  declareProperty("METContainerName", m_METContainerName = "MET_LocHadTopo");

  declareProperty("egammaContainerName", m_egammaContainerName = "HLT_egamma");

  declareProperty("TruthPhotonPtMin", m_truthPhotonPtMin = 5*GeV);


  declareProperty("TrackParticleContainerName", m_TrackParticleContainerName = 
		  "TrackParticleCandidate");
  declareProperty("TrackParticleTruthCollectionName", 
		  m_TrackParticleTruthCollectionName = "TrackParticleTruthCollection");

  // Name of the input conversion container
  declareProperty("ConversionsName",                 
		  m_ConversionsName="ConversionCandidate",
		  "Name of the input conversion container");

  declareProperty("runEvents", m_runEvents, "events to run over, though NULL is all");

  
//   // Name of the EMTrackFit container
//   declareProperty("EMTrackFitContainerName",
// 		  m_EMTrackFitContainerName="egDetailContainer",
// 		  "Name of the EMTrackFit container");

//   // Name of the EMErrorDetail container
//   declareProperty("EMErrorDetailContainerName",
//                   m_EMErrorDetailContainerName="egDetailAODAll",
//                   "Name of the EMErrorDetail container");

  // // Name of the McEventCollection Container
  // declareProperty("McEventCollectionContainerName",
  //                 m_McEventCollectionContainerName="GEN_AOD",
  //                 "Name of the McEventCollection container");
  
}

//================ Destructor =================================================

TestAlg::~TestAlg()
{
}


//================ Initialisation =================================================

StatusCode TestAlg::initialize()
{	
  
  ATH_MSG_DEBUG(name()<<" initialize()");

  /// histogram location
  StatusCode sc = service("THistSvc", m_thistSvc);
  if(sc.isFailure()) {
    ATH_MSG_ERROR("Unable to retrieve pointer to THistSvc");
    return sc;
  }

  m_theEvents.insert(m_runEvents.begin(), m_runEvents.end());
  m_runOnlySome = ! m_runEvents.empty();

  if (m_doTruth) {
    if(m_MCTruthClassifier.retrieve().isFailure()) {
      ATH_MSG_ERROR("Failed to retrieve " << m_MCTruthClassifier);
      return StatusCode::FAILURE; // why success?
    }
    else {
      ATH_MSG_DEBUG("Retrieved MCTruthClassifier " << m_MCTruthClassifier);   
    }
  }

  if(m_electronSelector.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_electronSelector);
    return StatusCode::FAILURE; // why success?
  }
  else {
    ATH_MSG_DEBUG("Retrieved ElectronSelector " << m_electronSelector);   
  }

  if(m_photonSelector.retrieve().isFailure()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_photonSelector);
    return StatusCode::FAILURE; // why success?
  }
  else {
    ATH_MSG_DEBUG("Retrieved PhotonSelector " << m_photonSelector);   
  }

  // if(m_TruthUtils.retrieve().isFailure()) {
  //   ATH_MSG_ERROR("Failed to retrieve " << m_TruthUtils);
  //   return StatusCode::FAILURE; // why success?
  // }
  // else {
  //   ATH_MSG_DEBUG("Retrieved TruthUtils " << m_TruthUtils);   
  // }

  // if(m_PAUcaloIsolationTool.retrieve().isFailure()) {
  //   ATH_MSG_ERROR("Failed to retrieve " << m_PAUcaloIsolationTool);
  //   return StatusCode::FAILURE; // why success?
  // }
  // else {
  //   ATH_MSG_DEBUG("Retrieved PAUcaloIsolationTool " << m_PAUcaloIsolationTool);   
  // }

  /// Defining Histograms
  m_histograms["Resolution"] = new TH1F("Resolution","Pseudorapidity Resolution;#eta_{reco} - #eta_{truth}", 240, -0.03,0.03);
  m_histograms["EtaReco"] = new TH1F("EtaReco","Reco Psuedorapidity;#eta_{reco}", 100, -3,3);
  m_histograms["EtaTruth"] = new TH1F("EtaTruth","Truth Psuedorapidity;#eta_{truth}", 100, -3,3);

  // only for unconverted
  m_histograms["Resolution0T"] = new TH1F("Resolution0T","Pseudorapidity Resolution, unconverted;#eta_{reco} - #eta_{truth}", 240, -0.03,0.03);
  m_histograms["EtaReco0T"] = new TH1F("EtaReco0T","Reco Psuedorapidity, unconverted;#eta_{reco}", 100, -3,3);
  m_histograms["EtaTruth0T"] = new TH1F("EtaTruth0T","Truth Psuedorapidity, unconverted;#eta_{truth}", 100, -3,3);

  // only for Si
  m_histograms["Resolution1T"] = new TH1F("Resolution1T","Pseudorapidity Resolution, 1-track Si conversion;#eta_{reco} - #eta_{truth}", 240, -0.03,0.03);
  m_histograms["EtaReco1T"] = new TH1F("EtaReco1T","Reco Psuedorapidity, 1-track Si conversion;#eta_{reco}", 100, -3,3);
  m_histograms["EtaTruth1T"] = new TH1F("EtaTruth1T","Truth Psuedorapidity, 1-track Si conversion;#eta_{truth}", 100, -3,3);

  m_histograms["Resolution2T"] = new TH1F("Resolution2T","Pseudorapidity Resolution, 2-track Si conversion;#eta_{reco} - #eta_{truth}", 240, -0.03,0.03);
  m_histograms["EtaReco2T"] = new TH1F("EtaReco2T","Reco Psuedorapidity, 2-track Si conversion;#eta_{reco}", 100, -3,3);
  m_histograms["EtaTruth2T"] = new TH1F("EtaTruth2T","Truth Psuedorapidity, 2-track Si conversion;#eta_{truth}", 100, -3,3);

  // TRT
  m_histograms["Resolution1TTRT"] = new TH1F("Resolution1TTRT","Pseudorapidity Resolution, 1-track TRT conversion;#eta_{reco} - #eta_{truth}", 240, -0.03,0.03);
  m_histograms["EtaReco1TTRT"] = new TH1F("EtaReco1TTRT","Reco Psuedorapidity, 1-track TRT conversion;#eta_{reco}", 100, -3,3);
  m_histograms["EtaTruth1TTRT"] = new TH1F("EtaTruth1TTRT","Truth Psuedorapidity, 1-track TRT conversion;#eta_{truth}", 100, -3,3);

  m_histograms["Resolution2TTRT"] = new TH1F("Resolution2TTRT","Pseudorapidity Resolution, 2-track TRT conversion;#eta_{reco} - #eta_{truth}", 240, -0.03,0.03);
  m_histograms["EtaReco2TTRT"] = new TH1F("EtaReco2TTRT","Reco Psuedorapidity, 2-track TRT conversion;#eta_{reco}", 100, -3,3);
  m_histograms["EtaTruth2TTRT"] = new TH1F("EtaTruth2TTRT","Truth Psuedorapidity, 2-track TRT conversion;#eta_{truth}", 100, -3,3);

  // and now phi
  m_histograms["PhiResolution"] = new TH1F("PhiResolution","#phi Resolution;#phi_{reco} - #phi_{truth}", 240, -0.03,0.03);
  m_histograms["PhiReco"] = new TH1F("PhiReco","Reco #phi;#phi_{reco}", 100, -4,4);
  m_histograms["PhiTruth"] = new TH1F("PhiTruth","Truth #phi;#phi_{truth}", 100, -4,4);

  // only for unconverted
  m_histograms["PhiResolution0T"] = new TH1F("PhiResolution0T","#phi Resolution, unconverted;#phi_{reco} - #phi_{truth}", 240, -0.03,0.03);
  m_histograms["PhiReco0T"] = new TH1F("PhiReco0T","Reco #phi, unconverted;#phi_{reco}", 100, -4,4);
  m_histograms["PhiTruth0T"] = new TH1F("PhiTruth0T","Truth #phi, unconverted;#phi_{truth}", 100, -4,4);

  /// Only for Si
  m_histograms["PhiResolution1T"] = new TH1F("PhiResolution1T","#phi Resolution, 1-track Si conversion;#phi_{reco} - #phi_{truth}", 240, -0.03,0.03);
  m_histograms["PhiReco1T"] = new TH1F("PhiReco1T","Reco #phi, 1-track Si conversion;#phi_{reco}", 100, -4,4);
  m_histograms["PhiTruth1T"] = new TH1F("PhiTruth1T","Truth #phi, 1-track Si conversion;#phi_{truth}", 100, -4,4);

  m_histograms["PhiResolution2T"] = new TH1F("PhiResolution2T","#phi Resolution, 2-track Si conversion;#phi_{reco} - #phi_{truth}", 240, -0.03,0.03);
  m_histograms["PhiReco2T"] = new TH1F("PhiReco2T","Reco #phi, 2-track Si conversion;#phi_{reco}", 100, -4,4);
  m_histograms["PhiTruth2T"] = new TH1F("PhiTruth2T","Truth #phi, 2-track Si conversion;#phi_{truth}", 100, -4,4);

  // TRT
  m_histograms["PhiResolution1TTRT"] = new TH1F("PhiResolution1TTRT","#phi Resolution, 1-track TRT conversion;#phi_{reco} - #phi_{truth}", 240, -0.03,0.03);
  m_histograms["PhiReco1TTRT"] = new TH1F("PhiReco1TTRT","Reco #phi, 1-track TRT conversion;#phi_{reco}", 100, -4,4);
  m_histograms["PhiTruth1TTRT"] = new TH1F("PhiTruth1TTRT","Truth #phi, 1-track TRT conversion;#phi_{truth}", 100, -4,4);

  m_histograms["PhiResolution2TTRT"] = new TH1F("PhiResolution2TTRT","#phi Resolution, 2-track TRT conversion;#phi_{reco} - #phi_{truth}", 240, -0.03,0.03);
  m_histograms["PhiReco2TTRT"] = new TH1F("PhiReco2TTRT","Reco #phi, 2-track TRT conversion;#phi_{reco}", 100, -4,4);
  m_histograms["PhiTruth2TTRT"] = new TH1F("PhiTruth2TTRT","Truth #phi, 2-track TRT conversion;#phi_{truth}", 100, -4,4);

  // PID studies
  m_histograms["TRTPID1Trk"] = new TH1F("TRTPID1Trk", "The TRT PID values of all single-track conversion vertices.", 100, 0, 1);
  m_histograms["TRTPID1TrkTRT"] = new TH1F("TRTPID1TrkTRT", "The TRT PID values of TRT-only single-track conversion vertices.", 100, 0, 1);

  m_histograms["TRTPID1Trk_fromPhotons"] = new TH1F("TRTPID1Trk_fromPhotons", "The TRT PID values of single-track conversion vertices matched to photons.", 100, 0, 1);
  m_histograms["TRTPID1TrkTRT_fromPhotons"] = new TH1F("TRTPID1TrkTRT_fromPhotons", "The TRT PID values of TRT-only single-track conversion vertices matched to photons.", 100, 0, 1);

  m_histograms["etcone20"] = new TH1F("etcone20", "The etcone20 distributions", 220, -10*GeV, 100*GeV);
  m_histograms["etcone30"] = new TH1F("etcone30", "The etcone30 distributions", 220, -10*GeV, 100*GeV);
  m_histograms["etcone40"] = new TH1F("etcone40", "The etcone40 distributions", 220, -10*GeV, 100*GeV);

  m_histograms["etcone20_corrected"] = new TH1F("etcone20_corrected", "The etcone20_corrected distributions", 220, -10*GeV, 100*GeV);
  m_histograms["etcone30_corrected"] = new TH1F("etcone30_corrected", "The etcone30_corrected distributions", 220, -10*GeV, 100*GeV);
  m_histograms["etcone40_corrected"] = new TH1F("etcone40_corrected", "The etcone40_corrected distributions", 220, -10*GeV, 100*GeV);

  // electron variables
  m_histograms["ElEtaReco"] = new TH1F("ElEtaReco","Reco Psuedorapidity;#eta_{reco}", 100, -3,3);
  m_histograms["minv"] = new TH1F("minv", "The invariante mass of the two leading electrons", 120, 0*GeV, 120*GeV);
  m_histograms["numEl"] = new TH1F("numEl", "The number of electrons that pass cuts", 9, -0.5, 8.5);

  // MET
  m_histograms["met"] = new TH1F("met", "The MET distribution of Z events", 100, 0*GeV, 250*GeV);


  /// Registering Histograms
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/Resolution" , m_histograms["Resolution"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco" , m_histograms["EtaReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaTruth" , m_histograms["EtaTruth"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/Resolution0T" , m_histograms["Resolution0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco0T" , m_histograms["EtaReco0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaTruth0T" , m_histograms["EtaTruth0T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/Resolution1T" , m_histograms["Resolution1T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1T" , m_histograms["EtaReco1T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaTruth1T" , m_histograms["EtaTruth1T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/Resolution2T" , m_histograms["Resolution2T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2T" , m_histograms["EtaReco2T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaTruth2T" , m_histograms["EtaTruth2T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/Resolution1TTRT" , m_histograms["Resolution1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1TTRT" , m_histograms["EtaReco1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaTruth1TTRT" , m_histograms["EtaTruth1TTRT"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/Resolution2TTRT" , m_histograms["Resolution2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TTRT" , m_histograms["EtaReco2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaTruth2TTRT" , m_histograms["EtaTruth2TTRT"]).ignore();

  // and phi plots
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiResolution" , m_histograms["PhiResolution"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiReco" , m_histograms["PhiReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiTruth" , m_histograms["PhiTruth"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiResolution0T" , m_histograms["PhiResolution0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiReco0T" , m_histograms["PhiReco0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiTruth0T" , m_histograms["PhiTruth0T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiResolution1T" , m_histograms["PhiResolution1T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiReco1T" , m_histograms["PhiReco1T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiTruth1T" , m_histograms["PhiTruth1T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiResolution2T" , m_histograms["PhiResolution2T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiReco2T" , m_histograms["PhiReco2T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiTruth2T" , m_histograms["PhiTruth2T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiResolution1TTRT" , m_histograms["PhiResolution1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiReco1TTRT" , m_histograms["PhiReco1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiTruth1TTRT" , m_histograms["PhiTruth1TTRT"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiResolution2TTRT" , m_histograms["PhiResolution2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiReco2TTRT" , m_histograms["PhiReco2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PhiTruth2TTRT" , m_histograms["PhiTruth2TTRT"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/TRTPID1Trk" , m_histograms["TRTPID1Trk"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/TRTPID1TrkTRT" , m_histograms["TRTPID1TrkTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/TRTPID1Trk_fromPhotons" , m_histograms["TRTPID1Trk_fromPhotons"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/TRTPID1TrkTRT_fromPhotons" , m_histograms["TRTPID1TrkTRT_fromPhotons"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/etcone20" , m_histograms["etcone20"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/etcone30" , m_histograms["etcone30"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/etcone40" , m_histograms["etcone40"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/etcone20_corrected" , m_histograms["etcone20_corrected"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/etcone30_corrected" , m_histograms["etcone30_corrected"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/etcone40_corrected" , m_histograms["etcone40_corrected"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/EtaReco" , m_histograms["ElEtaReco"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/minv" , m_histograms["minv"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/numEl" , m_histograms["numEl"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/met" , m_histograms["met"]).ignore();
  

  // initialize some constants
  numPhotons = 0;
  numUnconverted = 0;
  numUnconvertedDup = 0;

  numConversions = 0;
  numConversionsDup = 0;

  numConversionsSingleTrack = 0;
  numConversionsDoubleTrack = 0;
  numConversionsSingleTrackSi = 0;
  numConversionsDoubleTrackSi = 0;
  numConversionsSingleTrackTRT = 0;
  numConversionsDoubleTrackTRT = 0;
  numConversionsDoubleTrackMix = 0;


  numConversionsNotTP = 0;
  numConversionsNotTPTRTOnly = 0;

  numElectrons = 0;
  numTightElectrons = 0;
  numElectronsAuthorElectron = 0;
  numTightElectronsAuthorElectron = 0;
  numElectronsAuthorSofte = 0;
  numElectronsAuthorFrwd = 0;

  // ATH_MSG_DEBUG("detlaPhi(0.2,0.1) = " << P4Helpers::deltaPhi(0.2,0.1));
  // ATH_MSG_DEBUG("detlaPhi(0.1,0.2) = " << P4Helpers::deltaPhi(0.1,0.2));

  // ATH_MSG_DEBUG("detlaPhi(3.1,-3.1) = " << P4Helpers::deltaPhi(3.1,-3.1));
  // ATH_MSG_DEBUG("detlaPhi(-3.1,3.1) = " << P4Helpers::deltaPhi(-3.1,3.1));

  ATH_MSG_INFO("egammaPID::CONVMATCH_ELECTRON = " << egammaPID::CONVMATCH_ELECTRON);
  ATH_MSG_INFO("About to end initialize.");

  return StatusCode::SUCCESS;
}

//================ Finalisation =================================================

StatusCode TestAlg::finalize()
{
  ATH_MSG_DEBUG(name()<<" finalize()");
  std::cout << "  **** EGAMMA STATISTICS ****\n";
  std::cout << "  numPhotons                               = " << numPhotons << std::endl;
  std::cout << "  numUnconverted                           = " << numUnconverted << std::endl;
  std::cout << "  numConversions                           = " << numConversions << std::endl;
  std::cout << "  numUnconverted also electrons            = " << numUnconvertedDup << std::endl;
  std::cout << "  numConversions also electrons            = " << numConversionsDup << std::endl;
  std::cout << "  numConversions Single Track              = " << numConversionsSingleTrack << std::endl;
  std::cout << "  numConversions Double Track              = " << numConversionsDoubleTrack << std::endl;
  std::cout << "  numConversions Single Track Si           = " << numConversionsSingleTrackSi << std::endl;
  std::cout << "  numConversions Double Track Si           = " << numConversionsDoubleTrackSi << std::endl;
  std::cout << "  numConversions Single Track TRT          = " << numConversionsSingleTrackTRT << std::endl;
  std::cout << "  numConversions Double Track TRT          = " << numConversionsDoubleTrackTRT << std::endl;
  std::cout << "  numConversions Double Track Mix          = " << numConversionsDoubleTrackMix << std::endl;


  std::cout << "  numConversions not trackParticle         = " << numConversionsNotTP << std::endl;
  std::cout << "  numConversions not trackParticle TRTOnly = " << numConversionsNotTPTRTOnly << std::endl;
  std::cout << "  numConversions not trackParticle Si      = " 
	    << numConversionsNotTP - numConversionsNotTPTRTOnly << std::endl;
  std::cout << "     * * *\n";
  std::cout << "  numElectrons                             = " << numElectrons << std::endl;
  std::cout << "  numElectrons author Electron             = " << numElectronsAuthorElectron << std::endl;
  std::cout << "  numElectrons author Softe                = " << numElectronsAuthorSofte << std::endl;
  std::cout << "  numElectrons author Frwd                 = " << numElectronsAuthorFrwd << std::endl;
  std::cout << "  numTightElectrons                        = " << numTightElectrons << std::endl;
  std::cout << "  numTightElectrons author Electron        = " << numTightElectronsAuthorElectron << std::endl;
  // std::cout << "  numConversion in Electron Container      = " << numElConversions << std::endl;

  return StatusCode::SUCCESS;
}

//================ Execution ====================================================

StatusCode TestAlg::execute()
{ 

  StatusCode sc = StatusCode::SUCCESS;

  // ATH_MSG_DEBUG("Electron container name: " << m_ElectronContainerName);

  const EventInfo*  evtInfo = 0;
  sc = evtStore()->retrieve(evtInfo);
  if(sc.isFailure() || !evtInfo) {
    ATH_MSG_ERROR("could not retrieve event info");
    return StatusCode::RECOVERABLE;
  }
  
  const unsigned int eventNumber = evtInfo->event_ID()->event_number();
  if (m_runOnlySome && m_theEvents.find(eventNumber) == m_theEvents.end()) {
    // skip the event
    return StatusCode::SUCCESS;
  }

  const ElectronContainer* electrons;
  sc=evtStore()->retrieve( electrons, m_ElectronContainerName);
  if( sc.isFailure()  ||  !electrons ) {
    ATH_MSG_ERROR("No continer "<< m_ElectronContainerName <<" container found in TDS");
    return StatusCode::FAILURE;
  }

  // ATH_MSG_DEBUG("Photon container name: " << m_PhotonContainerName);

  const PhotonContainer* photons;
  sc=evtStore()->retrieve( photons, m_PhotonContainerName);
  if( sc.isFailure()  ||  !photons ) {
    ATH_MSG_ERROR("No continer "<< m_PhotonContainerName <<" container found in TDS");
    return StatusCode::FAILURE;
  }

  // // The missing ET object
  // const MissingET* met(0);
  // sc = evtStore()->retrieve( met, m_METContainerName );
  // if( sc.isFailure()  ||  !met ) {
  //   ATH_MSG_ERROR("No continer "<< m_METContainerName <<" container found in TDS");
  //   return StatusCode::FAILURE;
  // }

  // const egammaContainer* egammas;
  // sc=evtStore()->retrieve( egammas, m_egammaContainerName);
  // if( sc.isFailure()  ||  !egammas ) {
  //   ATH_MSG_ERROR("No continer "<< m_egammaContainerName <<" container found in TDS");
  //   return StatusCode::FAILURE;
  // }
  
  // retrieve Conversion Container
  const VxContainer* convcoll;
  if (evtStore()->contains<VxContainer>(m_ConversionsName)) {
    StatusCode sc = evtStore()->retrieve(convcoll,m_ConversionsName);
    if(sc.isFailure()) {
      ATH_MSG_ERROR("Could not retrieve Conversion container."); 
      return StatusCode::FAILURE;
    }
  } else {
    ATH_MSG_DEBUG("Could not find Conversion container: name doesn't exist"); 
    return StatusCode::FAILURE;
  }

  // /// Load Truth Container
  // const TruthParticleContainer*  mcpartTES;
  // sc=evtStore()->retrieve( mcpartTES, "SpclMC");
  // if( sc.isFailure()  ||  !mcpartTES ) {
  //   ATH_MSG_ERROR("No AOD MC truth particle container found in TDS");
  //   return StatusCode::FAILURE;
  // }

  // const HepMC::GenEvent *ge=mcpartTES->genEvent();
  // m_TruthUtils->FillTruthMap(ge, m_truthPhotonPtMin);

  // // and the actual McEventCollection
  // const McEventCollection* mcEventCollection(0);
  // sc = evtStore()->retrieve(mcEventCollection,m_McEventCollectionContainerName);
  // if( sc.isFailure()  ||  !mcEventCollection ) {
  //   ATH_MSG_ERROR("No AOD McEventCollection container found in TDS");
  //   return StatusCode::FAILURE;
  // }

  // const Rec::TrackParticleContainer* trackParts;
  // sc=evtStore()->retrieve(trackParts, m_TrackParticleContainerName);
  // if( sc.isFailure()  ||  !trackParts ) {
  //   ATH_MSG_ERROR("No AOD "<< m_TrackParticleContainerName <<" container found in TDS");
  //   return StatusCode::FAILURE;
  // }

  // const TrackParticleTruthCollection* trackPartsTruth;
  // sc=evtStore()->retrieve(trackPartsTruth, m_TrackParticleTruthCollectionName);
  // if( sc.isFailure()  ||  !trackParts ) {
  //   ATH_MSG_ERROR("No AOD "<< m_TrackParticleTruthCollectionName <<" container found in TDS");
  //   return StatusCode::FAILURE;
  // }
  
  //  PhotonContainer::const_iterator phold = photons->end();

  int numEl = 0; // this is per event
  int numElPass = 0; // this is per event
  Analysis::Electron *leading = 0;
  Analysis::Electron *second = 0;

  double leadPt = 0;
  double secondPt = 0;

  // loop over electrons
  for (ElectronContainer::const_iterator el  = electrons->begin();
       el != electrons->end();
       el++) {
    
    // try the selector

    if (EMType::isElectronNotForward(*el)) {
      const Root::TAccept& acc = m_electronSelector->accept(*el);
      const bool passid = (*el)->passID(egammaPID::ElectronIDMediumPP);
      
      if (acc && passid) {
      	ATH_MSG_DEBUG("Passed electron selector and passID.");
      } else if (acc) {
      	ATH_MSG_WARNING("Passed electron selector but not passID.");
      } else if (passid) {
      	ATH_MSG_WARNING("Failed electron selector but passed passID.");
      } else {
      	ATH_MSG_DEBUG("Failed electron selector.");
      }
    }
    bool passTruth = true;
    if (m_doTruth) {
      std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> res =
	m_MCTruthClassifier->particleTruthClassifier(*el);
      passTruth = (res.first == MCTruthPartClassifier::IsoElectron);
    }

    if (passTruth) {

      numElectrons++;
      if ((*el)->author(egammaParameters::AuthorElectron)) {
	numEl++;
	numElectronsAuthorElectron++;
      }
      if ((*el)->author(egammaParameters::AuthorSofte)) numElectronsAuthorSofte++;
      if ((*el)->author(egammaParameters::AuthorFrwd)) numElectronsAuthorFrwd++;

      ATH_MSG_INFO("Electron with author " << (*el)->author() << ", isem == " << std::hex << (*el)->isem() 
		   << ", eta = " << (*el)->eta() << ", phi = " << (*el)->phi());

      if ((*el)->passID(egammaPID::ElectronIDTight)) {
	ATH_MSG_DEBUG("Electron passes tight.");
	numTightElectrons++;
	if ((*el)->author(egammaParameters::AuthorElectron)) {
	  numTightElectronsAuthorElectron++;
	  numElPass++;
	}
      }
    
      
      if (!((*el)->author(egammaParameters::AuthorFrwd))) {
	//    if ((*el)->author(egammaParameters::AuthorElectron) && (*el)->isElectron(egammaPID::ElectronMedium_WithTrackMatch)) {
	// const double elPt = (*el)->pt();
	// if (elPt > leadPt) {
	// 	second = leading;
	// 	leading = *el;
	// 	secondPt = leadPt;
	// 	leadPt = elPt;
	// } else if (elPt > secondPt) {
	// 	second = *el;
	// 	secondPt = elPt;
	// }
      

	// const egamma::momentum_type& fourmom = (*el)->get4Mom(egamma::Uncombined);

	// const double manualCalc = (*el)->cluster()->e()/cosh((*el)->trackParticle()->eta());
	// const double autoCalc = fourmom.pt();

	// ATH_MSG_INFO("Manualy calculated pt = " << manualCalc << ", uncombined = " << autoCalc);
      
	// ATH_MSG_INFO("author is = " << (*el)->author() << ", and num cells = " 
	// 		   << (*el)->cluster()->getNumberOfCells());

	ATH_MSG_INFO("Electron OQ = " << std::hex << (*el)->isgoodoq());

	m_histograms["ElEtaReco"]->Fill((*el)->eta());

	const EMShower* shower = (*el)->detail<EMShower>();
	// std::cout << "About to print out electron shower" << std::endl;
	shower->print();

	//   if (fabs((*el)->phi()) > M_PI) {
	//     ATH_MSG_WARNING("Looking at electron (author = " << (*el)->author() 
	// 		      << ") with phi = " << (*el)->phi() 
	// 		      << ", eta = "<< (*el)->eta() 
	// 		      << ", et = " << (*el)->et() 
	// 		      << ", E = " << (*el)->e());
	//     ATH_MSG_WARNING("   Cluster phi = " << (*el)->cluster()->phi() 
	// 		      << ", eta = "<< (*el)->cluster()->eta() 
	// 		      << ", et = " << (*el)->cluster()->et()
	// 		      << ", E = " << (*el)->cluster()->e());

	const Rec::TrackParticle * trParticle = (*el)->trackParticle();
	if (!trParticle) {
	  ATH_MSG_WARNING("Electron has no track-particle");
	  continue;
	}

	ATH_MSG_DEBUG("   Track phi = " << trParticle->phi() 
		      << ", eta = "<< trParticle->eta() 
		      << ", et = " << trParticle->et()
		      << ", E = " << trParticle->e());
      
	// 	const Trk::TrackParticleBase* trkbase = dynamic_cast<const Trk::TrackParticleBase* >(trParticle);
	// 	const Trk::MeasuredPerigee* perigee = dynamic_cast<const Trk::MeasuredPerigee*>( &(trkbase->definingParameters()) );
	
	// 	double momentum = -999999.;
	// 	double momentumError = -999999.;
	
	// 	if (perigee->parameters()[Trk::qOverP] != 0 ) {
	// 	  momentum = 1./fabs(perigee->parameters()[Trk::qOverP]);
	// 	  momentumError =
	// 	    momentum*momentum * sqrt(perigee->localErrorMatrix().covariance()[Trk::qOverP][Trk::qOverP]);
	// 	}
	
	// 	const double energy =(*el)->detailValue(egammaParameters::EMPhoton_Eclus);
	// 	const double energyError =sqrt( std::max(0.,(*el)->detailValue(egammaParameters::EMPhoton_CovEclusEclus)) );   
	
	// 	const double chi = sqrt(((energy - momentum)*(energy - momentum))
	// 				/ ((energyError*energyError) + (momentumError*momentumError)));

	// 	ATH_MSG_WARNING("Chi for combining values = " << chi);

	//     }


	//     const double theta = (*el)->detailValue(egammaParameters::EMTrack_theta);
	//     const double eta = - log(tan(theta/2.0));

	//     ATH_MSG_WARNING("   Detail value EMTrack_phi0: " << (*el)->detailValue(egammaParameters::EMTrack_phi0) 
	// 		      << ", hasSiliconHits = " << (*el)->detailValue(egammaParameters::hasSiliconHits) 
	// 		      << ", EMTrackTheta = " << theta
	// 		      << ", eta = " << eta);

      

	const Trk::TrackSummary* sum = (*el)->trackParticle()->trackSummary();
	if (!sum) {
	  ATH_MSG_WARNING("trackParticle has no summary!");
	  continue;
	}
	int nSiliconHits_trk = sum->get(Trk::numberOfSCTHits)+ sum->get(Trk::numberOfPixelHits);
      
	const EMTrackMatch *trackmatch = (*el)->detail<EMTrackMatch>();
	if (!trackmatch) {
	  ATH_MSG_ERROR("No track match available");
	  continue;
	}

	ATH_MSG_DEBUG("   Number of silicon hits = " << nSiliconHits_trk << ", number of b-layer hits = " <<
		      sum->get(Trk::numberOfBLayerHits));

	ATH_MSG_DEBUG("   Electron tight blayer req = " << (*el)->isElectron(0x1 << egammaPID::TrackBlayer_Electron) 
		      << "; expect hit in b-layer from detail = " << trackmatch->expectHitInBLayer() 
		      << " and from track summary = " << sum->get(Trk::expectBLayerHit));


	//   } // if (fabs((*el)->phi()) > M_PI)

	//   if (!(*el)->isElectron(egammaPID::CONVMATCH_ELECTRON) && EMType::isElectronNotForward(*el)) {
	//     const unsigned int isem = (*el)->isem();
	//     ATH_MSG_DEBUG("Found a conv matched electron (author = " << (*el)->author() << ") based on PID == " << std::hex << isem);
	//     const Analysis::Photon *matchedPhoton = NULL;
	//     for (PhotonContainer::const_iterator ph  = photons->begin();
	// 	   ph != photons->end();
	// 	   ph++) {
	// 	if ((*el)->hasSameAthenaBarCodeExceptVersion(**ph)) {
	// 	  matchedPhoton = *ph;
	// 	  break;
	// 	}
	//     }
	//     if (matchedPhoton) {
	// 	const EMShower* shower = matchedPhoton->detail<EMShower>();
	
	// 	const CaloCluster* cluster = matchedPhoton->cluster();
	
	// 	double eta2   = fabs(cluster->etaBE(2)); 
	// 	double et     = cluster->energy()/cosh(eta2);
	// 	double ethad1 = shower->ethad1(); 
	
	// 	double hadleakage = et > 0. ? ethad1/et : 1.;
	
	// 	const double deltaR = P4Helpers::deltaR(*el, matchedPhoton);
	// 	if (matchedPhoton->conversion()) {
	// 	  ATH_MSG_INFO("Found a matched converted photon with PID == " << std::hex << isem << std::dec << ", had leakage == " << hadleakage << ", and deltaR == " << deltaR);
	// 	} else {
	// 	  ATH_MSG_INFO("Found a matched unconverted photon with PID == " << std::hex << isem << std::dec << ", had leakage == " << hadleakage << ", and deltaR == " << deltaR);
	// 	}
	//     } else {
	//      	ATH_MSG_WARNING("NO MATCHING PHOTON FOUND");
	//     }
	//   } else {
	//     const Analysis::Photon *matchedPhoton = NULL;
	//     for (PhotonContainer::const_iterator ph  = photons->begin();
	// 	   ph != photons->end();
	// 	   ph++) {
	// 	if ((*el)->hasSameAthenaBarCodeExceptVersion(**ph)) {
	// 	  matchedPhoton = *ph;
	// 	  break;
	// 	}
	//     }
	//     if (matchedPhoton) {
	// 	ATH_MSG_ERROR("A MATCH WAS NOT EXPECTED!");
	// 	const double deltaR = P4Helpers::deltaR(*el, matchedPhoton);
	// 	const unsigned int isem = (*el)->isem();
	// 	if (matchedPhoton->conversion()) {
	// 	  ATH_MSG_WARNING("  Found a matched converted photon with PID == " << std::hex << isem << std::dec << " and deltaR == " << deltaR);
	// 	} else {
	// 	  ATH_MSG_WARNING("  Found a matched unconverted photon with PID == " << std::hex << isem << std::dec << " and deltaR == " << deltaR);
	// 	}
	//     }
	//   }
      }
    }
  } // loop over electrons

  ATH_MSG_DEBUG("The event had " << numEl << " electrons out of which " << numElPass << " passed tight.");

  m_histograms["numEl"]->Fill(numElPass);


  // if (numElPass > 1) {
  //   const double minv = P4Helpers::invMass(leading, second);
  //   m_histograms["minv"]->Fill(minv);

  //   // if (minv > 71*GeV && minv < 111*GeV) {
  //   //   m_histograms["met"]->Fill(met->et());
  //   // }
  // }



  ATH_MSG_DEBUG("About to start photons");

  PhotonContainer* sortedPhotons = const_cast<PhotonContainer*>(photons);

  if (!sortedPhotons) {
    ATH_MSG_WARNING("Can't const-cast photons");
  }

  AnalysisUtils::Sort::pT(sortedPhotons);

  for (PhotonContainer::const_iterator ph  = sortedPhotons->begin();
       ph != sortedPhotons->end();
       ph++) {

    // if ((*ph)->isPhoton(egammaPID::PhotonTight)) {
    {

      //    if ((*ph)->detailValue(egammaParameters::ambiguityResult) <= EMAmbiguityType::LOOSE) continue;

      //     if ((!(*ph)->isPhoton(0x80000000)) || (!(*ph)->isPhoton(0x40000000)) || (!(*ph)->isPhoton(0x20000000))) {
      //       ATH_MSG_INFO("Photon fails isolation with isem == " << std::hex << (*ph)->isem());
      //     } else {
      //       ATH_MSG_DEBUG("Photon passes isolation with isem == " << std::hex << (*ph)->isem());
      //     }


      const EMShower* shower = (*ph)->detail<EMShower>();
      std::cout << "About to print out photon shower" << std::endl;
      shower->print();



      // ATH_MSG_DEBUG("Looking at photon (author = " << (*ph)->author() << ") with barcode = " << (*ph)->getAthenaBarCode());
    
      // const Trk::RecVertex* origin = (*ph)->origin();
    
      // if (origin) {
      //   ATH_MSG_DEBUG("Origin of photon is = " << *origin);
      // } else {
      //   ATH_MSG_DEBUG("Origin pointer is 0");
      // }
      // const Analysis::Electron *matchedEl = NULL;
      // for (ElectronContainer::const_iterator el  = electrons->begin();
      // 	 el != electrons->end();
      // 	 el++) {
      //   if ((*el)->hasSameAthenaBarCodeExceptVersion(**ph)) {
      // 	matchedEl = *el;
      // 	//break;
      //   }

      //   // const Trk::RecVertex* elorigin = (*el)->origin();

      //   // if (elorigin) {
      //   // 	ATH_MSG_DEBUG("Origin of electron (author = " << (*el)->author() << ") is = " << *elorigin);
      //   // } else {
      //   // 	ATH_MSG_DEBUG("Electron origin (author = " << (*el)->author() << ") pointer is 0");
      //   // }
      // }

      // if (matchedEl) {
      //   ATH_MSG_DEBUG("   Found an electron with same barcode");
      // } else if ((*ph)->author() == 16) {
      //   ATH_MSG_ERROR("   Did NOT find an electron with same barcode");
      // }      

      // if (phold != photons->end()) {
      //   ATH_MSG_DEBUG("   does it match the previous one without version: " << 
      // 		    (*ph)->hasSameAthenaBarCodeExceptVersion(**phold));
      // }
      // phold = ph;

      // ATH_MSG_DEBUG("Photon with author " << (*ph)->author() << " has number of details: " << (*ph)->nDetails());
      // for (int i = 0; i < (*ph)->nDetails(); i++) {
      //   ATH_MSG_DEBUG("photon detail: " << (*ph)->detailName(i));
      // }
      // ATH_MSG_DEBUG("");


      // let's mess around with uncombined 4-mom (makes no difference here
      // since now it's not combined for photons)

      // ATH_MSG_DEBUG("about to test uncombined");
      // const egamma::momentum_type& fourmom = (*ph)->get4Mom(egamma::Uncombined);

      // if (fourmom.pt() > 20*GeV) {
      // 	ATH_MSG_DEBUG("uncombined pt > 20 GeV");
      // }

      // const EMErrorDetail *errorDetail = (*ph)->detail<EMErrorDetail>();

      // ATH_MSG_DEBUG("author = " << std::hex << (*ph)->author());
      // //    ATH_MSG_DEBUG("authordec = " << std::dec << (*ph)->author());
      // ATH_MSG_DEBUG("errors = " << (*ph)->errors());
      // //    if ((*ph)->author() == 4) { 
      // ATH_MSG_DEBUG("eta error = " << (*ph)->errors()->etaError());
      // if ((*ph)->author() == 16) {
      //   ATH_MSG_DEBUG("eta error sq from detail = " << errorDetail->EMtrack_comb_Covetaeta());
      //   ATH_MSG_DEBUG("eta error from detail = " << sqrt(errorDetail->EMtrack_comb_Covetaeta()));
      //   ATH_MSG_DEBUG("eta error from cluster in detail = " << sqrt(errorDetail->EMphoton_Covetaeta()));
      // }
      // //}

      numPhotons++;

      if ((*ph)->author(egammaParameters::AuthorPhoton | egammaParameters::AuthorRConv)) {
	const Root::TAccept& acc = m_photonSelector->accept(*ph);
	const bool passid = (*ph)->passID(egammaPID::PhotonIDTightAR);
      
	if (acc && passid) {
	  ATH_MSG_DEBUG("Passed photon selector and passID.");
	} else if (acc) {
	  ATH_MSG_WARNING("Passed photon selector but not passID.");
	} else if (passid) {
	  ATH_MSG_WARNING("Failed photon selector but passed passID.");
	  // ATH_MSG_WARNING("  selector isEM = " << std::hex << m_photonSelector->IsemValue() 
	  // 		  << ", orig isEM = " << (*ph)->isem());
	} else {
	  ATH_MSG_DEBUG("Failed photon selector.");
	}
      }

      ATH_MSG_INFO("Photon with isem == " << std::hex << (*ph)->isem() 
		   << ", cluster = " << (*ph)->cluster());
      ATH_MSG_INFO("author is = " << (*ph)->author() << ", and num cells = " 
		   << (*ph)->cluster()->getNumberOfCells());
      ATH_MSG_INFO("Photon OQ = " << std::hex << (*ph)->isgoodoq());
      //		 << ", cluster phi = " << (*ph)->cluster()->phi());

      const double tppt = ((*ph)->trackParticle()) ? (*ph)->trackParticle()->pt() : -999; 
      ATH_MSG_INFO("Photon trackParticle (not conversion) pt = " << tppt);

      //ATH_MSG_DEBUG("(*ph)->conversion() = " << (*ph)->conversion());

      //const EMShower* shower = (*ph)->detail<EMShower>();

      // if ((*ph)->pt() > 20*GeV && (*ph)->isPhoton(egammaPID::PhotonTightAR)) { 
      // 	// do isolation test
      // 	const double pt_correction_20 =
      // 	  m_PAUcaloIsolationTool->EtConeCorrectionPt(*ph, .20) ;
      // 	const double pt_correction_30 =
      // 	  m_PAUcaloIsolationTool->EtConeCorrectionPt(*ph, .30) ;
      // 	const double pt_correction_40 =
      // 	  m_PAUcaloIsolationTool->EtConeCorrectionPt(*ph, .40) ;
    
      // 	const int removeNHardestJets = 0;  // default value for now
      // 	const double ED_correction_20 =
      // 	  m_PAUcaloIsolationTool->EtConeCorrectionJetAreas(*ph, .20,
      // 							   removeNHardestJets);
      // 	const double ED_correction_30 =
      // 	  m_PAUcaloIsolationTool->EtConeCorrectionJetAreas(*ph, .30,
      // 							   removeNHardestJets);
      // 	const double ED_correction_40 =
      // 	  m_PAUcaloIsolationTool->EtConeCorrectionJetAreas(*ph, .40,
      // 							   removeNHardestJets);
      
      // 	// Then, to calculate the "corrected" isolation variables:
    
      ATH_MSG_INFO("isos: " << shower->etcone20() << ", " << shower->etconoisedR03SigAbs3());

    
      // 	m_histograms["etcone20"]->Fill(etcone20);
      // 	m_histograms["etcone30"]->Fill(etcone30);
      // 	m_histograms["etcone40"]->Fill(etcone40);

      // 	const double etcone20_pt_corrected = etcone20 - pt_correction_20;
      // 	const double etcone30_pt_corrected = etcone30 - pt_correction_30;
      // 	const double etcone40_pt_corrected = etcone40 - pt_correction_40;
    
      // 	const double etcone20_ED_corrected = etcone20 - ED_correction_20;
      // 	const double etcone30_ED_corrected = etcone30 - ED_correction_30;
      // 	const double etcone40_ED_corrected = etcone40 - ED_correction_40;
    
      // 	const double etcone20_corrected = etcone20 - pt_correction_20 - ED_correction_20;
      // 	const double etcone30_corrected = etcone30 - pt_correction_30 - ED_correction_30;
      // 	const double etcone40_corrected = etcone40 - pt_correction_40 - ED_correction_40;

      // 	m_histograms["etcone20_corrected"]->Fill(etcone20_corrected);
      // 	m_histograms["etcone30_corrected"]->Fill(etcone30_corrected);
      // 	m_histograms["etcone40_corrected"]->Fill(etcone40_corrected);

    
      // 	ATH_MSG_INFO("econe20 = " << etcone20 
      // 		     << ", pt corrected = " << etcone20_pt_corrected
      // 		     << ", ED corrected = " << etcone20_ED_corrected
      // 		     << ", full corrected = " << etcone20_corrected);
      // 	ATH_MSG_INFO("econe30 = " << etcone30 
      // 		     << ", pt corrected = " << etcone30_pt_corrected
      // 		     << ", ED corrected = " << etcone30_ED_corrected
      // 		     << ", full corrected = " << etcone30_corrected);
      // 	ATH_MSG_INFO("econe40 = " << etcone40 
      // 		     << ", pt corrected = " << etcone40_pt_corrected
      // 		     << ", ED corrected = " << etcone40_ED_corrected
      // 		     << ", full corrected = " << etcone40_corrected);
      // }

      if ((*ph)->conversion()) {
	numConversions++;
	if ((*ph)->author(egammaParameters::AuthorRConv)) numConversionsDup++;

	if (!(*ph)->trackParticle()) numConversionsNotTP++;

	const Trk::VxCandidate*  convVtx = (*ph)->conversion();
	const std::vector<Trk::VxTrackAtVertex*> *trkAtVxPtr = convVtx->vxTrackAtVertex();
	if (trkAtVxPtr->size() == 1) {
	  numConversionsSingleTrack++;
	  int nSiliconHits_trk1=0;
	  // first track
	  Trk::VxTrackAtVertex* tmpTrkAtVtx1 = trkAtVxPtr->at(0);
	  const Trk::ITrackLink * trLink =tmpTrkAtVtx1->trackOrParticleLink();
	  const Trk::TrackParticleBase* tempTrk1PB(0);
	  if (0!= trLink) {
	    const Trk::LinkToTrackParticleBase * linkToTrackPB =  dynamic_cast<const Trk::LinkToTrackParticleBase *>(trLink);  
	    if (0!= linkToTrackPB) {
	      if(linkToTrackPB->isValid()) tempTrk1PB = linkToTrackPB->cachedElement(); 
	    } 
	  }
	  if ( tempTrk1PB!=NULL){  
	    const Trk::TrackSummary* summary1 = tempTrk1PB->trackSummary();
	    if (summary1 != NULL){
	      nSiliconHits_trk1 = summary1->get(Trk::numberOfSCTHits)+ summary1->get(Trk::numberOfPixelHits);
	      const double pid1 = summary1->getPID(Trk::eProbabilityComb);
	      m_histograms["TRTPID1Trk_fromPhotons"]->Fill(pid1);
	      if (nSiliconHits_trk1 < 4) {
		m_histograms["TRTPID1TrkTRT_fromPhotons"]->Fill(pid1);
	      }
	    }
	  }
	  const Trk::MeasuredPerigee* parAtVtx1 = 
	    dynamic_cast<const  Trk::MeasuredPerigee*>(tmpTrkAtVtx1->perigeeAtVertex());
	  if (nSiliconHits_trk1 < 4) {
	    numConversionsSingleTrackTRT++;
	    if (!(*ph)->trackParticle()) numConversionsNotTPTRTOnly++;
	    ATH_MSG_DEBUG("single-track trt photon with author = " << (*ph)->author() 
			  << ", pt = " << (*ph)->pt() 
			  << ", eta = " << (*ph)->eta() 
			  << ", phi = " << (*ph)->phi()
			  << ", conv vx tp pt = " << parAtVtx1->pT());
	  } else {
	    numConversionsSingleTrackSi++;
	    ATH_MSG_DEBUG("single-track si photon with author = " << (*ph)->author() 
			  << ", pt = " << (*ph)->pt() 
			  << ", eta = " << (*ph)->eta() 
			  << ", phi = " << (*ph)->phi()
			  << ", conv vx tp pt = " << parAtVtx1->pT());
	  }

	} else if (int(trkAtVxPtr->size())==2) {
	  numConversionsDoubleTrack++;
	    
	  int nSiliconHits_trk1=0;
	  int nSiliconHits_trk2=0;
	  //	  const Trk::MeasuredPerigee* perigee = 0;

	  // first track
	  Trk::VxTrackAtVertex* tmpTrkAtVtx1 = trkAtVxPtr->at(0);
	  const Trk::ITrackLink * trLink1 =tmpTrkAtVtx1->trackOrParticleLink();
	  const Trk::TrackParticleBase* tempTrk1PB(0);
	  if (0!= trLink1) {
	    const Trk::LinkToTrackParticleBase * linkToTrackPB =  dynamic_cast<const Trk::LinkToTrackParticleBase *>(trLink1);  
	    if (0!= linkToTrackPB) {
	      if(linkToTrackPB->isValid()) tempTrk1PB = linkToTrackPB->cachedElement(); 
	    } 
	  }
	  if ( tempTrk1PB!=NULL){  
	    const Trk::TrackSummary* summary1 = tempTrk1PB->trackSummary();
	    if (summary1 != NULL){
	      nSiliconHits_trk1 = summary1->get(Trk::numberOfSCTHits)+ summary1->get(Trk::numberOfPixelHits);
	    }
	  }
	  Trk::VxTrackAtVertex* tmpTrkAtVtx2 = trkAtVxPtr->at(1);
	  const Trk::ITrackLink * trLink2 =tmpTrkAtVtx2->trackOrParticleLink();
	  const Trk::TrackParticleBase* tempTrk2PB(0);
	  if (0!= trLink2) {
	    const Trk::LinkToTrackParticleBase * linkToTrackPB =  dynamic_cast<const Trk::LinkToTrackParticleBase *>(trLink2);  
	    if (0!= linkToTrackPB) {
	      if (linkToTrackPB->isValid()) tempTrk2PB = linkToTrackPB->cachedElement(); 
	    } 
	  }
	  if ( tempTrk2PB!=NULL) {
	    const Trk::TrackSummary* summary2 = tempTrk2PB->trackSummary();
	    if (summary2 != NULL){
	      nSiliconHits_trk2 = summary2->get(Trk::numberOfSCTHits)+ summary2->get(Trk::numberOfPixelHits);
	    }
	  
	    //       ATH_MSG_ERROR("found a conversion without a trackParticle");
	    //       return StatusCode::FAILURE;
	  }
	  if (nSiliconHits_trk1 < 4 &&  nSiliconHits_trk2 < 4) {
	    numConversionsDoubleTrackTRT++;
	    if (!(*ph)->trackParticle()) numConversionsNotTPTRTOnly++;
	    ATH_MSG_DEBUG("double-track trt photon with author = " << (*ph)->author() 
			  << ", pt = " << (*ph)->pt() 
			  << ", eta = " << (*ph)->eta() 
			  << ", phi = " << (*ph)->phi());
	  } else if (nSiliconHits_trk1 < 4 || nSiliconHits_trk2 < 4) {
	    numConversionsDoubleTrackMix++;
	    if (!(*ph)->trackParticle()) numConversionsNotTPTRTOnly++;
	    ATH_MSG_DEBUG("double-track mix photon with author = " << (*ph)->author() 
			  << ", pt = " << (*ph)->pt()
			  << ", eta = " << (*ph)->eta() 
			  << ", phi = " << (*ph)->phi());

	  } else {
	    numConversionsDoubleTrackSi++;
	    ATH_MSG_DEBUG("double-track Si photon with author = " << (*ph)->author() 
			  << ", pt = " << (*ph)->pt()
			  << ", eta = " << (*ph)->eta() 
			  << ", phi = " << (*ph)->phi());
	  }
	}
      } else {
	numUnconverted++;
	ATH_MSG_DEBUG("Unconverted photon with author = " << (*ph)->author() 
		      << ", pt = " << (*ph)->pt()
		      << ", eta = " << (*ph)->eta() 
		      << ", phi = " << (*ph)->phi());
	if ((*ph)->author(egammaParameters::AuthorRConv)) numUnconvertedDup++;
      }

      const Rec::TrackParticle * trParticle = (*ph)->trackParticle();
      if (trParticle) {
	const Trk::TrackSummary* sum = trParticle->trackSummary();
	int nSiliconHits_trk = -999;
	if (sum != NULL)  nSiliconHits_trk = sum->get(Trk::numberOfSCTHits)+ sum->get(Trk::numberOfPixelHits);
      
	ATH_MSG_DEBUG("   Number of silicon hits of tp = " << nSiliconHits_trk << ", pt = " << trParticle->pt());
      }

      ATH_MSG_DEBUG("   ambiguity result = " << (*ph)->detailValue(egammaParameters::ambiguityResult));
  
      // const HepMC::GenParticle *truthPhoton = 
      //   m_TruthUtils->DoTruthMatch(*ph, ge, trackParts,
      // 				 trackPartsTruth, true, true);

      // // for debug
      // {
      //   const Trk::VxCandidate*  convVtx = (*ph)->conversion();
      //   if (convVtx) {
      // 	const std::vector<Trk::VxTrackAtVertex*> *trkAtVxPtr = convVtx->vxTrackAtVertex();
      // 	ATH_MSG_DEBUG("trkAtVxPtr->size() = " << trkAtVxPtr->size());
      //   }
      // }

      // if (truthPhoton) {
      //   // if (1) {
      //   // ATH_MSG_DEBUG("Found truth photon");
      //   ATH_MSG_INFO("Photon 4-mom is = " << fourmom.hlv() << ", pt = " << fourmom.pt());
      //   const float recoeta = fourmom.eta(); 
      //   const float trutheta = (truthPhoton) ? (truthPhoton->momentum()).eta() : -999;
      m_histograms["EtaReco"]->Fill((*ph)->eta());
      //   m_histograms["EtaTruth"]->Fill(trutheta);
      //   m_histograms["Resolution"]->Fill(recoeta - trutheta);

      //   const float recophi = fourmom.phi(); 
      //   const float truthphi = (truthPhoton) ? (truthPhoton->momentum()).phi() : -999;
      //   m_histograms["PhiReco"]->Fill(recophi);
      //   m_histograms["PhiTruth"]->Fill(truthphi);
      //   m_histograms["PhiResolution"]->Fill(recophi - truthphi);

      //   // if ((*ph)->author(egammaParameters::AuthorRConv)) {
      //   // 	ATH_MSG_INFO("Recovered photon has isEM == " << std::hex << (*ph)->isem());
      //   // }

      //   const Trk::VxCandidate*  convVtx = (*ph)->conversion();
      //   if (convVtx) {
      // 	const std::vector<Trk::VxTrackAtVertex*> *trkAtVxPtr = convVtx->vxTrackAtVertex();
      // 	if (int(trkAtVxPtr->size())==1) {

      // 	  int nSiliconHits_trk1=0;
      // 	  //	  const Trk::MeasuredPerigee* perigee = 0;
	
      // 	  // first track
      // 	  Trk::VxTrackAtVertex* tmpTrkAtVtx1 = trkAtVxPtr->at(0);
      // 	  const Trk::ITrackLink * trLink =tmpTrkAtVtx1->trackOrParticleLink();
      // 	  const Trk::TrackParticleBase* tempTrk1PB(0);
      // 	  if (0!= trLink) {
      // 	    const Trk::LinkToTrackParticleBase * linkToTrackPB =  dynamic_cast<const Trk::LinkToTrackParticleBase *>(trLink);  
      // 	    if (0!= linkToTrackPB) {
      // 	      if(linkToTrackPB->isValid()) tempTrk1PB = linkToTrackPB->cachedElement(); 
      // 	    } 
      // 	  }
      // 	  if ( tempTrk1PB!=NULL){  
      // 	    const Trk::TrackSummary* summary1 = tempTrk1PB->trackSummary();
      // 	    if (summary1 != NULL){
      // 	      nSiliconHits_trk1 = summary1->get(Trk::numberOfSCTHits)+ summary1->get(Trk::numberOfPixelHits);
      // 	    }
      // 	  }
      // 	  // ATH_MSG_DEBUG("Single track with " << nSiliconHits_trk1 << " silicon hits.");
      // 	  if (nSiliconHits_trk1 > 3) {
      // 	    m_histograms["EtaReco1T"]->Fill(recoeta);
      // 	    m_histograms["EtaTruth1T"]->Fill(trutheta);
      // 	    m_histograms["Resolution1T"]->Fill(recoeta - trutheta);
      // 	    m_histograms["PhiReco1T"]->Fill(recophi);
      // 	    m_histograms["PhiTruth1T"]->Fill(truthphi);
      // 	    m_histograms["PhiResolution1T"]->Fill(recophi - truthphi);
      // 	  } else {
      // 	    m_histograms["EtaReco1TTRT"]->Fill(recoeta);
      // 	    m_histograms["EtaTruth1TTRT"]->Fill(trutheta);
      // 	    m_histograms["Resolution1TTRT"]->Fill(recoeta - trutheta);
      // 	    m_histograms["PhiReco1TTRT"]->Fill(recophi);
      // 	    m_histograms["PhiTruth1TTRT"]->Fill(truthphi);
      // 	    m_histograms["PhiResolution1TTRT"]->Fill(recophi - truthphi);
      // 	  }
      // 	} else if (int(trkAtVxPtr->size())==2) {
      // 	  // ATH_MSG_DEBUG("Two track");
      // 	  //	  ATH_MSG_DEBUG("vertex covariance = 
      // 	  int nSiliconHits_trk1=0;
      // 	  int nSiliconHits_trk2=0;
      // 	  //	  const Trk::MeasuredPerigee* perigee = 0;
	    
      // 	  // first track
      // 	  Trk::VxTrackAtVertex* tmpTrkAtVtx1 = trkAtVxPtr->at(0);
      // 	  const Trk::ITrackLink * trLink1 =tmpTrkAtVtx1->trackOrParticleLink();
      // 	  const Trk::TrackParticleBase* tempTrk1PB(0);
      // 	  if (0!= trLink1) {
      // 	    const Trk::LinkToTrackParticleBase * linkToTrackPB =  dynamic_cast<const Trk::LinkToTrackParticleBase *>(trLink1);  
      // 	    if (0!= linkToTrackPB) {
      // 	      if(linkToTrackPB->isValid()) tempTrk1PB = linkToTrackPB->cachedElement(); 
      // 	    } 
      // 	  }
      // 	  if ( tempTrk1PB!=NULL){  
      // 	    const Trk::TrackSummary* summary1 = tempTrk1PB->trackSummary();
      // 	    if (summary1 != NULL){
      // 	      nSiliconHits_trk1 = summary1->get(Trk::numberOfSCTHits)+ summary1->get(Trk::numberOfPixelHits);
      // 	    }
      // 	  }
      // 	  Trk::VxTrackAtVertex* tmpTrkAtVtx2 = trkAtVxPtr->at(1);
      // 	  const Trk::ITrackLink * trLink2 =tmpTrkAtVtx2->trackOrParticleLink();
      // 	  const Trk::TrackParticleBase* tempTrk2PB(0);
      // 	  if (0!= trLink2) {
      // 	    const Trk::LinkToTrackParticleBase * linkToTrackPB =  dynamic_cast<const Trk::LinkToTrackParticleBase *>(trLink2);  
      // 	    if (0!= linkToTrackPB) {
      // 	      if (linkToTrackPB->isValid()) tempTrk2PB = linkToTrackPB->cachedElement(); 
      // 	    } 
      // 	  }
      // 	  if ( tempTrk2PB!=NULL) {
      // 	    const Trk::TrackSummary* summary2 = tempTrk2PB->trackSummary();
      // 	    if (summary2 != NULL){
      // 	      nSiliconHits_trk2 = summary2->get(Trk::numberOfSCTHits)+ summary2->get(Trk::numberOfPixelHits);
      // 	    }
      // 	  }
      // 	  ATH_MSG_DEBUG("Two track with (" << nSiliconHits_trk1 << ", " << nSiliconHits_trk2 <<  ") silicon hits.");
      // 	  if (nSiliconHits_trk1 > 3 && nSiliconHits_trk2 > 3) {
      // 	    m_histograms["EtaReco2T"]->Fill(recoeta);
      // 	    m_histograms["EtaTruth2T"]->Fill(trutheta);
      // 	    m_histograms["Resolution2T"]->Fill(recoeta - trutheta);
      // 	    m_histograms["PhiReco2T"]->Fill(recophi);
      // 	    m_histograms["PhiTruth2T"]->Fill(truthphi);
      // 	    m_histograms["PhiResolution2T"]->Fill(recophi - truthphi);
      // 	  } else {
      // 	    m_histograms["EtaReco2TTRT"]->Fill(recoeta);
      // 	    m_histograms["EtaTruth2TTRT"]->Fill(trutheta);
      // 	    m_histograms["Resolution2TTRT"]->Fill(recoeta - trutheta);
      // 	    m_histograms["PhiReco2TTRT"]->Fill(recophi);
      // 	    m_histograms["PhiTruth2TTRT"]->Fill(truthphi);
      // 	    m_histograms["PhiResolution2TTRT"]->Fill(recophi - truthphi);
      // 	  }
      // 	}
      //   } else {
      // 	// ATH_MSG_DEBUG("Unconverted photon");
      // 	m_histograms["EtaReco0T"]->Fill(recoeta);
      // 	m_histograms["EtaTruth0T"]->Fill(trutheta);
      // 	m_histograms["Resolution0T"]->Fill(recoeta - trutheta);
      // 	m_histograms["PhiReco0T"]->Fill(recophi);
      // 	m_histograms["PhiTruth0T"]->Fill(truthphi);
      // 	m_histograms["PhiResolution0T"]->Fill(recophi - truthphi);
      //   }
      // }
    }
  } // loop over photons


  // // loop over egamma objects
  // for (egammaContainer::const_iterator eg  = egammas->begin();
  //      eg != egammas->end();
  //      eg++) {

  //   const EMShower* shower = (*eg)->detail<EMShower>();
  //   if (shower) {
  //     std::cout << "About to print out egamma shower" << std::endl;
  //     shower->print();
  //   } else {
  //     ATH_MSG_WARNING("egamma object has no shower container");
  //   }

  //   // ATH_MSG_DEBUG("egamma with author " << (*eg)->author() << " has number of details: " << (*eg)->nDetails());
  //   // for (int i = 0; i < (*eg)->nDetails(); i++) {
  //   //   ATH_MSG_DEBUG("egamma detail: " << (*eg)->detailName(i));
  //   // }
  //   // ATH_MSG_DEBUG("");

  // }
  // // loop over conversion vertices
  // for (VxContainer::const_iterator convVtxIt = convcoll->begin();
  //      convVtxIt != convcoll->end(); convVtxIt++) {
  //   ATH_MSG_DEBUG("Looping over conversions:");
  //   const std::vector<Trk::VxTrackAtVertex*> *trkAtVxPtr = (*convVtxIt)->vxTrackAtVertex();
  //   if (trkAtVxPtr->size() == 1) {
      
  //     // first track
  //     Trk::VxTrackAtVertex* tmpTrkAtVtx1 = trkAtVxPtr->at(0);
  //     const Trk::ITrackLink * trLink =tmpTrkAtVtx1->trackOrParticleLink();
  //     const Trk::TrackParticleBase* tempTrk1PB(0);
  //     if (0!= trLink) {
  // 	const Trk::LinkToTrackParticleBase * linkToTrackPB =  dynamic_cast<const Trk::LinkToTrackParticleBase *>(trLink);  
  // 	if (0!= linkToTrackPB) {
  // 	  if(linkToTrackPB->isValid()) tempTrk1PB = linkToTrackPB->cachedElement(); 
  // 	} 
  //     }
  //     if ( tempTrk1PB!=NULL){  
  // 	const Trk::TrackSummary* summary1 = tempTrk1PB->trackSummary();
  // 	if (summary1 != NULL){
  // 	  const int nSiliconHits_trk1 = summary1->get(Trk::numberOfSCTHits) + 
  // 	    summary1->get(Trk::numberOfPixelHits);
  // 	  const double pid1 = summary1->getPID(Trk::eProbabilityComb);
  // 	  m_histograms["TRTPID1Trk"]->Fill(pid1);
  // 	  if (nSiliconHits_trk1 < 4) {
  // 	    m_histograms["TRTPID1TrkTRT"]->Fill(pid1);
  // 	  }
  // 	}
  //     }

  //   }
  // } // end of loop over conversion vertices

  return sc;

}


