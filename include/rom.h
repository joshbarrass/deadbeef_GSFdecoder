#ifndef GSF_ROM_H
#define GSF_ROM_H 1

#include <vector>
#include <cstdint>

class GSF_ROM {
public:
  GSF_ROM();
  ~GSF_ROM();
  int GetSize() const { return fData.size(); }
  uint8_t *GetArray() { return fData.data(); }

  int WriteBytes(const uint8_t *, const size_t, const size_t);

private:
  std::vector<uint8_t> fData;
};

#endif
