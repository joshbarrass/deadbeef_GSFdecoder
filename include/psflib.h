#ifndef GSF_PSFLIB_FUNCS_H
#define GSF_PSFLIB_FUNCS_H 1

#include "psflib/psflib.h"

#ifdef __cplusplus
extern "C" {
#endif

  void* psf_fopen(void *context, const char *path);
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

#ifdef __cplusplus
}
#endif

#endif
