#ifndef GSF_TYPES_H
#define GSF_TYPES_H 1

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

// sample_count_t represents a number of samples
// used for e.g. tracking the current sample
// also used for samples rates (since this is #samples / second)
typedef long sample_count_t;

// sample_t represents a single audio sample (one channel)
// we are using signed 16-bit audio, so 1 sample = 16 bits
typedef int16_t sample_t;

#endif
