#ifndef GSF_CONSTS_H
#define GSF_CONSTS_H 1

#ifndef DEFAULTSAMPLERATE
#define DEFAULTSAMPLERATE 44100
#endif
#ifdef __cplusplus
constexpr int DEFAULT_SAMPLE_RATE = DEFAULTSAMPLERATE;
#else
#define DEFAULT_SAMPLE_RATE DEFAULTSAMPLERATE
#endif

#endif
