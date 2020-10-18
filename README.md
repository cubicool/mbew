# MBEW

MBEW (pronounced "imbue") is a robust and easy-to-use C library for
encoding and decoding WebM data. It's also the word "WebM" backwards.

# Quickstart

The first thing most programmers will want to do when experimenting with the
library is open a WebM file and quickly loop through all of the juicy data
therein. Ignoring error-checking, something like this would work:

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
it frame-by-frame. The optional *MBEW_ITERATE_RGB* flag passed to
`mbew_iterate()` instructs the library to convert and expose the video data as
simple, single-byte RGB color values (as opposed to YUV420, the default). We
check the "type" of each frame during iteration and ignore anything that isn't
strictly video frame data. However, without any synchronization logic, the
`mbew_iterate()` loop runs as fast as the system will allow it; too fast!

## Synchronization

*TODO*: Discuss how the mbew_t context manages iteration "state."

### Naive Synchronization

In our next example we will add some very basic error-checking, as well as
demonstrating a (naive) approach to frame synchronization using the
`MBEW_ITERATE_SYNC` flag. To keep things simple, we'll assume the existence of a
pseudo-function called `nanoseconds()` that returns the elapsed lifetime total
for the current process:

```c
mbew_t m = mbew_create(MBEW_SOURCE_FILE, "/path/to/file.webm");

/* Any status execpt MBEW_STATUS_SUCCESS (0) indicates a failure has occurred
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

This approach *still* calls the `mbew_iterate()` loop as fast as possible, but
now we can be sure that the internal state of the current iteration won't be updated
until the pending frame's timestamp is exceeded.

### Less Naive Synchronization

So, how would we save even more cycles in our core loop? Well, there are
numerous ways--mostly depending on whether you need blocking and non-blocking
input--but let's try this small change, once again using a pseudeo-function that
will "sleep" our process:

```c
mbew_ns_t start = nanoseconds();

while(mbew_iterate(m, 0)) {
    /* Now instead of "peeking" into the stream and seeing whether it's
     * appropriate to continue we simply retrieve the number of nanoseconds
     * between the the current elapsed time and the pending frame. */
    mbew_ns_t now = nanoseconds() - start;
    mbew_ns_t until = mbew_iter_next(m, now));

    nanosleep(until);

    /* DO STUFF! */
}
```

Take a look at the [examples](tree/master/examples/) included with libmbew for
more information!

# Compilation

MBEW compilation is facilitated through CMake. In addition, custom
CMakeLists.txt files for each submodule used in MBEW have also been added. This
means that the dependencies themselves can be easily controlled and built along
with the toplevel library, and as such, easily embedded into the final shared
(or static) result.

## Submodules

MBEW currently requires the [NestEgg](https://github.com/kinetiknz/nestegg) and
[LibVPX](http://www.webmproject.org/code/) submodules. These can be added
with the standard:

    git submodule update --init

The MBEW build system will statically integrate these projects.

# Major TODO Items

- Add Vorbis and Opus support.
- Support for preloading/caching and entire WebM stream for better performance.
- Make mbew_iterate() threadsafe; this will involve having it return a unique
  mbew_iter_t instance (another private implementation) per iteration.
- Choose tags for each submodule and use them (as opposed to simply using
  whatever was in the pull at the time).
- Hide the CMake "creep" that comes from the Ogg and Vorbis external submodules.
- Implement what `play -t raw -r 48k -e signed -b 16 -c 1 audio.pcm` does.

