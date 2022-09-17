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

int gsf_read(DB_fileinfo_t *_info, char *buffer, int nbytes) {
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  PluginState *state = get_plugin_state();

  #ifdef STDERR_DEBUGGING
    std::cerr << "readpos: " << _info->readpos << ", length: " << state->fMetadata.Length / 1000 << std::endl;
    #endif
    if (_info->readpos >= (float)state->fMetadata.Length / 1000) {
    trace("end of track\n");
    return 0;
  }

  #ifdef STDERR_DEBUGGING
  std::cerr << state->output.bytes_in_buffer << " bytes in buffer" << std::endl;
  #endif

  trace("have %d bytes\n", state->output.bytes_in_buffer);
  #ifdef STDERR_DEBUGGING
  std::cerr << "read " << state->output.bytes_in_buffer << " bytes" << std::endl;
  #endif

  #ifdef STDERR_DEBUGGING
  std::cerr << "buffer size " << state->output.sample_buffer.size() << std::endl;
  #endif

  trace("requested %d bytes\n", nbytes);
  #ifdef STDERR_DEBUGGING
  std::cerr << "requested " << nbytes << " bytes" << std::endl;
  #endif

  while (state->output.bytes_in_buffer < nbytes) {
    CPULoop(&state->fEmulator, 250000);
  }

  size_t to_copy = std::min(nbytes, (int)(state->output.bytes_in_buffer));
  #ifdef STDERR_DEBUGGING
  std::cerr << "Must copy " << to_copy << " samples" << std::endl;
  #endif
  unsigned char *head_sample = &state->output.sample_buffer[0];
  #ifdef STDERR_DEBUGGING
  std::cerr << (int)*head_sample << std::endl;
  #endif
  std::copy(head_sample, head_sample+to_copy, buffer);

  // move the excess samples down to the start of the buffer, then
  // trim the buffer in preparation for the next read
  if (state->output.bytes_in_buffer > to_copy) {
    std::copy(head_sample + to_copy,
              head_sample + state->output.bytes_in_buffer,
              head_sample);
    // state->output.sample_buffer.resize(state->output.bytes_in_buffer - to_copy);
  } else {
    // state->output.sample_buffer.resize(0);
  }
  state->output.bytes_in_buffer -= to_copy;

  // 16-bit samples, stereo, so 4 bytes per sample
  // 44100 samples per second
  _info->readpos += (float)nbytes / 44100 / 4;

  return to_copy;
}

int gsf_seek(DB_fileinfo_t *info, float seconds) {
  PluginState *state = get_plugin_state();

  // cannot emulate backwards; only option is to restart the emulator
  if (info->readpos > seconds) {
    CPUReset(&state->fEmulator);
    info->readpos = 0;
  }

  float to_seek = seconds - info->readpos;
  size_t &in_buffer = state->output.bytes_in_buffer;
  while (to_seek > 0) {
    #ifdef STDERR_DEBUGGING
    std::cerr << "Bytes in buffer: " << in_buffer << std::endl;
    std::cerr << "to_seek: " << to_seek << "s" << std::endl;
    #endif
    if (in_buffer > 0) {
      float seconds_in_buffer = (float)in_buffer / 44100 / 4;
      if (seconds_in_buffer <= to_seek) {
        #ifdef STDERR_DEBUGGING
        std::cerr << "Discarding buffer" << std::endl;
        #endif
        // discard the entire buffer if there's less data than we need
        in_buffer = 0;
        to_seek -= seconds_in_buffer;
        continue;
      }
      // otherwise, drop as many bytes as we need for the needed seek
      int bytes_needed = to_seek * 44100 * 4;
      // must ensure the bytes_needed is sample-aligned i.e. must be a
      // multiple of 4. If it isn't, the music will break
      while (bytes_needed % 4 != 0) {
        bytes_needed += 1;
      }
      unsigned char *head_sample = &state->output.sample_buffer[0];
      std::copy(head_sample + bytes_needed,
                head_sample + in_buffer,
                head_sample);
      in_buffer -= bytes_needed;
      to_seek = 0;
      break;
    } else {
      // buffer empty; get more samples
      CPULoop(&state->fEmulator, 250000);
    }
  }
  info->readpos = seconds;
  return 0;
}

DB_playItem_t *gsf_insert(ddb_playlist_t *plt, DB_playItem_t *after,
                      const char *fname) {
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  PluginState *state = get_plugin_state();
  // clear the metadata in the current state, else this can lead to
  // tracks with unspecified tags inheriting those tags from the last
  // played track
  // TODO: hopefully this will not be necessary once the state is
  // stored around the DB_fileinfo_t context
  state->fMetadata = TrackMetadata();
  
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
  if (meta.Artist.length() > 0) {
    deadbeef->pl_add_meta(it, "artist", meta.Artist.c_str());
  }
  if (meta.Year.length() > 0) {
    deadbeef->pl_add_meta(it, "year", meta.Year.c_str());
  }
  if (meta.Game.length() > 0) {
    deadbeef->pl_add_meta(it, "album", meta.Game.c_str());
  }
  if (meta.Comment.length() > 0) {
    deadbeef->pl_add_meta(it, "comment", meta.Comment.c_str());
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
