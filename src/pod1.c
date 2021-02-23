#include "pod_common.h"
#include "pod1.h"

bool pod_is_pod1(char* ident)
{
  return (POD1 == pod_type(ident) >= 0);
}

