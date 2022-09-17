#ifndef GSF_METADATA_H
#define GSF_METADATA_H 1

#include <string>
#ifdef __cplusplus
#include <cstdint>
#else
// TODO: is it right to include stdint.h in C instead of cstdint?
#include <stdint.h>
#endif

struct TrackMetadata {
  TrackMetadata();
  ~TrackMetadata();

  int64_t Length; // milliseconds
  int64_t Fadeout;
  std::string Title;
  std::string Artist;
  std::string Year;
  std::string Game;
  std::string Comment;
};

int64_t parse_time(const char *input);

#endif
