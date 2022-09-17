#ifndef GSF_PSFLIB_FUNCS_H
#define GSF_PSFLIB_FUNCS_H 1

#include "psflib/psflib.h"

constexpr char GSF_VERSION = 0x22;

#ifdef __cplusplus
extern "C" {
#endif

  void *psf_fopen(void *context, const char *path);
  size_t psf_fread(void * buffer, size_t size, size_t count, void * handle);
  int psf_fseek(void *handle, int64_t offset, int whence);
  int psf_fclose(void *handle);
  long psf_ftell(void *handle);

  const psf_file_callbacks psf_file_functions {
    "/",
      nullptr,
      psf_fopen,
      psf_fread,
      psf_fseek,
      psf_fclose,
      psf_ftell
      };

  int gsf_load_callback(void *context, const uint8_t *exe, size_t exe_size, const uint8_t *reserved, size_t reserved_size);

  int gsf_info_callback(void *context, const char *name, const char *value);

#ifdef __cplusplus
}
#endif

#endif
