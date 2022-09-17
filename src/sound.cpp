#include <algorithm>
#include "sound.h"

void GSFSoundOut::write(const void *samples, unsigned long bytes) {
    sample_buffer.resize(bytes_in_buffer + bytes);
    std::copy((uint8_t*)samples, (uint8_t*)samples+bytes, &sample_buffer[bytes_in_buffer]);
    bytes_in_buffer += bytes;
}
