#include "viogsf/vbam/gba/GBA.h"
#include "viogsf/vbam/gba/Sound.h"

#include "plugin.h"
#include "api.h"

#include <iostream>

// plugin state will often occupy the same memory as a previous state,
// must be careful to ensure everything is properly initialised to
// avoid segfaults
PluginState::PluginState() : fFileInfo(), readsample(0), max_samples(0), fInit(false), hints(0), fMetadata(), output(), ROM(), entry_point(0), set_entry(false), fEmulator() {}

PluginState::~PluginState() {
  if (fInit) {
    soundShutdown(&fEmulator);
    CPUCleanUp(&fEmulator);
    fInit = false;
  }
  // fEmulator.~GBASystem();
}

//

static DB_decoder_t *plugin;

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }

DB_decoder_t *get_plugin_pointer() {
  return plugin;
}

void set_plugin_pointer(DB_decoder_t *ptr) {
  plugin = ptr;
}
