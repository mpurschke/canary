
#include <iostream>
#include <pmonitor/pmonitor.h>
#include "xdrs.h"

#include <TH1.h>
#include <TH2.h>

int init_done = 0;

using namespace std;

//TH1F *h1; 
//TH2F *h2; 


TH1F *trace[4];

TH2F *x2;


int packetid=1001;

int pinit()
{

  if (init_done) return 1;
  init_done = 1;

  char name[512];
  char title[512];

 int i;
  for ( i = 0; i < 4; i++)
    {
      sprintf ( name, "ch%d", i);
      sprintf ( title , "waveform channel %d", i);
      trace[i] = new TH1F( name, title, 1024, -0.5, 1023.5);
      trace[i]->GetXaxis()->SetTitle("Sample Number");
      trace[i]->GetYaxis()->SetTitle("Amplitude [mv]");
      trace[i]->SetLineWidth(2);
      //trace[i]->SetOptStat(0);

    }
  trace[0]->SetLineColor(kViolet-4);
  trace[1]->SetLineColor(kBlue);
  trace[2]->SetLineColor(kPink+10);
  trace[3]->SetLineColor(kCyan);
  
  x2 = new TH2F ( "x2", "Scope persistency plot",1024, -0.5, 1023.5 , 128, -700, 350);
  x2->SetStats(0);
  x2->SetXTitle("Channel");
  x2->SetYTitle("Signal");
  return 0;
  
}

int old_runnumber = -1;  // impossible value

int process_event (Event * e)
{

  int sample,channel;

  // see if we have either the begin-run event or get a new run number
  if ( e->getEvtType() == BEGRUNEVENT || e->getRunNumber() != old_runnumber)
    {
      old_runnumber = e->getRunNumber();
      x2->Reset();
    }


  
  Packet *p = e->getPacket(packetid);
  if (p)
    {

      char name[512];
      char title[512];
      
      for ( channel = 0; channel < 4 ; channel++)
	{
	  sprintf ( title , "waveform channel %d  Run %d  Evt %d", channel, e->getRunNumber(), e->getEvtSequence());
	  trace[channel]->SetTitle(title);
	  trace[channel]->Reset();
	}

      
      for ( channel = 0; channel < 4 ; channel++)
	{
	  for (sample = 0; sample < 1024; sample++)
	    {
	      trace[channel]->Fill ( sample,  p->rValue(sample,channel) );
	    }
	}

      for (sample = 0; sample <1024; sample++)
	{
	  // x2->Fill ( p->rValue(sample,4),  p->rValue(sample,1));
	  x2->Fill ( sample,  p->rValue(sample,1));
	}
      delete p;

    }
  //  usleep(2000);
  return 0;
}

