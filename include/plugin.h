#ifndef GSF_PLUGIN_H
#define GSF_PLUGIN_H 1

#include "deadbeef/deadbeef.h"

#ifdef __cplusplus
extern "C" {
#endif

  DB_decoder_t *get_plugin_pointer();
  void set_plugin_pointer(DB_decoder_t *);

  void initialise_plugin_state();

#ifdef __cplusplus
}
#endif

// plugin state only used by C++ code and is incompatible with C code
#ifdef __cplusplus
#include "viogsf/vbam/gba/GBA.h"

class PluginState {
public:
  PluginState();
  ~PluginState();

private:
  bool fInit;
  // GBASystem fEmulator;
};

PluginState *get_plugin_state();

#endif

#endif
