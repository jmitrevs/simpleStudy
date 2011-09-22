/**  @class UserData_Photons
	Define your access IDs for your InsituObject. In this case we have 
	only two user specified quantaty which we call Distance and Isolation.
	
	Here an reminder of the InsituObject. The InsituObject is a simple implementation 
	of the I4Momentum interface and hence something like a TLorentzVector. It can be 
	read/writen from/to Storegate and from there to POOL-ROOT-Files, i.e. DPDs.
	
	There are two advantages of the InsituObject:
	- It is rather small to store (which is important for our Algorithms)
	- And the user can specify which quantaties he needs for the performance
	  evaluation. Automatically eta, phi, pt, charge and mass are provided.
	  
	  If the user wants to store his own quantaty, as in this case, he just access them via
	  get and set as demonstrated in the code below:
	  
	     ...
	     InsituObject insituObject(20000., 1.0, -3.2, 0.0, 1);		// Pt, Eta, Phi, Mass, Charge
	     insituObject.set(UserData_Photons::Distance, 0.2);
	     ...
	     cout<<"Distance: "<<insituObject.get(UserData_Photons::Distance)<<endl;
	     ...

	@author  Matthias Schott <mschott@cern.ch>
*/


#ifndef UserData_Photons_h
#define UserData_Photons_h

class UserData_Photons
{
public:
  enum IDs
    {
      mcE,
      clusterE,
      energy,
      momentum,
      passTight,
      isConversion,
      numTracks,		// only if conversion
      chiSq,			// only if 2-track
      energyTruthTrack0,		// only if conversion
      energyTruthTrack1,		// only if 2-track
      bremSi0,		// only if conversion
      bremSi1,		// only if 2-track
      brem0,		// only if conversion
      brem1,		// only if 2-track
      numSi0,		// only if conversion
      numSi1,		// only if 2-track
      refitSucceeded,	// only for convsersions
      EMConvertedPhoton_Eclus,
      EMConvertedPhoton_Eclus_Error,
      momentumError,		// only for 2-track

      // now the individual track fits and errors
      qOverP0_truth,
      qOverP0_reco,
      qOverP0_error,

      qOverP1_truth,
      qOverP1_reco,
      qOverP1_error,

      pdg_id0,
      pdg_id1,

      etaTruthTrack0,
      etaTruthTrack1,
      etaTrack0,
      etaTrack1,
      etaTracks,

      etaTrack0Error,
      etaTrack1Error,
      etaTracksError,

      etaTruth,
      etaError,

      // three components of vertex error (not nec. x,y,z)
      vertexError1,
      vertexError2,
      vertexError3,

      vertexPos1,
      vertexPos2,
      vertexPos3,

      vertexTruth1,
      vertexTruth2,
      vertexTruth3,

      // the conversion radius
      rConv

    };
};

#endif
