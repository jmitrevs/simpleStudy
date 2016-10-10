///////////////////////////////////////////////////////////////////
// TestAlg.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include <sstream>

#include "xAODEventInfo/EventInfo.h"

//#include "AthenaKernel/errorcheck.h"

#include "TH1.h"
#include "TH2.h"
#include "TProfile.h"
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

//#include "CaloUtils/CaloCellList.h"
 
#include "HepMC/GenParticle.h"
#include "HepMC/GenVertex.h"


//#include <gsl/gsl_math.h>
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
  AthAlgorithm(name,pSvcLocator),
  m_EResEBins{0, 20, 50, 100, 200, 500, 1000, 10000},
  //m_EResEBins{0, 20, 50, 100, 200, 500, 2000},
  //m_EResAbsEtaBins{0, 0.1, 0.6, 0.8, 1.15, 1.37, 1.52, 1.81, 2.01, 2.37, 2.47}
  m_EResAbsEtaBins{0, 0.6, 0.8, 1.15, 1.37, 1.52, 2.01, 2.47}
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
  declareProperty("TruthMatchElectrons", m_truthMatchElectrons = true, "Require electrons to be truth-matched");
  declareProperty("TruthMatchElectronAsPhotons", m_truthMatchElectronAsPhotons = false, "Require electrons to be truth-matched to photons");

  declareProperty("PhotonContainerName", m_PhotonContainerName = "Photons");
  declareProperty("PhotonPt",   m_photonPt=10*GeV);
  declareProperty("PhotonEta",  m_photonEta=3.2);
  declareProperty("PhotonIsEMFlag", m_photonIsEMFlag="Tight");
  declareProperty("PhotonIsEM", m_photonIsEM=0);
  declareProperty("PhotonAuthor", m_photonAuthor = xAOD::EgammaParameters::AuthorALL);

 //declareProperty("TruthPhotonPtMin", m_truthPhotonPtMin = 5*GeV);
  declareProperty("TruthMatchPhotons", m_truthMatchPhotons = true, "Require photons to be truth-matched");

  declareProperty("OnlyLookAtSingleClusters", m_onlyLookAtSingleClusters = false, 
		  "Only make plots for single-cluster electrons and photons");

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

  declareProperty("muCut", m_muCut = 40.0, "Above this mu value we define high mu events");
  declareProperty("PhotonRevmoveCrack", m_photonRemoveCrack = true, "Remove crack events from plots");

  
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


  const Int_t numPtBins = 200;
  const Double_t PtLow = 0; // in GeV, not MeV
  const Double_t PtHigh = 4000; // in GeV, not MeV

  const Int_t numMuBins = 30;
  const Double_t muLow = 0;
  const Double_t muHigh = 60;

  const Int_t numNumCellsBins = 300;
  const Double_t numCellsLow = 0;
  const Double_t numCellsHigh = 300;

  const std::vector<Double_t> cellEBins = {0, 20, 50, 100, 200, 500, 1000, 10000}; // in GeV

  ATH_MSG_INFO("DefaultSumw2 = " << TH1::GetDefaultSumw2());

  m_histograms["AverageInteractionsPerCrossing"] = new TH1F("AverageInteractionsPerCrossing", "AverageInteractionsPerCrossing;<#mu>", numMuBins, muLow, muHigh);

  /// Defining Histogramsquer
  m_histograms["EResolution"] = new TH1F("EResolution","Raw Energy Resolution;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolutionC"] = new TH1F("EResolutionC","Raw Energy Resolution, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolutionEC"] = new TH1F("EResolutionEC","Raw Energy Resolution, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution_highmu"] = new TH1F("EResolution_highmu","Raw Energy Resolution;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolutionC_highmu"] = new TH1F("EResolutionC_highmu","Raw Energy Resolution, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolutionEC_highmu"] = new TH1F("EResolutionEC_highmu","Raw Energy Resolution, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EtaReco"] = new TH1F("EtaReco","Reco Psuedorapidity;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco"] = new TH1F("PtReco","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtRecoC"] = new TH1F("PtRecoC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtRecoEC"] = new TH1F("PtRecoEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution_mu"] = new TProfile("EResolution_mu","Raw Energy Resolution;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolutionC_mu"] = new TProfile("EResolutionC_mu","Raw Energy Resolution, Central;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolutionEC_mu"] = new TProfile("EResolutionEC_mu","Raw Energy Resolution, End-cap;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");

  ATH_CHECK(initialize3DERes(""));

  m_histograms["NumCells"] = new TH2F("NumCells","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL0"] = new TH2F("NumCellsL0","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL1"] = new TH2F("NumCellsL1","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL2"] = new TH2F("NumCellsL2","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL3"] = new TH2F("NumCellsL3","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCellsC"] = new TH2F("NumCellsC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL0C"] = new TH2F("NumCellsL0C","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL1C"] = new TH2F("NumCellsL1C","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL2C"] = new TH2F("NumCellsL2C","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL3C"] = new TH2F("NumCellsL3C","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCellsEC"] = new TH2F("NumCellsEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL0EC"] = new TH2F("NumCellsL0EC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL1EC"] = new TH2F("NumCellsL1EC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL2EC"] = new TH2F("NumCellsL2EC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL3EC"] = new TH2F("NumCellsL3EC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  

  // only for unconverted
  m_histograms["EResolution0T"] = new TH1F("EResolution0T","Raw Energy Resolution, unconverted;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution0TC"] = new TH1F("EResolution0TC","Raw Energy Resolution, unconverted, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution0TEC"] = new TH1F("EResolution0TEC","Raw Energy Resolution, unconverted, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution0T_highmu"] = new TH1F("EResolution0T_highmu","Raw Energy Resolution, unconverted;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution0TC_highmu"] = new TH1F("EResolution0TC_highmu","Raw Energy Resolution, unconverted, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution0TEC_highmu"] = new TH1F("EResolution0TEC_highmu","Raw Energy Resolution, unconverted, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EtaReco0T"] = new TH1F("EtaReco0T","Reco Psuedorapidity, unconverted;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco0T"] = new TH1F("PtReco0T","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco0TC"] = new TH1F("PtReco0TC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco0TEC"] = new TH1F("PtReco0TEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution0T_mu"] = new TProfile("EResolution0T_mu","Raw Energy Resolution, unconverted;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution0TC_mu"] = new TProfile("EResolution0TC_mu","Raw Energy Resolution, unconverted, Central;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution0TEC_mu"] = new TProfile("EResolution0TEC_mu","Raw Energy Resolution, unconverted, End-cap;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");

  ATH_CHECK(initialize3DERes("0T"));

  m_histograms["NumCells0T"] = new TH2F("NumCells0T","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL00T"] = new TH2F("NumCellsL00T","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL10T"] = new TH2F("NumCellsL10T","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL20T"] = new TH2F("NumCellsL20T","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL30T"] = new TH2F("NumCellsL30T","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells0TC"] = new TH2F("NumCells0TC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL00TC"] = new TH2F("NumCellsL00TC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL10TC"] = new TH2F("NumCellsL10TC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL20TC"] = new TH2F("NumCellsL20TC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL30TC"] = new TH2F("NumCellsL30TC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells0TEC"] = new TH2F("NumCells0TEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL00TEC"] = new TH2F("NumCellsL00TEC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL10TEC"] = new TH2F("NumCellsL10TEC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL20TEC"] = new TH2F("NumCellsL20TEC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL30TEC"] = new TH2F("NumCellsL30TEC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  // only for Si
  m_histograms["EResolution1TSi"] = new TH1F("EResolution1TSi","Raw Energy Resolution, 1-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TSiC"] = new TH1F("EResolution1TSiC","Raw Energy Resolution, 1-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TSiEC"] = new TH1F("EResolution1TSiEC","Raw Energy Resolution, 1-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TSi_highmu"] = new TH1F("EResolution1TSi_highmu","Raw Energy Resolution, 1-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TSiC_highmu"] = new TH1F("EResolution1TSiC_highmu","Raw Energy Resolution, 1-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TSiEC_highmu"] = new TH1F("EResolution1TSiEC_highmu","Raw Energy Resolution, 1-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EtaReco1TSi"] = new TH1F("EtaReco1TSi","Reco Psuedorapidity, 1-track Si conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco1TSi"] = new TH1F("PtReco1TSi","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TSiC"] = new TH1F("PtReco1TSiC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TSiEC"] = new TH1F("PtReco1TSiEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution1TSi_mu"] = new TProfile("EResolution1TSi_mu","Raw Energy Resolution, 1-track Si conversion;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution1TSiC_mu"] = new TProfile("EResolution1TSiC_mu","Raw Energy Resolution, 1-track Si conversion, Central;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution1TSiEC_mu"] = new TProfile("EResolution1TSiEC_mu","Raw Energy Resolution, 1-track Si conversion, End-cap;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");

  ATH_CHECK(initialize3DERes("1TSi"));

  m_histograms["NumCells1TSi"] = new TH2F("NumCells1TSi","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL01TSi"] = new TH2F("NumCellsL01TSi","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL11TSi"] = new TH2F("NumCellsL11TSi","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL21TSi"] = new TH2F("NumCellsL21TSi","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL31TSi"] = new TH2F("NumCellsL31TSi","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells1TSiC"] = new TH2F("NumCells1TSiC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL01TSiC"] = new TH2F("NumCellsL01TSiC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL11TSiC"] = new TH2F("NumCellsL11TSiC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL21TSiC"] = new TH2F("NumCellsL21TSiC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL31TSiC"] = new TH2F("NumCellsL31TSiC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells1TSiEC"] = new TH2F("NumCells1TSiEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL01TSiEC"] = new TH2F("NumCellsL01TSiEC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL11TSiEC"] = new TH2F("NumCellsL11TSiEC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL21TSiEC"] = new TH2F("NumCellsL21TSiEC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL31TSiEC"] = new TH2F("NumCellsL31TSiEC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["EResolution2TSi"] = new TH1F("EResolution2TSi","Raw Energy Resolution, 2-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TSiC"] = new TH1F("EResolution2TSiC","Raw Energy Resolution, 2-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TSiEC"] = new TH1F("EResolution2TSiEC","Raw Energy Resolution, 2-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TSi_highmu"] = new TH1F("EResolution2TSi_highmu","Raw Energy Resolution, 2-track Si conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TSiC_highmu"] = new TH1F("EResolution2TSiC_highmu","Raw Energy Resolution, 2-track Si conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TSiEC_highmu"] = new TH1F("EResolution2TSiEC_highmu","Raw Energy Resolution, 2-track Si conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EtaReco2TSi"] = new TH1F("EtaReco2TSi","Reco Psuedorapidity, 2-track Si conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco2TSi"] = new TH1F("PtReco2TSi","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TSiC"] = new TH1F("PtReco2TSiC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TSiEC"] = new TH1F("PtReco2TSiEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution2TSi_mu"] = new TProfile("EResolution2TSi_mu","Raw Energy Resolution, 2-track Si conversion;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution2TSiC_mu"] = new TProfile("EResolution2TSiC_mu","Raw Energy Resolution, 2-track Si conversion, Central;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution2TSiEC_mu"] = new TProfile("EResolution2TSiEC_mu","Raw Energy Resolution, 2-track Si conversion, End-cap;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");

  ATH_CHECK(initialize3DERes("2TSi"));

  m_histograms["NumCells2TSi"] = new TH2F("NumCells2TSi","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TSi"] = new TH2F("NumCellsL02TSi","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TSi"] = new TH2F("NumCellsL12TSi","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TSi"] = new TH2F("NumCellsL22TSi","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TSi"] = new TH2F("NumCellsL32TSi","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells2TSiC"] = new TH2F("NumCells2TSiC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TSiC"] = new TH2F("NumCellsL02TSiC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TSiC"] = new TH2F("NumCellsL12TSiC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TSiC"] = new TH2F("NumCellsL22TSiC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TSiC"] = new TH2F("NumCellsL32TSiC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells2TSiEC"] = new TH2F("NumCells2TSiEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TSiEC"] = new TH2F("NumCellsL02TSiEC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TSiEC"] = new TH2F("NumCellsL12TSiEC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TSiEC"] = new TH2F("NumCellsL22TSiEC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TSiEC"] = new TH2F("NumCellsL32TSiEC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  // TRT
  m_histograms["EResolution1TTRT"] = new TH1F("EResolution1TTRT","Raw Energy Resolution, 1-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TTRTC"] = new TH1F("EResolution1TTRTC","Raw Energy Resolution, 1-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TTRTEC"] = new TH1F("EResolution1TTRTEC","Raw Energy Resolution, 1-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TTRT_highmu"] = new TH1F("EResolution1TTRT_highmu","Raw Energy Resolution, 1-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TTRTC_highmu"] = new TH1F("EResolution1TTRTC_highmu","Raw Energy Resolution, 1-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution1TTRTEC_highmu"] = new TH1F("EResolution1TTRTEC_highmu","Raw Energy Resolution, 1-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EtaReco1TTRT"] = new TH1F("EtaReco1TTRT","Reco Psuedorapidity, 1-track TRT conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco1TTRT"] = new TH1F("PtReco1TTRT","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TTRTC"] = new TH1F("PtReco1TTRTC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco1TTRTEC"] = new TH1F("PtReco1TTRTEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution1TTRT_mu"] = new TProfile("EResolution1TTRT_mu","Raw Energy Resolution, 1-track TRT conversion;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution1TTRTC_mu"] = new TProfile("EResolution1TTRTC_mu","Raw Energy Resolution, 1-track TRT conversion, Central;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution1TTRTEC_mu"] = new TProfile("EResolution1TTRTEC_mu","Raw Energy Resolution, 1-track TRT conversion, End-cap;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");

  ATH_CHECK(initialize3DERes("1TTRT"));

  m_histograms["NumCells1TTRT"] = new TH2F("NumCells1TTRT","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL01TTRT"] = new TH2F("NumCellsL01TTRT","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL11TTRT"] = new TH2F("NumCellsL11TTRT","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL21TTRT"] = new TH2F("NumCellsL21TTRT","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL31TTRT"] = new TH2F("NumCellsL31TTRT","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells1TTRTC"] = new TH2F("NumCells1TTRTC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL01TTRTC"] = new TH2F("NumCellsL01TTRTC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL11TTRTC"] = new TH2F("NumCellsL11TTRTC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL21TTRTC"] = new TH2F("NumCellsL21TTRTC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL31TTRTC"] = new TH2F("NumCellsL31TTRTC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells1TTRTEC"] = new TH2F("NumCells1TTRTEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL01TTRTEC"] = new TH2F("NumCellsL01TTRTEC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL11TTRTEC"] = new TH2F("NumCellsL11TTRTEC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL21TTRTEC"] = new TH2F("NumCellsL21TTRTEC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL31TTRTEC"] = new TH2F("NumCellsL31TTRTEC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["EResolution2TTRT"] = new TH1F("EResolution2TTRT","Raw Energy Resolution, 2-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TTRTC"] = new TH1F("EResolution2TTRTC","Raw Energy Resolution, 2-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TTRTEC"] = new TH1F("EResolution2TTRTEC","Raw Energy Resolution, 2-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TTRT_highmu"] = new TH1F("EResolution2TTRT_highmu","Raw Energy Resolution, 2-track TRT conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TTRTC_highmu"] = new TH1F("EResolution2TTRTC_highmu","Raw Energy Resolution, 2-track TRT conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TTRTEC_highmu"] = new TH1F("EResolution2TTRTEC_highmu","Raw Energy Resolution, 2-track TRT conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EtaReco2TTRT"] = new TH1F("EtaReco2TTRT","Reco Psuedorapidity, 2-track TRT conversion;#eta_{reco}, End-cap", 100, -3,3);
  m_histograms["PtReco2TTRT"] = new TH1F("PtReco2TTRT","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TTRTC"] = new TH1F("PtReco2TTRTC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TTRTEC"] = new TH1F("PtReco2TTRTEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution2TTRT_mu"] = new TProfile("EResolution2TTRT_mu","Raw Energy Resolution, 2-track TRT conversion;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution2TTRTC_mu"] = new TProfile("EResolution2TTRTC_mu","Raw Energy Resolution, 2-track TRT conversion, Central;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution2TTRTEC_mu"] = new TProfile("EResolution2TTRTEC_mu","Raw Energy Resolution, 2-track TRT conversion, End-cap;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");

  ATH_CHECK(initialize3DERes("2TTRT"));

  m_histograms["NumCells2TTRT"] = new TH2F("NumCells2TTRT","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TTRT"] = new TH2F("NumCellsL02TTRT","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TTRT"] = new TH2F("NumCellsL12TTRT","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TTRT"] = new TH2F("NumCellsL22TTRT","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TTRT"] = new TH2F("NumCellsL32TTRT","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells2TTRTC"] = new TH2F("NumCells2TTRTC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TTRTC"] = new TH2F("NumCellsL02TTRTC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TTRTC"] = new TH2F("NumCellsL12TTRTC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TTRTC"] = new TH2F("NumCellsL22TTRTC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TTRTC"] = new TH2F("NumCellsL32TTRTC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells2TTRTEC"] = new TH2F("NumCells2TTRTEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TTRTEC"] = new TH2F("NumCellsL02TTRTEC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TTRTEC"] = new TH2F("NumCellsL12TTRTEC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TTRTEC"] = new TH2F("NumCellsL22TTRTEC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TTRTEC"] = new TH2F("NumCellsL32TTRTEC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["EResolution2TMix"] = new TH1F("EResolution2TMix","Raw Energy Resolution, 2-track Mix conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TMixC"] = new TH1F("EResolution2TMixC","Raw Energy Resolution, 2-track Mix conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TMixEC"] = new TH1F("EResolution2TMixEC","Raw Energy Resolution, 2-track Mix conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TMix_highmu"] = new TH1F("EResolution2TMix_highmu","Raw Energy Resolution, 2-track Mix conversion;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TMixC_highmu"] = new TH1F("EResolution2TMixC_highmu","Raw Energy Resolution, 2-track Mix conversion, Central;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EResolution2TMixEC_highmu"] = new TH1F("EResolution2TMixEC_highmu","Raw Energy Resolution, 2-track Mix conversion, End-cap;(E_{reco} - E_{truth})/E_{truth}", m_numEResBins, m_EResLow, m_EResHigh);
  m_histograms["EtaReco2TMix"] = new TH1F("EtaReco2TMix","Reco Psuedorapidity, 2-track Mix conversion;#eta_{reco}", 100, -3,3);
  m_histograms["PtReco2TMix"] = new TH1F("PtReco2TMix","Reco p_{T};p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TMixC"] = new TH1F("PtReco2TMixC","Reco p_{T}, Central;p_{T} [GeV]", numPtBins, PtLow, PtHigh);
  m_histograms["PtReco2TMixEC"] = new TH1F("PtReco2TMixEC","Reco p_{T}, End-cap;p_{T} [GeV]", numPtBins, PtLow, PtHigh);

  m_histograms["EResolution2TMix_mu"] = new TProfile("EResolution2TMix_mu","Raw Energy Resolution, 2-track Mix conversion;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution2TMixC_mu"] = new TProfile("EResolution2TMixC_mu","Raw Energy Resolution, 2-track Mix conversion, Central;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");
  m_histograms["EResolution2TMixEC_mu"] = new TProfile("EResolution2TMixEC_mu","Raw Energy Resolution, 2-track Mix conversion, End-cap;<#mu>;(E_{reco} - E_{truth})/E_{truth}", numMuBins, muLow, muHigh, "s");

  ATH_CHECK(initialize3DERes("2TMix"));

  m_histograms["NumCells2TMix"] = new TH2F("NumCells2TMix","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TMix"] = new TH2F("NumCellsL02TMix","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TMix"] = new TH2F("NumCellsL12TMix","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TMix"] = new TH2F("NumCellsL22TMix","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TMix"] = new TH2F("NumCellsL32TMix","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells2TMixC"] = new TH2F("NumCells2TMixC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TMixC"] = new TH2F("NumCellsL02TMixC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TMixC"] = new TH2F("NumCellsL12TMixC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TMixC"] = new TH2F("NumCellsL22TMixC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TMixC"] = new TH2F("NumCellsL32TMixC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["NumCells2TMixEC"] = new TH2F("NumCells2TMixEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL02TMixEC"] = new TH2F("NumCellsL02TMixEC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL12TMixEC"] = new TH2F("NumCellsL12TMixEC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL22TMixEC"] = new TH2F("NumCellsL22TMixEC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["NumCellsL32TMixEC"] = new TH2F("NumCellsL32TMixEC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);


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

  m_histograms["ElNumCells"] = new TH2F("ElNumCells","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL0"] = new TH2F("ElNumCellsL0","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL1"] = new TH2F("ElNumCellsL1","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL2"] = new TH2F("ElNumCellsL2","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL3"] = new TH2F("ElNumCellsL3","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["ElNumCellsC"] = new TH2F("ElNumCellsC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL0C"] = new TH2F("ElNumCellsL0C","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL1C"] = new TH2F("ElNumCellsL1C","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL2C"] = new TH2F("ElNumCellsL2C","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL3C"] = new TH2F("ElNumCellsL3C","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);

  m_histograms["ElNumCellsEC"] = new TH2F("ElNumCellsEC","Number of Cells (all layers);N(all layers);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL0EC"] = new TH2F("ElNumCellsL0EC","Number of Cells (presampler);N^{cells} (L0);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL1EC"] = new TH2F("ElNumCellsL1EC","Number of Cells (EM layer 1);N^{cells} (L1);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL2EC"] = new TH2F("ElNumCellsL2EC","Number of Cells (EM layer 2);N^{cells} (L2);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
  m_histograms["ElNumCellsL3EC"] = new TH2F("ElNumCellsL3EC","Number of Cells (EM layer 3);N^{cells} (L3);E^{truth} [GeV]", numNumCellsBins, numCellsLow, numCellsHigh, cellEBins.size() - 1, &cellEBins[0]);
   

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
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/AverageInteractionsPerCrossing" , m_histograms["AverageInteractionsPerCrossing"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution" , m_histograms["EResolution"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionC" , m_histograms["EResolutionC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionEC" , m_histograms["EResolutionEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution_highmu" , m_histograms["EResolution_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionC_highmu" , m_histograms["EResolutionC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionEC_highmu" , m_histograms["EResolutionEC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco" , m_histograms["EtaReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco" , m_histograms["PtReco"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtRecoC" , m_histograms["PtRecoC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtRecoEC" , m_histograms["PtRecoEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells" , m_histograms["NumCells"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL0" , m_histograms["NumCellsL0"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL1" , m_histograms["NumCellsL1"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL2" , m_histograms["NumCellsL2"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL3" , m_histograms["NumCellsL3"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsC" , m_histograms["NumCellsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL0C" , m_histograms["NumCellsL0C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL1C" , m_histograms["NumCellsL1C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL2C" , m_histograms["NumCellsL2C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL3C" , m_histograms["NumCellsL3C"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsEC" , m_histograms["NumCellsEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL0EC" , m_histograms["NumCellsL0EC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL1EC" , m_histograms["NumCellsL1EC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL2EC" , m_histograms["NumCellsL2EC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL3EC" , m_histograms["NumCellsL3EC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution_mu" , m_histograms["EResolution_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionC_mu" , m_histograms["EResolutionC_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolutionEC_mu" , m_histograms["EResolutionEC_mu"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0T" , m_histograms["EResolution0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TC" , m_histograms["EResolution0TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TEC" , m_histograms["EResolution0TEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0T_highmu" , m_histograms["EResolution0T_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TC_highmu" , m_histograms["EResolution0TC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TEC_highmu" , m_histograms["EResolution0TEC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco0T" , m_histograms["EtaReco0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco0T" , m_histograms["PtReco0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco0TC" , m_histograms["PtReco0TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco0TEC" , m_histograms["PtReco0TEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0T_mu" , m_histograms["EResolution0T_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TC_mu" , m_histograms["EResolution0TC_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution0TEC_mu" , m_histograms["EResolution0TEC_mu"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells0T" , m_histograms["NumCells0T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL00T" , m_histograms["NumCellsL00T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL10T" , m_histograms["NumCellsL10T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL20T" , m_histograms["NumCellsL20T"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL30T" , m_histograms["NumCellsL30T"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells0TC" , m_histograms["NumCells0TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL00TC" , m_histograms["NumCellsL00TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL10TC" , m_histograms["NumCellsL10TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL20TC" , m_histograms["NumCellsL20TC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL30TC" , m_histograms["NumCellsL30TC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells0TEC" , m_histograms["NumCells0TEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL00TEC" , m_histograms["NumCellsL00TEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL10TEC" , m_histograms["NumCellsL10TEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL20TEC" , m_histograms["NumCellsL20TEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL30TEC" , m_histograms["NumCellsL30TEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSi" , m_histograms["EResolution1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiC" , m_histograms["EResolution1TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiEC" , m_histograms["EResolution1TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSi_highmu" , m_histograms["EResolution1TSi_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiC_highmu" , m_histograms["EResolution1TSiC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiEC_highmu" , m_histograms["EResolution1TSiEC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1TSi" , m_histograms["EtaReco1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TSi" , m_histograms["PtReco1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TSiC" , m_histograms["PtReco1TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TSiEC" , m_histograms["PtReco1TSiEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSi_mu" , m_histograms["EResolution1TSi_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiC_mu" , m_histograms["EResolution1TSiC_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TSiEC_mu" , m_histograms["EResolution1TSiEC_mu"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells1TSi" , m_histograms["NumCells1TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL01TSi" , m_histograms["NumCellsL01TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL11TSi" , m_histograms["NumCellsL11TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL21TSi" , m_histograms["NumCellsL21TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL31TSi" , m_histograms["NumCellsL31TSi"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells1TSiC" , m_histograms["NumCells1TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL01TSiC" , m_histograms["NumCellsL01TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL11TSiC" , m_histograms["NumCellsL11TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL21TSiC" , m_histograms["NumCellsL21TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL31TSiC" , m_histograms["NumCellsL31TSiC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells1TSiEC" , m_histograms["NumCells1TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL01TSiEC" , m_histograms["NumCellsL01TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL11TSiEC" , m_histograms["NumCellsL11TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL21TSiEC" , m_histograms["NumCellsL21TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL31TSiEC" , m_histograms["NumCellsL31TSiEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSi" , m_histograms["EResolution2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiC" , m_histograms["EResolution2TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiEC" , m_histograms["EResolution2TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSi_highmu" , m_histograms["EResolution2TSi_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiC_highmu" , m_histograms["EResolution2TSiC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiEC_highmu" , m_histograms["EResolution2TSiEC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TSi" , m_histograms["EtaReco2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TSi" , m_histograms["PtReco2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TSiC" , m_histograms["PtReco2TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TSiEC" , m_histograms["PtReco2TSiEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSi_mu" , m_histograms["EResolution2TSi_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiC_mu" , m_histograms["EResolution2TSiC_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TSiEC_mu" , m_histograms["EResolution2TSiEC_mu"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TSi" , m_histograms["NumCells2TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TSi" , m_histograms["NumCellsL02TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TSi" , m_histograms["NumCellsL12TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TSi" , m_histograms["NumCellsL22TSi"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TSi" , m_histograms["NumCellsL32TSi"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TSiC" , m_histograms["NumCells2TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TSiC" , m_histograms["NumCellsL02TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TSiC" , m_histograms["NumCellsL12TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TSiC" , m_histograms["NumCellsL22TSiC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TSiC" , m_histograms["NumCellsL32TSiC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TSiEC" , m_histograms["NumCells2TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TSiEC" , m_histograms["NumCellsL02TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TSiEC" , m_histograms["NumCellsL12TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TSiEC" , m_histograms["NumCellsL22TSiEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TSiEC" , m_histograms["NumCellsL32TSiEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRT" , m_histograms["EResolution1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTC" , m_histograms["EResolution1TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTEC" , m_histograms["EResolution1TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRT_highmu" , m_histograms["EResolution1TTRT_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTC_highmu" , m_histograms["EResolution1TTRTC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTEC_highmu" , m_histograms["EResolution1TTRTEC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco1TTRT" , m_histograms["EtaReco1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TTRT" , m_histograms["PtReco1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TTRTC" , m_histograms["PtReco1TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco1TTRTEC" , m_histograms["PtReco1TTRTEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRT_mu" , m_histograms["EResolution1TTRT_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTC_mu" , m_histograms["EResolution1TTRTC_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution1TTRTEC_mu" , m_histograms["EResolution1TTRTEC_mu"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells1TTRT" , m_histograms["NumCells1TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL01TTRT" , m_histograms["NumCellsL01TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL11TTRT" , m_histograms["NumCellsL11TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL21TTRT" , m_histograms["NumCellsL21TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL31TTRT" , m_histograms["NumCellsL31TTRT"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells1TTRTC" , m_histograms["NumCells1TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL01TTRTC" , m_histograms["NumCellsL01TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL11TTRTC" , m_histograms["NumCellsL11TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL21TTRTC" , m_histograms["NumCellsL21TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL31TTRTC" , m_histograms["NumCellsL31TTRTC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells1TTRTEC" , m_histograms["NumCells1TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL01TTRTEC" , m_histograms["NumCellsL01TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL11TTRTEC" , m_histograms["NumCellsL11TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL21TTRTEC" , m_histograms["NumCellsL21TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL31TTRTEC" , m_histograms["NumCellsL31TTRTEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRT" , m_histograms["EResolution2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTC" , m_histograms["EResolution2TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTEC" , m_histograms["EResolution2TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRT_highmu" , m_histograms["EResolution2TTRT_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTC_highmu" , m_histograms["EResolution2TTRTC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTEC_highmu" , m_histograms["EResolution2TTRTEC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TTRT" , m_histograms["EtaReco2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TTRT" , m_histograms["PtReco2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TTRTC" , m_histograms["PtReco2TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TTRTEC" , m_histograms["PtReco2TTRTEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRT_mu" , m_histograms["EResolution2TTRT_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTC_mu" , m_histograms["EResolution2TTRTC_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TTRTEC_mu" , m_histograms["EResolution2TTRTEC_mu"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TTRT" , m_histograms["NumCells2TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TTRT" , m_histograms["NumCellsL02TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TTRT" , m_histograms["NumCellsL12TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TTRT" , m_histograms["NumCellsL22TTRT"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TTRT" , m_histograms["NumCellsL32TTRT"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TTRTC" , m_histograms["NumCells2TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TTRTC" , m_histograms["NumCellsL02TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TTRTC" , m_histograms["NumCellsL12TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TTRTC" , m_histograms["NumCellsL22TTRTC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TTRTC" , m_histograms["NumCellsL32TTRTC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TTRTEC" , m_histograms["NumCells2TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TTRTEC" , m_histograms["NumCellsL02TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TTRTEC" , m_histograms["NumCellsL12TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TTRTEC" , m_histograms["NumCellsL22TTRTEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TTRTEC" , m_histograms["NumCellsL32TTRTEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMix" , m_histograms["EResolution2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixC" , m_histograms["EResolution2TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixEC" , m_histograms["EResolution2TMixEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMix_highmu" , m_histograms["EResolution2TMix_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixC_highmu" , m_histograms["EResolution2TMixC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixEC_highmu" , m_histograms["EResolution2TMixEC_highmu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EtaReco2TMix" , m_histograms["EtaReco2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TMix" , m_histograms["PtReco2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TMixC" , m_histograms["PtReco2TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/PtReco2TMixEC" , m_histograms["PtReco2TMixEC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMix_mu" , m_histograms["EResolution2TMix_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixC_mu" , m_histograms["EResolution2TMixC_mu"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/EResolution2TMixEC_mu" , m_histograms["EResolution2TMixEC_mu"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TMix" , m_histograms["NumCells2TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TMix" , m_histograms["NumCellsL02TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TMix" , m_histograms["NumCellsL12TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TMix" , m_histograms["NumCellsL22TMix"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TMix" , m_histograms["NumCellsL32TMix"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TMixC" , m_histograms["NumCells2TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TMixC" , m_histograms["NumCellsL02TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TMixC" , m_histograms["NumCellsL12TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TMixC" , m_histograms["NumCellsL22TMixC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TMixC" , m_histograms["NumCellsL32TMixC"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCells2TMixEC" , m_histograms["NumCells2TMixEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL02TMixEC" , m_histograms["NumCellsL02TMixEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL12TMixEC" , m_histograms["NumCellsL12TMixEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL22TMixEC" , m_histograms["NumCellsL22TMixEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/NumCellsL32TMixEC" , m_histograms["NumCellsL32TMixEC"]).ignore();

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

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCells" , m_histograms["ElNumCells"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL0" , m_histograms["ElNumCellsL0"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL1" , m_histograms["ElNumCellsL1"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL2" , m_histograms["ElNumCellsL2"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL3" , m_histograms["ElNumCellsL3"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsC" , m_histograms["ElNumCellsC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL0C" , m_histograms["ElNumCellsL0C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL1C" , m_histograms["ElNumCellsL1C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL2C" , m_histograms["ElNumCellsL2C"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL3C" , m_histograms["ElNumCellsL3C"]).ignore();

  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsEC" , m_histograms["ElNumCellsEC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL0EC" , m_histograms["ElNumCellsL0EC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL1EC" , m_histograms["ElNumCellsL1EC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL2EC" , m_histograms["ElNumCellsL2EC"]).ignore();
  m_thistSvc->regHist(std::string("/")+m_histFileName+"/Electron/NumCellsL3EC" , m_histograms["ElNumCellsL3EC"]).ignore();

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

  float weight = 1.0;

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

  if (isMC) {
    weight = evtInfo->mcEventWeight();
  }

  const auto mu = evtInfo->averageInteractionsPerCrossing();

  m_histograms["AverageInteractionsPerCrossing"]->Fill(mu, weight);

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

	const auto clus = el->caloCluster();
	static const SG::AuxElement::Accessor < std::vector< ElementLink< xAOD::CaloClusterContainer > > > caloClusterLinks("constituentClusterLinks");
	const auto clusLinks = caloClusterLinks(*clus);	
	const auto numClusters = clusLinks.size();
	
	const auto Ereco = clus->rawE();
	const auto Etruth = (truthParticle) ? truthParticle->e() : 0.0;
	//const auto Eres = (Ereco - Etruth)/Etruth;
	const auto eta = el->eta();
	const auto eta2 = clus->etaBE(2);
	//const auto etaTruth = (truthParticle) ? truthParticle->eta() : -999.0;
	const auto Et = Ereco/cosh(eta);
	//const auto EtTruth = (truthParticle) ? truthParticle->pt() : 0.0;
	const bool isC = std::abs(eta2) <= 1.37;
	const bool isEC = std::abs(eta2) >= 1.52;

	if (m_onlyLookAtSingleClusters && numClusters != 1) {
	  static const SG::AuxElement::Accessor<int> nWindowClustersAcc ("nWindowClusters");
	  const auto nWindowClusters = nWindowClustersAcc(*clus);
	  static const SG::AuxElement::Accessor<int> nExtraClustersAcc ("nExtraClusters");
	  const auto nExtraClusters = nExtraClustersAcc(*clus);
	  ATH_MSG_WARNING("Found an electron with numClusters = " << numClusters 
			  << ", nWindowClusters = " << nWindowClusters
			  << ", nExtraClusters = " << nExtraClusters);
	  return StatusCode::SUCCESS;
	}
	
	// get cell varialbles
	int numCells = 0;
	int numCellsL0 = 0;
	int numCellsL1 = 0;
	int numCellsL2 = 0;
	int numCellsL3 = 0;

	xAOD::CaloCluster::const_cell_iterator cell_itr = clus->begin();
	xAOD::CaloCluster::const_cell_iterator cell_end = clus->end();

	for (; cell_itr != cell_end; ++cell_itr) {
	  const CaloCell* cell = *cell_itr;
	  if (!cell)
	    continue;

	  numCells++;
	  const auto sampling = cell->caloDDE()->getSampling();

	  if (CaloCell_ID::PreSamplerB == sampling || CaloCell_ID::PreSamplerE == sampling) {
	    numCellsL0++;
	  } else if (CaloCell_ID::EMB1 == sampling || CaloCell_ID::EME1 == sampling) {
	    numCellsL1++;
	  } else if (CaloCell_ID::EMB2 == sampling || CaloCell_ID::EME2 == sampling) {
	    numCellsL2++;
	  } else if (CaloCell_ID::EMB3 == sampling || CaloCell_ID::EME3 == sampling) {
	    numCellsL3++;
	  } 
	} // end of for

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
			  eta, Et, Etruth, Eovp,
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
			  delEta2,
			  numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3,
			  weight
			  );

      } // truth-match 
    } // loop over electrons
  }

  ATH_MSG_DEBUG("The event had " << numEl << " electrons out of which " << numElPass << " passed tight.");

  //m_histograms["numEl"]->Fill(numElPass, weight);


  // if (numElPass > 1) {
  //   const double minv = P4Helpers::invMass(leading, second);
  //   m_histograms["minv"]->Fill(minv, weight);

  //   // if (minv > 71*GeV && minv < 111*GeV) {
  //   //   m_histograms["met"]->Fill(met->et(), weight);
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

	const auto clus = ph->caloCluster();

	static const SG::AuxElement::Accessor < std::vector< ElementLink< xAOD::CaloClusterContainer > > > caloClusterLinks("constituentClusterLinks");
	const auto clusLinks = caloClusterLinks(*clus);
	
	const auto numClusters = clusLinks.size();

	// if (!(numClusters == 1 && nWindowClusters == 0 && nExtraClusters == 0 ||
	//       numClusters > 1 && (nWindowClusters || nExtraClusters))) {
	//   ATH_MSG_ERROR("The cluster links or the counts are wrong!");
	//   ATH_MSG_ERROR("  numClusters = " << numClusters 
	// 		<< ", nWindowClusters = " << nWindowClusters
	// 		<< ", nExtraClusters = " << nExtraClusters);
	//   return StatusCode::RECOVERABLE;
	// }
	    

	const auto photonType = xAOD::EgammaHelpers::conversionType(ph);
	const auto Ereco = clus->rawE();
	const auto Etruth = (truthParticle) ? truthParticle->e() : 0.0;
	const auto Eres = (Ereco - Etruth)/Etruth;
	const auto eta = ph->eta();
	const auto eta2 = clus->etaBE(2);
	const auto etaTruth = (truthParticle) ? truthParticle->eta() : -999.0;
	const auto pt = Ereco/cosh(eta);
	const auto ptTruth = (truthParticle) ? truthParticle->pt() : 0.0;
	const bool isC = std::abs(eta2) <= 1.37;
	const bool isEC = std::abs(eta2) >= 1.52;

	if ((photonType == xAOD::EgammaParameters::unconverted && ph->vertex() != nullptr) ||
	    (photonType != xAOD::EgammaParameters::unconverted && ph->vertex() == nullptr)) {
	  ATH_MSG_FATAL("photonType = " << photonType << ", vertex pointer = " << ph->vertex());
	  return StatusCode::FAILURE;
	}

	if (m_onlyLookAtSingleClusters && numClusters != 1) {
	  static const SG::AuxElement::Accessor<int> nWindowClustersAcc ("nWindowClusters");
	  const auto nWindowClusters = nWindowClustersAcc(*clus);
	  static const SG::AuxElement::Accessor<int> nExtraClustersAcc ("nExtraClusters");
	  const auto nExtraClusters = nExtraClustersAcc(*clus);
	  ATH_MSG_WARNING("Found a photon of type " << photonType << " with numClusters = " << numClusters 
			  << ", nWindowClusters = " << nWindowClusters
			  << ", nExtraClusters = " << nExtraClusters);
	  return StatusCode::SUCCESS;
	}


	// get cell varialbles
	int numCells = 0;
	int numCellsL0 = 0;
	int numCellsL1 = 0;
	int numCellsL2 = 0;
	int numCellsL3 = 0;

	xAOD::CaloCluster::const_cell_iterator cell_itr = clus->begin();
	xAOD::CaloCluster::const_cell_iterator cell_end = clus->end();

	for (; cell_itr != cell_end; ++cell_itr) {
	  const CaloCell* cell = *cell_itr;
	  if (!cell)
	    continue;

	  numCells++;
	  const auto sampling = cell->caloDDE()->getSampling();

	  if (CaloCell_ID::PreSamplerB == sampling || CaloCell_ID::PreSamplerE == sampling) {
	    numCellsL0++;
	  } else if (CaloCell_ID::EMB1 == sampling || CaloCell_ID::EME1 == sampling) {
	    numCellsL1++;
	  } else if (CaloCell_ID::EMB2 == sampling || CaloCell_ID::EME2 == sampling) {
	    numCellsL2++;
	  } else if (CaloCell_ID::EMB3 == sampling || CaloCell_ID::EME3 == sampling) {
	    numCellsL3++;
	  } 
	} // end of for
	

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
	fillPhotonHists("", isC, isEC, eta, pt, Etruth, Eres, mu, numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3, weight);

	switch(photonType) {
	case xAOD::EgammaParameters::unconverted:
	  m_numUnconverted++;
	  fillPhotonHists("0T", isC, isEC, eta, pt, Etruth, Eres, mu, numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3, weight);
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
	      if (ph != ph2 && xAOD::P4Helpers::isInDeltaR(*clus, *(ph2->caloCluster()), 0.4, false)) {
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
				<< ", deltaR = " << xAOD::P4Helpers::deltaR(clus, ph2->caloCluster(), false)
				<< ", truthType = " << truthType2
				<< ", " << match
				);
	      }
	    }
	  }
	  break;
	case xAOD::EgammaParameters::singleSi:
	  m_numConversionsSingleTrackSi++;
	  fillPhotonHists("1TSi", isC, isEC, eta, pt, Etruth, Eres, mu, numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3, weight);
	  break;
	case xAOD::EgammaParameters::singleTRT:
	  m_numConversionsSingleTrackTRT++;
	  fillPhotonHists("1TTRT", isC, isEC, eta, pt, Etruth, Eres, mu, numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3, weight);
	  break;
	case xAOD::EgammaParameters::doubleSi:
	  m_numConversionsDoubleTrackSi++;
	  fillPhotonHists("2TSi", isC, isEC, eta, pt, Etruth, Eres, mu, numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3, weight);
	  break;
	case xAOD::EgammaParameters::doubleTRT:
	  m_numConversionsDoubleTrackTRT++;
	  fillPhotonHists("2TTRT", isC, isEC, eta, pt, Etruth, Eres, mu, numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3, weight);
	  break;
	case xAOD::EgammaParameters::doubleSiTRT:
	  m_numConversionsDoubleTrackMix++;
	  fillPhotonHists("2TMix", isC, isEC, eta, pt, Etruth, Eres, mu, numCells, numCellsL0, numCellsL1, numCellsL2, numCellsL3, weight);
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
  // 	  m_histograms["TRTPID1Trk"]->Fill(pid1, weight);
  // 	  if (nSiliconHits_trk1 < 4) {
  // 	    m_histograms["TRTPID1TrkTRT"]->Fill(pid1, weight);
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
			      float eta, float pt, float Etruth,
			      float Eres, 
			      float mu, 
			      int numCells,
			      int numCellsL0,
			      int numCellsL1,
			      int numCellsL2,
			      int numCellsL3,		       
			      float weight)
{
 
  if (m_photonRemoveCrack && (!isC && !isEC)) return;

  const std::string ptstr = "PtReco" + suffix;
  const std::string etastr = "EtaReco" + suffix;
  const std::string Eresstr = "EResolution" + suffix;
  const std::string numCellsstr = "NumCells" + suffix;
  const std::string numCellsL0str = "NumCellsL0" + suffix;
  const std::string numCellsL1str = "NumCellsL1" + suffix;
  const std::string numCellsL2str = "NumCellsL2" + suffix;
  const std::string numCellsL3str = "NumCellsL3" + suffix;

  const float ptGeV = pt/GeV;
  const float EtruthGeV = Etruth/GeV;

  ATH_MSG_INFO("ptstr = " << ptstr << ", pt/GeV = " << ptGeV);

  m_histograms.at(ptstr)->Fill(ptGeV, weight);
  m_histograms.at(etastr)->Fill(eta, weight);
  m_histograms.at(Eresstr)->Fill(Eres, weight);
  static_cast<TProfile*>(m_histograms.at(Eresstr+"_mu"))->Fill(mu, Eres, weight);
  if (mu > m_muCut) m_histograms.at(Eresstr+"_highmu")->Fill(Eres, weight);

  fill3DERes(suffix, Eres, EtruthGeV, std::abs(eta), weight);

  static_cast<TH2F*>(m_histograms.at(numCellsstr))->Fill(numCells, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at(numCellsL0str))->Fill(numCellsL0, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at(numCellsL1str))->Fill(numCellsL1, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at(numCellsL2str))->Fill(numCellsL2, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at(numCellsL3str))->Fill(numCellsL3, EtruthGeV, weight);

  if (isC) {
    const std::string EresstrC = Eresstr + "C";
    const std::string ptstrC = ptstr + "C";

    const std::string numCellsstrC = numCellsstr + "C";
    const std::string numCellsL0strC = numCellsL0str + "C";
    const std::string numCellsL1strC = numCellsL1str + "C";
    const std::string numCellsL2strC = numCellsL2str + "C";
    const std::string numCellsL3strC = numCellsL3str + "C";

    m_histograms.at(EresstrC)->Fill(Eres, weight);
    m_histograms.at(ptstrC)->Fill(ptGeV, weight);
    static_cast<TProfile*>(m_histograms.at(EresstrC+"_mu"))->Fill(mu, Eres, weight);
    if (mu > m_muCut) m_histograms.at(EresstrC+"_highmu")->Fill(Eres, weight);

    static_cast<TH2F*>(m_histograms.at(numCellsstrC))->Fill(numCells, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL0strC))->Fill(numCellsL0, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL1strC))->Fill(numCellsL1, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL2strC))->Fill(numCellsL2, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL3strC))->Fill(numCellsL3, EtruthGeV, weight);

  } else if (isEC) {
    const std::string EresstrEC = Eresstr + "EC";
    const std::string ptstrEC = ptstr + "EC";

    const std::string numCellsstrEC = numCellsstr + "EC";
    const std::string numCellsL0strEC = numCellsL0str + "EC";
    const std::string numCellsL1strEC = numCellsL1str + "EC";
    const std::string numCellsL2strEC = numCellsL2str + "EC";
    const std::string numCellsL3strEC = numCellsL3str + "EC";

    m_histograms.at(EresstrEC)->Fill(Eres, weight);
    m_histograms.at(ptstrEC)->Fill(ptGeV, weight);
    static_cast<TProfile*>(m_histograms.at(EresstrEC+"_mu"))->Fill(mu, Eres, weight);
    if (mu > m_muCut) m_histograms.at(EresstrEC+"_highmu")->Fill(Eres, weight);

    static_cast<TH2F*>(m_histograms.at(numCellsstrEC))->Fill(numCells, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL0strEC))->Fill(numCellsL0, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL1strEC))->Fill(numCellsL1, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL2strEC))->Fill(numCellsL2, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at(numCellsL3strEC))->Fill(numCellsL3, EtruthGeV, weight);

  }
}
 
void TestAlg::fillElectronHists(bool isC, bool isEC, 
				float eta, float pt, float Etruth,
				float Eovp,
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
				float delEta2,
				int numCells,
				int numCellsL0,
				int numCellsL1,
				int numCellsL2,
				int numCellsL3,		       
				float weight
				)
{
  auto nBLHitsOutls = nBL + nBLOutliers;
  auto nPixHitsOutls = nPix + nPixOutliers;
  auto nSCTHitsOutls = nSCT + nSCTOutliers;
  auto nSi = nPix + nSCT;
  auto nSiHitsOutls = nPixHitsOutls + nSCTHitsOutls;
  auto nBLHitsExp = (expectBLayerHit) ? nBLHitsOutls : 1;

  const float EtruthGeV = Etruth/GeV;

  m_histograms.at("ElEtaReco")->Fill(eta, weight);
  m_histograms.at("ElPtReco")->Fill(pt/GeV, weight);
  m_histograms.at("Eovp")->Fill(Eovp, weight);
  m_histograms.at("NumBLHits")->Fill(nBL, weight);
  m_histograms.at("NumBLHitsOutls")->Fill(nBLHitsOutls, weight);
  m_histograms.at("NumBLHitsExp")->Fill(nBLHitsExp, weight);
  m_histograms.at("NumPixHits")->Fill(nPix, weight);
  m_histograms.at("NumPixHitsOutls")->Fill(nPixHitsOutls, weight);
  m_histograms.at("NumSiHits")->Fill(nSi, weight);
  m_histograms.at("NumSiHitsOutls")->Fill(nSiHitsOutls, weight);
  m_histograms.at("DelPhi1")->Fill(delPhi1, weight);
  m_histograms.at("DelPhiRescaled1")->Fill(delPhiRescaled1, weight);
  m_histograms.at("DelPhi2")->Fill(delPhi2, weight);
  m_histograms.at("DelPhiRescaled2")->Fill(delPhiRescaled2, weight);
  m_histograms.at("DelEta2")->Fill(delEta2, weight);
  static_cast<TH2F*>(m_histograms.at("ElNumCells"))->Fill(numCells, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at("ElNumCellsL0"))->Fill(numCellsL0, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at("ElNumCellsL1"))->Fill(numCellsL1, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at("ElNumCellsL2"))->Fill(numCellsL2, EtruthGeV, weight);
  static_cast<TH2F*>(m_histograms.at("ElNumCellsL3"))->Fill(numCellsL3, EtruthGeV, weight);

  if (isC) {
    m_histograms.at("ElPtRecoC")->Fill(pt/GeV, weight);
    m_histograms.at("EovpC")->Fill(Eovp, weight);
    m_histograms.at("NumBLHitsC")->Fill(nBL, weight);
    m_histograms.at("NumBLHitsOutlsC")->Fill(nBLHitsOutls, weight);
    m_histograms.at("NumBLHitsExpC")->Fill(nBLHitsExp, weight);
    m_histograms.at("NumPixHitsC")->Fill(nPix, weight);
    m_histograms.at("NumPixHitsOutlsC")->Fill(nPixHitsOutls, weight);
    m_histograms.at("NumSiHitsC")->Fill(nSi, weight);
    m_histograms.at("NumSiHitsOutlsC")->Fill(nSiHitsOutls, weight);
    m_histograms.at("DelPhi1C")->Fill(delPhi1, weight);
    m_histograms.at("DelPhiRescaled1C")->Fill(delPhiRescaled1, weight);
    m_histograms.at("DelPhi2C")->Fill(delPhi2, weight);
    m_histograms.at("DelPhiRescaled2C")->Fill(delPhiRescaled2, weight);
    m_histograms.at("DelEta2C")->Fill(delEta2, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsC"))->Fill(numCells, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL0C"))->Fill(numCellsL0, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL1C"))->Fill(numCellsL1, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL2C"))->Fill(numCellsL2, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL3C"))->Fill(numCellsL3, EtruthGeV, weight);
  } else if (isEC) {
    m_histograms.at("ElPtRecoEC")->Fill(pt/GeV, weight);
    m_histograms.at("EovpEC")->Fill(Eovp, weight);
    m_histograms.at("NumBLHitsEC")->Fill(nBL, weight);
    m_histograms.at("NumBLHitsOutlsEC")->Fill(nBLHitsOutls, weight);
    m_histograms.at("NumBLHitsExpEC")->Fill(nBLHitsExp, weight);
    m_histograms.at("NumPixHitsEC")->Fill(nPix, weight);
    m_histograms.at("NumPixHitsOutlsEC")->Fill(nPixHitsOutls, weight);
    m_histograms.at("NumSiHitsEC")->Fill(nSi, weight);
    m_histograms.at("NumSiHitsOutlsEC")->Fill(nSiHitsOutls, weight);
    m_histograms.at("DelPhi1EC")->Fill(delPhi1, weight);
    m_histograms.at("DelPhiRescaled1EC")->Fill(delPhiRescaled1, weight);
    m_histograms.at("DelPhi2EC")->Fill(delPhi2, weight);
    m_histograms.at("DelPhiRescaled2EC")->Fill(delPhiRescaled2, weight);
    m_histograms.at("DelEta2EC")->Fill(delEta2, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsEC"))->Fill(numCells, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL0EC"))->Fill(numCellsL0, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL1EC"))->Fill(numCellsL1, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL2EC"))->Fill(numCellsL2, EtruthGeV, weight);
    static_cast<TH2F*>(m_histograms.at("ElNumCellsL3EC"))->Fill(numCellsL3, EtruthGeV, weight);
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

StatusCode TestAlg::initialize3DERes(std::string suffix)
{

  const std::string base = "EResolution3D" + suffix;

  for (size_t etaBin = 1; etaBin < m_EResAbsEtaBins.size(); etaBin++)  {
    for (size_t EBin = 1; EBin < m_EResEBins.size(); EBin++)  {

      std::ostringstream histnamess;

      histnamess << base << "_Eta_" <<  m_EResAbsEtaBins[etaBin] << "_Etruth_" <<  m_EResEBins[EBin];
      m_histograms[histnamess.str()] = new TH1F(histnamess.str().c_str(), 
						"Raw Energy Resolution;(E_{reco} - E_{truth})/E_{truth}", 
						m_numEResBins, m_EResLow, m_EResHigh);
      ATH_CHECK(m_thistSvc->regHist(std::string("/")+m_histFileName+"/Photon/" + histnamess.str(),
				    m_histograms[histnamess.str()]));
    }
  }
  return StatusCode::SUCCESS;
}

void TestAlg::fill3DERes(std::string suffix, 
			 float Eres, float EtruthGeV, float abseta,
			 float weight)
{
  const std::string base = "EResolution3D" + suffix;

  ATH_MSG_DEBUG("Photon with E = " << EtruthGeV << " GeV and |eta| = " << abseta);

  // first find eta bin
  size_t etaBin = 1;

  ATH_MSG_DEBUG("m_EResAbsEtaBins[" << etaBin << "] = " << m_EResAbsEtaBins[etaBin] << ", m_EResAbsEtaBins.size() = " << m_EResAbsEtaBins.size());
  ATH_MSG_DEBUG("abseta < m_EResAbsEtaBins[etaBin] = " << (abseta > m_EResAbsEtaBins[etaBin]));
  ATH_MSG_DEBUG("etaBin < m_EResAbsEtaBins.size() = " << (etaBin < m_EResAbsEtaBins.size()));
  while (abseta > m_EResAbsEtaBins[etaBin] && etaBin < m_EResAbsEtaBins.size()) {
    etaBin++;
    ATH_MSG_DEBUG("m_EResAbsEtaBins[" << etaBin << "] = " << m_EResAbsEtaBins[etaBin]);
  }
  if (etaBin == m_EResAbsEtaBins.size()) {
    // put it in the last bin
    --etaBin;
  }

  size_t EBin = 1;
  while (EtruthGeV > m_EResEBins[EBin] && EBin < m_EResEBins.size()) {
    EBin++;
  }
  if (EBin == m_EResEBins.size()) {
    // put it in the last bin
    --EBin;
  }

  ATH_MSG_DEBUG("  put in EBin = " << EBin << " and etaBin = " << etaBin); 


  std::ostringstream histnamess;
  histnamess << base << "_Eta_" <<  m_EResAbsEtaBins[etaBin] << "_Etruth_" <<  m_EResEBins[EBin];
  m_histograms[histnamess.str()]->Fill(Eres, weight);
}

