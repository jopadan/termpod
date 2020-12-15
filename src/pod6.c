#include "pod_common.h"
#include "pod6.h"

bool is_pod6(char* ident)
{
  return (POD6 == pod_type(ident) >= 0);
}
