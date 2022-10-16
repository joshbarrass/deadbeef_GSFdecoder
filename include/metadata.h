#ifndef GSF_METADATA_H
#define GSF_METADATA_H 1

#include <string>
#include <cstdint>
#include <unordered_map>

struct TrackMetadata {
  TrackMetadata();
  ~TrackMetadata();

  int64_t Length; // milliseconds
  int64_t LengthSamples;
  int64_t Fadeout; // milliseconds
  int64_t FadeoutSamples;
  std::string Title;
  std::string Artist;
  std::string Year;
  std::string Game;
  std::string Comment;
  // explicit replaygain tags
  bool set_RG_album; // TODO: should these bools be for each property?
  float RG_AGAIN;
  float RG_APEAK;
  bool set_RG_track;
  float RG_TGAIN;
  float RG_TPEAK;
  // generic tags
  std::unordered_map<std::string, std::string> OtherMeta;
};

int64_t parse_time(const char *input);

int parse_rg_gain(const char *input, float &output);
int parse_rg_peak(const char *input, float &output);

// returns negative on failure
int load_metadata(const char *uri, TrackMetadata *metadata);

#endif
