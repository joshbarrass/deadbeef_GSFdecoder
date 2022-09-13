#include "viogsf/vbam/gba/GBA.h"
#include "viogsf/vbam/gba/Sound.h"

#include "plugin.h"
#include "api.h"

#include <iostream>

DB_decoder_t *plugin;

PluginState state;

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }

DB_decoder_t *get_plugin_pointer() {
  return plugin;
}

void set_plugin_pointer(DB_decoder_t *ptr) {
  plugin = ptr;
}

void initialise_plugin_state() {
  state = PluginState();
}

PluginState *get_plugin_state() {
  return &state;
}

PluginState::PluginState() : fInit(false) {}

PluginState::~PluginState() {
  if (fInit) {
    // soundShutdown(&fEmulator);
    // CPUCleanUp(&fEmulator);
  }
}
