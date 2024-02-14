#include "rom.h"
#include <algorithm>

GSF_ROM::GSF_ROM() : fData(0) {}

GSF_ROM::~GSF_ROM() {}

// writes n bytes from data into the internal data structure at a
// given offset location. Bounds checking will be performed to ensure
// the internal data structure is of a sufficient size to store the
// data
int GSF_ROM::WriteBytes(const uint8_t *data, const size_t n, const size_t offset) {
  // ensure vector is sufficiently large
  size_t required_size = offset + n;
  if (fData.size() < required_size) {
    fData.resize(required_size, 0);
  }

  std::copy(data, data+n, fData.data() + offset);
  
  return 0;
}
