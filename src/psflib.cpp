#include "psflib/psflib.h"
#include "deadbeef/deadbeef.h"
#include "api.h"
#include "plugin.h"

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }

void* psf_fopen(void *context, const char *path) {
  DB_functions_t *deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  DB_FILE *file = deadbeef->fopen(path);
  if (!file) {
    trace("psf: failed to fopen\n");
    return nullptr;
  }
  return file;
}

size_t psf_fread(void * buffer, size_t size, size_t count, void * handle) {
  DB_functions_t *deadbeef = get_API_pointer();
  DB_FILE *file = static_cast<DB_FILE*>(handle);
  return deadbeef->fread(buffer, size, count, file);
}

int psf_fseek(void *handle, int64_t offset, int whence) {
  DB_functions_t *deadbeef = get_API_pointer();
  DB_FILE *file = static_cast<DB_FILE*>(handle);
  return deadbeef->fseek(file, offset, whence);
}

int psf_fclose(void *handle) {
  DB_functions_t *deadbeef = get_API_pointer();
  DB_FILE *file = static_cast<DB_FILE*>(handle);
  deadbeef->fclose(file);
  return 0;
}

long psf_ftell(void *handle) {
  DB_functions_t *deadbeef = get_API_pointer();
  DB_FILE *file = static_cast<DB_FILE*>(handle);
  return deadbeef->ftell(file);
}
