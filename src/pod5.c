#include "pod_common.h"
#include "pod5.h"

bool pod_is_pod5(char* ident)
{
  return (POD5 == pod_type(ident) >= 0);
}

