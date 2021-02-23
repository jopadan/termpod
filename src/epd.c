#include "pod_common.h"
#include "epd.h"

bool pod_is_epd(char* ident)
{
  return (EPD == pod_type(ident) >= 0);
}
