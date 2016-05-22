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
using CLHEP::TeV;

namespace {
  bool isBestMatch(const xAOD::TruthParticle *truthParticle, 
		   const xAOD::Photon *ph, 
		   const xAOD::TruthParticleContainer *egTruthContainer);
}

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
  declareProperty("ElectronAuthor", m_electronAuthor = xAOD::EgammaParameters::AuthorALL);

  //declareProperty("TruthElectronPtMin", m_truthElectronPtMin = 5*GeV);
  declareProperty("TruthMatchElectrons", m_truthMatchElectrons = false, "Require electrons to be truth-matched");
  declareProperty("TruthMatchElectronAsPhotons", m_truthMatchElectronAsPhotons = true, "Require electrons to be truth-matched to photons");

  declareProperty("PhotonContainerName", m_PhotonContainerName = "Photons");
  declareProperty("PhotonPt",   m_photonPt=10*GeV);
  declareProperty("PhotonEta",  m_photonEta=3.2);
  declareProperty("PhotonIsEMFlag", m_photonIsEMFlag="Tight");
  declareProperty("PhotonIsEM", m_photonIsEM=0);
  declareProperty("PhotonAuthor", m_photonAuthor = xAOD::EgammaParameters::AuthorALL);

 //declareProperty("TruthPhotonPtMin", m_truthPhotonPtMin = 5*GeV);
  declareProperty("TruthMatchPhotons", m_truthMatchPhotons = true, "Require photons to be truth-matched");

  declareProperty("EgammaTruthContainerName", m_egammaTruthParticleContainerName = "egammaTruthParticles",
    "Name of the output egamma truth particle container");


  //declareProperty("METContainerName", m_METContainerName = "MET_LocHadTopo");

  declareProperty("egammaContainerName", m_egammaContainerName = "HLT_egamma");

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

  const Int_t numPtBins = 200;
  const Double_t PtLow = 0; // in GeV, not MeV
  const Double_t PtHigh = 4000; // in GeV, not MeV

  /// Defining Histogramsquer
  m_histograms["EResolution"] = new TH1F("EResolution","Raw Energy Resolution;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolutionC"] = new TH1F("EResolutionC","Raw Energy Resolution, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolutionEC"] = new TH1F("EResolutionEC","Raw Energy Resolution, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco"] = new TH1F("EtaReco","Reco Psuedorapidity;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco"] = new TH1F("PtReco","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtRecoC"] = new TH1F("PtRecoC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtRecoEC"] = new TH1F("PtRecoEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  // only for unconverted
  m_histograms["EResolution0T"] = new TH1F("EResolution0T","Raw Energy Resolution, unconverted;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution0TC"] = new TH1F("EResolution0TC","Raw Energy Resolution, unconverted, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution0TEC"] = new TH1F("EResolution0TEC","Raw Energy Resolution, unconverted, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco0T"] = new TH1F("EtaReco0T","Reco Psuedorapidity, unconverted;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco0T"] = new TH1F("PtReco0T","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco0TC"] = new TH1F("PtReco0TC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco0TEC"] = new TH1F("PtReco0TEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  // only for Si
  m_histograms["EResolution1TSi"] = new TH1F("EResolution1TSi","Raw Energy Resolution, 1-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TSiC"] = new TH1F("EResolution1TSiC","Raw Energy Resolution, 1-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TSiEC"] = new TH1F("EResolution1TSiEC","Raw Energy Resolution, 1-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco1TSi"] = new TH1F("EtaReco1TSi","Reco Psuedorapidity, 1-track Si conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco1TSi"] = new TH1F("PtReco1TSi","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TSiC"] = new TH1F("PtReco1TSiC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TSiEC"] = new TH1F("PtReco1TSiEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution2TSi"] = new TH1F("EResolution2TSi","Raw Energy Resolution, 2-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TSiC"] = new TH1F("EResolution2TSiC","Raw Energy Resolution, 2-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TSiEC"] = new TH1F("EResolution2TSiEC","Raw Energy Resolution, 2-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco2TSi"] = new TH1F("EtaReco2TSi","Reco Psuedorapidity, 2-track Si conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco2TSi"] = new TH1F("PtReco2TSi","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TSiC"] = new TH1F("PtReco2TSiC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TSiEC"] = new TH1F("PtReco2TSiEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  // TRT
  m_histograms["EResolution1TTRT"] = new TH1F("EResolution1TTRT","Raw Energy Resolution, 1-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TTRTC"] = new TH1F("EResolution1TTRTC","Raw Energy Resolution, 1-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution1TTRTEC"] = new TH1F("EResolution1TTRTEC","Raw Energy Resolution, 1-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco1TTRT"] = new TH1F("EtaReco1TTRT","Reco Psuedorapidity, 1-track TRT conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco1TTRT"] = new TH1F("PtReco1TTRT","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TTRTC"] = new TH1F("PtReco1TTRTC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TTRTEC"] = new TH1F("PtReco1TTRTEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution2TTRT"] = new TH1F("EResolution2TTRT","Raw Energy Resolution, 2-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TTRTC"] = new TH1F("EResolution2TTRTC","Raw Energy Resolution, 2-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TTRTEC"] = new TH1F("EResolution2TTRTEC","Raw Energy Resolution, 2-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco2TTRT"] = new TH1F("EtaReco2TTRT","Reco Psuedorapidity, 2-track TRT conversion;#eta_{reco}, End-cap", 100, -3,3);
  m_histograms["PtReco2TTRT"] = new TH1F("PtReco2TTRT","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TTRTC"] = new TH1F("PtReco2TTRTC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TTRTEC"] = new TH1F("PtReco2TTRTEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution2TMix"] = new TH1F("EResolution2TMix","Raw Energy Resolution, 2-track Mix conversion;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TMixC"] = new TH1F("EResolution2TMixC","Raw Energy Resolution, 2-track Mix conversion, Central;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EResolution2TMixEC"] = new TH1F("EResolution2TMixEC","Raw Energy Resolution, 2-track Mix conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", numEResBins, EResLow, EResHigh);
  m_histograms["EtaReco2TMix"] = new TH1F("EtaReco2TMix","Reco Psuedorapidity, 2-track Mix conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco2TMix"] = new TH1F("PtReco2TMix","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TMixC"] = new TH1F("PtReco2TMixC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TMixEC"] = new TH1F("PtReco2TMixEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);


  // Now for electron histograms 
  m_histograms["ElEtaReco"] = new TH1F("ElEtaReco","Electron reco Psuedorapidity;#eta_{reco}", 100, -3,3);
  m_histograms["ElPtReco"] = new TH1F("ElPtReco","Electron reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["ElPtRecoC"] = new TH1F("ElPtRecoC","Electron reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["ElPtRecoEC"] = new TH1F("ElPtRecoEC","Electron reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["NumBLHits"] = new TH1F("NumBLHits","Electron Number BLayer Hits;N_{hits}", 3, -0.5, 2.5);
  m_histograms["NumBLHitsC"] = new TH1F("NumBLHitsC","Electron Number BLayer Hits;N_{hits}", 3, -0.5, 2.5);
  m_histograms["NumBLHitsEC"] = new TH1F("NumBLHitsEC","Electron Number BLayer Hits;N_{hits}", 3, -0.5, 2.5);

  m_histograms["NumBLHitsExp"] = new TH1F("NumBLHitsExp","Electron Number BLayer Hits (assume hit if not expected);N_{hits}", 3, -0.5, 2.5);
  m_histograms["NumBLHitsExpC"] = new TH1F("NumBLHitsExpC","Electron Number BLayer Hits (assume hit if not expected);N_{hits}", 3, -0.5, 2.5);
  m_histograms["NumBLHitsExpEC"] = new TH1F("NumBLHitsExpEC","Electron Number BLayer Hits (assume hit if not expected);N_{hits}", 3, -0.5, 2.5);

  m_histograms["NumBLHitsOutls"] = new TH1F("NumBLHitsOutls","Electron Number BLayer Hits + Outliers;N_{hits+outls}", 3, -0.5, 2.5);
  m_histograms["NumBLHitsOutlsC"] = new TH1F("NumBLHitsOutlsC","Electron Number BLayer Hits + Outliers;N_{hits+outls}", 3, -0.5, 2.5);
  m_histograms["NumBLHitsOutlsEC"] = new TH1F("NumBLHitsOutlsEC","Electron Number BLayer Hits + Outliers;N_{hits+outls}", 3, -0.5, 2.5);

  m_histograms["NumPixHits"] = new TH1F("NumPixHits","Electron Number Pixel Hits;N_{hits}", 6, -0.5, 5.5);
  m_histograms["NumPixHitsC"] = new TH1F("NumPixHitsC","Electron Number Pixel Hits;N_{hits}", 6, -0.5, 5.5);
  m_histograms["NumPixHitsEC"] = new TH1F("NumPixHitsEC","Electron Number Pixel Hits;N_{hits}", 6, -0.5, 5.5);

  m_histograms["NumPixHitsOutls"] = new TH1F("NumPixHitsOutls","Electron Number Pixel Hits + Outliers;N_{hits+outls}", 6, -0.5, 5.5);
  m_histograms["NumPixHitsOutlsC"] = new TH1F("NumPixHitsOutlsC","Electron Number Pixel Hits + Outliers;N_{hits+outls}", 6, -0.5, 5.5);
  m_histograms["NumPixHitsOutlsEC"] = new TH1F("NumPixHitsOutlsEC","Electron Number Pixel Hits + Outliers;N_{hits+outls}", 6, -0.5, 5.5);

  m_histograms["NumSiHits"] = new TH1F("NumSiHits","Electron Number Si Hits;N_{hits}", 15, -0.5, 14.5);
  m_histograms["NumSiHitsC"] = new TH1F("NumSiHitsC","Electron Number Si Hits;N_{hits}", 15, -0.5, 14.5);
  m_histograms["NumSiHitsEC"] = new TH1F("NumSiHitsEC","Electron Number Si Hits;N_{hits}", 15, -0.5, 14.5);

  m_histograms["NumSiHitsOutls"] = new TH1F("NumSiHitsOutls","Electron Number Si Hits + Outliers;N_{hits+outls}", 15, -0.5, 14.5);
  m_histograms["NumSiHitsOutlsC"] = new TH1F("NumSiHitsOutlsC","Electron Number Si Hits + Outliers;N_{hits+outls}", 15, -0.5, 14.5);
  m_histograms["NumSiHitsOutlsEC"] = new TH1F("NumSiHitsOutlsEC","Electron Number Si Hits + Outliers;N_{hits+outls}", 15, -0.5, 14.5);
  

  const Int_t numEovpBins = 100;
  const Double_t EovpLow = 0;
  const Double_t EovpHigh = 30;
  m_histograms["Eovp"] = new TH1F("Eovp","E over p;E/p", numEovpBins, EovpLow, EovpHigh);
  m_histograms["EovpC"] = new TH1F("EovpC","E over p, Central;E/p", numEovpBins, EovpLow, EovpHigh);
  m_histograms["EovpEC"] = new TH1F("EovpEC","E over p, End-cap;E/p", numEovpBins, EovpLow, EovpHigh);

  const Int_t numDelPhiBins = 100;
  const Int_t numDelEtaBins = 100;
  const Double_t DelPhiLow = -0.3;
  const Double_t DelPhiHigh = 0.3;
  const Double_t DelEtaLow = -0.3;
  const Double_t DelEtaHigh = 0.3;
  m_histograms["DelPhi1"] = new TH1F("DelPhi1","Track match #Delta#phi (layer 1);#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhi1C"] = new TH1F("DelPhi1C","Track match #Delta#phi (layer 1) , Central;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhi1EC"] = new TH1F("DelPhi1EC","Track match #Delta#phi (layer 1), End-cap;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhi2"] = new TH1F("DelPhi2","Track match #Delta#phi (layer 2);#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhi2C"] = new TH1F("DelPhi2C","Track match #Delta#phi (layer 2) , Central;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhi2EC"] = new TH1F("DelPhi2EC","Track match #Delta#phi (layer 2), End-cap;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);

  m_histograms["DelPhiRescaled1"] = new TH1F("DelPhiRescaled1","Track match #Delta#phi (layer 1, rescaled);#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhiRescaled1C"] = new TH1F("DelPhiRescaled1C","Track match #Delta#phi (layer 1, rescaled) , Central;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhiRescaled1EC"] = new TH1F("DelPhiRescaled1EC","Track match #Delta#phi (layer 1, rescaled), End-cap;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhiRescaled2"] = new TH1F("DelPhiRescaled2","Track match #Delta#phi (layer 2, rescaled);#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhiRescaled2C"] = new TH1F("DelPhiRescaled2C","Track match #Delta#phi (layer 2, rescaled) , Central;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);
  m_histograms["DelPhiRescaled2EC"] = new TH1F("DelPhiRescaled2EC","Track match #Delta#phi (layer 2, rescaled), End-cap;#Delta#phi", numDelPhiBins, DelPhiLow, DelPhiHigh);

  m_histograms["DelEta2"] = new TH1F("DelEta2","Track match #Delta#eta (layer 2);#Delta#eta", numDelEtaBins, DelEtaLow, DelEtaHigh);
  m_histograms["DelEta2C"] = new TH1F("DelEta2C","Track match #Delta#eta (layer 2) , Central;#Delta#eta", numDelEtaBins, DelEtaLow, DelEtaHigh);
  m_histograms["DelEta2EC"] = new TH1F("DelEta2EC","Track match #Delta#eta (layer 2), End-cap;#Delta#eta", numDelEtaBins, DelEtaLow, DelEtaHigh);


  /// Registering Histograms
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution" , m_histograms["EResolution"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionC" , m_histograms["EResolutionC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionEC" , m_histograms["EResolutionEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco" , m_histograms["EtaReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco" , m_histograms["PtReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtRecoC" , m_histograms["PtRecoC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtRecoEC" , m_histograms["PtRecoEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0T" , m_histograms["EResolution0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TC" , m_histograms["EResolution0TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TEC" , m_histograms["EResolution0TEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco0T" , m_histograms["EtaReco0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco0T" , m_histograms["PtReco0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco0TC" , m_histograms["PtReco0TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco0TEC" , m_histograms["PtReco0TEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSi" , m_histograms["EResolution1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiC" , m_histograms["EResolution1TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiEC" , m_histograms["EResolution1TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1TSi" , m_histograms["EtaReco1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TSi" , m_histograms["PtReco1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TSiC" , m_histograms["PtReco1TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TSiEC" , m_histograms["PtReco1TSiEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSi" , m_histograms["EResolution2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiC" , m_histograms["EResolution2TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiEC" , m_histograms["EResolution2TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TSi" , m_histograms["EtaReco2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TSi" , m_histograms["PtReco2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TSiC" , m_histograms["PtReco2TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TSiEC" , m_histograms["PtReco2TSiEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRT" , m_histograms["EResolution1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTC" , m_histograms["EResolution1TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTEC" , m_histograms["EResolution1TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1TTRT" , m_histograms["EtaReco1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TTRT" , m_histograms["PtReco1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TTRTC" , m_histograms["PtReco1TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TTRTEC" , m_histograms["PtReco1TTRTEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRT" , m_histograms["EResolution2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTC" , m_histograms["EResolution2TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTEC" , m_histograms["EResolution2TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TTRT" , m_histograms["EtaReco2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TTRT" , m_histograms["PtReco2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TTRTC" , m_histograms["PtReco2TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TTRTEC" , m_histograms["PtReco2TTRTEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMix" , m_histograms["EResolution2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixC" , m_histograms["EResolution2TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixEC" , m_histograms["EResolution2TMixEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TMix" , m_histograms["EtaReco2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TMix" , m_histograms["PtReco2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TMixC" , m_histograms["PtReco2TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TMixEC" , m_histograms["PtReco2TMixEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/EtaReco" , m_histograms["ElEtaReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/PtReco" , m_histograms["ElPtReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/PtRecoC" , m_histograms["ElPtRecoC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/PtRecoEC" , m_histograms["ElPtRecoEC"]).ignore();
  
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHits", m_histograms["NumBLHits"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsC", m_histograms["NumBLHitsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsEC", m_histograms["NumBLHitsEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsExp", m_histograms["NumBLHitsExp"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsExpC", m_histograms["NumBLHitsExpC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsExpEC", m_histograms["NumBLHitsExpEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsOutls", m_histograms["NumBLHitsOutls"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsOutlsC", m_histograms["NumBLHitsOutlsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumBLHitsOutlsEC", m_histograms["NumBLHitsOutlsEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumPixHits", m_histograms["NumPixHits"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumPixHitsC", m_histograms["NumPixHitsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumPixHitsEC", m_histograms["NumPixHitsEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumPixHitsOutls", m_histograms["NumPixHitsOutls"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumPixHitsOutlsC", m_histograms["NumPixHitsOutlsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumPixHitsOutlsEC", m_histograms["NumPixHitsOutlsEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumSiHits", m_histograms["NumSiHits"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumSiHitsC", m_histograms["NumSiHitsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumSiHitsEC", m_histograms["NumSiHitsEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumSiHitsOutls", m_histograms["NumSiHitsOutls"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumSiHitsOutlsC", m_histograms["NumSiHitsOutlsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumSiHitsOutlsEC", m_histograms["NumSiHitsOutlsEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/Eovp" , m_histograms["Eovp"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/EovpC" , m_histograms["EovpC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/EovpEC" , m_histograms["EovpEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhi1" , m_histograms["DelPhi1"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhi1C" , m_histograms["DelPhi1C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhi1EC" , m_histograms["DelPhi1EC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhiRescaled1" , m_histograms["DelPhiRescaled1"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhiRescaled1C" , m_histograms["DelPhiRescaled1C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhiRescaled1EC" , m_histograms["DelPhiRescaled1EC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhi2" , m_histograms["DelPhi2"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhi2C" , m_histograms["DelPhi2C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhi2EC" , m_histograms["DelPhi2EC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhiRescaled2" , m_histograms["DelPhiRescaled2"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhiRescaled2C" , m_histograms["DelPhiRescaled2C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelPhiRescaled2EC" , m_histograms["DelPhiRescaled2EC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelEta2" , m_histograms["DelEta2"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelEta2C" , m_histograms["DelEta2C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/DelEta2EC" , m_histograms["DelEta2EC"]).ignore();

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

  // static int count = 0;
  // ATH_MSG_WARNING("Skip event: " << count++); 
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
    for (auto el : *electrons) {
    
      const xAOD::TruthParticle *truthParticle{nullptr};
      int truthType{0};
      int truthOrigin{0};

      //bool alreadySeen = false;

      if (isMC) {
	truthParticle = xAOD::TruthHelpers::getTruthParticle(*el);
	truthType = xAOD::TruthHelpers::getParticleTruthType(*el);
	truthOrigin = xAOD::TruthHelpers::getParticleTruthOrigin(*el);

	ATH_MSG_DEBUG("Truth-match electron: type: " << truthType << ", origin: " 
		      << truthOrigin << ", TruthParticle*: " << truthParticle);
      }
      
      if (el->author(m_electronAuthor) && 
	  (!(m_truthMatchElectrons || m_truthMatchElectronAsPhotons) ||
	   (m_truthMatchElectrons && truthType == MCTruthPartClassifier::IsoElectron) ||
	   (m_truthMatchElectronAsPhotons && truthType == MCTruthPartClassifier::IsoPhoton))) {
	
	m_numElectrons++;
	
	const auto Ereco = el->caloCluster()->rawE();
	const auto Etruth = (truthParticle) ? truthParticle->e() : 0.0;
	//const auto Eres = (Ereco - Etruth)/Etruth;
	const auto eta = el->eta();
	const auto eta2 = el->caloCluster()->etaBE(2);
	//const auto etaTruth = (truthParticle) ? truthParticle->eta() : -999.0;
	const auto Et = Ereco/cosh(eta);
	const auto EtTruth = (truthParticle) ? truthParticle->pt() : 0.0;
	const bool isC = std::abs(eta2) <= 1.37;
	const bool isEC = std::abs(eta2) >= 1.52;
	
	ATH_MSG_INFO("Electron with author " << el->author() << ", Et == " << Et 
		     << ", eta = " << eta);
	
	
	const xAOD::TrackParticle * trParticle = el->trackParticle();
	if (!trParticle) {
	  ATH_MSG_ERROR("Electron has no track-particle");
	  return StatusCode::FAILURE;
	}

	const auto invP = std::abs(trParticle->qOverP());
	const auto Eovp = Ereco * invP;

	ATH_MSG_DEBUG("   Track phi = " << trParticle->phi() 
		      << ", eta = "<< trParticle->eta() 
		      << ", pt = " << trParticle->pt()
		      << ", E = " << trParticle->e());
      
      

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
	uint8_t  expectBLayerHit = true;
      
	bool allFound = true;
      
	allFound = allFound && trParticle->summaryValue(nBL, xAOD::numberOfBLayerHits);
	allFound = allFound && trParticle->summaryValue(nPi, xAOD::numberOfPixelHits);
	allFound = allFound && trParticle->summaryValue(nSCT, xAOD::numberOfSCTHits);
	allFound = allFound && trParticle->summaryValue(nBLOutliers, xAOD::numberOfBLayerOutliers);
	allFound = allFound && trParticle->summaryValue(nPiOutliers, xAOD::numberOfPixelOutliers);
	allFound = allFound && trParticle->summaryValue(nSCTOutliers, xAOD::numberOfSCTOutliers);
      
	allFound = allFound && trParticle->summaryValue(nTRThigh, xAOD::numberOfTRTHighThresholdHits);
	allFound = allFound && trParticle->summaryValue(nTRThighOutliers, xAOD::numberOfTRTHighThresholdOutliers);
	allFound = allFound && trParticle->summaryValue(nTRT, xAOD::numberOfTRTHits);
	allFound = allFound && trParticle->summaryValue(nTRTOutliers, xAOD::numberOfTRTOutliers);
	allFound = allFound && trParticle->summaryValue(nTRTXenonHits, xAOD::numberOfTRTXenonHits);
      
	allFound = allFound && trParticle->summaryValue(expectBLayerHit, xAOD::expectBLayerHit);

	if (!allFound) {
	  ATH_MSG_WARNING("Not all track values found");
	}

	int nSiliconHits_trk = nPi + nSCT;

	ATH_MSG_DEBUG("   Number of silicon hits = " << (int) nSiliconHits_trk << ", number of b-layer hits = " << (int) nBL 
		      << ", expected hit in b-layer = " << (bool) expectBLayerHit);

	const auto delPhi1 = el->trackCaloMatchValue(xAOD::EgammaParameters::deltaPhi1);
	const auto delPhiRescaled1 = el->trackCaloMatchValue(xAOD::EgammaParameters::deltaPhiRescaled1);
	const auto delPhi2 = el->trackCaloMatchValue(xAOD::EgammaParameters::deltaPhi2);
	const auto delPhiRescaled2 = el->trackCaloMatchValue(xAOD::EgammaParameters::deltaPhiRescaled2);
	const auto delEta2 = el->trackCaloMatchValue(xAOD::EgammaParameters::deltaEta2);

	ATH_MSG_WARNING("Event " << runNumber << ", " << lumiBlock << ", " << eventNumber 
			<< ", Eovp = " << Eovp
			<< ", Ereco = " << Ereco
			<< ", Et = " << Et
			<< ", eta2 = " << eta2
			<< ", clphi = " << el->caloCluster()->phi()
			<< ", nBL+outl = " << nBL + nBLOutliers
			<< ", expectBLayerHit = " << expectBLayerHit
			<< ", nSi+outl = " << nPi + nPiOutliers + nSCT + nSCTOutliers
			<< ", DelEta2 = " << delEta2
			<< ", DelPhi1 = " << delPhi1
			<< ", DelPhiRescaled1 = " << delPhiRescaled1
			);
	

	fillElectronHists(isC, isEC, 
			  eta, Et, Eovp,
			  nBL,
			  nBLOutliers,
			  nPi,
			  nPiOutliers,
			  nSCT,
			  nSCTOutliers,
			  expectBLayerHit,
			  delPhi1,
			  delPhiRescaled1,
			  delPhi2,
			  delPhiRescaled2,
			  delEta2
			  );

      } // truth-match 
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
    //std::set<const xAOD::TruthParticle *> seenTruths;

    const xAOD::TruthParticleContainer* egammaTruthParticles{nullptr};
    if (isMC && m_truthMatchPhotons) {
      ATH_CHECK(evtStore()->retrieve(egammaTruthParticles, m_egammaTruthParticleContainerName));
    }

    for (auto ph : *photons) {

      const xAOD::TruthParticle *truthParticle{nullptr};
      int truthType{0};
      int truthOrigin{0};

      //bool alreadySeen = false;

      if (isMC) {
	truthParticle = xAOD::TruthHelpers::getTruthParticle(*ph);
	truthType = xAOD::TruthHelpers::getParticleTruthType(*ph);
	truthOrigin = xAOD::TruthHelpers::getParticleTruthOrigin(*ph);

	ATH_MSG_DEBUG("Truth-match photon: type: " << truthType << ", origin: " 
		      << truthOrigin << ", TruthParticle*: " << truthParticle);

	// if (seenTruths.count(truthParticle)) {
	//   // have already seen the particle
	//   alreadySeen = true;
	//   ATH_MSG_DEBUG("Have already seen a partcle matched to same truth");
	// } else {
	//   seenTruths.insert(truthParticle);
	// }
      }


      if (ph->author(m_photonAuthor) && 
	  (!m_truthMatchPhotons || 
	   (truthType == MCTruthPartClassifier::IsoPhoton && isBestMatch(truthParticle, ph, egammaTruthParticles)))) {

	m_numPhotons++;

	ATH_MSG_DEBUG("author is = " << ph->author());

	const auto photonType = xAOD::EgammaHelpers::conversionType(ph);
	const auto Ereco = ph->caloCluster()->rawE();
	const auto Etruth = (truthParticle) ? truthParticle->e() : 0.0;
	const auto Eres = (Ereco - Etruth)/Etruth;
	const auto eta = ph->eta();
	const auto eta2 = ph->caloCluster()->etaBE(2);
	const auto etaTruth = (truthParticle) ? truthParticle->eta() : -999.0;
	const auto pt = Ereco/cosh(eta);
	const auto ptTruth = (truthParticle) ? truthParticle->pt() : 0.0;
	const bool isC = std::abs(eta2) <= 1.37;
	const bool isEC = std::abs(eta2) >= 1.52;


	if (pt > 1*TeV) {
	  ATH_MSG_WARNING("High PT Photon with pt = " << pt
			  << ", ptTruth = " << ptTruth
			  << ", eta = " << eta 
			  << ", eta2 = " << eta2
			  << ", phi = " << ph->phi());
	} else {
	  ATH_MSG_INFO("Photon with pt = " << pt
		       << ", ptTruth = " << ptTruth
		       << ", eta = " << eta 
		       << ", eta2 = " << eta2
		       << ", phi = " << ph->phi());
	}

	

	// first do general all-photons
	fillPhotonHists("", isC, isEC, eta, pt, Eres);

	switch(photonType) {
	case xAOD::EgammaParameters::unconverted:
	  m_numUnconverted++;
	  fillPhotonHists("0T", isC, isEC, eta, pt, Eres);
	  if (Eres < -0.95 && isEC) {
	    ATH_MSG_WARNING("Event " << runNumber << ", " << lumiBlock << ", " << eventNumber 
			    << ", Eres = " << Eres
			    << ", pt = " << pt
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
	  fillPhotonHists("1TSi", isC, isEC, eta, pt, Eres);
	  break;
	case xAOD::EgammaParameters::singleTRT:
	  m_numConversionsSingleTrackTRT++;
	  fillPhotonHists("1TTRT", isC, isEC, eta, pt, Eres);
	  break;
	case xAOD::EgammaParameters::doubleSi:
	  m_numConversionsDoubleTrackSi++;
	  fillPhotonHists("2TSi", isC, isEC, eta, pt, Eres);
	  break;
	case xAOD::EgammaParameters::doubleTRT:
	  m_numConversionsDoubleTrackTRT++;
	  fillPhotonHists("2TTRT", isC, isEC, eta, pt, Eres);
	  break;
	case xAOD::EgammaParameters::doubleSiTRT:
	  m_numConversionsDoubleTrackMix++;
	  fillPhotonHists("2TMix", isC, isEC, eta, pt, Eres);
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
			      float eta, float pt, float Eres)
{
 
  const std::string ptstr = "PtReco" + suffix;
  const std::string etastr = "EtaReco" + suffix;
  const std::string Eresstr = "EResolution" + suffix;

  ATH_MSG_INFO("ptstr = " << ptstr << ", pt/GeV = " << pt/GeV);

  m_histograms.at(ptstr)->Fill(pt/GeV);
  m_histograms.at(etastr)->Fill(eta);
  m_histograms.at(Eresstr)->Fill(Eres);
  if (isC) {
    const std::string EresstrC = Eresstr + "C";
    const std::string ptstrC = ptstr + "C";
    m_histograms.at(EresstrC)->Fill(Eres);
    m_histograms.at(ptstrC)->Fill(pt/GeV);
  } else if (isEC) {
    const std::string EresstrEC = Eresstr + "EC";
    const std::string ptstrEC = ptstr + "EC";
    m_histograms.at(EresstrEC)->Fill(Eres);
    m_histograms.at(ptstrEC)->Fill(pt/GeV);
  }
}
 
void TestAlg::fillElectronHists(bool isC, bool isEC, 
				float eta, float pt, float Eovp,
				uint8_t nBL,
				uint8_t nBLOutliers,
				uint8_t nPix,
				uint8_t nPixOutliers,
				uint8_t nSCT,
				uint8_t nSCTOutliers,
				uint8_t expectBLayerHit,
				float delPhi1,
				float delPhiRescaled1,
				float delPhi2,
				float delPhiRescaled2,
				float delEta2
				)
{
  auto nBLHitsOutls = nBL + nBLOutliers;
  auto nPixHitsOutls = nPix + nPixOutliers;
  auto nSCTHitsOutls = nSCT + nSCTOutliers;
  auto nSi = nPix + nSCT;
  auto nSiHitsOutls = nPixHitsOutls + nSCTHitsOutls;
  auto nBLHitsExp = (expectBLayerHit) ? nBLHitsOutls : 1;

  m_histograms.at("ElEtaReco")->Fill(eta);
  m_histograms.at("ElPtReco")->Fill(pt/GeV);
  m_histograms.at("Eovp")->Fill(Eovp);
  m_histograms.at("NumBLHits")->Fill(nBL);
  m_histograms.at("NumBLHitsOutls")->Fill(nBLHitsOutls);
  m_histograms.at("NumBLHitsExp")->Fill(nBLHitsExp);
  m_histograms.at("NumPixHits")->Fill(nPix);
  m_histograms.at("NumPixHitsOutls")->Fill(nPixHitsOutls);
  m_histograms.at("NumSiHits")->Fill(nSi);
  m_histograms.at("NumSiHitsOutls")->Fill(nSiHitsOutls);
  m_histograms.at("DelPhi1")->Fill(delPhi1);
  m_histograms.at("DelPhiRescaled1")->Fill(delPhiRescaled1);
  m_histograms.at("DelPhi2")->Fill(delPhi2);
  m_histograms.at("DelPhiRescaled2")->Fill(delPhiRescaled2);
  m_histograms.at("DelEta2")->Fill(delEta2);

  if (isC) {
    m_histograms.at("ElPtRecoC")->Fill(pt/GeV);
    m_histograms.at("EovpC")->Fill(Eovp);
    m_histograms.at("NumBLHitsC")->Fill(nBL);
    m_histograms.at("NumBLHitsOutlsC")->Fill(nBLHitsOutls);
    m_histograms.at("NumBLHitsExpC")->Fill(nBLHitsExp);
    m_histograms.at("NumPixHitsC")->Fill(nPix);
    m_histograms.at("NumPixHitsOutlsC")->Fill(nPixHitsOutls);
    m_histograms.at("NumSiHitsC")->Fill(nSi);
    m_histograms.at("NumSiHitsOutlsC")->Fill(nSiHitsOutls);
    m_histograms.at("DelPhi1C")->Fill(delPhi1);
    m_histograms.at("DelPhiRescaled1C")->Fill(delPhiRescaled1);
    m_histograms.at("DelPhi2C")->Fill(delPhi2);
    m_histograms.at("DelPhiRescaled2C")->Fill(delPhiRescaled2);
    m_histograms.at("DelEta2C")->Fill(delEta2);
  } else if (isEC) {
    m_histograms.at("ElPtRecoEC")->Fill(pt/GeV);
    m_histograms.at("EovpEC")->Fill(Eovp);
    m_histograms.at("NumBLHitsEC")->Fill(nBL);
    m_histograms.at("NumBLHitsOutlsEC")->Fill(nBLHitsOutls);
    m_histograms.at("NumBLHitsExpEC")->Fill(nBLHitsExp);
    m_histograms.at("NumPixHitsEC")->Fill(nPix);
    m_histograms.at("NumPixHitsOutlsEC")->Fill(nPixHitsOutls);
    m_histograms.at("NumSiHitsEC")->Fill(nSi);
    m_histograms.at("NumSiHitsOutlsEC")->Fill(nSiHitsOutls);
    m_histograms.at("DelPhi1EC")->Fill(delPhi1);
    m_histograms.at("DelPhiRescaled1EC")->Fill(delPhiRescaled1);
    m_histograms.at("DelPhi2EC")->Fill(delPhi2);
    m_histograms.at("DelPhiRescaled2EC")->Fill(delPhiRescaled2);
    m_histograms.at("DelEta2EC")->Fill(delEta2);
  }
}

namespace {
  bool isBestMatch(const xAOD::TruthParticle *truthParticle, 
		   const xAOD::Photon *ph,
		   const xAOD::TruthParticleContainer *egTruthContainer)
  {
    if (!egTruthContainer) {
      return false;
    }
    static const SG::AuxElement::Accessor<ElementLink<xAOD::TruthParticleContainer> > truthParticleLink("truthParticleLink");
    static const SG::AuxElement::Accessor<ElementLink<xAOD::PhotonContainer> > recoPhotonLink("recoPhotonLink");
      
    for (auto egtp : *egTruthContainer) {
      const auto tpl = truthParticleLink(*egtp);
      if (tpl.isValid() && *tpl == truthParticle) {
	return (ph == *(recoPhotonLink(*egtp)));
      }
    }
    return false;
  }    
}
