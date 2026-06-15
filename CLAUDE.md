# MBEW

MBEW (pronounced "imbue", "WebM" backwards) is a C library for decoding WebM
files. It wraps nestegg (demuxer) and libvpx (VP8/VP9 codec), with vorbis/ogg
and opus bundled for audio. All dependencies are built as internal static
submodules — nothing is expected to be installed system-wide.

## Build

```bash
mkdir BUILD && cd BUILD
cmake_locald ..   # see ~/.bashrc for alias; sets CMAKE_INSTALL_PREFIX=~/local-NOASAN
make
```

The user installs all self-compiled software into `~/local-NOASAN` (or
`~/local-ASAN`), symlinked as `~/local`. CMake's `find_package()` picks this
up automatically via `CMAKE_PREFIX_PATH`. OSG in particular lives there.

## CMake notes

The build was modernized from CMake 2.x-era ALL_CAPS style to modern
target-based CMake (3.10+ minimum). Key things to know:

- All `ext/` submodules use `target_include_directories(... PUBLIC ...)` so
  include paths propagate transitively — no manual `get_directory_property`
  pulling needed at the top level.
- `ext/mbew-vorbis/CMakeLists.txt` pre-sets `OGG_FOUND`, `OGG_INCLUDE_DIRS`,
  and `OGG_LIBRARIES` before calling `add_subdirectory("vorbis")` to satisfy
  vorbis's internal `find_package(OGG)` without a system install.
- Assembly (libvpx) uses **yasm** (not nasm — nasm 2.15+ breaks the old
  x86inc.asm macros). Set via `CMAKE_ASM_NASM_COMPILER` before
  `enable_language(ASM_NASM)`.
- `-DPIC` is passed via `CMAKE_ASM_NASM_FLAGS` for PIE compatibility on modern
  Linux.
- C compiler `-m*` SIMD flags are gated with `$<COMPILE_LANGUAGE:C>` generator
  expressions so they don't get passed to the assembler.
- Optional examples (OSG, SDL2, Cairo, libao) use `find_package` /
  `pkg_check_modules` with `QUIET` and print a status message if skipped.

## Examples

| Example | Status | Notes |
|---|---|---|
| `mbew-example-c++` | ✅ | Basic C++ API iteration |
| `mbew-example-iterate` | ✅ | C iteration |
| `mbew-example-properties` | ✅ | Queries codec, dimensions, duration |
| `mbew-example-src-memory` | ✅ | In-memory source |
| `mbew-example-strings` | ✅ | Status string API |
| `mbew-example-video-cairo` | ✅ | Decodes every frame to PNG via Cairo |
| `mbew-example-video-osg` | ✅ | Live playback in an OSG window |
| `mbew-example-audio-sdl` | ✅ | Audio via SDL2 (include updated from SDL1) |
| `mbew-example-audio-ao` | ⚠️ | Skipped — libao not installed |
| `mbew-example-video-sdl` | ❌ | Uses SDL1-only APIs (SDL_SetVideoMode, SDL_CreateYUVOverlay); needs rewrite for SDL2 |

## OSG example specifics

The original example used Fixed Function Pipeline (FFP) which the user's OSG
build has disabled. A GLSL 3.30 shader pair was added:

- Vertex: transforms via `osg_ModelViewProjectionMatrix`, passes
  `osg_MultiTexCoord0` through as `texCoord`
- Fragment: samples a `sampler2DRect` (matching `osg::TextureRectangle`)

OSG automatically updates `osg_ModelViewProjectionMatrix` and binds
`osg_Vertex` (location 0) and `osg_MultiTexCoord0` (location 8) when FFP
is off. The `tex` uniform is explicitly set to texture unit 0.

The current playback approach is unoptimized (see Next Session below).

## Next session: modern GL4 video playback

The current OSG example does a full CPU→GPU texture upload every frame via
`image->dirty()`, and decodes synchronously on the render thread. The plan
for next session is to implement a proper modern GL4 playback pipeline:

### Goals

1. **Threaded decode** — move `mbew_iterate()` onto a dedicated thread. The
   mbew TODO list already calls this out (making iterate threadsafe, returning
   a unique `mbew_iter_t` per iteration). Feed decoded frames into a ring
   buffer shared with the render thread.

2. **Persistent-mapped PBO ring buffer** — allocate 3 PBOs with
   `glBufferStorage(GL_PIXEL_UNPACK_BUFFER, ..., GL_MAP_WRITE_BIT |
   GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT)`. Keep them permanently
   mapped. The decode thread writes directly into GPU-visible memory — no
   `glMapBuffer`/`glUnmapBuffer` round trips. Use `glFenceSync` to guard
   slot reuse.

   ```
   PBO[0] → uploading frame N
   PBO[1] → GPU consuming frame N-1
   PBO[2] → decoder writing frame N+1
   ```

3. **YUV upload + shader conversion** — instead of `MBEW_ITERATE_RGB`
   (CPU-side YUV→RGB), upload the raw YUV planes directly as three separate
   `GL_RED` / `GL_RG` textures and do BT.601/BT.709 matrix conversion in the
   fragment shader. Eliminates the CPU conversion cost and cuts upload
   bandwidth roughly in half.

4. **DSA (`glTextureStorage2D` / `glTextureSubImage2D`)** — no bind-to-modify;
   cleaner and more explicit than the OSG `osg::Image` dirty-flag approach.

### Key GL4 APIs

- `glBufferStorage` + `GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT`
- `glMapBufferRange` with `GL_MAP_UNSYNCHRONIZED_BIT`
- `glFenceSync` / `glClientWaitSync`
- `glTextureStorage2D` / `glTextureSubImage2D` (DSA)
- `glTexSubImage2D` to kick the PBO→texture DMA

### Note on `GL_TEXTURE_BUFFER`

`osg::TextureBuffer` / `GL_TEXTURE_BUFFER` is *not* the right tool here —
it's for binding a raw buffer as a 1D texel array for random access in
shaders (lookup tables, structured data), not for 2D image data.
