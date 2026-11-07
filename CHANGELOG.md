# Changelog

Every fix made while bringing Condemned 2: Bloodshot up on the Switch, newest first. Each entry says what was seen on
the console, what caused it and where the fix lives. SDK fixes are patches in `condemned2/sdk-patches/`, applied in
order on top of `rexglue-sdk-nx`.

## 0.2.2 (testing)

### Resolution option
- **RESOLUTION** (`c2_resolucion`): 720P (default), 630P or 540P. Below 720 the 3D scene and its post-processing
  are drawn at 1120x630 or 960x540 into the corner of their target, and every resolve from it is stretched back to
  1280x720 (bilinear for color, nearest for depth), so all of the game's textures keep their sizes. The HUD is part
  of the scene and gets softer too; the options menu is drawn at full resolution.
- Needs SCENE RENDERING = ONE PASS (choosing 630P or 540P turns it on).
- `sdk-patches/0030-internal-render-scale.patch`: `FijarEscalaRender` in the renderer, scaled pass framing, scaled
  color and depth copies on the 3D engine, scaled partial clears.

## 0.2.1

Confirmed on the console: the half-screen seam, the dark boxes and the see-through walls are gone. The NRO shows
version 0.2.1 and author ipungsx in the Homebrew Menu.

### New options for frame rate
- **MOTION BLUR** (`c2_desenfoque_movimiento`): off leaves out the velocity pass (the whole scene drawn a second time
  at 320x180, about 270 draws a frame) and the blur after it; the velocity buffer is cleared to "no motion".
- **SHADOWS** (`c2_sombras`): off leaves out the stencil shadow volumes; no dynamic shadows.
- Both are on by default, so the default image is unchanged.

### Dark boxes and shadow streaks on walls, right half darker (root cause)
- **Seen:** with the one-pass build the seam was gone, but dark see-through boxes with streaks showed on the walls;
  before it, the right half was darker and showed structures through walls.
- **Cause:** before a light's shadow volumes Direct3D clears the stencil in that light's box only. When the box
  crosses a tile edge the clear is cut into several rectangles (12 vertices, depth-only mode, depthcontrol 8701) and
  it is predicated for the tile it falls in. That shape was not recognised as Direct3D's clear and was emulated as a
  whole clear of depth *and* stencil: in the middle of the right tile's lights its depth was wiped, so the lights
  after it failed their depth test (darker) and later draws went over the walls. In the one-pass build the clear
  predicated for the right tile only was left out, and the light drew over the stencil of the previous one.
- **Fix:** a stencil-only clear of any number of rectangles is emulated as a stencil-only clear, and in one pass the
  stencil clears predicated for the second tile are done as well (`src/c2_gpu.cpp`).

### Right half still darker, walls see-through on the right (other areas)
- **Seen:** after 0.2.0 the edge moved from x = 672 to x = 640, and in other places the right half also showed
  structures through walls, like trails of earlier frames.
- **Cause:** the scene is still rendered as two tiles that are put together later; every pass of the second tile
  depends on textures the first one wrote, and any difference shows as a seam.
- **Fix:** the scene is rendered in one pass. The first tile is widened to the whole screen (surface pitch and window
  scissor 1280, its copies cut at the tile edge go on to the right edge) and the second tile's draws and copies are
  skipped (`src/c2_gpu.cpp`, `MosaicoAplicar`). Half the draws of the scene, so also faster. The options menu row
  "SCENE RENDERING" switches back to two tiles (`c2_mosaico_unico = false`).

### GPU stuck at 307.2 MHz with Horizon OC (18 fps)
- **Cause:** 0.2.0 trusted the clock sysmodule's answer: the override for 768 MHz was accepted but not applied, and
  Horizon's own 460 MHz profile had been skipped.
- **Fix:** the 460 MHz profile is always applied first; the GPU is read back once per second and, if the sysmodule has
  not set what it accepted after three seconds, the override is released and the GPU is held at 460.8 MHz through
  clkrst (`sdk-patches/0029`, `src/c2_app.h`).

## 0.2.0

### Right half of the screen darker in gameplay
- **Seen:** during gameplay the left half of the picture was brighter than the right one, with a hard vertical edge
  at x = 672.
- **Cause:** Condemned 2 renders 1280x720 as two 2X EDRAM tiles (predicated tiling, window offset -608) and resolves
  each tile's colour *and* depth into one texture, moving the copy base to the tile's position. Colour already went
  into the left tile's texture at +608 (`0020`), but depth did not: the right tile's depth landed in a texture of its
  own, the left tile's depth texture kept "far" from x = 672 on, and the post-processing that reads it (fog, depth of
  field) treated the whole right half as infinitely far away.
- **Fix:** `0028-depth-resolve-tiles-into-their-texture.patch`. Colour and depth share one helper (`BaseDeMosaico`);
  a stale texture left at the tile's address (another level, the menus) no longer blocks it.

### Performance
- **Finding:** in gameplay the GPU is the limit, not the CPU: at the stock handheld 460.8 MHz it is busy about 95 % of
  every frame (23 to 37 ms), which caps the game at roughly 25 to 40 fps.
- **Handheld GPU clock** (`c2_gpu_portatil_mhz`): 384 and 460 MHz through Horizon's own performance profiles, 614 and
  768 MHz (the console's docked rate) through the override of sys-clk / Horizon OC when one is installed. Default
  768; without a clock sysmodule it stays at 460. The request is lifted while the console is in the dock.
- **CPU clock** (`c2_cpu_mhz`): default 1785 MHz, the console's own boost rate. Through the sys-clk / Horizon OC
  override when present (a rate set with clkrst alone is rewritten by the sysmodule on its next tick), through clkrst
  otherwise, watched once per second and given back at exit. More headroom for the audio threads too.
- **Fix:** `0029-cpu-and-handheld-gpu-clock-requests.patch`, `src/c2_app.h`.

### Graphics options menu adapted to Condemned 2
- ZL + ZR + Minus. Rows: CPU, GPU (handheld), texture filtering, anti-aliasing (FXAA), film grain, depth of field,
  sharpen, brightness, gamma, vibration, FPS counter. Saved to `c2_opciones.toml` next to the NRO.
- **Film grain did nothing:** the row still zeroed Condemned 1's register (PS c1). Condemned 2 keeps `k_fFilmGrain`
  in c226 (composite pass with bleach bypass) or c219 (grain-only pass). The effect constants are now found by name
  in each pixel shader's own constant table and zeroed per draw, so the pass still runs and leaves the image as it is.
- **New rows:** depth of field (`k_fDepthOfField`, `k_fDepthOfField_Near`) and sharpen (`k_fSharpen`).
- **Removed:** "volumetric light", a Condemned 1 effect that Condemned 2 does not have.
- **Fix:** `src/c2_menu.cpp`, `src/c2_gpu.cpp`.

## 0.1.3

### Videos yellow and red, video sky green and cyan
- **Seen:** every Bink video (intros, in-game screens) and the sky, which is a Bink video texture, came out in
  saturated yellow, red, green or cyan.
- **Cause:** Direct3D patches each vertex fetch of a vertex shader in place. The renderer looked the patched fetch up
  by destination register and write mask only, and the Bink quad fetches its position and then its colour into the
  same register with the same mask, so the colour input received the position (x, y in pixels, z = 1).
- **Fix:** `0027-vfetch-own-instruction.patch`: the element's own instruction is checked first.
- Several other hypotheses were tested and ruled out on the console before this (texture fingerprints, descriptor
  cache, R8 planes, signed formats); the full-texture-hash default added during that search was reverted.

## 0.1.2

### 3D world not drawn
- **Seen:** menus and videos played, the 3D world was missing.
- **Cause:** Condemned 2 packs its normals and tangents as `k_10_11_11` vertices (format 16), which the renderer did
  not take: every world draw was rejected (cause 316).
- **Fix:** `0026-vertex-10-11-11.patch`: the attribute is fetched as a raw 32-bit word and decoded in the shader
  (`decode101111` in XenosRecomp), signed, unsigned, normalised or integer as the fetch says.

## 0.1.1

### Shader library would not build
- **1D textures:** DXC stopped on undeclared `Texture1D` descriptors. 1D fetches are now 2D fetches at
  `(u, 0.5)` in XenosRecomp.
- **Pixel shader constants:** uam refused shaders indexing past c223. The constant block now holds all 256 PS
  constants (`0025-ps-constants-256.patch`).

### Build
- `build_linux.sh` still passed the Condemned 1 name `CCO_GEN_DIR`, so configure failed; now `C2_GEN_DIR`.

## 0.1.0

### Black screen with sound only (earlier NRO)
- **Cause:** the shader hooks pointed at Condemned 1's addresses, so Condemned 2's shaders were never picked up and
  nothing on screen could be drawn although the game ran.
- **Fix:** a clean port made from Condemned 2's own `default.xex` (Condemned 1's port used only as a reference):
  - recompile configuration: 2 function size hints, 261 gap functions, setjmp `0x8281FC40` and longjmp `0x8281F920`;
  - shader hooks on Condemned 2's own `D3DDevice_CreateVertexShader` (`0x8265DF50`) and `CreatePixelShader`
    (`0x8265DE40`), found by `tools/find_shader_hooks.py`; the first run dumps every shader to `shaders_dump/`, and
    `tools/build_shaders.sh` turns them into `c2_shaders.dslib` and the DKSH cache;
  - disc layout (`Layer01.Arch02`, `Layer02.Arch02`, `default.archcfg`), icon from the XEX, SD staging script;
  - devkitA64 built from its GitHub sources for machines that cannot reach devkitpro.org.
