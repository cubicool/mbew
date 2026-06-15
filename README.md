# MBEW

MBEW (pronounced "imbue") is a robust, API-rich C library for decoding WebM
data. It's also the word "WebM" backwards, which is *ultra-clever*.

This library was created with sponsorship by
[AlphaPixel](https://alphapixel.com).

# Table Of Contents

  * [AI Disclosure](#ai-disclosure)
  * [Quickstart](#quickstart)
    * [Naive Synchronization](#naive-sychronization)
    * [Less Naive Synchronization](#less-naive-synchronization)
  * [Compilation](#compilation)
    * [Submodules](#submodules)
  * [TODO](#todo)

# AI Disclosure

On June 15, 2026--after having finally jumped whole-hog into the AI/LLM
mindset--I "turned Claude loose" on this library! It helped me fix a number of
small bugs (from TEN YEARS LATER!), and modernize the CMake setup. There will
likely be a *LOT* of updates in the coming days as he (it? how **does** Claude
"identify?" :)) helps clean up the rough edges. I'll include the `CLAUDE.md` so
others can jump in as well!

## AI Assessment

> I'm going to have `claude` itself insert an "assessment" of the state of
> things. I *do* have the CLI on "friendly" mode for my own sanity, but we'll
> see how it goes...

The core library is in remarkably good shape for its age. The public API is
clean and well thought-out: the iteration model with optional `MBEW_ITERATE_SYNC`
and `MBEW_ITERATE_RGB` flags is genuinely ergonomic, and the nanosecond-based
timing design aged well. The C++ wrapper (`mbew.hpp`) is a nice touch that
makes the OSG example read naturally.

The main areas that need attention are in the *integration* layer rather than
the decode layer itself. The current approach to feeding decoded frames to a
renderer — converting YUV→RGB on the CPU and uploading the full frame every
tick — works, but leaves significant performance on the table. A persistent-
mapped PBO ring buffer with YUV-plane upload and shader-side color conversion
would be the modern replacement, and the existing `mbew_iter_yuv_planes()` /
`mbew_iter_yuv_stride()` API already exposes exactly what's needed for that.
The other gap is thread safety: `mbew_iterate()` is single-threaded by design,
which means the decode and render loops are coupled. The fix (returning a
unique `mbew_iter_t` per call) is already on the TODO list and would unlock
the threaded decode pipeline that a proper video player needs.

In short: solid foundation, the rough edges are all at the boundary between
the library and the outside world.

> I expected `claude` to join in on the "how does Claude identify?" joke, but
> it/he/other did not. :) Maybe next update.

# Quickstart

The first thing most programmers will want to do when experimenting with the library
is open a WebM file and quickly loop through all of the data therein. Ignoring the
burden of any error-checking, something like this would work:

```c
mbew_t m = mbew_create(MBEW_SOURCE_FILE, "/path/to/file.webm");

while(mbew_iterate(m, MBEW_ITERATE_RGB)) {
    if(mbew_iter_type(m) != MBEW_DATA_VIDEO) continue;

    mbew_bytes_t rgb = mbew_iter_rgb(m);

    /* DO STUFF! */
}

mbew_destroy(m);
```

The code above opens a WebM file from disk and immediately begins iterating over
it, frame-by-frame, skipping any iteration that isn't video data. By passing the
additional *MBEW_ITERATE_RGB* flag we instruct the library to convert each frame
from the default YUV420 format into a simple RGBA color format, which can
sometimes simplify interaction with other libraries. However, without any
synchronization logic, the `mbew_iterate()` loop runs as fast as the system
will allow it: too fast!

### Naive Synchronization

In our next example we will add some very basic error-checking, as well as
demonstrating a (naive) approach to frame synchronization using the
`MBEW_ITERATE_SYNC` flag. To keep things simple, we'll assume the existence of a
pseudo-function called `nanoseconds()` that returns the elapsed lifetime total
for the current process:

```c
mbew_t m = mbew_create(MBEW_SOURCE_FILE, "/path/to/file.webm");

/* Any status except MBEW_STATUS_SUCCESS (0) indicates a failure has occurred
 * somewhere within the mbew_t context. */
if(mbew_valid(m)) {
    /* Grab a base "starting" time from our fake function. */
    mbew_ns_t start = nanoseconds();

    /* This time we will ensure that only video data is yielded via the
     * MBEW_ITERATE_VIDEO flag. Furthermore, with the introduction of the
     * MBEW_ITERATE_SYNC flag, mbew_iterate()'s internal logic changes so that
     * new frames are only advanced when the elapsed time (provided by our fake
     * function) equals or exceeds the timestamp of the pending frame. This
     * condition being met is communicated to the caller via the return value of
     * mbew_iter_sync(). */
    while(mbew_iterate(m, MBEW_ITERATE_VIDEO | MBEW_ITERATE_SYNC)) {
        if(!mbew_iter_sync(m, nanoseconds() - start)) continue;

        /* Otherwise, DO STUFF! */
    }
}

else printf("Error: %s\n", mbew_string(mbew_status(m)));

mbew_destroy(m);
```

The 2nd example *still* calls the `mbew_iterate()` loop as fast as possible, but
now we can be sure that the internal state of the iteration won't be updated
until the pending frames timestamp is exceeded.

### Less Naive Synchronization

So, how would we save even more cycles in our core loop? Well, there are
numerous ways--mostly depending on whether you need blocking or non-blocking
input--but let's try this small change, once again using a psudeo-function that
will "sleep" our process:

```c
mbew_ns_t start = nanoseconds();

while(mbew_iterate(m, 0)) {
    /* Now instead of "peeking" into the stream and seeing whether it's
     * appropriate to continue we simply retrieve the number of nanoseconds
     * between the the current time and the pending frame. */
    mbew_ns_t now = nanoseconds() - start;
    mbew_ns_t until = mbew_iter_next(m, now));

    nanosleep(until);

    /* DO STUFF! */
}
```

Take a look at the [examples](examples/) included with libmbew for
more information.

# Compilation

MBEW compilation is facilitated through CMake (3.10+). Custom CMakeLists.txt
files for each submodule use modern target-based CMake, so all include paths
and link dependencies propagate transitively — no manual configuration needed.

On Linux you will also need **yasm** installed (`sudo apt install yasm`). The
libvpx assembly code is not compatible with NASM 2.15+ due to stricter opcode
checking in newer versions; yasm handles the old `x86inc.asm` macro style
without issues.

```bash
mkdir BUILD && cd BUILD
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## Submodules

MBEW bundles all of its dependencies as git submodules:

- [NestEgg](https://github.com/kinetiknz/nestegg) — WebM demuxer
- [LibVPX](http://www.webmproject.org/code/) — VP8/VP9 decoder
- [libogg](https://gitlab.xiph.org/xiph/ogg) — Ogg container (required by Vorbis)
- [libvorbis](https://gitlab.xiph.org/xiph/vorbis) — Vorbis audio decoder
- [Opus](https://gitlab.xiph.org/xiph/opus) — Opus audio decoder

Initialize them all with:

    git submodule update --init

The MBEW build system will statically integrate all of these projects.

# TODO

- ~~Add Vorbis and Opus support.~~ *(submodules integrated)*
- Support for preloading/caching an entire WebM stream for better performance.
- Make `mbew_iterate()` threadsafe; this will involve having it return a unique
  `mbew_iter_t` instance (another private implementation) per iteration.
- Implement a modern GL4 playback pipeline: persistent-mapped PBO ring buffer,
  threaded decode, and YUV-plane upload with shader-side color conversion.
- Rewrite `mbew-example-video-sdl` for SDL2 (the original used SDL1-only APIs
  that were removed: `SDL_SetVideoMode`, `SDL_CreateYUVOverlay`, etc.).
- Choose tags for each submodule and use them (as opposed to simply using
  whatever was in the pull at the time).
- Implement what `play -t raw -r 48k -e signed -b 16 -c 1 audio.pcm` does.
