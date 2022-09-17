#include "viogsf/vbam/gba/GBA.h"
#include "viogsf/vbam/gba/Sound.h"

#include "plugin.h"
#include "api.h"

#include <iostream>

PluginState::PluginState() : fInit(false), fMetadata(), ROM(), entry_point(0) {}

PluginState::~PluginState() {
  if (fInit) {
    soundShutdown(&fEmulator);
    CPUCleanUp(&fEmulator);
  }
  fEmulator.~GBASystem();
}

//

static DB_decoder_t *plugin;

static PluginState state = PluginState();

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }

DB_decoder_t *get_plugin_pointer() {
  return plugin;
}

void set_plugin_pointer(DB_decoder_t *ptr) {
  plugin = ptr;
}

void initialise_plugin_state() {
  // source: https://stackoverflow.com/a/2166155/7471232
  // deconstruct and completely reinitialise the plugin state
  // cannot just use
  //    state = PluginState()
  // here as this produces:
  //    ...is implicitly deleted because the default definition would be ill-formed
  // see https://stackoverflow.com/a/49826562/7471232
  state.~PluginState();
  new(&state) PluginState();
}

PluginState *get_plugin_state() {
  return &state;
}
