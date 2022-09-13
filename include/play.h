#ifndef GSF_PLAY_H
#define GSF_PLAY_H 1

#include "deadbeef/deadbeef.h"

#ifdef __cplusplus
extern "C" {
#endif

  DB_fileinfo_t *gsf_open(uint32_t hints);
  int gsf_init(DB_fileinfo_t *_info, DB_playItem_t *it);
  
#ifdef __cplusplus
}
#endif

#endif
