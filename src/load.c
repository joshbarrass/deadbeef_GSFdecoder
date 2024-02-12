#define DDB_API_LEVEL 12
#define DDB_WARN_DEPRECATED 1
#include "deadbeef/deadbeef.h"
#include "api.h"
#include "plugin.h"
#include "play.h"
#include "consts.h"

// used for injecting definitions into string literals
#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

DB_plugin_t* GSFdecoder_load (DB_functions_t *api);

static int gsf_start(void) { return 0; }

static int gsf_stop(void) { return 0; }

static const char *exts[] = {"minigsf", NULL};

static const char settings_dlg[] =
  "property \"Sample rate\" entry gsf.samplerate " EXPAND(DEFAULTSAMPLERATE) ";\n"
  "property \"Use logarithmic fadeout\" checkbox gsf.log_fade 1;\n" // this could be a dropdown in future to choose from different fadeout algorithms?
  ;

static DB_decoder_t plugin = {
    DDB_REQUIRE_API_VERSION(1, 12)
 
    .plugin.version_major = 0,
    .plugin.version_minor = 11,
    .plugin.type = DB_PLUGIN_DECODER,
    .plugin.id = "gsfdecoder",
    .plugin.name = "GSF Decoder",
    .plugin.descr = "GSF Decoder based on viogsf library utilising VBA-M."
    "\n\nCompile Parameters:\n"
    #ifdef BUILD_RELEASE
    " + BUILD_RELEASE\n"
    #endif
    #ifdef BUILD_DEBUG
    " + BUILD_DEBUG"
    #endif
    #ifdef ENABLE_LOGGING
    " + ENABLE_LOGGING\n"
    #endif
    #ifdef STDERR_DEBUGGING
    " + STDERR_DEBUGGING\n"
    #endif
    #ifdef DEFAULTSAMPLERATE
    " + DEFAULT_SAMPLE_RATE: " EXPAND(DEFAULTSAMPLERATE)
    #endif
    ,
    .plugin.website = "https://github.com/joshbarrass/deadbeef_GSFdecoder",
    .plugin.copyright = "Licensed under GPL v2",
    .plugin.start = gsf_start,
    .plugin.stop = gsf_stop,
    .plugin.configdialog = settings_dlg,
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
    .seek_sample = gsf_seek_sample,
    .insert = gsf_insert,
    .exts = exts,
};

DB_plugin_t *GSFdecoder_load(DB_functions_t *api) {
  set_API_pointer(api);
  set_plugin_pointer(&plugin);
  return DB_PLUGIN(&plugin);
}
