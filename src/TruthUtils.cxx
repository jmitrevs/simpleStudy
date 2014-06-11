#include  "simpleStudy/TruthUtils.h"

#include  "TrkTrack/LinkToTrack.h"
#include  "TrkTrackLink/ITrackLink.h"
#include  "TrkParticleBase/LinkToTrackParticleBase.h"
#include  "Particle/TrackParticle.h"
#include  "TrkTrackSummary/TrackSummary.h"
#include  "VxVertex/VxTrackAtVertex.h"
#include  "FourMomUtils/P4Helpers.h"

#include "egammaEvent/EMConvert.h"
#include "egammaEvent/EMErrorDetail.h"

static const InterfaceID IID_ITruthUtils("TruthUtils", 1, 0);


TruthUtils::TruthUtils(const std::string& type, const std::string& name, const IInterface* parent) :
  AthAlgTool(type, name, parent),
  m_truthClassifier("MCTruthClassifier")
{
  //* Initialize the McVertexFilter 
  m_mcVtxFilter.setMatchSign(true);
  m_mcVtxFilter.setMatchBranches(true);
  m_mcVtxFilter.setDecayPattern("25-> 22 + 22");

  declareInterface<TruthUtils>(this);
  declareProperty("MCTruthClassifierTool",             m_truthClassifier);
  declareProperty("TruthConeMatch",                    m_truth_cone_max);
}

const InterfaceID& TruthUtils::interfaceID() {
  return IID_ITruthUtils;
}

TruthUtils::~TruthUtils() {}

StatusCode TruthUtils::initialize() {
  msg(MSG::INFO) 
      << "Initializing " << name() << "..." 
      << endreq;
  
  //Retrieve MCTruthClassifier tool
  if ( m_truthClassifier.retrieve().isFailure() ) {
    msg(MSG::ERROR) << "Cannot retrieve MCTruthClassifier " << m_truthClassifier << endreq;
    return StatusCode::FAILURE;
  }
  else {
    ATH_MSG_DEBUG("Retrieved MCTruthClassifier tool " << m_truthClassifier);
  }
  m_truthClassifier->setdeltaRMatchCut(10000.);

  msg(MSG::INFO) << "Initialization successful" << endreq;

  return StatusCode::SUCCESS;
}

StatusCode TruthUtils::finalize() {
  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve the global photon truth                                                                           
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<const TruthParticle*>    TruthUtils::getPhotonTruth(const TruthParticleContainer* mcpartTES) {

  std::vector<const TruthParticle*>      mcPhotonTES;
  mcPhotonTES.clear();

  TruthParticleContainer::const_iterator itr  = mcpartTES->begin();
  TruthParticleContainer::const_iterator itrE = mcpartTES->end();
  for(; itr!=itrE; ++itr) {

    int pdgId = (*itr)->pdgId();
    if(pdgId == PDG::Higgs0 && (*itr)->hasMother(PDG::Higgs0)) {
      if((*itr)->nDecay()==2) {   //* photon->e+e-                                                            

        if((*itr)->child(0)->pdgId()==PDG::gamma && (*itr)->child(1)->pdgId()==PDG::gamma) {

          //  std::cout << "HIGGS  "  << (*itr)->status() << "  RECORDED TRUTH PHOTON  " <<  (*itr)->child(0)->barcode() << "  PDGID  " << (*itr)->child(0)->pdgId() << "  STATUS  " << (*itr)->child(0)->status() <<  "  SECOND  "<< (*itr)->child(1)->barcode() << "  PDGID  " << (*itr)->child(1)->pdgId() << "  STATUS  " << (*itr)->child(1)->status()<< std::endl; 
          for( unsigned u=0 ; u<(*itr)->nDecay() ; ++u )  mcPhotonTES.push_back((*itr)->child(u));
	    }
      }  //* Higgs->2Photons                                                                                  
    }
  } // End of looping over the TruthParticles                                                                 
  return mcPhotonTES;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve the global photon conversion truth                                                                
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<const TruthParticle*>  TruthUtils::getConversionTruth(const TruthParticleContainer* mcpartTES) {
  std::vector<const TruthParticle*> mcPhotonTES;
  mcPhotonTES.clear();

  TruthParticleContainer::const_iterator itr  = mcpartTES->begin();
  TruthParticleContainer::const_iterator itrE = mcpartTES->end();
  for(; itr!=itrE; ++itr) {
    int pdgId = (*itr)->pdgId();
    if(pdgId == PDG::gamma && (*itr)->hasMother(PDG::gamma)) {
      //  if(pdgId == PDG::gamma ) {
      if((*itr)->nDecay()==2) { // gamma's children ...
	if((*itr)->child(0)->pdgId()==PDG::e_minus || (*itr)->child(1)->pdgId()==PDG::e_minus )
	  {
	    const TruthParticle* epart = (*itr);
	    mcPhotonTES.push_back(epart);
	  }// check children                                                                          
      } // number of children 
    }	// grand-mothers
  } // End of looping over the TruthParticles                                                                 
  return mcPhotonTES;
}


int  TruthUtils::getLPhotonTruth( const TruthParticle* mcPhoton ) {
  int pdgId = mcPhoton->pdgId();
  if(pdgId == PDG::gamma && mcPhoton->hasMother(PDG::gamma)) return 1;  
  return 0;
}
  


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve TrackParticle from VxCandidate
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<const Rec::TrackParticle*>  TruthUtils::vertexToTrackParticle(const Trk::VxCandidate* vxCand) const 
{

  std::vector<const Rec::TrackParticle*> recTrackParticle;
  recTrackParticle.clear();
  int numTracksPerVertex =vxCand->vxTrackAtVertex()->size();
  
  ATH_MSG_DEBUG("Number of tracks per vertex = " << numTracksPerVertex);

  for( int counter =0; counter < numTracksPerVertex; ++counter) {
    
    Trk::VxTrackAtVertex* trackAtVtx = (*vxCand->vxTrackAtVertex())[counter];

    ATH_MSG_DEBUG("trackAtVtx = " << trackAtVtx);
    
    const Trk::ITrackLink*  trLink = trackAtVtx->trackOrParticleLink();

    ATH_MSG_DEBUG("trLink = " << trLink);
    const Trk::TrackParticleBase* tempTrkPB(0);
    
    if(trLink!=0) {
      const Trk::LinkToTrackParticleBase* linkToTrackPB = dynamic_cast<const Trk::LinkToTrackParticleBase*>(trLink);
      if(linkToTrackPB!=0) {
	if(linkToTrackPB->isValid())  tempTrkPB = linkToTrackPB->cachedElement();
      }
    }
    if(tempTrkPB) {
      //* Dynamic_cast TrackParticleBase to TrackParticle
     const Rec::TrackParticle* tpTrk  = dynamic_cast<const Rec::TrackParticle*>(tempTrkPB);
     recTrackParticle.push_back(tpTrk);
    }

  }
    
  return recTrackParticle;

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve TrackParticle from VxCandidate
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<const Rec::TrackParticle*>  TruthUtils::vertexToTrackParticle( Trk::VxCandidate* vxCand) const {

  std::vector<const Rec::TrackParticle*> recTrackParticle;
  recTrackParticle.clear();

  int numTracksPerVertex =vxCand->vxTrackAtVertex()->size();
  
  for( int counter =0; counter < numTracksPerVertex; ++counter) {
    
    Trk::VxTrackAtVertex* trackAtVtx = (*vxCand->vxTrackAtVertex())[counter];
    
    const Trk::ITrackLink*  trLink = trackAtVtx->trackOrParticleLink();
    const Trk::TrackParticleBase* tempTrkPB(0);
    
    if(trLink!=0) {
      const Trk::LinkToTrackParticleBase* linkToTrackPB = dynamic_cast<const Trk::LinkToTrackParticleBase*>(trLink);
      if(linkToTrackPB!=0) {
	if(linkToTrackPB->isValid())  tempTrkPB = linkToTrackPB->cachedElement();
      }
    }
    if(tempTrkPB) {
      //* Dynamic_cast TrackParticleBase to TrackParticle
     const Rec::TrackParticle* tpTrk  = dynamic_cast<const Rec::TrackParticle*>(tempTrkPB);
     recTrackParticle.push_back(tpTrk);
    }

  }
    
  return recTrackParticle;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Retrieve TrackParticle from VxTrackAtVertex
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const Rec::TrackParticle*  TruthUtils::vertexTrackToTrackParticle(const Trk::VxTrackAtVertex* trackAtVtx) {

  const Rec::TrackParticle* tpTrk(0);
  const Trk::ITrackLink*  trLink = trackAtVtx->trackOrParticleLink();
    
  if(trLink!=0) {
    const Trk::LinkToTrackParticleBase* linkToTrackPB = dynamic_cast<const Trk::LinkToTrackParticleBase*>(trLink);
    if(linkToTrackPB!=0 && linkToTrackPB->isValid()) {
      const Trk::TrackParticleBase* tempTrkPB = linkToTrackPB->cachedElement();
      if(tempTrkPB) tpTrk = dynamic_cast<const Rec::TrackParticle*>(tempTrkPB);
    }
  }

  return tpTrk;
}


//// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TrackParticle -> TrackTruthParticle -> GenParticle ->TruthParticle
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const TruthParticle* TruthUtils::trkParticleToTruthParticle(const Rec::TrackParticle* trkPart, 
                                                            const Rec::TrackParticleContainer* trkPartCont, 
                                                            const TrackParticleTruthCollection* trkPartTruthCol,  
                                                            const TruthParticleContainer* mcPartCont) {

  const TruthParticle* mcPart(0);
  
  ElementLink<Rec::TrackParticleContainer> myLink;
  myLink.setElement(const_cast<Rec::TrackParticle*>(trkPart));
  myLink.setStorableObject(*trkPartCont);

  TrackParticleTruthCollection::const_iterator TPItr = trkPartTruthCol->find(myLink);
  if(TPItr!=trkPartTruthCol->end()) {
    const HepMcParticleLink& HepMCLink = (*TPItr).second.particleLink();
    const HepMC::GenParticle* truthParticle = HepMCLink.cptr();

    if(truthParticle) {
      mcPart =  new TruthParticle(truthParticle, mcPartCont); 
    }
  }

  return mcPart;
}

//// /////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TrackParticle -> TrackTruthParticle -> GenParticle
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const HepMC::GenParticle* TruthUtils::trkParticleToGenParticle(const Rec::TrackParticle* trkPart, 
							       const Rec::TrackParticleContainer* trkPartCont, 
							       const TrackParticleTruthCollection* trkPartTruthCol) {

  const HepMC::GenParticle* mcPart(0);
  
  ElementLink<Rec::TrackParticleContainer> myLink;
  myLink.setElement(const_cast<Rec::TrackParticle*>(trkPart));
  myLink.setStorableObject(*trkPartCont);

  TrackParticleTruthCollection::const_iterator TPItr = trkPartTruthCol->find(myLink);
  if(TPItr!=trkPartTruthCol->end()) {
    const HepMcParticleLink& HepMCLink = (*TPItr).second.particleLink();
    mcPart = HepMCLink.cptr();
    //const HepMC::GenParticle* truthParticle = HepMCLink.cptr();
  }

  return mcPart;
}


bool  TruthUtils::TruthParticleTotrkParticle(const TruthParticle* epart,
					     const Rec::TrackParticleContainer* trkPartCont, 
					     const TrackParticleTruthCollection* trkPartTruthCol,  
					     const TruthParticleContainer* mcPartCont) {
  
  //  std::cout << " try to find !!! epart == " << "  " << epart->pt()  << std::endl;

  const TruthParticle* mcPart(0);    
  typedef Rec::TrackParticleContainer::const_iterator TPCIter;
  for (TPCIter i=trkPartCont->begin(); i!=trkPartCont->end(); i++) {
   
    ElementLink<Rec::TrackParticleContainer> myLink;
    myLink.setElement(const_cast<Rec::TrackParticle*>(*i));
    myLink.setStorableObject(*trkPartCont);
    
    TrackParticleTruthCollection::const_iterator TPItr = trkPartTruthCol->find(myLink);
    if(TPItr!=trkPartTruthCol->end()) {
      const HepMcParticleLink& HepMCLink = (*TPItr).second.particleLink();
      const HepMC::GenParticle* truthParticle = HepMCLink.cptr();
      
      if(truthParticle) {
	//	std::cout << " candidate !!!  in TruthParticleTotrkParticle !" << std::endl;
	mcPart =  new TruthParticle(truthParticle, mcPartCont); 
	//	std::cout << " candidate !!! Pt mcPart and epart == " << mcPart->pt() << "  "  << epart->pt() << std::endl;
	if( int(mcPart->pt()) == int(epart->pt()) ){
	  return true;
	}
      }
    }    
  }

  return false;
}

  


const TruthParticle*  TruthUtils::getMotherTruthParticle (std::vector<const TruthParticle*>  mcPartV) {
  // Input : MC particle matched to reconstructed tracks
  const TruthParticle* mcPart(0);
  const TruthParticle* tmpPart(0);
  int numOfMc = mcPartV.size();

  std::vector<const TruthParticle*>::iterator itrk = mcPartV.begin();
  if((*itrk)->nParents()==1 && (*itrk)->mother())     tmpPart= (*itrk)->mother();
  if(tmpPart) {
    if(numOfMc==1) {
      return tmpPart;
    }else if(numOfMc==2) {
      ++itrk;
      if((*itrk)->nParents()==1 &&(*itrk)->mother())
        {
          if(tmpPart == (*itrk)->mother()){
	    return tmpPart;
	  }
        }     
    }
  }
  return mcPart;
  
}

const HepMC::GenParticle*  TruthUtils::getMotherGenParticle(std::vector<const HepMC::GenParticle*>  mcPartV) {
  // Input : MC particle matched to reconstructed tracks
  const HepMC::GenParticle* mcPart(0);
  const HepMC::GenParticle* tmpPart(0);
  int numOfMc = mcPartV.size();

  if(numOfMc > 0){

    std::vector<const HepMC::GenParticle*>::iterator itrk = mcPartV.begin();
    if((*itrk)->production_vertex()->particles_in_size()==1){
      const HepMC::GenVertex* vtx = (*itrk)->production_vertex();
      HepMC::GenVertex::particles_in_const_iterator it_in_p  = vtx->particles_in_const_begin();
      tmpPart = (*it_in_p);
    }
    if(tmpPart) {
      if(numOfMc==1) {
	return tmpPart;
      }else if(numOfMc==2) {
	++itrk;
	if((*itrk)->production_vertex()->particles_in_size()==1) {
	  const HepMC::GenVertex* vtx_next = (*itrk)->production_vertex();
	  HepMC::GenVertex::particles_in_const_iterator it_in_p_next  = vtx_next->particles_in_const_begin();
	  const HepMC::GenParticle* parent = (*it_in_p_next);
	  if(tmpPart == parent) return tmpPart;
	}     
      }
    }

  }

  return mcPart;
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
////                                                                                                     
// GenVertex -> vector GenParticle (out)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
std::vector<const HepMC::GenParticle*> TruthUtils::getVertexOutGenParticle(const HepMC::GenVertex* v){
  std::vector<const HepMC::GenParticle*> vecVtxGenPart;

  if(v->particles_out_size() != 2) return vecVtxGenPart;  //Concentrate on decay vertices with two outgoing particles

  int nchildren = 0;
  HepMC::GenVertex::particles_out_const_iterator it_out_p= v->particles_out_const_begin();
  HepMC::GenVertex::particles_out_const_iterator it_out_p_e= v->particles_out_const_end();

  for(; it_out_p!=it_out_p_e;++it_out_p){
    const HepMC::GenParticle* child = *it_out_p;
    if(child){
      if(child->pdg_id() == 11) vecVtxGenPart.push_back(child);
      if(child->pdg_id() == -11) vecVtxGenPart.push_back(child);
    }
    nchildren++;
  }

  return vecVtxGenPart;
}


std::vector<const HepMC::GenParticle*>* TruthUtils::getNearbyGenParticles(const HepMC::GenEvent* myEvent, const HepMC::GenParticle* tpart, float cut){

  std::vector<const HepMC::GenParticle*>* gp_vec = new std::vector<const HepMC::GenParticle*>();

  if(!myEvent) return gp_vec;
  if(!tpart)   return gp_vec;

  HepMC::GenEvent::particle_const_iterator pitr;
  for(pitr=myEvent->particles_begin(); pitr!=myEvent->particles_end(); ++pitr){
    const HepMC::GenParticle* thePart = (*pitr);
    if(thePart->pdg_id() != 22)         continue;
    if(thePart == tpart)                continue;

    double delta_eta = (tpart->momentum().pseudoRapidity() - thePart->momentum().pseudoRapidity());
    double delta_phi = P4Helpers::deltaPhi(tpart->momentum().phi(), thePart->momentum().phi());
    
    double delta_R = hypot(delta_eta, delta_phi);
    
    if(delta_R < cut) gp_vec->push_back(thePart);
  }  

  return gp_vec;
}


std::vector<const HepMC::GenParticle*>* TruthUtils::getNearbyGenParticles(const HepMC::GenEvent* myEvent, Analysis::Photon* p, float cut){

  std::vector<const HepMC::GenParticle*>* gp_vec = new std::vector<const HepMC::GenParticle*>();

  if(!myEvent) return gp_vec;
  if(!p)       return gp_vec;

  HepMC::GenEvent::particle_const_iterator pitr;
  for(pitr=myEvent->particles_begin(); pitr!=myEvent->particles_end(); ++pitr){
    const HepMC::GenParticle* thePart = (*pitr);
    if(thePart->pdg_id() != 22)                        continue;
    if(thePart->status() < 1 || thePart->status() > 2) continue;

    double delta_eta = (p->eta() - thePart->momentum().pseudoRapidity());
    double delta_phi = P4Helpers::deltaPhi(p->phi(), thePart->momentum().phi());
    
    double delta_R = hypot(delta_eta, delta_phi);
    
    if(delta_R < cut) gp_vec->push_back(thePart);
  }  

  return gp_vec;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
////                                                                                                     
// TrackParticle -> TrackTruthParticle -> GenParticle ->TruthParticle                                    
/////////////////////////////////////////////////////////////////////////////////////////////////////////
TruthUtils::TruthType  TruthUtils::getTruthType (std::vector<const TruthParticle*>  mcPartV) {

  TruthType  type = unknownType; 
  int numOfMc = mcPartV.size();
  
  if(numOfMc <1)  return type;

  return type;
}



TruthUtils::PairType  TruthUtils::getPairType (std::vector<const Rec::TrackParticle*>  trackPartV) {

  PairType  type = unknownPair; 

  int numOfTrack = trackPartV.size();
  int numOfTrt = 0;

  std::vector<const Rec::TrackParticle*>::iterator itrk = trackPartV.begin();
  for(; itrk!=trackPartV.end(); ++itrk) {

    const Trk::TrackSummary*  trkSum= (*itrk)->trackSummary();
    int numSiHit = trkSum->get(Trk::numberOfPixelHits) + trkSum->get(Trk::numberOfSCTHits);
    if(numSiHit==0)   numOfTrt++;
  }

  if(numOfTrack==1) {
    if(numOfTrt==1)       type = Trt;
    else                  type = Si;

  }else if(numOfTrack==2) {
    if(numOfTrt==1)       type = SiTrt;
    else if(numOfTrt==2)  type = TrtTrt;
    else                  type = SiSi;

  } 
    //    std::cout << "No Tracks To Be Investigated!" << std::endl;

  return type;

}

void  TruthUtils::MC_ConvType( Trk::VxCandidate* ConvVx, bool& rec_TrackmatchtoMC, 
			       bool& SignalMC_match, bool& SecondMC_match, int& gambarcode, 
			       const TruthParticleContainer*  mcpartTES, 
			       const Rec::TrackParticleContainer*  trkParticleTES, 
			       const TrackParticleTruthCollection*  trkParTruthTES){
  SignalMC_match = false;
  SecondMC_match = false;
  rec_TrackmatchtoMC = false;
  
  gambarcode = -1;

  std::multimap<long,const TruthParticle*>  mcSignalConvertedPhoton;

  // require.... signal photon's conversion
  std::vector<const TruthParticle*> SelectedMCConversion = getConversionTruth(mcpartTES);
  ATH_MSG_DEBUG("SelectedMCConversion size == " <<  SelectedMCConversion.size());

  // fill signale photon's conversion information to vector
  std::vector<const TruthParticle*>::iterator    imc     = SelectedMCConversion.begin();
  for(; imc!=SelectedMCConversion.end();++imc) {
    mcSignalConvertedPhoton.insert(std::make_pair((*imc)->barcode(), (*imc)));  
  }
  
  // to get track particle of vertex tracks
  std::vector<const Rec::TrackParticle*> recTrackParticle = vertexToTrackParticle( ConvVx );  
  std::vector<const TruthParticle*> mcPartVect;
  std::vector<const Rec::TrackParticle*>::iterator  itrk = recTrackParticle.begin();

  //  check if the track particles match to truth track particle..... 
  for(; itrk!=recTrackParticle.end(); ++itrk) {
    const TruthParticle* tmpMcPart = trkParticleToTruthParticle((*itrk), trkParticleTES, trkParTruthTES, mcpartTES);
    if(tmpMcPart)   mcPartVect.push_back(tmpMcPart);
  }
  
   unsigned int numTrackAtVertex = ConvVx->vxTrackAtVertex()->size();
   if(msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)<<"Tracks at Vertex for REC and MC "<<recTrackParticle.size()<<" "<<mcPartVect.size()<<endreq;
  if(mcPartVect.size()==recTrackParticle.size()){
    rec_TrackmatchtoMC = true;
    //* Find the mother TruthParticle of the TrackParticle(s)
    const TruthParticle* mcMotherParticle = getMotherTruthParticle(mcPartVect);

    if((numTrackAtVertex==2) || (numTrackAtVertex==1)){  //* Double-track or single-track conversion
      if(mcMotherParticle && (mcMotherParticle->pdgId()==PDG::gamma)) {
	int barcode  = mcMotherParticle->barcode();
	if(mcSignalConvertedPhoton.count(barcode)>0) {	   	    
	  SignalMC_match = true;
	  SecondMC_match = false;
	  gambarcode = barcode; 
	  return;
	}
	else {                                            	    
	  SignalMC_match = false;
	  SecondMC_match = true;
	  return;
	}
      }
    }else{
      //std::cout << "Unknown Type!" << std::endl;
      SignalMC_match = false;
      SecondMC_match = false;
      return;
    }
  }  
  
  SignalMC_match = false;
  SecondMC_match = false;

  mcSignalConvertedPhoton.erase( mcSignalConvertedPhoton.begin(),mcSignalConvertedPhoton.end() );

  return;
}


std::pair<double,double> TruthUtils::truthBrem(const HepMC::GenParticle* gp, const McEventCollection* truthCollection){
  std::pair<double,double> bremPhotonEnergy;
  int electron_id = 11;

  int bar = gp->barcode();
  div_t answer = div(bar,100000);
  double bremPhotonEnergySi = 0.;
  double bremPhotonEnergyTotal = 0.;
  double maxEnergyFraction = 0.;
  double maxEnergyFractionLoc = 0.;
  
  double initMom = gp->momentum().e();

  McEventCollection::const_iterator itr = truthCollection->begin(), itre = truthCollection->end();
  for(;itr!=itre;++itr){
    const HepMC::GenEvent* myEvent=(*itr);

    HepMC::GenEvent::vertex_const_iterator Vert = myEvent->vertices_begin();
    HepMC::GenEvent::vertex_const_iterator Vert_end = myEvent->vertices_end();
    for (;Vert!=Vert_end; ++Vert){
      const HepMC::GenVertex* v = *Vert;
   
      HepMC::GenVertex::particles_in_const_iterator it_in_p= v->particles_in_const_begin();
      HepMC::GenVertex::particles_in_const_iterator it_in_p_e= v->particles_in_const_end();
      int nparents = 0; int bar0 = 0; int pdgid0 = 0;
      // loop over in coming particles.
      for(; it_in_p!=it_in_p_e;++it_in_p) {
	const HepMC::GenParticle * parent = *it_in_p;
	bar0 = parent->barcode();
	pdgid0 = parent->pdg_id();
	nparents++;
      }
        
      if(nparents==1 && fabs(pdgid0)==electron_id) {
	div_t answer0 = div(bar0,100000);
	if(answer.rem==answer0.rem) {
	  HepMC::GenVertex::particles_in_const_iterator it_out_p= v->particles_out_const_begin();
	  HepMC::GenVertex::particles_in_const_iterator it_out_p_e= v->particles_out_const_end();
	  // loop over out going particles
	  for(; it_out_p!=it_out_p_e;++it_out_p){
	    const HepMC::GenParticle * child = *it_out_p;
	    if(child->pdg_id()==22) {
	      double energyFraction = child->momentum().e()/initMom;
	      
	      if(v->point3d().perp()<=1000.) bremPhotonEnergyTotal += child->momentum().e();
	      if(v->point3d().perp()<=560.) bremPhotonEnergySi += child->momentum().e();
	      if(v->point3d().perp()<=650. && energyFraction>maxEnergyFraction) {
		maxEnergyFraction = energyFraction;
		maxEnergyFractionLoc = v->point3d().perp();
	      }
	    }
	  }  //Loop over outgoing particles at brem vertex
	}  //It's a brem event related to the electron under investigation
      }  //It's a brem event

    }  ///Loop over event vertices
    bremPhotonEnergy = std::make_pair(bremPhotonEnergySi,bremPhotonEnergyTotal);
  } //Loop over all MC events

   return bremPhotonEnergy;
 }



// ********************************************************************************************//
//                                 Truth Matching Stuff                                        //
//                              (Taken from NTuple dumper)                                     //
// ********************************************************************************************//

//*************************************************************************
const HepMC::GenParticle* TruthUtils::DoTruthMatch(Analysis::Photon* p, const HepMC::GenEvent* myEvent,
						   const Rec::TrackParticleContainer*  trkParticleTES, 
						   const TrackParticleTruthCollection*  trkPartTruthTES,
						   bool doSpecialConversionTruthMatching,
						   bool useTruthClassifier)
{

  const HepMC::GenParticle* truthmatch = 0;
  const Trk::VxCandidate*  convVtx = p->conversion();

  //  ATH_MSG_DEBUG("EMConvertContainerName = " << EMConvertContainerName);

//   if (useBremFit && convVtx) {
//     ATH_MSG_DEBUG("Using refitted values");


//     const EMConvert* emConvert = 0;
//     const Trk::VxCandidate* newConvVtx = 0;
//     for(int id=0;id<p->nDetails();++id){
//       ATH_MSG_DEBUG("Found detail with name: "<< p->detailName(id));
//       if(p->detailName(id)!=EMConvertContainerName) continue;
//       const egDetail* myDetail = p->detail(id);
//       emConvert = dynamic_cast<const EMConvert*>(myDetail);
//       if(emConvert) {
// 	int author = emConvert->vertex_track1_author();
// 	ATH_MSG_DEBUG("Found EMConvert, author " << author);
// 	if((useGSF && author==4)||(!useGSF && author==3)) {
// 	  newConvVtx = emConvert->getVxCandidate();
// 	}
//       }
//     }
//     if (newConvVtx) {
//       convVtx = newConvVtx;
//     } else {
//       ATH_MSG_WARNING("Refit did not work");
//       convVtx = 0;	  
//     }
//   }
  
  ATH_MSG_DEBUG("Entering DoTruthMatch");
  // get the vertex -- if converted photon do truth matching based on the track particles
  if(convVtx){

    std::vector<const Rec::TrackParticle*> trkAtVtx = vertexToTrackParticle(convVtx);
    
    //    ATH_MSG_DEBUG("trkAtVtx size is " << trkAtVtx.size());

    if(trkAtVtx.size()>1 && doSpecialConversionTruthMatching) { 

      // First try doing it the right way for converted photons
      ATH_MSG_DEBUG("Converted photon. Doing special truth matching.");
      truthmatch = FindConversionTruthMatch(convVtx, myEvent, trkParticleTES, trkPartTruthTES, p, useTruthClassifier);

      // If that doesn't work, try the other way.
      if(truthmatch <= 0){
// 	ATH_MSG_DEBUG("Converted photon. Failed special truth matching for double track conversion.  Trying with another method.");
// 	truthmatch = FindNonConversionTruthMatch(p, myEvent);
	truthmatch = 0; // only look at gold-plated matches.
      }

    }
    else if(myEvent){
      ATH_MSG_DEBUG("Converted photon. Non-special way for truth matching");
      truthmatch = FindNonConversionTruthMatch(p, myEvent);
    }
    else{
      truthmatch = 0;
    }
  }
  else if(myEvent){
    ATH_MSG_DEBUG("Unconverted photon. Different way for truth matching");
    truthmatch = FindNonConversionTruthMatch(p, myEvent);
  }
  else{
    truthmatch = 0;
  }

  return truthmatch;
}
//*************************************************************************

//*************************************************************************
// This fills the raw truth information into a few branches - it's so we can check the truth information
// directly, without any bias from an algorithm that matches reconstructed particles to truth
std::vector<const HepMC::GenParticle*>* TruthUtils::FillTruthMap(const HepMC::GenEvent* myEvent,
								float photon_pt_min){

  m_photon_pt_min = photon_pt_min;

  m_truthMap.clear();
  if(!myEvent) return &m_truthMap;

  HepMC::GenEvent::particle_const_iterator pitr;
  for(pitr=myEvent->particles_begin(); pitr!=myEvent->particles_end(); ++pitr){
    
    const HepMC::GenParticle* thePart = (*pitr);    
    int iParticlePDG = thePart->pdg_id();
    if(iParticlePDG!=22 || thePart->status()<1 || thePart->status()>2){
      continue;
    }
    if(thePart->momentum().perp()<m_photon_pt_min){
      continue;
    }

    std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> photres = 
      m_truthClassifier->particleTruthClassifier(thePart);
    MCTruthPartClassifier::ParticleType iTypeOfPht = photres.first;

    ATH_MSG_DEBUG("Type of photon found: " << (int)iTypeOfPht);

    if(iTypeOfPht==MCTruthPartClassifier::UnknownPhoton 
       || iTypeOfPht==MCTruthPartClassifier::IsoPhoton) {
      m_truthMap.push_back(thePart);
    }
  }

  return &m_truthMap;
}

//*************************************************************************
// This fills the raw truth information into a few branches - it's so we can check the truth information
// directly, without any bias from an algorithm that matches reconstructed particles to truth
std::vector<const HepMC::GenParticle*>* TruthUtils::FillTruthMapEl(const HepMC::GenEvent* myEvent,
								   float electron_pt_min){
  
  m_electron_pt_min = electron_pt_min;

  m_truthMap.clear();
  if(!myEvent) return &m_truthMap;
  
  HepMC::GenEvent::particle_const_iterator pitr;
  for(pitr=myEvent->particles_begin(); pitr!=myEvent->particles_end(); ++pitr){
    
    const HepMC::GenParticle* thePart = (*pitr);    
    int iParticlePDG = thePart->pdg_id();
    if((!(iParticlePDG==11 || iParticlePDG == -11)) || thePart->status()<1 || thePart->status()>2){
      continue;
    }
    if(thePart->momentum().perp()<m_electron_pt_min){
      continue;
    }

    std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> photres = 
      m_truthClassifier->particleTruthClassifier(thePart);
    MCTruthPartClassifier::ParticleType iTypeOfPht = photres.first;
    
    if(iTypeOfPht==MCTruthPartClassifier::UnknownElectron
       || iTypeOfPht==MCTruthPartClassifier::IsoElectron){
      m_truthMap.push_back(thePart);
    }
  }
  
  return &m_truthMap;
}


//*************************************************************************
const std::vector<const HepMC::GenParticle*> TruthUtils::getTruthTracks(const Trk::VxCandidate*  convVtx) const
{

  //get the vector of corresponding track particles at vertex
  std::vector<const Rec::TrackParticle*> trkAtVtx = vertexToTrackParticle(convVtx);

  //get vector of linked GenParticles to them
  std::vector<const HepMC::GenParticle*> vecGenPart;

  // for( int counter =0; counter < int(trkAtVtx.size()); ++counter) {
  //   const Rec::TrackParticle* trkP = trkAtVtx[counter];
  //   const HepMC::GenParticle* genP = m_truthClassifier->getGenPart(trkP);
  //   if(genP) {
  //     const int pid = genP->pdg_id();
  //     if (pid == 11 || pid == -11) {
  // 	vecGenPart.push_back(genP);
  //     }
  //   }
  // }
  return vecGenPart;
}

//*************************************************************************
const HepMC::GenParticle* TruthUtils::FindConversionTruthMatch(const Trk::VxCandidate*  convVtx, const HepMC::GenEvent* myEvent,
							       const Rec::TrackParticleContainer*  trkParticleTES, 
							       const TrackParticleTruthCollection*  trkPartTruthTES, 
							       Analysis::Photon* p, bool useTruthClassifier){

  //get the vector of corresponding track particles at vertex
  std::vector<const Rec::TrackParticle*> trkAtVtx = vertexToTrackParticle(convVtx);

  //get vector of linked GenParticles to them
  std::vector<const HepMC::GenParticle*> vecGenPart;

  int motherPDG=-1;
  int motherBar=-1;

  // Here, we need to make a choice between using Olegs truthClassifier, or our older version
//   if(useTruthClassifier){

//     std::vector<std::pair<int,int>* > vecElecPDGBar;

//     for( int counter =0; counter < int(trkAtVtx.size()); ++counter) {
//       const Rec::TrackParticle* trkP = trkAtVtx[counter];
//       //const HepMC::GenParticle* genP = m_truthTool->trkParticleToGenParticle(trkP, trkParticleTES, trkPartTruthTES);
//       const HepMC::GenParticle* genP = m_truthClassifier->getGenPart(trkP);
//       if(!genP) continue;
//       const int pid = genP->pdg_id();
//       if (pid != 11 && pid != -11) continue;
      
// //       std::pair<egammaElecTruthClassifier::TrackPartType,
// // 	egammaElecTruthClassifier::ElectronOrigin>  elecres = m_truthClassifier->electronTrackClassifier(trkP);
//       std::pair<unsigned int, unsigned int> electres = m_truthClassifier->trackClassifier(trkP);
//       int elecPDG = m_truthClassifier->getMotherPDG();
//       int elecBar = m_truthClassifier->getMotherBarcode();
//       vecGenPart.push_back(genP);
//       vecElecPDGBar.push_back(new std::pair<int,int>(elecPDG,elecBar));
//     }

//     // now, the vecElecPDGBar should consist of between 0 and 2 entries that are all the same.  Check to see if this is the case,
//     // and if so, no need to hold onto the vector...  just take the mother PDG.
//     for(std::vector<std::pair<int,int>* >::iterator elecPDG_iter=vecElecPDGBar.begin(); elecPDG_iter != vecElecPDGBar.end(); ++elecPDG_iter){
//       if(elecPDG_iter==vecElecPDGBar.begin()){
// 	motherPDG = (*elecPDG_iter)->first;
// 	motherBar = (*elecPDG_iter)->second;
//       }
//       else if(motherPDG != (*elecPDG_iter)->first){
// 	msg(MSG::WARNING) << "The PDG ID's returned for the mother of this conversion do not match!"
// 			  << "  first is " << motherPDG << " and this one is " << (*elecPDG_iter)->first << endreq;
// 	return 0; // consider it to be a fake conversion
//       }
//     }
//     if(motherPDG==-1){
//       msg(MSG::WARNING) << "Got no PDG IDs back for the mother of this conversion.  Weird." << endreq;
//     }
//   }
//   else
    {

    for( int counter =0; counter < int(trkAtVtx.size()); ++counter) {
      const Rec::TrackParticle* trkP = trkAtVtx[counter];
      const HepMC::GenParticle* genP = trkParticleToGenParticle(trkP, trkParticleTES, trkPartTruthTES);
      if(!genP)	continue;
      vecGenPart.push_back(genP);
    }

    const HepMC::GenParticle* motherPart = getMotherGenParticle(vecGenPart);
    if(motherPart){
      motherPDG = motherPart->pdg_id();
      motherBar = motherPart->barcode();
    }
    else{
      motherPDG = -1;
      motherBar = -1;
    }
  }

  
  /** at this point, we should have our first choice of a best match to the mother photon of these two tracks. **/


  // if we don't have a genparticle for every track, then return -1, and let
  // some other routine try.
  if(vecGenPart.size() != trkAtVtx.size()) {
    return 0;
  } 


  // this will give us the "best" match to the genparticle, using the tracks.
  //const HepMC::GenParticle* motherPart = getMotherGenParticle(vecGenPart);
  const HepMC::GenParticle* motherPart = 0;
  HepMC::GenEvent::particle_const_iterator pitr;
  for(pitr=myEvent->particles_begin(); pitr!=myEvent->particles_end(); ++pitr){
    if((*pitr)->barcode() == motherBar) {motherPart = (*pitr); break;}
  }


  // that's all well and good, but in busy events, it could be that the cluster associated
  // with this photon has the right eta/phi info compared with truth, but the tracks
  // themselves come from some intermediate particle in the middle of the mess.  In that
  // case, we call it a conversion still, but we want to match it with the "interesting"
  // photon.  Let's say that we're allowed to do this if there's an "interesting"
  // photon inside of a cone of .1.  So, use that motherPart to search for other photons
  // that lie inside of a cone of .1, and fill a local vector with those GP's.
  std::vector<const HepMC::GenParticle*>* nearby_GPs(0);
  if(motherPart)
    nearby_GPs = getNearbyGenParticles(myEvent, motherPart, .1);
  else
    nearby_GPs = getNearbyGenParticles(myEvent, p, .1);

  const HepMC::GenParticle* bestmatch(0);
  std::vector<const HepMC::GenParticle*>::iterator gp_iter;
  for(gp_iter = nearby_GPs->begin(); gp_iter != nearby_GPs->end(); ++gp_iter){
    if(gp_iter == nearby_GPs->begin()) 
      bestmatch = *gp_iter;
    else if(bestmatch->momentum().perp() < (*gp_iter)->momentum().perp())
      bestmatch = *gp_iter;
  }
  delete nearby_GPs;
  bool swapped = false;


  // if the photon we matched to isn't very good (i.e. not a photon), 
  // then we should try looking nearby too.
  if(motherPDG!=22){
    ATH_MSG_DEBUG("Fake converted photon. No truth mother matching.  Try looking nearby."); 
    //std::cout << "barcode = " << motherBar << " and PDG ID = " << motherPDG << std::endl;

    // check to see if any of the nearby particles is a good match...  take the one with
    // the highest p_t, I guess.
    if(bestmatch!=0){
      motherPart = bestmatch;
      motherPDG = motherPart->pdg_id();
      motherBar = motherPart->barcode();
      swapped = true;
    }
    else{
      return 0;
    }
  }

  
  // now, at this point, we know that "motherPart" is a photon, that's within .1 of the
  // best match photon that we got from a classifer.


  // try to see if this photon is in the truth map.
  ATH_MSG_DEBUG("Truth matching is good - retrieving mother particle from truth map.");
  std::vector<const HepMC::GenParticle*>::const_iterator imap;
  for(imap = m_truthMap.begin(); imap != m_truthMap.end(); ++imap){
    if((*imap)->barcode() == motherBar) break;
  }

  // if we've already looked through the vector and selected out the photon with the highest p_t,
  // and it's still not in the truth map, then we're done.
  if(imap == m_truthMap.end() && swapped == true){
    return 0;
  }
  
  // if we haven't tried swapping yet, then try swapping and see if we get something then.
  else if(imap == m_truthMap.end()){
    if(bestmatch!=0){
      motherPart = bestmatch;
      motherPDG = motherPart->pdg_id();
      motherBar = motherPart->barcode();
      swapped = true;
    }
    for(imap = m_truthMap.begin(); imap != m_truthMap.end(); ++imap){
      if((*imap)->barcode() == motherBar) break;
    }

    if(imap == m_truthMap.end()){
      return 0;
    }
  }

  // not sure why this check is here, but I guess it shouldn't hurt anything
  //HepMC::GenEvent::particle_const_iterator pitr;
  //for(pitr=myEvent->particles_begin(); pitr!=myEvent->particles_end(); ++pitr){
  //  if((*pitr)->barcode() == motherBar) break;
  //}
	     
  //if(pitr == myEvent->particles_end()) truthmatch = 0;

  
  return motherPart;
}
//*************************************************************************


//*************************************************************************
const HepMC::GenParticle* TruthUtils::FindNonConversionTruthMatch(Analysis::Photon* p, const HepMC::GenEvent* myEvent){

  ATH_MSG_DEBUG("in FindNonConversionTruthMatch("<< p << ", " << myEvent << ")");
  // for now, just do simple eta/phi matching.  All of the truth information is saved in the NTuple anyway, so
  // all this represents is a best guess.
  int nMatchedIsoPhotons = 0;
  float bestpdiff = 6; float bestMom = 0.;
  const HepMC::GenParticle* bestmatch = 0;

  HepMC::GenEvent::particle_const_iterator pitr;
  for(pitr=myEvent->particles_begin(); pitr!=myEvent->particles_end(); ++pitr){
    
    const HepMC::GenParticle* thePart = (*pitr);    
    int iParticlePDG = thePart->pdg_id();
    if(iParticlePDG!=22 || thePart->status()<1 || thePart->status()>2) continue;
    if(thePart->momentum().perp()<m_photon_pt_min) continue;

    std::pair<MCTruthPartClassifier::ParticleType, MCTruthPartClassifier::ParticleOrigin> photres = 
      m_truthClassifier->particleTruthClassifier(thePart);
    MCTruthPartClassifier::ParticleType iTypeOfPht = photres.first;

    if(!(iTypeOfPht==MCTruthPartClassifier::UnknownPhoton 
	 || iTypeOfPht==MCTruthPartClassifier::IsoPhoton)) {
      continue;
    }

    double etadiff = thePart->momentum().pseudoRapidity() - p->eta();
    double phidiff = P4Helpers::deltaPhi(thePart->momentum().phi(), p->phi());
    if(hypot(etadiff, phidiff) > m_truth_cone_max) continue;

    //Best match. An isolated photon always wins. If more than one isolated photons, the smallest cone and largest pT wins.
    //If not matched to any isolated photons, then the matched photon with the smallest cone and largest pT wins.
    if(photres.first == MCTruthPartClassifier::IsoPhoton){
      if(sqrt(etadiff*etadiff + phidiff*phidiff) < bestpdiff && thePart->momentum().perp()> bestMom){
	bestpdiff = sqrt(etadiff*etadiff + phidiff*phidiff);
	bestMom   = thePart->momentum().perp();
	bestmatch = thePart;
	nMatchedIsoPhotons++;
      }
    }else if(photres.first!=MCTruthPartClassifier::IsoPhoton && nMatchedIsoPhotons==0){
      if(sqrt(etadiff*etadiff + phidiff*phidiff) < bestpdiff && thePart->momentum().perp()> bestMom){
	bestpdiff = sqrt(etadiff*etadiff + phidiff*phidiff);
	bestMom   = thePart->momentum().perp();
	bestmatch = thePart;
      }
    }
  }
  
  if(bestmatch!=0){
    std::vector<const HepMC::GenParticle*>::const_iterator imap;
    int vector_position=0;
    for(imap = m_truthMap.begin(); imap != m_truthMap.end(); ++imap, vector_position++){
      if((*imap)->barcode() == bestmatch->barcode()) break;
    }

    if(bestpdiff>1 || imap == m_truthMap.end()){
      bestmatch = 0;
    }
  }

  return bestmatch;

}
//************************************************************************
