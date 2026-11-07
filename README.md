# condemned2-nx

Buy me a coffee, and support my next project,
Thanks
https://saweria.co/ipungsx

Condemned 2: Bloodshot (Xbox 360) on the Nintendo Switch as a homebrew NRO. It's not an emulator: the game's
PowerPC code is statically recompiled to C++ with [ReXGlue](https://github.com/rexglue/rexglue-sdk) v0.10.0 and
built for the Switch on [rexglue-sdk-nx](https://github.com/nx-mod/rexglue-sdk-nx) with a deko3d renderer.
The app shell follows [condemned-nx](https://github.com/ipungsx/condemnedCOx360-Port-Switch) (Condemned 1) and
[ds2-nx](https://github.com/VasilkovND/ds2-nx); everything tied to the game itself was redone from Condemned 2's
own `default.xex`.

> [!NOTE]
> You need your own copy of the game (the retail Xbox 360 disc). This repository contains no game data and no
> game code: the recompiled C++ is generated on your computer from your `default.xex`.

**Status: 0.1.0, bring-up, not yet tested on hardware.** The recompile is clean (no unresolved branches) and the
shader hooks are verified against the machine code. The renderer still comes from the Condemned 1 port, so some
effects may need fixes once logs from the console come in. Installation guide (in Indonesian): [INSTALL.md](INSTALL.md).

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
