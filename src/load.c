#define DDB_API_LEVEL 12
#define DDB_WARN_DEPRECATED 1
#include "deadbeef/deadbeef.h"

DB_plugin_t* GSFdecoder_load (DB_functions_t *api);

static int gsf_start(void) { return 0; }

static int gsf_stop(void) { return 0; }

static DB_decoder_t plugin = {
    DDB_REQUIRE_API_VERSION(1, 12)
 
    .plugin.version_major = 0,
    .plugin.version_minor = 1,
    .plugin.type = DB_PLUGIN_DECODER,
    .plugin.id = "gsfdecoder",
    .plugin.name = "GSF Decoder",
    .plugin.descr = "GSF Decoder based on viogsf library utilising VBA-M.",
    .plugin.start = gsf_start,
    .plugin.stop = gsf_stop
};

DB_plugin_t *GSFdecoder_load(DB_functions_t *api) {
  return DB_PLUGIN(&plugin);
}
