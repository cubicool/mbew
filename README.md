# MBEW

MBEW (pronounced "imbue") is a tidy, intuitive, robust C library for decoding
WebM data. It is currently a very large work in progress, but I am confident
that it will becoming a leading WebM decoding--and possibly encoding--library.


# Submodules

MBEW currently requires the [NestEgg](https://github.com/kinetiknz/nestegg) and
[LibVPX](http://www.webmproject.org/code/) submodules. These can be added
with the standard:

  git submodule update --init

Subsequent to checking these modules out, the MBEW build system will statically
integrate their code.

# Major TODO Items

- Add Vorbis and Opus support.
- Add CMake support.
- Create C++11 wrapper code.
- Support for preloading/caching and entire WebM stream for better performance.

