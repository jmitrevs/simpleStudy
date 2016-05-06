///////////////////////////////////////////////////////////////////
// TestAlg.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

//#include <TDirectory.h>
//#include <TFile.h>
//#include <TROOT.h>

// #include "EventInfo/EventInfo.h"
// #include "EventInfo/EventID.h"
// #include "EventInfo/EventType.h"

#include "xAODEventInfo/EventInfo.h"

//#include "AthenaKernel/errorcheck.h"

#include "TH1.h"
//#include "McParticleEvent/TruthParticle.h"
//#include "McParticleEvent/TruthParticleContainer.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/EgammaDefs.h"

#include "MCTruthClassifier/MCTruthClassifierDefs.h"
#include "xAODTruth/xAODTruthHelpers.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthVertexContainer.h"

#include "AnalysisUtils/AnalysisMisc.h"

//#include "simpleStudy/TruthUtils.h"

//#include "ElectronPhotonSelectorTools/IAthElectronIsEMSelector.h"
//#include "ElectronPhotonSelectorTools/IAthPhotonIsEMSelector.h"
#include "PATCore/IAsgSelectionTool.h"

#include "simpleStudy/TestAlg.h"

// #include "egammaInterfaces/IEMBremsstrahlungBuilder.h"
// #include "egammaInterfaces/IEMFourMomBuilder.h"

//#include "VxVertex/ExtendedVxCandidate.h"
#include "FourMomUtils/xAODP4Helpers.h"

//#include "PhotonAnalysisUtils/IPAUcaloIsolationTool.h"

#include "GeneratorObjects/McEventCollection.h"
 
#include "HepMC/GenParticle.h"
#include "HepMC/GenVertex.h"


#include <gsl/gsl_math.h>
#include <set>

using CLHEP::GeV;

//================ Constructor =================================================

TestAlg::TestAlg(const std::string& name, 
		 ISvcLocator* pSvcLocator) : 
  AthAlgorithm(name,pSvcLocator)
{

  declareProperty("HistFileName", m_histFileName = "TestHistograms");

  declareProperty("ElectronSelector", m_electronSelector);
  declareProperty("PhotonSelector", m_photonSelector);

  declareProperty("DoTruth", m_doTruth = false);
  declareProperty("DoElectrons", m_doElectrons = false);
  declareProperty("DoPhotons", m_doPhotons = false);

  /** Electron selection */
  declareProperty("ElectronContainerName", m_ElectronContainerName = "Electrons");
  declareProperty("ElectronPt",   m_electronPt=10*GeV);
  declareProperty("ElectronEta",  m_electronEta=3.2);
  declareProperty("ElectronIsEMFlag", m_electronIsEMFlag="Medium");
  declareProperty("ElectronIsEM", m_electronIsEM=0);

  declareProperty("TruthElectronPtMin", m_truthElectronPtMin = 5*GeV);

  declareProperty("PhotonContainerName", m_PhotonContainerName = "Photons");
  declareProperty("PhotonPt",   m_photonPt=10*GeV);
  declareProperty("PhotonEta",  m_photonEta=3.2);
  declareProperty("PhotonIsEMFlag", m_photonIsEMFlag="Tight");
  declareProperty("PhotonIsEM", m_photonIsEM=0);
  declareProperty("PhotonAuthor", m_photonAuthor = xAOD::EgammaParameters::AuthorALL);

  declareProperty("METContainerName", m_METContainerName = "MET_LocHadTopo");

  declareProperty("egammaContainerName", m_egammaContainerName = "HLT_egamma");

  declareProperty("TruthPhotonPtMin", m_truthPhotonPtMin = 5*GeV);
  declareProperty("TruthMatchPhotons", m_truthMatchPhotons = true, "Require photons to be truth-matched");

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

  // Name of the McEventCollection Container
  declareProperty("McEventCollectionContainerName",
		  m_McEventCollectionContainerName="TruthEvent",
		  "Name of the McEventCollection container");

  declareProperty("xAODTruthEventContainerName",
		  m_xAODTruthEventContainerName="TruthEvents",
		  "Name of the xAOD::TruthEvent container");
  declareProperty("xAODTruthParticleContainerName",
		  m_xAODTruthParticleContainerName="TruthParticles",
		  "Name of the xAOD::TruthParticle container");
  declareProperty("xAODTruthVertexContainerName",
		  m_xAODTruthVertexContainerName="TruthVertices",
		  "Name of the xAOD::TruthVertex container");
  
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

  // if (m_doTruth) {
  //   if(m_MCTruthClassifier.retrieve().isFailure()) {
  //     ATH_MSG_ERROR("Failed to retrieve " << m_MCTruthClassifier);
  //     return StatusCode::FAILURE; // why success?
  //   }
  //   else {
  //     ATH_MSG_DEBUG("Retrieved MCTruthClassifier " << m_MCTruthClassifier);   
  //   }
  // }

  if (m_electronSelector.empty()) {
    ATH_MSG_DEBUG("The electron selector is empty");
  } else {
    if(m_electronSelector.retrieve().isFailure()) {
      ATH_MSG_ERROR("Failed to retrieve " << m_electronSelector);
      return StatusCode::FAILURE; // why success?
    }
    else {
      ATH_MSG_DEBUG("Retrieved ElectronSelector " << m_electronSelector);   
    }
  }

  if (m_photonSelector.empty()) {
    ATH_MSG_DEBUG("The photon selector is empty");
  } else {
    if(m_photonSelector.retrieve().isFailure()) {
      ATH_MSG_ERROR("Failed to retrieve " << m_photonSelector);
      return StatusCode::FAILURE; // why success?
    }
    else {
      ATH_MSG_DEBUG("Retrieved PhotonSelector " << m_photonSelector);   
    }
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

  const Int_t numEResBins = 220;
  const Double_t EResLow = -1.1;
  const Double_t EResHigh = 1.1;

  /// Defining Histogramsquer
  m_histograms["EResolution"] = new TH1F("EResolution","Raw Energy Resolution;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolutionC"] = new TH1F("EResolutionC","Raw Energy Resolution, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolutionEC"] = new TH1F("EResolutionEC","Raw Energy Resolution, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco"] = new TH1F("EtaReco","Reco Psuedorapidity;#eta_{reco}", 100, -3,3);

  // only for unconverted
  m_histograms["EResolution0T"] = new TH1F("EResolution0T","Raw Energy Resolution, unconverted;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution0TC"] = new TH1F("EResolution0TC","Raw Energy Resolution, unconverted, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution0TEC"] = new TH1F("EResolution0TEC","Raw Energy Resolution, unconverted, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco0T"] = new TH1F("EtaReco0T","Reco Psuedorapidity, unconverted;#eta_{reco}", 100, -3,3);

  // only for Si
  m_histograms["EResolution1TSi"] = new TH1F("EResolution1TSi","Raw Energy Resolution, 1-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TSiC"] = new TH1F("EResolution1TSiC","Raw Energy Resolution, 1-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TSiEC"] = new TH1F("EResolution1TSiEC","Raw Energy Resolution, 1-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco1TSi"] = new TH1F("EtaReco1TSi","Reco Psuedorapidity, 1-track Si conversion;#eta_{reco}", 100, -3,3);

  m_histograms["EResolution2TSi"] = new TH1F("EResolution2TSi","Raw Energy Resolution, 2-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TSiC"] = new TH1F("EResolution2TSiC","Raw Energy Resolution, 2-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TSiEC"] = new TH1F("EResolution2TSiEC","Raw Energy Resolution, 2-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco2TSi"] = new TH1F("EtaReco2TSi","Reco Psuedorapidity, 2-track Si conversion;#eta_{reco}", 100, -3,3);

  // TRT
  m_histograms["EResolution1TTRT"] = new TH1F("EResolution1TTRT","Raw Energy Resolution, 1-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TTRTC"] = new TH1F("EResolution1TTRTC","Raw Energy Resolution, 1-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TTRTEC"] = new TH1F("EResolution1TTRTEC","Raw Energy Resolution, 1-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco1TTRT"] = new TH1F("EtaReco1TTRT","Reco Psuedorapidity, 1-track TRT conversion;#eta_{reco}", 100, -3,3);

  m_histograms["EResolution2TTRT"] = new TH1F("EResolution2TTRT","Raw Energy Resolution, 2-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TTRTC"] = new TH1F("EResolution2TTRTC","Raw Energy Resolution, 2-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TTRTEC"] = new TH1F("EResolution2TTRTEC","Raw Energy Resolution, 2-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco2TTRT"] = new TH1F("EtaReco2TTRT","Reco Psuedorapidity, 2-track TRT conversion;#eta_{reco}, End-cap", 100, -3,3);

  m_histograms["EResolution2TMix"] = new TH1F("EResolution2TMix","Raw Energy Resolution, 2-track Mix conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TMixC"] = new TH1F("EResolution2TMixC","Raw Energy Resolution, 2-track Mix conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TMixEC"] = new TH1F("EResolution2TMixEC","Raw Energy Resolution, 2-track Mix conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco2TMix"] = new TH1F("EtaReco2TMix","Reco Psuedorapidity, 2-track Mix conversion;#eta_{reco}", 100, -3,3);



  /// Registering Histograms
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution" , m_histograms["EResolution"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionC" , m_histograms["EResolutionC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionEC" , m_histograms["EResolutionEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco" , m_histograms["EtaReco"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0T" , m_histograms["EResolution0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TC" , m_histograms["EResolution0TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TEC" , m_histograms["EResolution0TEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco0T" , m_histograms["EtaReco0T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSi" , m_histograms["EResolution1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiC" , m_histograms["EResolution1TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiEC" , m_histograms["EResolution1TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1TSi" , m_histograms["EtaReco1TSi"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSi" , m_histograms["EResolution2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiC" , m_histograms["EResolution2TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiEC" , m_histograms["EResolution2TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TSi" , m_histograms["EtaReco2TSi"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRT" , m_histograms["EResolution1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTC" , m_histograms["EResolution1TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTEC" , m_histograms["EResolution1TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1TTRT" , m_histograms["EtaReco1TTRT"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRT" , m_histograms["EResolution2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTC" , m_histograms["EResolution2TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTEC" , m_histograms["EResolution2TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TTRT" , m_histograms["EtaReco2TTRT"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMix" , m_histograms["EResolution2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixC" , m_histograms["EResolution2TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixEC" , m_histograms["EResolution2TMixEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TMix" , m_histograms["EtaReco2TMix"]).ignore();

  

  // initialize some constants
  m_numPhotons = 0;
  m_numUnconverted = 0;

  m_numConversionsSingleTrackSi = 0;
  m_numConversionsDoubleTrackSi = 0;
  m_numConversionsSingleTrackTRT = 0;
  m_numConversionsDoubleTrackTRT = 0;
  m_numConversionsDoubleTrackMix = 0;

  m_numElectrons = 0;
  m_numTightElectrons = 0;
  m_numElectronsAuthorElectron = 0;
  m_numTightElectronsAuthorElectron = 0;
  m_numElectronsAuthorSofte = 0;
  m_numElectronsAuthorFrwd = 0;

  // ATH_MSG_DEBUG("detlaPhi(0.2,0.1) = " << P4Helpers::deltaPhi(0.2,0.1));
  // ATH_MSG_DEBUG("detlaPhi(0.1,0.2) = " << P4Helpers::deltaPhi(0.1,0.2));

  // ATH_MSG_DEBUG("detlaPhi(3.1,-3.1) = " << P4Helpers::deltaPhi(3.1,-3.1));
  // ATH_MSG_DEBUG("detlaPhi(-3.1,3.1) = " << P4Helpers::deltaPhi(-3.1,3.1));

  ATH_MSG_INFO("About to end initialize.");

  return StatusCode::SUCCESS;
}

//================ Finalisation =================================================

StatusCode TestAlg::finalize()
{
  const auto numConversionsSingle = m_numConversionsSingleTrackSi + m_numConversionsSingleTrackTRT;
  const auto numConversionsDouble = m_numConversionsDoubleTrackSi + m_numConversionsDoubleTrackTRT + m_numConversionsDoubleTrackMix;
  const auto numConversions = numConversionsSingle + numConversionsDouble;
  ATH_MSG_DEBUG(name()<<" finalize()");
  std::cout << "  **** EGAMMA STATISTICS ****\n";
  std::cout << "  numPhotons                               = " << m_numPhotons << std::endl;
  std::cout << "  numUnconverted                           = " << m_numUnconverted << std::endl;
  std::cout << "  numConversions                           = " << numConversions << std::endl;
  std::cout << "  numConversions Single Track              = " << numConversionsSingle << std::endl;
  std::cout << "  numConversions Double Track              = " << numConversionsDouble << std::endl;
  std::cout << "  numConversions Single Track Si           = " << m_numConversionsSingleTrackSi << std::endl;
  std::cout << "  numConversions Double Track Si           = " << m_numConversionsDoubleTrackSi << std::endl;
  std::cout << "  numConversions Single Track TRT          = " << m_numConversionsSingleTrackTRT << std::endl;
  std::cout << "  numConversions Double Track TRT          = " << m_numConversionsDoubleTrackTRT << std::endl;
  std::cout << "  numConversions Double Track Mix          = " << m_numConversionsDoubleTrackMix << std::endl;
  std::cout << "     * * *\n";
  std::cout << "  numElectrons                             = " << m_numElectrons << std::endl;
  std::cout << "  numElectrons author Electron             = " << m_numElectronsAuthorElectron << std::endl;
  std::cout << "  numElectrons author Softe                = " << m_numElectronsAuthorSofte << std::endl;
  std::cout << "  numElectrons author Frwd                 = " << m_numElectronsAuthorFrwd << std::endl;
  std::cout << "  numTightElectrons                        = " << m_numTightElectrons << std::endl;
  std::cout << "  numTightElectrons author Electron        = " << m_numTightElectronsAuthorElectron << std::endl;
  // std::cout << "  numConversion in Electron Container      = " << m_numElConversions << std::endl;

  return StatusCode::SUCCESS;
}

//================ Execution ====================================================

StatusCode TestAlg::execute()
{ 

  StatusCode sc = StatusCode::SUCCESS;

  static int count = 0;
  //ATH_MSG_WARNING("Skip event: " << count++); 
  // ATH_MSG_DEBUG("Electron container name: " << m_ElectronContainerName);

  const xAOD::EventInfo*  evtInfo = 0;
  sc = evtStore()->retrieve(evtInfo);
  if(sc.isFailure() || !evtInfo) {
    ATH_MSG_ERROR("could not retrieve event info");
    return StatusCode::RECOVERABLE;
  }
  
  const auto eventNumber = evtInfo->eventNumber();
  const auto runNumber = evtInfo->runNumber();
  const auto lumiBlock = evtInfo->lumiBlock();
  if (m_runOnlySome && m_theEvents.find(eventNumber) == m_theEvents.end()) {
    // skip the event
    return StatusCode::SUCCESS;
  }

  const bool isMC = evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION);

  ATH_MSG_INFO("Run: " << runNumber << ", lumi = " << lumiBlock << ", Event: " << eventNumber << ", isMC = " << isMC);

  const xAOD::ElectronContainer* electrons(0);
  if (m_doElectrons) {
    sc=evtStore()->retrieve( electrons, m_ElectronContainerName);
    if( sc.isFailure()  ||  !electrons ) {
      ATH_MSG_ERROR("No continer "<< m_ElectronContainerName <<" container found in TDS");
      return StatusCode::FAILURE;
    }
  }
  // ATH_MSG_DEBUG("Photon container name: " << m_PhotonContainerName);

  const xAOD::PhotonContainer* photons(0);
  if (m_doPhotons) {
    sc=evtStore()->retrieve( photons, m_PhotonContainerName);
    if( sc.isFailure()  ||  !photons ) {
      ATH_MSG_ERROR("No continer "<< m_PhotonContainerName <<" container found in TDS");
      return StatusCode::FAILURE;
    }
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
  
  // // retrieve Conversion Container
  // const VxContainer* convcoll;
  // if (evtStore()->contains<VxContainer>(m_ConversionsName)) {
  //   StatusCode sc = evtStore()->retrieve(convcoll,m_ConversionsName);
  //   if(sc.isFailure()) {
  //     ATH_MSG_ERROR("Could not retrieve Conversion container."); 
  //     return StatusCode::FAILURE;
  //   }
  // } else {
  //   ATH_MSG_DEBUG("Could not find Conversion container: name doesn't exist"); 
  //   return StatusCode::FAILURE;
  // }

  // /// Load Truth Container
  // const TruthParticleContainer*  mcpartTES;
  // sc=evtStore()->retrieve( mcpartTES, "SpclMC");
  // if( sc.isFailure()  ||  !mcpartTES ) {
  //   ATH_MSG_ERROR("No AOD MC truth particle container found in TDS");
  //   return StatusCode::FAILURE;
  // }

  // const HepMC::GenEvent *ge=mcpartTES->genEvent();
  // m_TruthUtils->FillTruthMap(ge, m_truthPhotonPtMin);

  // and the actual McEventCollection

  const McEventCollection* mcEventCollection(0);
  const xAOD::TruthEventContainer *truthEvents(0);
  const xAOD::TruthParticleContainer *truthParticles(0);
  const xAOD::TruthVertexContainer *truthVertices(0);
  if (m_doTruth) {
    if (evtStore()->contains<McEventCollection>(m_McEventCollectionContainerName)) {
      CHECK(evtStore()->retrieve(mcEventCollection,m_McEventCollectionContainerName));
    }
  
   if (evtStore()->contains<xAOD::TruthEventContainer>(m_xAODTruthEventContainerName)) {
     CHECK(evtStore()->retrieve(truthEvents,m_xAODTruthEventContainerName));
   }

   if (evtStore()->contains<xAOD::TruthParticleContainer>(m_xAODTruthParticleContainerName)) {
     CHECK(evtStore()->retrieve(truthParticles,m_xAODTruthParticleContainerName));
   }

   if (evtStore()->contains<xAOD::TruthVertexContainer>(m_xAODTruthVertexContainerName)) {
     CHECK(evtStore()->retrieve(truthVertices,m_xAODTruthVertexContainerName));
   }
  }
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
  //Analysis::Electron *leading = 0;
  //Analysis::Electron *second = 0;

  //double leadPt = 0;
  //double secondPt = 0;

  if (m_doElectrons) {
    // loop over electrons
    for (xAOD::ElectronContainer::const_iterator el  = electrons->begin();
	 el != electrons->end();
	 el++) {
    
      // try the selector

      if (xAOD::EgammaHelpers::isElectron(*el)) {
	const Root::TAccept& acc = m_electronSelector->accept(*el);
      
	if (acc) {
	  numElPass++;
	  ATH_MSG_DEBUG("Passed electron selector.");
	} else {
	  ATH_MSG_DEBUG("Failed electron selector.");
	}
      }
      bool passTruth = true;
      // if (m_doTruth) {
      //   std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> res =
      // 	m_MCTruthClassifier->particleTruthClassifier(*el);
      //   passTruth = (res.first == MCTruthPartClassifier::IsoElectron);
      // }

      if (passTruth) {

	m_numElectrons++;
	// if ((*el)->author(egammaParameters::AuthorElectron)) {
	// 	numEl++;
	// 	numElectronsAuthorElectron++;
	// }
	// if ((*el)->author(egammaParameters::AuthorSofte)) numElectronsAuthorSofte++;
	// if ((*el)->author(egammaParameters::AuthorFrwd)) numElectronsAuthorFrwd++;

	ATH_MSG_INFO("Electron with author " << (*el)->author() << ", pt == " << (*el)->pt() 
		     << ", eta = " << (*el)->eta() << ", phi = " << (*el)->phi());

	ATH_MSG_INFO("Cluster eta = " << (*el)->caloCluster()->eta());

      
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

	//ATH_MSG_INFO("Electron OQ = " << std::hex << (*el)->isgoodoq());

	//m_histograms["ElEtaReco"]->Fill((*el)->eta());

	//const EMShower* shower = (*el)->detail<EMShower>();
	// std::cout << "About to print out electron shower" << std::endl;
	//shower->print();

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

	const xAOD::TrackParticle * trParticle = (*el)->trackParticle();
	if (!trParticle) {
	  ATH_MSG_WARNING("Electron has no track-particle");
	  continue;
	}

	ATH_MSG_DEBUG("   Track phi = " << trParticle->phi() 
		      << ", eta = "<< trParticle->eta() 
		      << ", pt = " << trParticle->pt()
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

      

	uint8_t nBL = 0;
	uint8_t nBLOutliers = 0;
	// number of Pixel hits
	uint8_t nPi = 0;
	uint8_t nPiOutliers = 0;
	// number of SCT hits
	uint8_t nSCT = 0;
	uint8_t nSCTOutliers = 0;
	uint8_t nTRThigh          = 0;
	uint8_t nTRThighOutliers  = 0;
	uint8_t nTRT         = 0;
	uint8_t nTRTOutliers = 0;
	uint8_t nTRTXenonHits = 0;
	uint8_t expectHitInBLayer = true;
      
	bool allFound = true;
      
	allFound &= trParticle->summaryValue(nBL, xAOD::numberOfBLayerHits);
	allFound &= trParticle->summaryValue(nPi, xAOD::numberOfPixelHits);
	allFound &= trParticle->summaryValue(nSCT, xAOD::numberOfSCTHits);
	allFound &= trParticle->summaryValue(nBLOutliers, xAOD::numberOfBLayerOutliers);
	allFound &= trParticle->summaryValue(nPiOutliers, xAOD::numberOfPixelOutliers);
	allFound &= trParticle->summaryValue(nSCTOutliers, xAOD::numberOfSCTOutliers);
      
	allFound &= trParticle->summaryValue(nTRThigh, xAOD::numberOfTRTHighThresholdHits);
	allFound &= trParticle->summaryValue(nTRThighOutliers, xAOD::numberOfTRTHighThresholdOutliers);
	allFound &= trParticle->summaryValue(nTRT, xAOD::numberOfTRTHits);
	allFound &= trParticle->summaryValue(nTRTOutliers, xAOD::numberOfTRTOutliers);
	allFound &= trParticle->summaryValue(nTRTXenonHits, xAOD::numberOfTRTXenonHits);
      
	allFound &= trParticle->summaryValue(expectHitInBLayer, xAOD::expectBLayerHit);

	if (!allFound) {
	  ATH_MSG_WARNING("Not all track values found");
	}

	int nSiliconHits_trk = nPi + nSCT;

	ATH_MSG_DEBUG("   Number of silicon hits = " << (int) nSiliconHits_trk << ", number of b-layer hits = " << (int) nBL 
		      << ", expected hit in b-layer = " << (bool) expectHitInBLayer);


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
    } // loop over electrons
  }

  ATH_MSG_DEBUG("The event had " << numEl << " electrons out of which " << numElPass << " passed tight.");

  //m_histograms["numEl"]->Fill(numElPass);


  // if (numElPass > 1) {
  //   const double minv = P4Helpers::invMass(leading, second);
  //   m_histograms["minv"]->Fill(minv);

  //   // if (minv > 71*GeV && minv < 111*GeV) {
  //   //   m_histograms["met"]->Fill(met->et());
  //   // }
  // }



  ATH_MSG_DEBUG("About to start photons");

  if (m_doPhotons) {
    std::set<const xAOD::TruthParticle *> seenTruths;
    for (auto ph : *photons) {

      const xAOD::TruthParticle *truthParticle{nullptr};
      int truthType{0};
      int truthOrigin{0};

      bool alreadySeen = false;

      if (isMC) {
	truthParticle = xAOD::TruthHelpers::getTruthParticle(*ph);
	truthType = xAOD::TruthHelpers::getParticleTruthType(*ph);
	truthOrigin = xAOD::TruthHelpers::getParticleTruthOrigin(*ph);

	ATH_MSG_DEBUG("Truth-match photon: type: " << truthType << ", origin: " 
		      << truthOrigin << ", TruthParticle*: " << truthParticle);

	if (seenTruths.count(truthParticle)) {
	  // have already seen the particle
	  alreadySeen = true;
	  ATH_MSG_DEBUG("Have already seen a partcle matched to same truth");
	} else {
	  seenTruths.insert(truthParticle);
	}
      }


      if (ph->author(m_photonAuthor) && 
	  (!m_truthMatchPhotons || (truthType == MCTruthPartClassifier::IsoPhoton && !alreadySeen))) {

	m_numPhotons++;

	ATH_MSG_DEBUG("author is = " << ph->author());

	const auto photonType = xAOD::EgammaHelpers::conversionType(ph);
	const auto Ereco = ph->caloCluster()->rawE();
	const auto Etruth = (truthParticle) ? truthParticle->e() : 0.0;
	const auto Eres = (Ereco - Etruth)/Etruth;
	const auto eta = ph->eta();
	const auto eta2 = ph->caloCluster()->etaBE(2);
	const auto etaTruth = (truthParticle) ? truthParticle->eta() : -999.0;
	const bool isC = std::abs(eta2) <= 1.37;
	const bool isEC = std::abs(eta2) >= 1.52;

	ATH_MSG_INFO("Photon with pt = " << ph->pt()
		     << ", eta = " << eta 
		     << ", eta2 = " << eta2
		     << ", phi = " << ph->phi());


	// first do general all-photons
	fillPhotonHists("", isC, isEC, eta, Eres);

	switch(photonType) {
	case xAOD::EgammaParameters::unconverted:
	  m_numUnconverted++;
	  fillPhotonHists("0T", isC, isEC, eta, Eres);
	  if (Eres < -0.95 && isEC) {
	    ATH_MSG_WARNING("Event " << runNumber << ", " << lumiBlock << ", " << eventNumber 
			    << ", Eres = " << Eres
			    << ", pt = " << ph->pt()
			    // << ", eta = " << eta 
			    << ", eta2 = " << eta2
			    << ", etaTruth = " << etaTruth
			    // << ", phi = " << ph->phi()
			    << ", Ereco = " << Ereco
			    << ", Etruth = " << Etruth
			    << ", author = " << ph->author()
			    );

	    // let's find if there are better-matched nearby cells
	    
	    for (auto ph2 : *photons) {
	      if (ph != ph2 && xAOD::P4Helpers::isInDeltaR(*(ph->caloCluster()), *(ph2->caloCluster()), 0.4, false)) {
		const xAOD::TruthParticle *truthParticle2{nullptr};
		int truthType2{0};

		if (isMC) {
		  truthParticle2 = xAOD::TruthHelpers::getTruthParticle(*ph2);
		  truthType2 = xAOD::TruthHelpers::getParticleTruthType(*ph2);
		}

		const std::string match = 
		  (truthParticle == truthParticle2) ? 
		  "match same truthParticle" : "do not match same truthParticle";

		ATH_MSG_WARNING("Found cluster with energy: " << ph2->caloCluster()->rawE() 
				<< ", deltaR = " << xAOD::P4Helpers::deltaR(ph->caloCluster(), ph2->caloCluster(), false)
				<< ", truthType = " << truthType2
				<< ", " << match
				);
	      }
	    }
	  }
	  break;
	case xAOD::EgammaParameters::singleSi:
	  m_numConversionsSingleTrackSi++;
	  fillPhotonHists("1TSi", isC, isEC, eta, Eres);
	  break;
	case xAOD::EgammaParameters::singleTRT:
	  m_numConversionsSingleTrackTRT++;
	  fillPhotonHists("1TTRT", isC, isEC, eta, Eres);
	  break;
	case xAOD::EgammaParameters::doubleSi:
	  m_numConversionsDoubleTrackSi++;
	  fillPhotonHists("2TSi", isC, isEC, eta, Eres);
	  break;
	case xAOD::EgammaParameters::doubleTRT:
	  m_numConversionsDoubleTrackTRT++;
	  fillPhotonHists("2TTRT", isC, isEC, eta, Eres);
	  break;
	case xAOD::EgammaParameters::doubleSiTRT:
	  m_numConversionsDoubleTrackMix++;
	  fillPhotonHists("2TMix", isC, isEC, eta, Eres);
	  break;
	default:
	  ATH_MSG_ERROR("Uknown photon type: " << photonType);
	  return StatusCode::FAILURE;
	}

     }
    } // loop over photons
  }

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

  if (m_doTruth) {
    // loop over xAODTruthParticles
    if (truthParticles) {
      for (auto tp : *truthParticles) {
	if (tp->eta() == 0 && tp->phi() == 0) {
	  ATH_MSG_WARNING("TruthPartcile with zero eta and phi; pt = " << tp->pt()
			  << ", status = " << tp->status()
			  << ", pdg_id = " << tp->pdgId());
	}
      }
    }
    
    if (mcEventCollection) {
      size_t size = mcEventCollection->size();
      ATH_MSG_INFO("Number of GenEvent = " << size);
      for (size_t i = 0; i < size; i++) {
	auto ge = mcEventCollection->at(i);
	
	ATH_MSG_DEBUG("Looking at a new GenEvent");
	for (auto pcl = ge->particles_begin(); pcl!= ge->particles_end(); ++pcl) {
	  const auto& p = (*pcl)->momentum();
	  ATH_MSG_INFO("GenPartcile in event " << i << "pt = " << p.perp()
		       << ", eta = " << p.eta()
		       << ", phi = " << p.phi()
		       << ", status = " << (*pcl)->status()
		       << ", pdg_id = " << (*pcl)->pdg_id());
	}
      }
    }
  }

  return sc;

}


void TestAlg::fillPhotonHists(std::string suffix, 
			      bool isC, bool isEC, 
			      float eta, float Eres)
{
 
  const std::string etastr = "EtaReco" + suffix;
  const std::string Eresstr = "EResolution" + suffix;

  m_histograms.at(etastr)->Fill(eta);
  m_histograms.at(Eresstr)->Fill(Eres);
  if (isC) {
    const std::string EresstrC = Eresstr + "C";
    m_histograms.at(EresstrC)->Fill(Eres);
  } else if (isEC) {
    const std::string EresstrEC = Eresstr + "EC";
    m_histograms.at(EresstrEC)->Fill(Eres);
  }
}
 
