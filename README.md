# MBEW

MBEW (pronounced "imbue") is a tidy, intuitive, robust C library for decoding
WebM data. It is currently a very large work in progress, but I am confident
that it will becoming a leading WebM decoding--and possibly encoding--library.

# Submodules

MBEW currently requires the [NestEgg](https://github.com/kinetiknz/nestegg) and
[LibVPX](http://www.webmproject.org/code/) submodules. These can be added
with the standard:

    git submodule update --init

The MBEW build system will statically integrate these projects.

# Compilation

MBEW compilation is facilitated through CMake. In addition, custom
CMakeLists.txt files for each submodule used in MBEW have also been added. This
means that the dependencies themselves can be easily controlled and built along
with the toplevel library, and as such, easily embedded into the final shared
(or static) result.

# Major TODO Items

- Add Vorbis and Opus support.
- Create C++11 wrapper code.
- Support for preloading/caching and entire WebM stream for better performance.
- mbew_iter_t instances created within mbew_iterate() need to be aware of stream
  resets/seeks/etc. as they occur. One solution would be introducing either
  mbew_lock()/unlock() or mbew_iter_lock()/unlock().
- Implement MBEW_SRC_MEMORY.
- Enable nestegg logging.

