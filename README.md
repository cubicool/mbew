# MBEW

MBEW (pronounced "imbue") is a tidy, intuitive, robust C library for decoding
WebM data. It is currently a very large work in progress, but I am confident
that it will becoming a leading WebM decoding--and possibly encoding--library.

# Quickstart

The first thing most people will want to do when experimenting with the library
is open a WebM and quickly loop through all of the juicy data it provides.
Without much error-checking, something like:

```c
mbew_t m = mbew_create(MBEW_SRC_FILE, "/path/to/file.webm");

while(mbew_iterate(m, MBEW_ITER_FORMAT_RGB)) {
    if(mbew_iter_type(m) != MBEW_DATA_VIDEO) continue;

    mbew_bytes_t rgb = mbew_iter_video_rgb(m);

    /* ... */
}

mbew_destroy(m);
```

The code above opens (we assume!) a WebM file from disk and immediately begins
iterating over it, frame-by-frame, passing the additional *MBEW_ITER_FORMAT_RGB*
flag to make sure the library provides the data in a trivial format. We check
the "type" of each frame during iteration, and ignore anything that isn't video
data. However, without any synchronization logic, the `mbew_iterate()` loop runs
as fast as the system will allow it; too fast!

Now let's introduce some basic error-checking, in addition to some other
cleanup. To keep things simple, we'll be using a pseudo-function called
`nanoseconds` that returns the elapsed process running time (in nanoseconds, of
course!):

```c
mbew_t m = mbew_create(MBEW_SRC_FILE, "/path/to/file.webm");
mbew_status_t status;

/* Any status execpt MBEW_STATUS_SUCCESS (0) indicates a failure has
 * occurred somewhere within the mbew_t context. */
if(!(status = mbew_status(m))) {
    /* Grab a base "starting" time from out fake function. */
    mbew_ns_t start = nanoseconds();

    /* Begin iterating, but this time we will enfore that only video data is
     * yielded to us via MBEW_ITER_VIDEO_ONLY. Furthermore, with the
     * introduction of the MBEW_ITER_SYNC flag, mbew_iterate()'s internal
     * logic changes so that new frames are only advanced when the elapsed
     * time (provided by our fake function) equals or exceeds the timestamp
     * of the pending frame. This condition being met is communicated to the
     * caller via the return value of mbew_iter_sync(). */
    while(mbew_iterate(m, MBEW_ITER_VIDEO_ONLY | MBEW_ITER_SYNC)) {
        if(!mbew_iter_sync(m, nanoseconds() - start)) continue;

        /* Otherwise, DO STUFF! */
    }
}

else printf("Error: %s\n", mbew_string(MBEW_TYPE_STATUS, status));

mbew_destroy(m);
```

The 2nd example *still* calls the mbew_iterate() loop as fast as possible, but
now we can be sure the internal state of the iteration won't be updated until
the frames timestamp is exceeded.

So, how would we save even more cycles in our core loop? Well, there are
numerous ways--mostly depending on whether you need blocking and non-blocking
input--but let's try this small change, once again using a pseudeo-function that
will "sleep" our process:

```c
mbew_ns_t start = nanoseconds();

while(mbew_iterate(m, 0)) {
    /* Now instead of "peeking" into the stream and seeing whether it's
     * appropriate to continue we simply retrive the number of nanoseconds
     * between the the current elapsed time and the pending frame. */
    mbew_ns_t now = nanoseconds() - start;
    mbew_ns_t until = mbew_iter_next(m, now));

    nanosleep(until);

    /* DO STUFF! */
}
```

Take a look at the [examples](tree/master/examples/) included with libmbew for
more information!

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

