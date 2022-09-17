#include <string>
#include "deadbeef/deadbeef.h"
#include "play.h"
#include "plugin.h"
#include "api.h"
#include "psflib/psflib.h"
#include "psflib.h"

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }

// open function
// provides deadbeef with somewhere to put the file info
// file info will be stored in the PluginState
DB_fileinfo_t *gsf_open(uint32_t hints) {
  PluginState *state = get_plugin_state();
  return &(state->fFileInfo);
}

// TODO: init function
// prepares song for loading
//
int gsf_init(DB_fileinfo_t *_info, DB_playItem_t *it) {
  return 0;
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

  float total_duration = (meta.Length+meta.Fadeout)/1000.;
  deadbeef->plt_set_item_duration(plt, it, total_duration);

  after = deadbeef->plt_insert_item(plt, after, it);
  deadbeef->pl_item_unref(it);

  return after;
}
