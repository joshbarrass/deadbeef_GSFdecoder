#include <regex>
#include "metadata.h"
#include "psflib.h"

TrackMetadata::TrackMetadata()
  : Length(0), Fadeout(0), Title(""), Artist(""), Year(""), Game(""), Comment(""), set_RG_album(false), RG_AGAIN(0), RG_APEAK(1), set_RG_track(false), RG_TGAIN(0), RG_TPEAK(1) {
  // explicitly clear the map to avoid any issues with reused memory
  OtherMeta.clear();
}

TrackMetadata::~TrackMetadata() {}


// time can be either seconds.decimal, minutes:seconds.decimal, or
// hours:minutes:seconds.decimal. The decimal can also be omitted.
// source: audiodecoder.gsf
constexpr unsigned long BORK_TIME = 0xDEADBEEF;
const std::regex duration_regex("^(?:(?:(\\d+):)?(\\d?\\d):)?(\\d+)(?:\\.(\\d*))?$");
int64_t parse_time(const char *input) {
  constexpr int MILLISECONDS = 4;
  constexpr int SECONDS = 3;
  constexpr int MINUTES = 2;
  constexpr int HOURS = 1;
  
  if (!input) {
    return BORK_TIME;
  }
  std::cmatch match;
  if (!std::regex_match(input, match, duration_regex)) {
    return BORK_TIME;
  }

#define to_str() str().c_str()
  int milliseconds = atoi(match[MILLISECONDS].to_str());
  int seconds = atoi(match[SECONDS].to_str());
  int minutes = atoi(match[MINUTES].to_str());
  int hours = atoi(match[HOURS].to_str());
  while (milliseconds >= 1000) {
    milliseconds /= 10;
  }

  int64_t total_ms = milliseconds + 1000*seconds + 60*1000*minutes + 60*60*1000*hours;
  
  return total_ms;
}

const std::regex replaygain_gain_regex("^([+-]?\\d+(?:\\.(?:\\d+)*)?) dB$");
const std::regex replaygain_peak_regex("^([+-]?\\d+(?:\\.(?:\\d+)*)?)$");
int parse_rg_gain(const char *input, float &output) {
  if (!input) {
    return -1;
  }
  std::cmatch match;
  if (!std::regex_match(input, match, replaygain_gain_regex)) {
    return -2;
  }
  output = std::stof(match[1].str().c_str());
  return 0;
}

int parse_rg_peak(const char *input, float &output) {
  if (!input) {
    return -1;
  }
  std::cmatch match;
  if (!std::regex_match(input, match, replaygain_peak_regex)) {
    return -2;
  }
  output = std::stof(match[1].str().c_str());
  return 0;
}

int load_metadata(const char *uri, TrackMetadata *metadata) {
  if (psf_load(uri, &psf_file_functions, GSF_VERSION, 0, 0, gsf_info_callback,
               metadata, 0, nullptr, nullptr) != GSF_VERSION) 
    return -1;
  return 0;
}
