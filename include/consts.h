#ifndef GSF_CONSTS_H
#define GSF_CONSTS_H 1

#ifndef SAMPLERATE
#define SAMPLERATE 44100
#endif
#ifdef __cplusplus
constexpr int SAMPLE_RATE = SAMPLERATE;
#else
#define SAMPLE_RATE SAMPLERATE
#endif

#endif
