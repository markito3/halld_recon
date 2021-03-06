// $Id$
//
//    File: DTRDDigiHit.h
//

#ifndef _DTRDDigiHit_
#define _DTRDDigiHit_

#include <JANA/JObject.h>
#include <JANA/JFactory.h>

class DTRDDigiHit:public jana::JObject{
	public:
		JOBJECT_PUBLIC(DTRDDigiHit);
		
		uint32_t plane;
		uint32_t strip;
		uint32_t pulse_peak;           ///< identified pulse peak as returned by FPGA algorithm
		uint32_t pulse_time;           ///< identified pulse time as returned by FPGA algorithm
		uint32_t pedestal;             ///< pedestal info used by FPGA (if any)
		uint32_t QF;                   ///< Quality Factor from FPGA algorithms
		uint32_t nsamples_integral;    ///< number of samples used in integral 
		uint32_t nsamples_pedestal;    ///< number of samples used in pedestal
		
		// This method is used primarily for pretty printing
		// the second argument to AddString is printf style format
		void toStrings(vector<pair<string,string> > &items)const{
			AddString(items, "plane", "%d", plane);
			AddString(items, "strip", "%d", strip);
			AddString(items, "pulse_peak", "%d", pulse_peak);
			AddString(items, "pulse_time", "%d", pulse_time);
			AddString(items, "pedestal", "%d", pedestal);
			AddString(items, "QF", "%d", QF);
			AddString(items, "nsamples_integral", "%d", nsamples_integral);
			AddString(items, "nsamples_pedestal", "%d", nsamples_pedestal);
		}		
};

#endif // _DTRDDigiHit_

