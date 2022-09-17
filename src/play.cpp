#include <string>
#include "deadbeef/deadbeef.h"
#include "play.h"
#include "plugin.h"
#include "api.h"
#include "psflib/psflib.h"
#include "psflib.h"

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }

#ifdef STDERR_DEBUGGING
#include <iostream>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// open function
// provides deadbeef with somewhere to put the file info
// file info will be stored in the PluginState
DB_fileinfo_t *gsf_open(uint32_t hints) {
  PluginState *state = get_plugin_state();
  return &(state->fFileInfo);
}

// init function
// prepares song for playing
//
int gsf_init(DB_fileinfo_t *info, DB_playItem_t *it) {
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  PluginState *state = get_plugin_state();

  info->fmt.bps = 16;
  info->fmt.channels = 2;
  info->fmt.samplerate = deadbeef->conf_get_int ("synth.samplerate", 44100);
  info->fmt.channelmask = info->fmt.channels == 1 ? DDB_SPEAKER_FRONT_LEFT : (DDB_SPEAKER_FRONT_LEFT | DDB_SPEAKER_FRONT_RIGHT);
  info->readpos = 0;
  info->plugin = plugin;

  deadbeef->pl_lock ();
  std::string uri(deadbeef->pl_find_meta (it, ":URI"));
  deadbeef->pl_unlock ();

  // if we read metadata again here, this populates the length fields etc.
  if (psf_load(uri.c_str(), &psf_file_functions, GSF_VERSION, 0, 0, gsf_info_callback,
               state, 0, nullptr, nullptr) != GSF_VERSION) {
    trace("failed to load GSF metadata\n");
    return -1;
  }

  if (psf_load(uri.c_str(), &psf_file_functions, GSF_VERSION, gsf_load_callback,
               state, 0, 0, 0, nullptr, nullptr) != GSF_VERSION) {
    trace("failed to load GSF program data\n");
    return -2;
  }

  #ifdef STDERR_DEBUGGING
  std::cerr << "ROM Size: " << state->ROM.GetSize() << std::endl;
  // unsigned char *ROM = state->ROM.GetArray();
  // for (int i = 0; i < state->ROM.GetSize(); ++i) {
  //   std::cerr << ROM[i];
  // }
  // std::cerr << std::endl;
  #endif

  // initialise the emulator
  // multiboot corresponds to 0x2000000 entry point
  state->fEmulator.cpuIsMultiBoot = (state->entry_point >> 24 == 2);
  int size = CPULoadRom(&state->fEmulator, state->ROM.GetArray(), state->ROM.GetSize());
  if (!size) {
    trace("failed to load ROM\n");
    return -3;
  }
  soundInit(&state->fEmulator, &state->output);
  soundReset(&state->fEmulator);
  CPUInit(&state->fEmulator);
  CPUReset(&state->fEmulator);
  state->fInit = true;

  #ifdef STDERR_DEBUGGING
  std::cerr << "Init!" << std::endl;
  #endif

  return 0;
}

void gsf_free(DB_fileinfo_t *_info) {
  initialise_plugin_state();
}

DB_playItem_t *gsf_insert(ddb_playlist_t *plt, DB_playItem_t *after,
                      const char *fname) {
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  PluginState *state = get_plugin_state();
  
  std::string uri(fname);
  if (psf_load(uri.c_str(), &psf_file_functions, GSF_VERSION, 0, 0, gsf_info_callback,
               state, 0, nullptr, nullptr) != GSF_VERSION) {
    trace("failed to load GSF metadata\n");
    return nullptr;
  }

  DB_playItem_t *it = deadbeef->pl_item_alloc_init(fname, plugin->plugin.id);
  deadbeef->pl_add_meta(it, ":FILETYPE", "GSF");

  TrackMetadata &meta = state->fMetadata;
  if (meta.Title.length() > 0) {
    deadbeef->pl_add_meta(it, "title", meta.Title.c_str());
  }

  float total_duration = (float)meta.Length/1000.;
  deadbeef->plt_set_item_duration(plt, it, total_duration);

  after = deadbeef->plt_insert_item(plt, after, it);
  deadbeef->pl_item_unref(it);

  return after;
}

#ifdef __cplusplus
}
#endif
