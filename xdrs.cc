
#include <iostream>
#include <pmonitor/pmonitor.h>
#include <Event/oncsSubConstants.h>

#include <vector>


#include "xdrs.h"

#include <TH1.h>
#include <TH2.h>

int init_done = 0;

using namespace std;

//TH1F *h1; 
//TH2F *h2; 


int baseline_limit=200;
int integral_start=310;


TH1F *trace[4];

TH1F *h_signal;
// TH1F *h_signal_in;
// TH1F *h_signal_out;

TH1F *tdelta;
TH1F *ttimeline_zs =0;
TH1F *ttimeline =0;
// TH1F *ttimeline_in =0;
// TH1F *ttimeline_out =0;

TH2F *x2;

TH1F *h_baseline;

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
  trace[0]->SetLineColor(kViolet-4);
  trace[1]->SetLineColor(kBlue);
  trace[2]->SetLineColor(kPink+10);
  trace[3]->SetLineColor(kCyan);

  h_signal = new TH1F( "h_signal", "signal height distribution", 512, 0, 100);
  h_signal->GetXaxis()->SetTitle("signal height [mV]");
  h_signal->GetYaxis()->SetTitle("counts");

  // h_signal_in = new TH1F( "h_signal_in", "signal height distribution during high rates", 512, 0, 100);
  // h_signal_in->GetXaxis()->SetTitle("signal height [mV]");
  // h_signal_in->GetYaxis()->SetTitle("counts");

  // h_signal_out = new TH1F( "h_signal_out", "signal height distribution during low rates", 512, 0, 100);
  // h_signal_out->GetXaxis()->SetTitle("signal height [mV]");
  // h_signal_out->GetYaxis()->SetTitle("counts");


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

  return 0;
  
}

int old_runnumber = -1;  // impossible value
int starttime = 0;

int process_event (Event * e)
{

  int sample,channel;

  // see if we have either the begin-run event or get a new run number
  if ( e->getEvtType() == BEGRUNEVENT || e->getRunNumber() != old_runnumber)
    {

  
      // this for run 121 only
      // lowbound.push_back( 230);
      // highbound.push_back(380);
      
      // lowbound.push_back( 1591);
      // highbound.push_back(1998);
      
      // lowbound.push_back( 4011);
      // highbound.push_back( 4339);
      
      // lowbound.push_back( 7090);
      // highbound.push_back( 7430);
      
      // lowbound.push_back( 11169);
      // highbound.push_back( 11491);
      
      // cout << "size is  " << lowbound.size() << endl;
      


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

	  // ttimeline_in = new TH1F( "ttimeline_in", "event rate during high-rate periods", tbins, tb, te);
	  // ttimeline_in->GetXaxis()->SetTitle("time ");
	  // ttimeline_in->GetXaxis()->SetTimeDisplay(1);
	  // ttimeline_in->GetYaxis()->SetTitle("events/10s");

	  // ttimeline_out = new TH1F( "ttimeline_out", "event rate during low-rate periods", tbins, tb, te);
	  // ttimeline_out->GetXaxis()->SetTitle("time ");
	  // ttimeline_out->GetXaxis()->SetTimeDisplay(1);
	  // ttimeline_out->GetYaxis()->SetTitle("events/10s");

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

      // int its_in = 0;
      // int its_not_in = 0;

      // cout << "size is  " << lowbound.size() << endl;
      // for ( int r = 0; r < lowbound.size() ; r++)
      // 	{
      // 	  cout << delta << "   "<< lowbound[r] << "  " << highbound[r] << endl;  
      // 	  if ( delta >= lowbound[r] && delta <= highbound[r])
      // 	    {
      // 	      its_in = 1;
      // 	    }

      // 	  if ( delta >= lowbound[r]-30 && delta <= highbound[r]+30)
      // 	    {
      // 	      its_not_in = 1;
      // 	    }
      // 	}

      // its_not_in = 1-its_not_in;

      // if ( its_in) ttimeline_in->Fill( e->getTime() );
      // if ( its_not_in) ttimeline_out->Fill( e->getTime() );
      
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
	}


      double  xsignal = 0;  // "signal" is a keyword
      count = 0;  // because I'm lazy
      int take_this = 1; // we set this to 0 if we see that the signal clips
      for (sample = integral_start; sample < 1024; sample++)
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
