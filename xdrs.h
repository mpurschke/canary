#ifndef __XDRS_H__
#define __XDRS_H__

#include <pmonitor/pmonitor.h>
#include <Event/Event.h>
#include <Event/EventTypes.h>

int process_event (Event *e); //++CINT 

void set_baseline_limit(const int l);
void set_integral_start(const int l);

#endif /* __XDRS_H__ */
