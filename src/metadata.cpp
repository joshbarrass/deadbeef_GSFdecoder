#include <regex>
#include "metadata.h"

TrackMetadata::TrackMetadata()
  : Length(0), Fadeout(0), Title(""), Artist(""), Year(""), Game(""), Comment("") {}

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
