#include "pod_common.h"
#include "pod5.h"

bool is_pod5(char* ident)
{
  return (POD5 == pod_type(ident) >= 0);
}

