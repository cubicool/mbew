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
| `mbew-example-video-osg` | ✅ | Live playback in an OSG window. Upload modernized to a persistent-mapped PBO ring + DSA immutable texture storage (see below); still `osg::TextureRectangle`/`sampler2DRect`, decode still synchronous |
| `mbew-example-audio-sdl` | ✅ | Audio via SDL2 (include updated from SDL1) |
| `mbew-example-audio-ao` | ⚠️ | Skipped — libao not installed |
| `mbew-example-video-sdl` | ✅ | Rewritten for SDL3 (was SDL1-only: SDL_SetVideoMode, SDL_CreateYUVOverlay). Uploads mbew's YUV planes directly via SDL_UpdateYUVTexture against SDL_PIXELFORMAT_IYUV — no plane swap needed since mbew's [Y,U,V] order matches IYUV exactly |

## OSG example specifics

The original example used Fixed Function Pipeline (FFP) which the user's OSG
build has disabled. A GLSL 3.30 shader pair was added:

- Vertex: transforms via `osg_ModelViewProjectionMatrix`, passes
  `osg_MultiTexCoord0` through as `texCoord`
- Fragment: samples a `sampler2DRect` (matching `osg::TextureRectangle`)

OSG automatically updates `osg_ModelViewProjectionMatrix` and binds
`osg_Vertex` (location 0) and `osg_MultiTexCoord0` (location 8) when FFP
is off. The `tex` uniform is explicitly set to texture unit 0.

`createTexturedQuadGeometry`'s `(l, b, r, t)` texture coords are unnormalized
pixels for `TextureRectangle`, `(0,0)` at bottom-left. The `b`/`t` swap (top of
quad ↔ texture row 0) is required because mbew's RGB buffer is row-major
top-down while GL's texture convention is bottom-up — that part is correct.
There is no equivalent need to swap `l`/`r`: column order already agrees
between the buffer and the `s` coordinate. A prior version of this example
swapped `l`/`r` too, which mirrored the video horizontally.

## GL4 video playback upload (done 2026-06-16)

The old per-frame `image->dirty()` CPU→GPU upload (decoding synchronously on
the render thread) was replaced with a persistent-mapped PBO ring + DSA
immutable texture storage, still decoding synchronously — see "Next session"
below for what's still deferred.

- `MBEWUpdateCallback` → `MBEWDrawCallback` (an `osg::Drawable::DrawCallback`,
  not `UpdateCallback`). GL calls need a current context, which
  `osg::RenderInfo` guarantees during the draw traversal but the update
  traversal does not — relying on that previously only worked by accident
  under `SingleThreaded` viewer mode.
- 3-slot PBO ring, each `glBufferStorage`'d with `GL_MAP_WRITE_BIT |
  GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT` and mapped once via
  `glMapBufferRange`, kept mapped for the program's lifetime.
  `glFenceSync`/`glClientWaitSync` guard slot reuse.
- Immutable texture storage via `glTextureStorage2D` (DSA), uploaded per-frame
  via `glTexSubImage2D` reading from the bound `GL_PIXEL_UNPACK_BUFFER` (this
  build's OSG `GLExtensions` table exposes `glTextureStorage2D` but not
  `glTextureSubImage2D`/`glCreateTextures`/`glBindTextureUnit` — those calls
  use plain core GL instead of the extension dispatch table).
- **Texture type deliberately unchanged**: stayed on `osg::TextureRectangle`/
  `sampler2DRect` rather than switching to `Texture2D`/`sampler2D`. Discussed
  cross-project with `osgSlug` (lives at `~/dev/osgSlug`, used by the separate
  `~/dev/slughorn` project, which wants to eventually sample mbew's decoded
  frame as a HUD video-fill) — `osgSlug`'s `Atlas.cpp:499-503` already reserves
  texture units 0-4 for its own curve/band/gradient/msdf/effect textures, so a
  future video-fill binding should land on unit 5+ regardless of target type.
  The GL4 PBO/DSA mechanics don't care about `GL_TEXTURE_RECTANGLE` vs.
  `GL_TEXTURE_2D` — only the consuming shader's coordinate convention differs
  (unnormalized pixels + no mipmap/repeat, vs. normalized `[0,1]`). A future
  `osgSlug` binding will need its own `sampler2DRect` uniform + UV-by-texture-
  size scaling rather than being a drop-in alongside its other `sampler2D`
  units — a small, contained difference, not a redesign.
- **Two bugs hit and fixed during this pass, both worth remembering**:
  1. `drawable->osg::Drawable::drawImplementation(renderInfo)` (explicit
     base-class qualification) suppresses virtual dispatch — it calls
     `Drawable`'s empty stub instead of `Geometry`'s real draw call, so
     *nothing* renders, not even an unrelated debug shader. Fix: drop the
     qualifier (`drawable->drawImplementation(renderInfo)`) so the call
     dispatches virtually to the actual `Geometry` override.
  2. Leaving `GL_PIXEL_UNPACK_BUFFER` bound to our PBO after the upload
     corrupted `osgViewer::StatsHandler`'s text (its font-atlas glyph upload
     read garbage from our still-bound/mapped PBO instead of the real glyph
     bitmap — a one-time corruption that then persisted, since glyphs aren't
     re-uploaded once "successfully" rasterized). `GL_PIXEL_UNPACK_BUFFER` is
     a single global binding point, not scoped to one texture/unit; OSG's own
     `PixelDataBufferObject` (`src/osg/BufferObject.cpp`) always rebinds it to
     `0` right after use for exactly this reason. Fix: `glBindBuffer(
     GL_PIXEL_UNPACK_BUFFER, 0)` immediately after our own `glTexSubImage2D`
     call. General lesson: any raw GL call mixed into OSG-driven rendering
     must leave *global* (non-per-attribute) binding points back at their
     default, since OSG's own state-application code doesn't expect them to
     have moved.
- Verified: old (pre-PBO) vs. new approach showed near-identical total
  Update+Draw time on `data/reticle.webm` (a single-digit-KB/frame clip too
  light to show a real signal) — Update dropped, Draw rose by about the same
  amount, since the same upload work just moved from the update traversal to
  the draw traversal. GPU time was identical in both. The CPU→GPU transfer
  itself is unavoidable as long as decode is on the CPU; this pass doesn't
  reduce that cost, it lays groundwork (the ring + fence) that only pays off
  once decode is threaded (see below).

## Next session: threaded decode + YUV shader conversion

Two goals carried over from the original GL4 plan, deferred during the PBO/DSA
pass above (per the same `osgSlug`-side discussion: get single-threaded PBO
ring + DSA upload correct first, threaded decode is a separable follow-up):

1. **Threaded decode** — move `mbew_iterate()` onto a dedicated thread,
   mirroring `mbew-example-video-sdl-threaded.c`'s ring-buffer pattern. This
   is what makes the PBO ring's 3-slot/fence-sync machinery actually pay off:
   the decode thread can write frame N+1 into a free PBO slot while the GPU is
   still consuming frame N, instead of the upload sitting on the critical path
   synchronously like it does today.
2. **YUV upload + shader conversion** — instead of `MBEW_ITERATE_RGB`
   (CPU-side YUV→RGB), upload the raw YUV planes directly as three separate
   `GL_RED`/`GL_RG` textures and do BT.601/BT.709 matrix conversion in the
   fragment shader. Cuts upload bandwidth roughly in half and removes the CPU
   conversion cost. Optional polish per the `osgSlug`-side feedback, not a
   hard requirement — `osgSlug` doesn't care whether mbew resolves to RGB or
   exposes raw YUV planes, it just samples one final color either way.

### Note on `GL_TEXTURE_BUFFER`

`osg::TextureBuffer` / `GL_TEXTURE_BUFFER` is *not* the right tool here —
it's for binding a raw buffer as a 1D texel array for random access in
shaders (lookup tables, structured data), not for 2D image data.

## Future idea: GPU-side decode

Not currently feasible without a real architecture change: `libvpx` is a
software-only decoder (SIMD-optimized for CPU, no GPU offload path or backend
switch). True hardware decode would mean bypassing `libvpx` for an alternate,
platform-specific backend — VA-API (Linux/Intel/AMD, has VP8/VP9 decode
profiles and `vaExportSurfaceHandle` for EGL/GL texture interop with zero CPU
readback) or NVDEC (NVIDIA, would need CUDA↔GL interop) — with `libvpx`
staying as the universal software fallback. `nestegg` (the demuxer) is
unaffected either way; only the decode step would branch.

Worth studying **Bink Video** (RAD Game Tools/Epic Games Tools) as prior art
before attempting this — it's the de facto standard for video in shipped
games. Bink can't use fixed-function hardware decode blocks at all (those
only support standardized bitstreams, and Bink's format is proprietary), so
its "BinkGPU" mode instead splits the pipeline: CPU does the bitstream/entropy
decode, then motion compensation, transform/reconstruction, and YUV→RGB
conversion all run as GPU compute shaders. Published numbers: ~4ms CPU-only
for a 4K frame, ~1.4-2.3ms with GPU assist (roughly 2-4x). That's the same
idea as the YUV-shader-conversion goal above, just taken further — worth
revisiting once threaded decode lands, as inspiration for how much of the
post-bitstream-decode math could move to compute shaders rather than just
color conversion.
