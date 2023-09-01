
#include <iostream>
#include <pmonitor/pmonitor.h>
#include <Event/oncsSubConstants.h>
#include "xdrs.h"

#include <TH1.h>
#include <TH2.h>

int init_done = 0;

using namespace std;

//TH1F *h1; 
//TH2F *h2; 


TH1F *trace[4];

TH1F *h_signal;
TH1F *tdelta;

TH2F *x2;

unsigned long long  res = 1000000000;   // from clockres system call

int packetid=4500;

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

  tdelta = new TH1F( "tdelta", "time difference distribution", 512, 0, 3*res);
  tdelta->GetXaxis()->SetTitle("time diff [ns]");
  tdelta->GetYaxis()->SetTitle("counts");

  h_signal = new TH1F( "h_signal", "signal height distribution", 512, 0, 100);
  h_signal->GetXaxis()->SetTitle("signal height [mV]");
  h_signal->GetYaxis()->SetTitle("counts");


  
  x2 = new TH2F ( "x2", "Scope persistency plot",1024, -0.5, 1023.5 , 512, -700, 200);
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
      tdelta->Reset();
      h_signal->Reset();
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
	  x2->Fill ( sample,  p->rValue(sample,0) );
	}

      // the actual analysis part

      double  xsignal = 0;  // "signal" is a keyword
      double count = 0;  // because I'm lazy
      int take_this = 1; // we set this to 0 if we see that the signal clips
      for (sample = 310; sample <1024; sample++)
	{
	  float s =  p->rValue(sample,0);
	  if ( s < -495)
	    {
	      take_this = 0;
	      break;
	    }
	  xsignal += s;
	  count++;
        }
      if (take_this)
	{
	  xsignal /= count;
	  xsignal *=-1;
	  // cout << xsignal << endl;
	  h_signal->Fill(xsignal);
	}

      delete p;

    }


  p = e->getPacket(1010, ID4EVT);
  if (p)
    {
      //p->dump();

      // we calculate the difference of the seconds field
      unsigned long long sec_delta = p->iValue(0) - p->iValue(3);

      // we get the ns value. Since the resolution is 10^9 (1ns),
      // the upper 32 bit of the 64-bit ns value will always be 0.
      // but who knows, maybe one day we will get a system with more resolution, so we
      // do it all the way
      unsigned long long ns0 = (unsigned int) p->iValue(2);
      ns0 <<=32;
      ns0 |= (unsigned int) p->iValue(1);

      // now we add the difference in seconds as nanoseconds
      ns0 += sec_delta * res;
      
      unsigned long long ns1 = (unsigned int) p->iValue(5);
      ns1 <<=32;
      ns1 |= (unsigned int) p->iValue(4);


      unsigned long long nsdelta = ns0 - ns1;
      //cout << hex << ns0 << " " << ns1 << dec << "  "  << nsdelta << endl;
      tdelta->Fill(nsdelta);
      delete p;
    }

  
  return 0;
}
