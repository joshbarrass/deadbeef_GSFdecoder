#ifdef __cplusplus
#include <cstdint>
#else
// TODO: is it right to include stdint.h in C instead of cstdint?
#include <stdint.h>
#endif
#include <cstring>
#include "psflib/psflib.h"
#include "deadbeef/deadbeef.h"
#include "api.h"
#include "plugin.h"
#include "metadata.h"

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }
#define tracedbg(...) { deadbeef->log_detailed (&plugin->plugin, 1, __VA_ARGS__); }

#ifdef STDERR_DEBUGGING
#include <iostream>
#endif

/* Wrappers around deadbeef's file functions, required by psflib */

#ifdef __cplusplus
extern "C" {
#endif

void* psf_fopen(void *context, const char *path) {
  DB_functions_t *deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  DB_FILE *file = deadbeef->fopen(path);
  if (!file) {
    trace("GSF ERR: failed to fopen\n");
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

/*
  PSF File Format
  Sources:
    https://gist.github.com/SaxxonPike/11618bd321a45a70c01febae43ff564e
    https://gsf.caitsith2.net/gsf%20spec.txt

  0x00     (3 bytes): PSF magic string
  0x03     (1 byte):  Version byte (0x22 for GSF)
  0x04     (4 bytes): Size of reserved area (bytes) (can be zero) = R
  0x08     (4 bytes): Size of compressed program (bytes) = N
  0x0C     (4 bytes): Compressed program CRC-32
  0x10     (R bytes): Reserved area
  0x10+R   (N bytes): Program area
  0x10+R+N (5 bytes): [TAG], followed by tag data

  psflib will handle reading of the PSF file, decompression of the
  program area, validation of checksum, etc. and pass the program area
  and reserved area (and their respective sizes) to the load
  callback. It will do this for each of the libs specified in the
  [TAG] region, in the order they need to be loaded.

  The program area has the following structure:
  0x00 (4 bytes): GSF Entry Point
  0x04 (4 bytes): GSF Offset
  0x08 (4 bytes): Size of ROM = M
  0x0C (M bytes): ROM area

  The program area must be parsed and loaded by the load callback
  function.

  The valid GSF_Entry_Points are 0x2000000 (Multiboot region), and 0x8000000 (Rom region).
  The The High byte of the Offset should match the high byte of Entry_Point.

  The reserved area for the GSF format is unlikely to be used, as the
  purpose of it has not been fully figured out. Therefore, the decoder
  can typically just ignore it (though psflib will still pass it to
  the player if it is found)

*/

// reads a little-endian long (4 bytes -- QuickBMS definition) from a
// given memory location
inline uint32_t read_long_le(const uint8_t *v) {
  return (uint32_t)(
                    ((uint32_t)(v[3]) << 3*8) |
                    ((uint32_t)(v[2]) << 2*8) |
                    ((uint32_t)(v[1]) <<   8) |
                    ((uint32_t)(v[0]))
                    );
}

// will be passed each lib in order to load into the ROM object
// non-zero return value = error
int gsf_load_callback(void *context, const uint8_t *exe, size_t exe_size,
                      const uint8_t *reserved, size_t reserved_size) {
  PluginState *state = (PluginState*)(context);
  DB_decoder_t *plugin = get_plugin_pointer();
  DB_functions_t *deadbeef = get_API_pointer();

  // reserved area has indeterminate/non-standard usage in GSF, so it
  // is unexpected to find data here
  if (reserved_size > 0) {
    trace("GSF WARN: reserved section contains data, so the file may not be played properly\n");
  }

  uint32_t entry_point = read_long_le(exe);
  // must mask the offset
  // if you don't, a HUGE buffer will be allocated (which is mostly
  // empty space) and the emulator won't play the ROM properly
  uint32_t offset = read_long_le(exe + 0x04) & 0x1ffffff;
  uint32_t rom_size = read_long_le(exe + 0x08);

  // standard specifies only two valid entry points, but there may be
  // some non-compliance
  // TODO: should this only be set once? (i.e. by the first loaded
  // GSF?) see: audiodecoder.gsf
  if (entry_point != 0x2000000 || entry_point != 0x8000000) {
    trace("GSF WARN: unexpected entry point %X\n", entry_point);
  }
  if (!state->set_entry) {
    state->entry_point = entry_point;
    tracedbg("GSF DEBUG: Entry point: %X\n", entry_point);
    #ifdef STDERR_DEBUGGING
    std::cerr << "GSF DEBUG: Entry point: " << entry_point << std::endl;
    #endif
    state->set_entry = true;
  }

  // ensure we do not over-read if asked to by a malformed GSF file
  if (rom_size > exe_size - 12) {
    trace("GSF ERR: exe_size (%d) too small for given rom size (%d)\n", exe_size, rom_size);
    return -1;
  }

  state->ROM.WriteBytes(exe + 0x0C, rom_size, offset);

  return 0;
}

// metadata callback for psflib
int gsf_info_callback(void *context, const char *name, const char *value) {
  PluginState *state = (PluginState*)context;

  if (!strcasecmp(name, "length")) {
    state->fMetadata.Length = parse_time(value);
  } else if (!strcasecmp(name, "fade")) {
    state->fMetadata.Fadeout = parse_time(value);
  } else if (!strcasecmp(name, "title")) {
    state->fMetadata.Title = value;
  } else if (!strcasecmp(name, "artist")) {
    state->fMetadata.Artist = value;
  } else if (!strcasecmp(name, "year")) {
    state->fMetadata.Year = value;
  } else if (!strcasecmp(name, "game")) {
    state->fMetadata.Game = value;
  } else if (!strcasecmp(name, "comment")) {
    state->fMetadata.Comment = value;
  }

  return 0;
}

#ifdef __cplusplus
}
#endif
