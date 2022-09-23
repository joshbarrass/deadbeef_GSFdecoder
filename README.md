# GSF Decoder

A [GSF](http://www.vgmpf.com/Wiki/index.php?title=GSF) decoder plugin for [DeaDBeeF](https://github.com/DeaDBeeF-Player/deadbeef), based on viogsf/VBA-M.

----------------------------------------

## Building From Source

Ensure that when you clone this repository, you clone recursively to download the necessary submodules. The project can then be built the same way as any other CMake project. To fully clone and build this plugin:

```
git clone --recursive https://github.com/joshbarrass/deadbeef_GSFdecoder
cd deadbeef_GSFdecoder
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

By defualt, running `make install` will then install the plugin to `/opt/deadbeef/lib/deadbeef` (global install). If this is not correct, replace the fourth line above with:

```
cmake -DCMAKE_INSTALL_PREFIX=<path> -DCMAKE_BUILD_TYPE=Release .. 
```

where `<path>` is the target path you want to install to. If you wish to install the plugin for your user only, you can set this to `$HOME/.local/lib/deadbeef/`. You should be able to run this command at any time without having to completely recompile the plugin.

If you wish to install the plugin manually, simply copy the `GSFdecoder.so` to one of DeaDBeeF's library paths (probably either `/opt/deadbeef/lib/deadbeef` or `$HOME/.local/lib/deadbeef/`).

### Enabling Logging

If you are encountering any errors with the plugin, it may be helpful to enable logging to figure out what error, if any, is occurring. Currently, logging can only be toggled at compile time. To do so, add the `-DENABLE_LOGGING=ON` switch to your cmake command and recompile.

### Debug Builds

If you want to compile a debug build, just change the CMake build type to `Debug`. Be warned, however, that debug builds have considerably more verbose logging, which is ordinarily disabled in a regular build.

## Credit

This plugin would not have been possible without extensive reference to [audiodecoder.gsf](https://github.com/xbmc/audiodecoder.gsf), the GSF decoder for Kodi. This project has been instrumental in understanding the GSF decoding process, particularly how to use the [viogsf](https://github.com/kode54/viogsf) and [psflib](https://github.com/kode54/psflib) libraries that this project depends on. 

The [viogsf](https://github.com/kode54/viogsf) library itself has been extremely useful for developing this plugin, packaging a stripped-down version of VBA-M into a convenient library. This has saved me a significant amount of time, as I have not needed to isolate this from [Caitsith2/Zoopd's original player](https://caitsith2.com/gsf/) myself.
