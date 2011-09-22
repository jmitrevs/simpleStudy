#ifndef   SIMPLESTUDY_TRUTHUTILS_H
#define   SIMPLESTUDY_TRUTHUTILS_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include  "HepMC/GenParticle.h"
#include  "HepMC/GenVertex.h"
#include  "ParticleTruth/TrackParticleTruthCollection.h"
#include  "Particle/TrackParticleContainer.h"
#include  "VxVertex/VxContainer.h"
#include  "egammaEvent/PhotonContainer.h"
#include  "egammaEvent/ElectronContainer.h"
#include  "McParticleEvent/TruthParticleContainer.h"
#include  "McParticleUtils/McVtxFilter.h"
#include  "GeneratorObjects/McEventCollection.h"
#include  "MCTruthClassifier/IMCTruthClassifier.h"

class TruthUtils : public AthAlgTool {

 public:
  
  TruthUtils (const std::string& type,const std::string& name, const IInterface* parent);
  virtual ~TruthUtils();

  static const InterfaceID& interfaceID();
  virtual StatusCode initialize();
  virtual StatusCode finalize();

  float m_truth_cone_max;
  float m_photon_pt_min;
  float m_electron_pt_min;
  
  std::vector<const HepMC::GenParticle*>  m_truthMap;

  ToolHandle<IMCTruthClassifier> m_truthClassifier  ;

 public:
  enum TruthType {
    unknownType = 0,
    fakePair    = 1,
    truthPair   = 2,
    fakeSingle  = 3,
    truthSingle = 4
  };

  enum PairType {
    unknownPair = 0,
    SiSi        = 1,
    SiTrt       = 2,
    TrtTrt      = 3,
    Si          = 4,
    Trt         = 5
  };

  //* Retrieve TrackParticles
  std::vector<const Rec::TrackParticle*> vertexToTrackParticle(const Trk::VxCandidate*) const;
  std::vector<const Rec::TrackParticle*> vertexToTrackParticle( Trk::VxCandidate*) const;
  const Rec::TrackParticle*              vertexTrackToTrackParticle(const Trk::VxTrackAtVertex*);

  const TruthParticle* trkParticleToTruthParticle(const Rec::TrackParticle*,
                                                  const Rec::TrackParticleContainer*,
                                                  const TrackParticleTruthCollection*,
                                                  const TruthParticleContainer*);

  const HepMC::GenParticle* trkParticleToGenParticle(const Rec::TrackParticle*, 
						     const Rec::TrackParticleContainer*, 
						     const TrackParticleTruthCollection*);

  std::vector<const HepMC::GenParticle*> getVertexOutGenParticle(const HepMC::GenVertex*);

  //* Global truth 
  McVtxFilter  m_mcVtxFilter;
  std::vector<const TruthParticle*>  getPhotonTruth(const TruthParticleContainer* );
  std::vector<const TruthParticle*>  getConversionTruth(const TruthParticleContainer* );
  int  getLPhotonTruth( const TruthParticle* );

  bool TruthParticleTotrkParticle(const TruthParticle* epart,
				  const Rec::TrackParticleContainer* trkPartCont, 
				  const TrackParticleTruthCollection* trkPartTruthCol,  
				  const TruthParticleContainer* mcPartCont);
  
  const TruthParticle* getMotherTruthParticle(std::vector<const TruthParticle*> );
  const HepMC::GenParticle*  getMotherGenParticle(std::vector<const HepMC::GenParticle*> );
  PairType   getPairType(std::vector<const Rec::TrackParticle*> );
  TruthType  getTruthType(std::vector<const TruthParticle*> );

  void  MC_ConvType( Trk::VxCandidate*, bool&, bool&, bool&, int&, 
		     const TruthParticleContainer*, 
		     const Rec::TrackParticleContainer*, 
		     const TrackParticleTruthCollection*);
  
  const std::vector<const HepMC::GenParticle*> getTruthTracks(const Trk::VxCandidate*  convVtx) const;
  std::pair<double,double> truthBrem(const HepMC::GenParticle*, const McEventCollection*);

  std::vector<const HepMC::GenParticle*>* getNearbyGenParticles(const HepMC::GenEvent*, 
								const HepMC::GenParticle*, 
								float);

  std::vector<const HepMC::GenParticle*>* getNearbyGenParticles(const HepMC::GenEvent*, 
								Analysis::Photon*, 
								float);

  // for photons
  std::vector<const HepMC::GenParticle*>* FillTruthMap(const HepMC::GenEvent*, float);

  // for electrons
  std::vector<const HepMC::GenParticle*>* FillTruthMapEl(const HepMC::GenEvent*, float);
  
  const HepMC::GenParticle* DoTruthMatch(Analysis::Photon*, const HepMC::GenEvent*,
					 const Rec::TrackParticleContainer*, 
					 const TrackParticleTruthCollection*, bool, bool);
  
  const HepMC::GenParticle* FindConversionTruthMatch(const Trk::VxCandidate*, const HepMC::GenEvent*, 
						     const Rec::TrackParticleContainer*, 
						     const TrackParticleTruthCollection*,
						     Analysis::Photon*, bool);
  const HepMC::GenParticle* FindNonConversionTruthMatch(Analysis::Photon*, const HepMC::GenEvent*);
  
};

#endif  // TRUTHUTILS_H
