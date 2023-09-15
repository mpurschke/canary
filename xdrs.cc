
#include <iostream>
#include <pmonitor/pmonitor.h>
#include <Event/oncsSubConstants.h>

#include <vector>


#include "xdrs.h"

#include <TH1.h>
#include <TH2.h>
#include <TMath.h>
#include <TF1.h>

int init_done = 0;

using namespace std;


int baseline_limit=60;
int integral_start=400;


TH1F *trace[4];

TH1F *trace0;

TH1F *h_signal;

TH1F *tdelta;
TH1F *ttimeline_zs =0;
TH1F *ttimeline =0;

TH2F *x2;

TH1F *h_baseline;

TH1F *cfd0; 

unsigned long long  res = 1000000000;   // from clockres system call

int packetid=4500;

static std::vector<int> lowbound, highbound;

void set_baseline_limit(const int l)
{
  baseline_limit = l;
}

void set_integral_start(const int l)
{
  integral_start = l;
}


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
  trace[0]->SetLineColor(kBlue);
  trace[1]->SetLineColor(kViolet-4);
  trace[2]->SetLineColor(kPink+10);
  trace[3]->SetLineColor(kCyan);

  trace0 = new TH1F( "trace0", "channel 0 WF rebinned", 256, -0, 1024);
  trace0->GetXaxis()->SetTitle("Sample Number");
  trace0->GetYaxis()->SetTitle("Amplitude [mv]");
  trace0->SetLineWidth(2);
  trace0->SetLineColor(kBlue);

  
  h_signal = new TH1F( "h_signal", "signal height distribution", 512, 0, 100);
  h_signal->GetXaxis()->SetTitle("signal height [mV]");
  h_signal->GetYaxis()->SetTitle("counts");


  tdelta = new TH1F( "tdelta", "time difference distribution", 512, 0, 5*res);
  tdelta->GetXaxis()->SetTitle("time diff [ns]");
  tdelta->GetYaxis()->SetTitle("counts");

  h_baseline = new TH1F( "h_baseline", "baseline distribution", 128, 0, 50);
  h_baseline->GetXaxis()->SetTitle("baseline [mV]");
  h_baseline->GetYaxis()->SetTitle("counts");


  
  x2 = new TH2F ( "x2", "Scope persistency plot",1024, -0.5, 1023.5 , 512, -700, 200);
  x2->SetStats(0);
  x2->SetXTitle("Sample Nr");
  x2->SetYTitle("Signal");

  cfd0 = new TH1F( "cfd0", "CFD Signal", 256, 0, 1024);

  
  return 0;
  
}


void cfd( TH1F *h, TH1F *cfd, const int delay = 50, const double fraction = 0.6)
{

  cfd->Reset();

  int i;

  for ( i = 1; i < h->GetNbinsX() - delay -1; i++)
    {
      Double_t d = h->GetBinContent(i) - fraction * h->GetBinContent(i+delay);
      cfd->SetBinContent(i, d);
    }
      
}

double find_zero_crossing(TH1F *h)
{

  int zc_bin;
  //cout << "max, min bin : " <<   h->GetMaximumBin() << " " << h->GetMinimumBin() << endl;
  
  for ( zc_bin = h->GetMaximumBin(); zc_bin < h->GetMinimumBin(); zc_bin++)
    {
      // cout << "zc_bin : " <<  zc_bin << endl;
      if ( h->GetBinContent(zc_bin) <= 0 ) return h->GetBinCenter(zc_bin); 
      //      if ( h->GetBinContent(zc_bin) >= 0 ) break; //return h->GetBinCenter(zc_bin); 
    }
  return 0;
  
  // Double_t range = h->GetMaximumBin() - h->GetMinimumBin();
  // range  *= 0.25;
  // int from = zc_bin - range;
  // int to = zc_bin + range;

  // h->Fit("pol1", "" , "", from,  to);
  // TF1 *x = h->GetFunction("pol1");

  // Double_t zc = -1 * x->GetParameter(0) / x->GetParameter(1);
  
  // return zc;
}


int old_runnumber = -1;  // impossible value
int starttime = 0;

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
      
      if ( !ttimeline)
	{
	  int tb = e->getTime();
	  starttime = tb;
	  int te = tb + 3600*4;
	  int tbins = 360*4;
	  
	  ttimeline = new TH1F( "ttimeline", "event rate as function of time", tbins, tb, te);
	  ttimeline->GetXaxis()->SetTitle("time ");
	  ttimeline->GetXaxis()->SetTimeDisplay(1);
	  ttimeline->GetYaxis()->SetTitle("events/10s");

	  ttimeline_zs = new TH1F( "ttimeline_zs", "event rate as function of time, offset subtracted", tbins, 0, 3600*4);
	  ttimeline_zs->GetXaxis()->SetTitle("time ");
	  ttimeline_zs->GetXaxis()->SetTimeDisplay(1);
	  ttimeline_zs->GetYaxis()->SetTitle("events/10s");


	}
    }

  
  Packet *p = e->getPacket(packetid);
  if (p)
    {

      ttimeline->Fill( e->getTime() );
      
      int delta = e->getTime() -starttime;
      ttimeline_zs->Fill( delta);

      
      char name[512];
      char title[512];
      
      for ( channel = 0; channel < 4 ; channel++)
	{
	  sprintf ( title , "waveform channel %d  Run %d  Evt %d", channel, e->getRunNumber(), e->getEvtSequence());
	  trace[channel]->SetTitle(title);
	  trace[channel]->Reset();
	}

      trace0->Reset();
      
      for ( channel = 0; channel < 4 ; channel++)
	{
	  for (sample = 0; sample < 1024; sample++)
	    {
	      trace[channel]->Fill ( sample,  p->rValue(sample,channel) );
	    }
	}

      // the actual analysis part
      double baseline = 0;
      double count = 0; 
      for (sample = 1; sample < baseline_limit; sample++)
	{
	  float s =  p->rValue(sample,0);
	  baseline += s;
	  count++;
        }
      baseline /= count;

      h_baseline->Fill(baseline);

      for (sample = 0; sample <1024; sample++)
	{
	  // x2->Fill ( p->rValue(sample,4),  p->rValue(sample,1));
	  x2->Fill ( sample,  p->rValue(sample,0) -baseline );
	  trace0->Fill ( sample,  p->rValue(sample,0) -baseline);
	}

      cfd( trace0, cfd0);

      double xstart  = find_zero_crossing(cfd0);
      cout << " event " << e->getEvtSequence() << " cfd zero = " << xstart << endl;
      if ( xstart < 5) return 0;

      double  xsignal = 0;  // "signal" is a keyword
      count = 0;  // because I'm lazy
      int take_this = 1; // we set this to 0 if we see that the signal clips
      for (sample = xstart - 139; sample < xstart - 139 + 510; sample++)
	{
	  float s =  p->rValue(sample,0);
	  if ( s < -495)
	    {
	      take_this = 0;
	      break;
	    }
	  xsignal += (s - baseline);
	  count++;
        }
      
      if (take_this)
	{
	  xsignal /= count;
	  xsignal *=-1;
	  // cout << xsignal << endl;
	  h_signal->Fill(xsignal);
	  // if ( its_in) h_signal_in->Fill( xsignal);
	  // if ( its_not_in) h_signal_out->Fill( xsignal);

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
