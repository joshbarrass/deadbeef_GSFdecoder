#ifndef GSF_API_H
#define GSF_API_H 1

#include "deadbeef/deadbeef.h"

#ifdef __cplusplus
extern "C" {
#endif

  DB_functions_t *get_API_pointer();
  void set_API_pointer(DB_functions_t *);

#ifdef __cplusplus
}
#endif

#endif
