/** @class TestAlgESD

    Put code to test various things here.
    
    @author Jovan Mitrevski
*/


#ifndef INSITUANALYSIS_TESTALGESD_H
#define INSITUANALYSIS_TESTALGESD_H

/// Gaudi Tools
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgorithm.h"
/// Storegate
#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/DataHandle.h"
#include "GaudiKernel/ITHistSvc.h"

//#include "TrkEventPrimitives/ParticleHypothesis.h"
//#include "CLHEP/Matrix/SymMatrix.h"



//class AtlasDetectorID;
//class Identifier;
class TruthUtils;
class StoreGateSvc;
//class IEMBremsstrahlungBuilder;
//class IEMFourMomBuilder;
//class EMErrorDetail;

namespace Analysis {
  class Electron;
}

// namespace InDet 
// {
//   class InDetJetFitterUtils;
// }

// namespace Trk {
//   class MeasuredNeutralPerigee;
//   class VxCandidate;
// }

class TestAlgESD : public AthAlgorithm
{
public:
  
  /** Standard Athena-Algorithm Constructor */
  TestAlgESD(const std::string& name, ISvcLocator* pSvcLocator);
  /** Default Destructor */
  ~TestAlgESD();
  
  /** standard Athena-Algorithm method */
  StatusCode          initialize();
  /** standard Athena-Algorithm method */
  StatusCode          execute();
  /** standard Athena-Algorithm method */
  StatusCode          finalize();
  
private:

  /// a handle on Store Gate 
  StoreGateSvc* m_storeGate;

  ToolHandle<TruthUtils> m_TruthUtils;
  std::string m_TruthUtilsName;

  /// a handle on the Hist/TTree registration service
  ITHistSvc * m_thistSvc;
  std::string m_histFileName;

  std::map<std::string, TH1*> m_histograms;

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

  std::string m_TrackParticleContainerName;
  std::string m_TrackParticleTruthCollectionName;

//   std::string m_EMTrackFitContainerName;
//   std::string m_EMErrorDetailContainerName;
  std::string m_McEventCollectionContainerName;

  int numPhotons;
  int numConversions;
  int numConversionsNotTP;
  int numConversionsNotTPTRTOnly;

  int numElectrons;
  int numElConversions;

};

#endif 
