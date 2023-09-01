#include "xdrs.h"
R__LOAD_LIBRARY(libxdrs.so)

void xdrs(const char * filename)
{
  if ( filename != NULL)
    {
      pfileopen(filename);
    }
}
