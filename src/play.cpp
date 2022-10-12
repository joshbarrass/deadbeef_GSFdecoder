#include <string>
#include "deadbeef/deadbeef.h"
#include "play.h"
#include "plugin.h"
#include "api.h"
#include "psflib/psflib.h"
#include "psflib.h"
#include "metadata.h"

#define trace(...) { deadbeef->log_detailed (&plugin->plugin, 0, __VA_ARGS__); }
#define tracedbg(...) { deadbeef->log_detailed (&plugin->plugin, 1, __VA_ARGS__); }

#ifdef STDERR_DEBUGGING
#include <iostream>
#endif

inline PluginState *get_plugin_state(DB_fileinfo_t *_info) {
  return (PluginState*)_info;
}

#ifdef __cplusplus
extern "C" {
#endif

// open function
// provides deadbeef with somewhere to put the file info
// file info will be stored in the PluginState
DB_fileinfo_t *gsf_open(uint32_t hints) {
  PluginState *state = new PluginState();
  #ifdef BUILD_DEBUG
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  tracedbg("GSF DEBUG: made new plugin state (%X)\n", state);
  #endif
  state->hints = hints;
  return &(state->fFileInfo);
}

// init function
// prepares song for playing
//
int gsf_init(DB_fileinfo_t *info, DB_playItem_t *it) {
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  PluginState *state = get_plugin_state(info);

  #ifdef BUILD_DEBUG
  if (state->hints & DDB_DECODER_HINT_CAN_LOOP) {
    tracedbg("GSF DEBUG: CAN_LOOP is set\n");
  } else {
    tracedbg("GSF DEBUG: CAN_LOOP is NOT set\n");
  }
  #endif

  info->fmt.bps = 16;
  info->fmt.channels = 2;
  info->fmt.samplerate = deadbeef->conf_get_int ("synth.samplerate", 44100);
  info->fmt.channelmask = info->fmt.channels == 1 ? DDB_SPEAKER_FRONT_LEFT : (DDB_SPEAKER_FRONT_LEFT | DDB_SPEAKER_FRONT_RIGHT);
  info->readpos = 0;
  info->plugin = plugin;

  deadbeef->pl_lock ();
  std::string uri(deadbeef->pl_find_meta (it, ":URI"));
  deadbeef->pl_unlock ();

  // read metadata again here to populate the context struct
  if (load_metadata(uri.c_str(), &state->fMetadata) < 0) {
    trace("GSF ERR: (init) failed to load GSF metadata\n");
    return -1;
  }

  if (psf_load(uri.c_str(), &psf_file_functions, GSF_VERSION, gsf_load_callback,
               state, 0, 0, 0, nullptr, nullptr) != GSF_VERSION) {
    trace("GSF ERR: failed to load GSF program data\n");
    return -2;
  }

  #ifdef BUILD_DEBUG
  tracedbg("GSF DEBUG: ROM Size: %d\n", state->ROM.GetSize());
  #ifdef STDERR_DEBUGGING
  std::cerr << "GSF INFO: ROM Size: " << state->ROM.GetSize() << std::endl;
  #endif
  #endif

  // initialise the emulator
  // multiboot corresponds to 0x2000000 entry point
  state->fEmulator.cpuIsMultiBoot = (state->entry_point >> 24 == 2);
  int size = CPULoadRom(&state->fEmulator, state->ROM.GetArray(), state->ROM.GetSize());
  if (!size) {
    trace("GSF ERR: failed to load ROM\n");
    return -3;
  }
  soundInit(&state->fEmulator, &state->output);
  soundReset(&state->fEmulator);
  CPUInit(&state->fEmulator);
  CPUReset(&state->fEmulator);
  state->fInit = true;

  #ifdef BUILD_DEBUG
  tracedbg("GSF DEBUG: Init!\n");
  #ifdef STDERR_DEBUGGING
  std::cerr << "GSF DEBUG: Init!" << std::endl;
  #endif
  #endif

  return 0;
}

void gsf_free(DB_fileinfo_t *_info) {
  PluginState* state = get_plugin_state(_info);
  #ifdef BUILD_DEBUG
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  #endif
  if (state) {
    delete state;
    #ifdef BUILD_DEBUG
    tracedbg("GSF DEBUG: freed state (%X)\n", state);
    #endif
  }
}

int gsf_read(DB_fileinfo_t *_info, char *buffer, int nbytes) {
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  PluginState *state = get_plugin_state(_info);

  if (!state->fInit) {
    trace("GSF ERR: attempt to read from uninitialised plugin state\n");
    return -1;
  }

#ifdef BUILD_DEBUG
  tracedbg("GSF DEBUG: readsample: %d, length: %d\n", state->readsample, state->fMetadata.LengthSamples);
  #ifdef STDERR_DEBUGGING
  std::cerr << "GSF DEBUG: readsample: " << state->readsample << ", length: " << state->fMetadata.LengthSamples << std::endl;
  #endif
  #endif
  if (!(deadbeef->streamer_get_repeat () == DDB_REPEAT_SINGLE) || !(state->hints & DDB_DECODER_HINT_CAN_LOOP)) {
    if (state->readsample >= (float)state->fMetadata.LengthSamples) {
#ifdef BUILD_DEBUG
      tracedbg("GSF DEBUG: end of track\n");
#endif
      return 0;
    }
  }

  #ifdef BUILD_DEBUG
  tracedbg("GSF DEBUG: %d bytes in buffer\n", state->output.bytes_in_buffer);
  #ifdef STDERR_DEBUGGING
  std::cerr << "GSF DEBUG: " << state->output.bytes_in_buffer << " bytes in buffer" << std::endl;
  #endif
  tracedbg("GSF DEBUG: buffer size %d\n", state->output.sample_buffer.size());
  #ifdef STDERR_DEBUGGING
  std::cerr << "GSF DEBUG: buffer size " << state->output.sample_buffer.size() << std::endl;
  #endif
  tracedbg("GSF DEBUG: requested %d bytes\n", nbytes);
  #ifdef STDERR_DEBUGGING
  std::cerr << "GSF DEBUG: requested " << nbytes << " bytes" << std::endl;
  #endif
  #endif

  while (state->output.bytes_in_buffer < nbytes) {
    #ifdef BUILD_DEBUG
    int old_size = state->output.bytes_in_buffer;
    #endif
    CPULoop(&state->fEmulator, 250000);
    #ifdef BUILD_DEBUG
    tracedbg("GSF DEBUG: emulator yielded %d bytes\n", state->output.bytes_in_buffer - old_size);
    #endif
  }

  size_t to_copy = std::min(nbytes, (int)(state->output.bytes_in_buffer));
  #ifdef BUILD_DEBUG
  tracedbg("GSF DEBUG: Must copy %d bytes\n", to_copy);
  #ifdef STDERR_DEBUGGING
  std::cerr << "GSF DEBUG: Must copy " << to_copy << " bytes" << std::endl;
  #endif
  #endif
  // if we would copy more samples than the length of the file, we
  // need to trim the buffer
  size_t remaining_samples = state->fMetadata.LengthSamples - state->readsample;
  if (to_copy > remaining_samples)
    to_copy = remaining_samples;
  unsigned char *head_sample = &state->output.sample_buffer[0];
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
  state->readsample += nbytes / 4;
  _info->readpos += (float)nbytes / 44100 / 4;

  return to_copy;
}

int gsf_seek(DB_fileinfo_t *info, float seconds) {
  auto deadbeef = get_API_pointer();
  auto plugin = get_plugin_pointer();
  PluginState *state = get_plugin_state(info);

  // cannot emulate backwards; only option is to restart the emulator
  if (info->readpos > seconds) {
    CPUReset(&state->fEmulator);
    info->readpos = 0;
    state->readsample = 0;
  }

  float to_seek = seconds - info->readpos;
  size_t &in_buffer = state->output.bytes_in_buffer;
  while (to_seek > 0) {
    #ifdef BUILD_DEBUG
    tracedbg("GSF DEBUG: Bytes in buffer: %d\n", in_buffer);
    tracedbg("GSF DEBUG: to_seek: %d\n", to_seek);
    #ifdef STDERR_DEBUGGING
    std::cerr << "GSF DEBUG: Bytes in buffer: " << in_buffer << std::endl;
    std::cerr << "GSF DEBUG: to_seek: " << to_seek << "s" << std::endl;
    #endif
    #endif
    if (in_buffer > 0) {
      float seconds_in_buffer = (float)in_buffer / 44100 / 4;
      if (seconds_in_buffer <= to_seek) {
        #ifdef BUILD_DEBUG
        tracedbg("GSF DEBUG: Discarding buffer\n");
        #ifdef STDERR_DEBUGGING
        std::cerr << "GSF DEBUG: Discarding buffer" << std::endl;
        #endif
        #endif
        // discard the entire buffer if there's less data than we need
        state->readsample += in_buffer / 4;
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
      state->readsample += in_buffer / 4;
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

  DB_playItem_t *it = deadbeef->pl_item_alloc_init(fname, plugin->plugin.id);
  deadbeef->pl_add_meta(it, ":FILETYPE", "GSF");

  TrackMetadata meta = TrackMetadata();
  if (load_metadata(fname, &meta) < 0) {
    trace("GSF ERR: (insert) failed to load GSF metadata\n");
    return nullptr;
  }

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

  // add replaygain
  if (meta.set_RG_album) {
    deadbeef->pl_set_item_replaygain(it, DDB_REPLAYGAIN_ALBUMGAIN, meta.RG_AGAIN);
    deadbeef->pl_set_item_replaygain(it, DDB_REPLAYGAIN_ALBUMPEAK, meta.RG_APEAK);
  }
  if (meta.set_RG_track) {
    deadbeef->pl_set_item_replaygain(it, DDB_REPLAYGAIN_TRACKGAIN, meta.RG_TGAIN);
    deadbeef->pl_set_item_replaygain(it, DDB_REPLAYGAIN_TRACKPEAK, meta.RG_TPEAK);
  }

  // add others
  for (auto itr = meta.OtherMeta.begin(); itr != meta.OtherMeta.end(); ++itr) {
    deadbeef->pl_add_meta(it, itr->first.c_str(), itr->second.c_str());
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
