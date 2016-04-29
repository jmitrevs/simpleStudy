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

  void fillPhotonHists(std::string suffix, bool isC, bool isEC, float eta, float Eres);

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
  double m_truthElectronPtMin;

  /** Photon selection */
  std::string m_PhotonContainerName;
  double m_photonPt;
  double m_photonEta;
  std::string m_photonIsEMFlag;
  int m_photonIsEM;
  double m_truthPhotonPtMin;
  bool m_truthMatchPhotons;

  /** MET selecton */
  std::string m_METContainerName;

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
