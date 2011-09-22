/**  @class UserData_Electrons
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
	     insituObject.set(UserData_Electrons::Distance, 0.2);
	     ...
	     cout<<"Distance: "<<insituObject.get(UserData_Electrons::Distance)<<endl;
	     ...

	@author  Matthias Schott <mschott@cern.ch>
*/


#ifndef UserData_Electrons_h
#define UserData_Electrons_h

class UserData_Electrons
{
public:
  enum IDs
    {
      mcE,
      clusterE,
      energy,
      momentum,
      passTight,
      numTracks,	
      chiSq,		
      energyTruthTrack0,
      bremSi0,		
      brem0,		
      numSi0,		
      refitSucceeded,	
      EMConvertedPhoton_Eclus,
      EMConvertedPhoton_Eclus_Error,
      momentumError,	

      // now the individual track fits and errors
      qOverP0_truth,
      qOverP0_reco,
      qOverP0_error,

      pdg_id0,

      etaTruthTrack0,
      etaTrack0,

      etaTrack0Error
    };
};

#endif
