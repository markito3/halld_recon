// $Id$
//
//    File: JEventProcessor_F250_mode8_pedestal.h
// Created: Mon Apr 13 14:49:24 EDT 2015
// Creator: dalton (on Linux gluon02.jlab.org 2.6.32-358.18.1.el6.x86_64 x86_64)
//

#ifndef _JEventProcessor_F250_mode8_pedestal_
#define _JEventProcessor_F250_mode8_pedestal_

#include <JANA/JEventProcessor.h>
#include "TH1I.h"
#include "TProfile2D.h"

class JEventProcessor_F250_mode8_pedestal:public jana::JEventProcessor{
	public:
		JEventProcessor_F250_mode8_pedestal();
		~JEventProcessor_F250_mode8_pedestal();
		const char* className(void){return "JEventProcessor_F250_mode8_pedestal";}

		static const int highcratenum=100;
		static const int highslotnum=21;
		static const int highchannum=16;
		// root hist pointers
		TH1I *samples_meansub[highcratenum][highslotnum][highchannum];  /// find the mean in window and them subtract from all samples
		TH1I *samplesum_pedsub[highcratenum][highslotnum][highchannum];  /// measure the pedestal, subtract and then sum the samples
		TH1I *samplesum[highcratenum][highslotnum][highchannum];         /// just sum the samples
		//TH1I *samples_mean[highcratenum][highslotnum][highchannum];
		TProfile2D *mean_crates[highcratenum];
		TH2D *samples_rms_summary[highcratenum];
		TH2D *samplesum_pedsub_rms_summary[highcratenum];
		TH2D *samplesum_rms_summary[highcratenum];


	private:
		uint32_t NSA_NSB;

		jerror_t init(void);						///< Called once at program start.
		jerror_t brun(jana::JEventLoop *eventLoop, int runnumber);	///< Called everytime a new run number is detected.
		jerror_t evnt(jana::JEventLoop *eventLoop, int eventnumber);	///< Called every event.
		jerror_t erun(void);						///< Called everytime run number changes, provided brun has been called.
		jerror_t fini(void);						///< Called after last event of last event source has been processed.
};

#endif // _JEventProcessor_F250_mode8_pedestal_

