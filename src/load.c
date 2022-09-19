#define DDB_API_LEVEL 12
#define DDB_WARN_DEPRECATED 1
#include "deadbeef/deadbeef.h"
#include "api.h"
#include "plugin.h"

#include "play.h"

DB_plugin_t* GSFdecoder_load (DB_functions_t *api);

static int gsf_start(void) { return 0; }

static int gsf_stop(void) { return 0; }

static const char *exts[] = {"minigsf", NULL};

static DB_decoder_t plugin = {
    DDB_REQUIRE_API_VERSION(1, 12)
 
    .plugin.version_major = 0,
    .plugin.version_minor = 4,
    .plugin.type = DB_PLUGIN_DECODER,
    .plugin.id = "gsfdecoder",
    .plugin.name = "GSF Decoder",
    .plugin.descr = "GSF Decoder based on viogsf library utilising VBA-M.",
    .plugin.website = "https://github.com/joshbarrass/deadbeef_GSFdecoder",
    .plugin.copyright = "Licensed under GPL v2",
    .plugin.start = gsf_start,
    .plugin.stop = gsf_stop,
    .plugin.flags = 0
    #ifdef ENABLE_LOGGING
    | DDB_PLUGIN_FLAG_LOGGING
    #endif
    ,
    .open = gsf_open,
    .init = gsf_init,
    .free = gsf_free,
    .read = gsf_read,
    .seek = gsf_seek,
    .insert = gsf_insert,
    .exts = exts,
};

DB_plugin_t *GSFdecoder_load(DB_functions_t *api) {
  set_API_pointer(api);
  set_plugin_pointer(&plugin);
  initialise_plugin_state();
  return DB_PLUGIN(&plugin);
}
