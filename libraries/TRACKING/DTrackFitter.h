// $Id$
//
//    File: DTrackFitter.h
// Created: Mon Sep  1 10:30:04 EDT 2008
// Creator: davidl (on Darwin Amelia.local 8.11.1 i386)
//

#ifndef _DTrackFitter_
#define _DTrackFitter_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>
#include <JANA/JEventLoop.h>

#include <DANA/DApplication.h>
#include <PID/DKinematicData.h>
#include <HDGEOMETRY/DMagneticFieldMap.h>
#include <CDC/DCDCTrackHit.h>
#include <FDC/DFDCPseudo.h>

//////////////////////////////////////////////////////////////////////////////////
/// The DTrackFitter class is a base class for different charged track
/// fitting algorithms. It does not actually fit the track itself, but
/// provides the interface and some common support features most algorthims
/// will need to implement.
///
/// The reason this is needed (instead of just using the mechanism already
/// built into JANA) is due to the nature of charged track fitting.
/// Specifically, tracks are usually fit first to the wire positions
/// and then to the drift times. The algorithm for both is (at least
/// usually) the same. However, we want to separate the wire-based and
/// time-based fitting into 2 distinct stages allowing easy access to the 
/// wire-based fits.
///
/// There were a few options on how to handle this within the JANA framework
/// but it was decided passing DTrackFitter objects through the framework
/// was the best way to address it. Sub-classes of DTrackFitter will
/// implement the actual algorithms, but JANA will only see these
/// objects as pointers to the DTrackFitter base class. Only one
/// DTrackFitterXXX object will exist for each thread (i.e. each JEventLoop).
/// As such, the state of that object will likely be overwritten
/// many times in a single event and it's internal data never used
/// by anything outside of the TRACKING package. Also, the factories that
/// produce the DTrackFitterXXX objects will make them as persistent
/// and will turn off the the WRITE_TO_OUTPUT bit by default.
//////////////////////////////////////////////////////////////////////////////////

class DTrackFitter:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DTrackFitter);
		
		enum fit_type_t{
			kWireBased,
			kTimeBased
		};
		
		enum fit_status_t{
			kFitNotDone,
			kFitSuccess,
			kFitFailed
		};
		
		// Constructor and destructor
		DTrackFitter(JEventLoop *loop);	// require JEventLoop in constructor
		virtual ~DTrackFitter();
		
		void Reset(void);
		
		// Hit accessor methods
		void AddHit(const DCDCTrackHit* cdchit);
		void AddHits(vector<const DCDCTrackHit*> cdchits);
		void AddHit(const DFDCPseudo* fdchit);
		void AddHits(vector<const DFDCPseudo*> fdchits);
		const vector<const DCDCTrackHit*>& GetCDCInputHits(void){return cdchits;}
		const vector<const DFDCPseudo*>&   GetFDCInputHits(void){return fdchits;}
		const vector<const DCDCTrackHit*>& GetCDCFitHits(void){return cdchits_used_in_fit;}
		const vector<const DFDCPseudo*>&   GetFDCFitHits(void){return fdchits_used_in_fit;}
		
		// Fit parameter accessor methods
		DKinematicData& GetInputParameters(void){return input_params;}
		DKinematicData& GetFitParameters(void){return fit_params;}
		fit_type_t GetFitType(void){return fit_type;}
		void SetFitType(fit_type_t type){fit_type=type;}
		
		// Wrappers
		fit_status_t FitTrack(const DVector3 &pos, const DVector3 &mom, double q, double mass);
		fit_status_t FitTrack(const DKinematicData &starting_params);
		
		//---- The following needs to be supplied by the subclass ----
		virtual fit_status_t FitTrack(void)=0;

	protected:

		// The following should be used as inputs by FitTrack(void)
		vector<const DCDCTrackHit*> cdchits;	//< Hits in the CDC
		vector<const DFDCPseudo*> fdchits;		//< Hits in the FDC
		DKinematicData input_params;				//< Starting parameters for the fit
		fit_type_t fit_type;							//< kWireBased or kTimeBased
		const DMagneticFieldMap *bfield;			//< Magnetic field map for current event (acquired through loop)
		JEventLoop *loop;								//< Pointer to JEventLoop object handling the current event

		// The following should be set as outputs by FitTrack(void)
		DKinematicData fit_params;									//< Results of last fit
		double chisq;													//< Chi-sq of final track fit (not the chisq/dof!)
		int Ndof;														//< Number of degrees of freedom for final fit parameters
		fit_status_t fit_status;									//< Status of values in fit_params (kFitSuccess, kFitFailed, ...)
		vector<const DCDCTrackHit*> cdchits_used_in_fit;	//< The CDC hits actually used in the fit
		vector<const DFDCPseudo*> fdchits_used_in_fit;		//< The FDC hits actually used in the fit

	private:
		
		// Prohibit default constructor
		DTrackFitter();
		
};

#endif // _DTrackFitter_

