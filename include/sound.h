// -*- C++ -*-
#ifndef GSF_SOUND_H
#define GSF_SOUND_H 1

#include "viogsf/vbam/gba/GBA.h"
#include "viogsf/vbam/gba/Sound.h"
#include <algorithm>
#include <vector>

// original source, with modification: audiodecoder.gsf
struct GSFSoundOut : public GBASoundOut {
  GSFSoundOut() : bytes_in_buffer(0) {}
  virtual ~GSFSoundOut() {}
  size_t bytes_in_buffer;
  std::vector<uint8_t> sample_buffer;
  // Receives signed 16-bit stereo audio and a byte count
  virtual void write(const void *samples, unsigned long bytes);
};

#endif
