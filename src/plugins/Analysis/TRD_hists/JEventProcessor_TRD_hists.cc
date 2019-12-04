#include <stdint.h>
#include <vector>

#include "JEventProcessor_TRD_hists.h"
#include <JANA/JApplication.h>

using namespace std;
using namespace jana;

#include <DANA/DApplication.h>

#include <TRIGGER/DL1Trigger.h>
#include <TRD/DTRDDigiHit.h>
#include <TRD/DTRDHit.h>
#include <TRD/DTRDStripCluster.h>
#include <TRD/DTRDPoint.h>

#include <DAQ/DGEMSRSWindowRawData.h>
#include <TRD/DGEMDigiWindowRawData.h>
#include <TRD/DGEMHit.h>
#include <TRD/DGEMStripCluster.h>
#include <TRD/DGEMPoint.h>

#include <TDirectory.h>
#include <TH1.h>
#include <TH2.h>
#include <TProfile.h>

// root hist pointers
const int NTRDplanes = 4;
const int NTRDstrips = 240;
const int NTRDwires = 24;
const int NGEMplanes = 10;
const int NGEMstrips = 256;
const int NAPVchannels = 128;
const int NGEMsamples = 21;

static TH1I *trd_num_events;

static TH2I *hWireTRDPoint_TrackX, *hWireTRDPoint_TrackY, *hGEMTRDHit_TrackX;
static TH2I *hWireTRDPoint_DeltaXY, *hGEMTRDHit_DeltaX_T;
static TH1I *hWireTRDPoint_Time;

static TH2I *hWire_GEMTRDXstrip, *hWire_GEMTRDX_DeltaT;

//----------------------------------------------------------------------------------


// Routine used to create our JEventProcessor
extern "C"{
    void InitPlugin(JApplication *app){
        InitJANAPlugin(app);
        app->AddProcessor(new JEventProcessor_TRD_hists());
    }
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_hists::JEventProcessor_TRD_hists() {
}


//----------------------------------------------------------------------------------


JEventProcessor_TRD_hists::~JEventProcessor_TRD_hists() {
}


//----------------------------------------------------------------------------------

jerror_t JEventProcessor_TRD_hists::init(void) {

    // create root folder for TRD and cd to it, store main dir
    TDirectory *mainDir = gDirectory;
    TDirectory *trdDir = gDirectory->mkdir("TRD_hists");
    trdDir->cd();
    // book hists
    trd_num_events = new TH1I("trd_hists_num_events","TRD number of events",1,0.5,1.5);

    // digihit-level hists
    trdDir->cd();
    gDirectory->mkdir("StraightTracks")->cd();

    hWireTRDPoint_TrackX = new TH2I("WireTRDPoint_TrackX","; Wire TRD X; Extrapolated track X",200,25,55,200,25,55);
    hWireTRDPoint_TrackY = new TH2I("WireTRDPoint_TrackY","; Wire TRD Y; Extrapolated track Y",200,-85,-65,200,-85,-65);
    hGEMTRDHit_TrackX = new TH2I("GEMTRDHit_TrackX","; GEM TRD X; Extrapolated track X",200,25,55,200,25,55);
    hWireTRDPoint_DeltaXY = new TH2I("WireTRDPoint_DeltaXY","; #Delta X; #Delta Y",100,-5,5,100,-5,5);
    hWireTRDPoint_Time = new TH1I("WireTRDPoint_Time","; hit time",1000,0,1000);
    hGEMTRDHit_DeltaX_T = new TH2I("GEMTRDHit_DeltaX_T","; #Delta X; hit time",1000,-20,20,1000,0,1000);

    // GEM-Wire TRD correlatioin
    hWire_GEMTRDXstrip = new TH2I("Wire_GEMTRDXstrip", "GEM TRD X strip vs TRD wire # ; TRD wire # ; GEM TRD X strip #", NTRDwires, -0.5, -0.5+NTRDwires, NGEMstrips, -0.5, -0.5+NGEMstrips);
    hWire_GEMTRDX_DeltaT = new TH2I("Wire_GEMTRDX_DeltaT", "GEM TRD X Amplitude vs #Delta t ; #Delta t (ns) ; GEM TRD X Amplitude", 500, -500, 500, 100, 0, 10000);
    
    // back to main dir
    mainDir->cd();

    return NOERROR;
}

//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::brun(JEventLoop *eventLoop, int32_t runnumber) {
    // This is called whenever the run number changes

    // special conditions for different geometries
    if(runnumber < 70000) wirePlaneOffset = 0;
    else wirePlaneOffset = 4;

    DApplication* dapp = dynamic_cast<DApplication*>(eventLoop->GetJApplication());
    const DGeometry *geom = dapp->GetDGeometry(runnumber);
    vector<double> z_trd;
    geom->GetTRDZ(z_trd);

    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::evnt(JEventLoop *eventLoop, uint64_t eventnumber) {
    // This is called for every event. Use of common resources like writing
    // to a file or filling a histogram should be mutex protected. Using
    // loop-Get(...) to get reconstructed objects (and thereby activating the
    // reconstruction algorithm) should be done outside of any mutex lock
    // since multiple threads may call this method at the same time.

/*
    // Get trigger words and filter on PS trigger (if it exists?)
    const DL1Trigger *trig_words = nullptr;
    uint32_t trig_mask, fp_trig_mask;
    try {
        eventLoop->GetSingle(trig_words);
    } catch(...) {};
    if (trig_words != nullptr) {
        trig_mask = trig_words->trig_mask;
        fp_trig_mask = trig_words->fp_trig_mask;
    }
    else {
        trig_mask = 0;
        fp_trig_mask = 0;
    }
    int trig_bits = fp_trig_mask > 0 ? 10 + fp_trig_mask:trig_mask;
    // Select PS-triggered events
    if (trig_bits != 8) {
        return NOERROR;
    }
*/

    vector<const DTRDDigiHit*> digihits;
    eventLoop->Get(digihits);
    vector<const DTRDHit*> hits;
    eventLoop->Get(hits);
    vector<const DTRDStripCluster*> clusters;
    eventLoop->Get(clusters);
    vector<const DTRDPoint*> points;
    eventLoop->Get(points);

    vector<const DGEMDigiWindowRawData*> windowrawdata;
    eventLoop->Get(windowrawdata);
    vector<const DGEMHit*> gem_hits;
    eventLoop->Get(gem_hits);
    vector<const DGEMStripCluster*> gem_clusters;
    eventLoop->Get(gem_clusters);
    vector<const DGEMPoint*> gem_points;
    eventLoop->Get(gem_points);

    vector<const DTrackWireBased*> straight_tracks;
    eventLoop->Get(straight_tracks, "StraightLine");

    // FILL HISTOGRAMS
    // Since we are filling histograms local to this plugin, it will not interfere with other ROOT operations: can use plugin-wide ROOT fill lock
    japp->RootFillLock(this); //ACQUIRE ROOT FILL LOCK

    ///////////////////////////
    // TRD DigiHits and Hits //
    ///////////////////////////

    if (digihits.size() > 0) trd_num_events->Fill(1);

    // check if have good extrapolated track with TRD wire point
    bool goodTrack = false;

    for (const auto& straight_track : straight_tracks) {
	    vector<DTrackFitter::Extrapolation_t>extrapolations=straight_track->extrapolations.at(SYS_TRD);
	    //cout<<"found straight track with "<<extrapolations.size()<<" extrapolations"<<endl;

	    for (const auto& extrapolation : extrapolations) {
	
		    // correlate wire TRD with extrapolated tracks
		    for (const auto& point : points) {

			    if(point->detector == 0 && fabs(extrapolation.position.Z() - 548.8) < 5.) { 

				    hWireTRDPoint_TrackX->Fill(point->x, extrapolation.position.X());
				    hWireTRDPoint_TrackY->Fill(point->y, extrapolation.position.Y());
				    
				    double locDeltaX = point->x - extrapolation.position.X();
				    double locDeltaY = point->y - extrapolation.position.Y();
				    hWireTRDPoint_DeltaXY->Fill(locDeltaX, locDeltaY);

				    if(fabs(locDeltaX) < 5. && fabs(locDeltaY) < 5.) {
					    hWireTRDPoint_Time->Fill(point->time);
					    goodTrack = true;
				    }
			    }
		    }

		    // correlate GEM TRD with extrapolated tracks
		    for (const auto& hit : hits) {
			    if(hit->plane == 6 && fabs(extrapolation.position.Z() - 570.7) < 5.) {

				    // only look at tracks with good wire hit
				    if(!goodTrack) continue;
				    
				    double locStripX = 32.6 + hit->strip*0.04;
				    hGEMTRDHit_TrackX->Fill(locStripX, extrapolation.position.X());
				    
				    double locDeltaX = locStripX - extrapolation.position.X();
				    hGEMTRDHit_DeltaX_T->Fill(locDeltaX, hit->t);
			    }
		    }
	    }
    }

    if(goodTrack) {
	    for (const auto& hit : hits) {
		    if(hit->plane != 0 && hit->plane != 4) continue; // only Wire TRD
		    int wire = hit->strip;

		    // GEM TRD hits
		    for (const auto& gemtrd_hit : hits) {
			    if(gemtrd_hit->plane != 2 && gemtrd_hit->plane != 6) continue; 
			    double locDeltaT = gemtrd_hit->t - hit->t;
			    
			    hWire_GEMTRDX_DeltaT->Fill(locDeltaT, gemtrd_hit->pulse_height);
			   
			    //if(fabs(locDeltaT) < 20.)
			    hWire_GEMTRDXstrip->Fill(wire, gemtrd_hit->strip);
		    }
	    }
    }

    japp->RootFillUnLock(this); //RELEASE ROOT FILL LOCK

    return NOERROR;
}
//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::erun(void) {
    // This is called whenever the run number changes, before it is
    // changed to give you a chance to clean up before processing
    // events from the next run number.
    return NOERROR;
}


//----------------------------------------------------------------------------------


jerror_t JEventProcessor_TRD_hists::fini(void) {
    // Called before program exit after event processing is finished.
    return NOERROR;
}

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------