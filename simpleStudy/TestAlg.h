/** @class TestAlg

    Put code to test various things here.
    
    @author Jovan Mitrevski
*/


#ifndef SIMPLESTUDY_TESTALG_H
#define SIMPLESTUDY_TESTALG_H

/// Gaudi Tools
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
/// Storegate
//#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/DataHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include <string>
#include <vector>
#include <cstdint>

//class AtlasDetectorID;
//class Identifier;
//class TruthUtils;
//class StoreGateSvc;
//class IEMBremsstrahlungBuilder;
//class IEMFourMomBuilder;
//class EMErrorDetail;
//class IPAUcaloIsolationTool;
//class IMCTruthClassifier;
//class IAthElectronIsEMSelector;
//class IAthPhotonIsEMSelector;
class IAsgSelectionTool;

#include "TH1.h"

class TestAlg : public AthAlgorithm
{
public:
  
  /** Standard Athena-Algorithm Constructor */
  TestAlg(const std::string& name, ISvcLocator* pSvcLocator);
  /** Default Destructor */
  ~TestAlg();
  
  /** standard Athena-Algorithm method */
  StatusCode          initialize();
  /** standard Athena-Algorithm method */
  StatusCode          execute();
  /** standard Athena-Algorithm method */
  StatusCode          finalize();
  
private:

  std::vector<Double_t> m_EResEBins; // in GeV
  std::vector<Double_t> m_EResAbsEtaBins;

  static constexpr int m_numEResBins = 170;
  static constexpr double m_EResLow = -1.1;
  static constexpr double m_EResHigh = 0.6;

  StatusCode initialize3DERes(std::string suffix);

  void fill3DERes(std::string suffix, 
		  float Eres, float EtruthGeV, float abseta,
		  float weight);

  void fillPhotonHists(std::string suffix, bool isC, bool isEC, 
		       float eta, float pt, float Etruth,
		       float Eres, float mu, 
		       float widths1,
		       float weta1,
		       int numCells,
		       int numCellsL0,
		       int numCellsL1,
		       int numCellsL2,
		       int numCellsL3,		       
		       float weight);

  void fillElectronHists(bool isC, bool isEC, 
			 float eta, float pt, float Etruth,
			 float Eovp,
			 uint8_t nBL,
			 uint8_t nBLOutliers,
			 uint8_t nPi,
			 uint8_t nPiOutliers,
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
			 );

  //ToolHandle<IAthElectronIsEMSelector> m_electronSelector;
  ToolHandle<IAsgSelectionTool> m_electronSelector;
  //ToolHandle<IAthPhotonIsEMSelector> m_photonSelector;
  ToolHandle<IAsgSelectionTool> m_photonSelector;

  //ToolHandle<TruthUtils> m_TruthUtils;
  //ToolHandle<IMCTruthClassifier> m_MCTruthClassifier;
  //ToolHandle<IPAUcaloIsolationTool> m_PAUcaloIsolationTool;

  /// a handle on the Hist/TTree registration service
  ITHistSvc * m_thistSvc;
  std::string m_histFileName;

  std::map<std::string, TH1*> m_histograms;

  std::vector<unsigned int> m_runEvents; // events to execute (if not null)
  std::set<unsigned int> m_theEvents; // conversion of above

  bool m_runOnlySome; // set to true if m_runEvents is not null

  bool m_doTruth;
  bool m_doElectrons;
  bool m_doPhotons;

  /** Electron selection */
  std::string m_ElectronContainerName;
  double m_electronPt;
  double m_electronEta;
  std::string m_electronIsEMFlag;
  int m_electronIsEM;
  //double m_truthElectronPtMin;
  bool m_truthMatchElectrons;
  bool m_truthMatchElectronAsPhotons;
  uint16_t m_electronAuthor;

  /** Photon selection */
  std::string m_PhotonContainerName;
  double m_photonPt;
  double m_photonEta;
  std::string m_photonIsEMFlag;
  int m_photonIsEM;
  //double m_truthPhotonPtMin;
  bool m_truthMatchPhotons;
  bool m_truthMatchConversions; //< Additionally check that converted/unonverted match truth
  uint16_t m_photonAuthor;
  float m_muCut; // how to define high mu events
  bool m_photonRemoveCrack;

  bool m_onlyLookAtSingleClusters; // if true only look at single clus el and ph

  std::string m_egammaTruthParticleContainerName;

  /** MET selecton */
  //std::string m_METContainerName;

  /** egamma selecton (for HLT) */
  std::string m_egammaContainerName;

  std::string m_TrackParticleContainerName;
  std::string m_TrackParticleTruthCollectionName;

  std::string m_ConversionsName;
//   std::string m_EMTrackFitContainerName;
//   std::string m_EMErrorDetailContainerName;

  std::string m_McEventCollectionContainerName;
  std::string m_xAODTruthEventContainerName;
  std::string m_xAODTruthParticleContainerName;
  std::string m_xAODTruthVertexContainerName;

  int m_numPhotons;
  int m_numUnconverted;

  int m_numConversionsSingleTrackSi;
  int m_numConversionsDoubleTrackSi;
  int m_numConversionsSingleTrackTRT;
  int m_numConversionsDoubleTrackTRT;
  int m_numConversionsDoubleTrackMix;

  int m_numElectrons;
  int m_numTightElectrons;
  int m_numElectronsAuthorElectron;
  int m_numTightElectronsAuthorElectron;
  int m_numElectronsAuthorSofte;
  int m_numElectronsAuthorFrwd;
  int m_numElConversions;

};

#endif 
