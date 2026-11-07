# condemned2-nx

Buy me a coffee, and support my next project,
Thanks

> https://ko-fi.com/ipungsx
> https://saweria.co/ipungsx

<img width="800" height="480" alt="2026110710082100" src="https://github.com/user-attachments/assets/e3af2752-c414-478c-9e37-7be3f01568f5" />
<img width="800" height="480" alt="2026110710080500" src="https://github.com/user-attachments/assets/b8d33e5c-1f37-46df-af89-4dee5aa38af0" />

Condemned 2: Bloodshot (Xbox 360) on the Nintendo Switch as a homebrew NRO. It's not an emulator: the game's
PowerPC code is statically recompiled to C++ with [ReXGlue](https://github.com/rexglue/rexglue-sdk) v0.10.0 and
built for the Switch on [rexglue-sdk-nx](https://github.com/nx-mod/rexglue-sdk-nx) with a deko3d renderer.
The app shell follows [condemned-nx](https://github.com/ipungsx/condemnedCOx360-Port-Switch) (Condemned 1) and
[ds2-nx](https://github.com/VasilkovND/ds2-nx); everything tied to the game itself was redone from Condemned 2's
own `default.xex`.

> [!NOTE]
> You need your own copy of the game (the retail Xbox 360 disc). This repository contains no game data and no
> game code: the recompiled C++ is generated on your computer from your `default.xex`.

**Status: 0.2.1.** Runs on the console: menus, Bink videos, the 3D world and the video sky render correctly. This
release fixes the last known rendering bug (the right half of the screen darker in gameplay), raises the CPU and
handheld GPU clocks and adds a graphics options menu made for Condemned 2. Installation guide (in Indonesian):
[INSTALL.md](INSTALL.md). Every fix so far: [CHANGELOG.md](CHANGELOG.md).

## Fix history

| Version | Problem on the console | Fix |
|---|---|---|
| 0.2.1 | Few ways to trade image for frame rate | Motion blur and shadows options |
| 0.2.1 | Dark boxes and streaks on walls, right half darker and see-through; GPU stuck at 307.2 MHz | Light stencil clears of several rectangles kept stencil-only; scene rendered in one pass; GPU clock read back, 460.8 MHz fallback |
| 0.2.0 | Right half of the screen darker in gameplay | Right tile's depth resolved into the left tile's texture (`sdk-patches/0028`) |
| 0.2.0 | 25 to 40 fps, GPU busy ~95 % of each frame | CPU 1785 MHz, handheld GPU up to 768 MHz via sys-clk / Horizon OC (`sdk-patches/0029`) |
| 0.2.0 | Film grain option did nothing, no C2 effects in the menu | Effect constants found per shader by name: film grain, depth of field, sharpen |
| 0.1.3 | Videos yellow and red, video sky green and cyan | Vertex fetch matched to its own instruction (`sdk-patches/0027`) |
| 0.1.2 | 3D world not drawn | `k_10_11_11` packed normals decoded in the shader (`sdk-patches/0026`) |
| 0.1.1 | Shader library would not build | 1D textures as 2D, all 256 PS constants (`sdk-patches/0025`); build script fix |
| 0.1.0 | Earlier NRO: black screen, sound only | Clean port from C2's `default.xex`: recompile config, C2 shader hooks, shader dump and library |

## Options menu

Press **ZL + ZR + Minus** in game. Up / Down choose a row, Left / Right change it, A saves to `c2_opciones.toml`
next to the NRO, B closes.

| Row | Values |
|---|---|
| CPU | stock, 1224, 1428, 1581, 1785 MHz (default 1785), applied at once |
| GPU (handheld) | 307, 384, 460 MHz (Horizon profiles); 614, 768 MHz with sys-clk / Horizon OC (default 768, else 460); next start |
| Texture filtering | trilinear to 16x anisotropic; next start |
| Anti-aliasing | off, FXAA |
| Scene rendering | one pass (default) or two tiles as on the Xbox 360 |
| Motion blur | on, or off for frame rate (its velocity pass and blur left out) |
| Shadows | on, or off for frame rate (no stencil shadow volumes) |
| Film grain, depth of field, sharpen | the game's own passes, on or off |
| Brightness, gamma | output pass |
| Vibration, FPS counter | |

The game is GPU-bound in handheld, so the GPU row is the one that moves the frame rate without touching the image.
Higher clocks use more battery and run warmer. With sys-clk / Horizon OC the clocks are requested through its
override and released when the game exits normally; after a forced close they stay until changed in its overlay or
the console restarts.

## Condemned 2 specifics

| | |
|---|---|
| default.xex | Title ID 534507EF, version 0.0.0.1, built 2008-02-17, sha256 `554b2ae2...0310fd` |
| Archives | `Layer01.Arch02`, `Layer02.Arch02` |
| `D3DDevice_CreateVertexShader` / `CreatePixelShader` | `0x8265DF50` / `0x8265DE40` (`tools/find_shader_hooks.py`) |
| setjmp / longjmp | `0x8281FC40` / `0x8281F920` |
| Recompile hints | `recomp/condemned2_functions.toml` (2), `recomp/condemned2_leaf_functions.toml` (261 gap functions) |
| Predicated tiling | 1280x720 as two 2X EDRAM tiles (window offset -608); colour and depth of the right tile resolve into the left tile's texture at +608 (`sdk-patches/0020`, `0028`) |
| Options menu | ZL + ZR + Minus: CPU and handheld GPU clocks (sys-clk / Horizon OC override when present), film grain, depth of field, sharpen, FXAA, anisotropy, brightness, gamma |

## Layout

| Folder | Contents |
|---|---|
| `condemned2/` | The game project: CMake, Switch glue (`src/`), codegen manifest (`recomp/`), scripts (`tools/`), SDK patches |
| `shaders/` | Shader toolchain from ds2-nx: Xenos microcode -> HLSL -> SPIR-V -> shader library and DKSH cache |
| `sdk/` | **Submodule** -> `rexglue-sdk-nx` (`switch` branch, pinned at `f961303`) |
| `toolchain/` | `build_devkita64.sh`: devkitA64 from its GitHub sources, for machines that cannot reach devkitpro.org |
| `tools/switch/cmake/` | `switch-devkitA64.cmake`, the CMake toolchain file |

## Build

```
python3 condemned2/tools/stage_sd.py "Condemned 2 Bloodshot.iso" -o game   # extract your disc
condemned2/tools/codegen.sh game/default.xex                                # recompile it (~1-2 min)
condemned2/tools/build_linux.sh                                             # -> condemned2/out/sw/condemned2.nro
# run it once on the console (black screen is expected), copy shaders_dump/ back, then:
DKSH=1 condemned2/tools/build_shaders.sh shaders_dump shaders_out           # -> c2_shaders.dslib, shadercache_dksh.bin
```

## Credits

Everything here stands on ds2-nx (VasilkovND) and the projects it credits: ReXGlue (Tom Clay and contributors,
built on Xenia), rexglue-sdk-nx (nx-mod), NFSMW-NX (StevensND), XenosRecomp (hedge-dev), SwitchWakerHD's uam
(centollOS), devkitPro and switchbrew, Atmosphère, FFmpeg. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## Legal

Unofficial fan port with no affiliation to SEGA, Monolith Productions or Warner Bros. "Condemned" is a trademark of
its owner. You provide your own legally obtained copy of the game. No prebuilt files are distributed: the NRO contains
code recompiled from the game, so it is made on your own computer from your copy. Running homebrew requires custom
firmware, which violates Nintendo's terms and can get a console banned.

Source code is under the GPL-3.0 license (see [LICENSE](LICENSE)), as ds2-nx. The SDK changes are under the SDK's
BSD-3-Clause license, and the shader translator changes under MIT.
