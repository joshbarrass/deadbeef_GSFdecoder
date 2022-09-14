#include "deadbeef/deadbeef.h"
#include "play.h"
#include "plugin.h"

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
