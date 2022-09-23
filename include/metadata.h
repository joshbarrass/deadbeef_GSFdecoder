#ifndef GSF_METADATA_H
#define GSF_METADATA_H 1

#include <string>
#include <cstdint>
#include <unordered_map>

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
  // explicit replaygain tags
  std::string RG_AGAIN;
  std::string RG_APEAK;
  std::string RG_TGAIN;
  std::string RG_TPEAK;
  // generic tags
  std::unordered_map<std::string, std::string> OtherMeta;
};

int64_t parse_time(const char *input);

// returns negative on failure
int load_metadata(const char *uri, TrackMetadata *metadata);

#endif
