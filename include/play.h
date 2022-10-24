#ifndef GSF_PLAY_H
#define GSF_PLAY_H 1

#include "deadbeef/deadbeef.h"

#ifdef __cplusplus
extern "C" {
#endif

  DB_fileinfo_t *gsf_open(uint32_t hints);
  int gsf_init(DB_fileinfo_t *_info, DB_playItem_t *it);
  void gsf_free(DB_fileinfo_t *_info);
  int gsf_read(DB_fileinfo_t *_info, char *buffer, int nbytes);
  int gsf_seek(DB_fileinfo_t *info, float seconds);
  int gsf_seek_sample(DB_fileinfo_t *info, int sample);
  DB_playItem_t *gsf_insert(ddb_playlist_t *plt, DB_playItem_t *after, const char *fname);
  
#ifdef __cplusplus
}
#endif

#endif
