#include "pod_common.h"
#include "pod4.h"

bool is_pod4(char* ident)
{
  return (POD4 == pod_type(ident) >= 0);
}


