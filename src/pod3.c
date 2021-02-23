#include "pod_common.h"
#include "pod3.h"

bool pod_is_pod3(char* ident)
{
  return (POD3 == pod_type(ident) >= 0);
}

