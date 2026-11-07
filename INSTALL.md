# condemned2-nx: Condemned 2: Bloodshot (Xbox 360) di Nintendo Switch

Port homebrew `.nro` dari **Condemned 2: Bloodshot** dengan rekompilasi statis ReXGlue v0.10.0. Proyek
condemned-nx (Condemned 1) hanya dipakai sebagai acuan kerangka aplikasi. Semua bagian yang terikat pada game
dibuat ulang dari `default.xex` Condemned 2: daftar fungsi, setjmp/longjmp, hook shader, berkas arsip, dan ikon.

## Yang berbeda dari Condemned 1

| Bagian | Condemned 1 | Condemned 2 (proyek ini) |
|---|---|---|
| default.xex | Title ID 534507D2, v0.0.0.2 | Title ID 534507EF, v0.0.0.1, SHA-256 `554b2ae2...0310fd` |
| Arsip data | `Layer0.Arch00`, `Layer1.Arch00` | `Layer01.Arch02`, `Layer02.Arch02` |
| Hook `D3DDevice_CreateVertexShader` | `0x826D2780` | `0x8265DF50` |
| Hook `D3DDevice_CreatePixelShader` | `0x826D25F0` | `0x8265DE40` |
| setjmp / longjmp | `0x82B0D9E0` / `0x82B0D790` | `0x8281FC40` / `0x8281F920` (tidak dipanggil, aman) |
| Fungsi tambahan | 3 + 1.808 potongan gap | 2 fungsi eksplisit + 261 potongan gap |
| Pustaka shader | `cco_shaders.dslib` | `c2_shaders.dslib` |
| Folder SD | `sdmc:/switch/condemned-nx/` | `sdmc:/switch/condemned2-nx/` |

**Penyebab layar hitam pada NRO sebelumnya.** Hook shader adalah satu-satunya jalan bagi renderer deko3d untuk
mengenali shader game. Bila hook dipasang pada alamat Condemned 1, fungsi yang dibungkus bukan fungsi pembuat
shader, sehingga tidak ada shader yang direkam maupun didaftarkan. Game tetap berjalan (suara keluar), tetapi
tidak ada yang digambar. Proyek ini memakai alamat milik Condemned 2, yang ditemukan dengan
`tools/find_shader_hooks.py`.

## Mengambil repositori

```
git clone --recursive https://github.com/ipungsx/condemned2RecompSwitch.git condemned2-nx
cd condemned2-nx
```

Kebutuhan komputer sama dengan port Condemned 1: Linux, `clang` 19 atau lebih baru, `cmake`, `ninja`, `python3`,
`git`, devkitPro (`switch-dev`, `deko3d`), serta `dxc` dan `spirv-val` untuk pustaka shader.

## Langkah 1. Folder game

Folder game yang sudah Anda ekstrak (`default.xex`, `default.archcfg`, `autoexec.cfg`, `Layer01.Arch02`,
`Layer02.Arch02`, `game/`, `$systemupdate/`) bisa langsung dipakai. Dari ISO:

```
python3 condemned2/tools/stage_sd.py "Condemned 2 Bloodshot.iso" -o game
```

## Langkah 2. Rekompilasi XEX (opsional bila memakai zip generated)

```
condemned2/tools/codegen.sh game/default.xex
```

Hasilnya 263 berkas C++ (sekitar 136 MB) di `condemned2/recomp/generated/default/`. Codegen harus selesai tanpa
satu pun pesan `Unresolved`.

## Langkah 3. Build NRO

```
condemned2/tools/build_linux.sh
```

Hasil: `condemned2/out/sw/condemned2.nro`. Bila muncul pesan `sdk/ has changes`, jalankan `git -C sdk checkout -- .`
lalu ulangi.

## Langkah 4. Mengambil shader dari konsol (wajib, sekali saja)

1. Salin `condemned2.nro` ke `sdmc:/switch/condemned2-nx/condemned2-nx.nro`, dan folder game ke
   `sdmc:/switch/condemned2-nx/game/`.
2. Jalankan sebuah game sambil **menahan R**, lalu buka Condemned 2 dari Homebrew Menu (title takeover).
3. **Layar hitam pada tahap ini memang wajar** karena pustaka shader belum ada. Tunggu 2 sampai 3 menit, lalu
   tutup dengan HOME. Kalau bisa, tekan A atau Start beberapa kali supaya game masuk menu dan membuat lebih banyak
   shader.
4. Salin `sdmc:/switch/condemned2-nx/shaders_dump/` ke komputer. Kalau folder ini kosong atau tidak ada, hook
   shader tidak terpanggil: kirim folder `logs/`.

## Langkah 5. Pustaka shader

```
DXC=$(command -v dxc) VAL=$(command -v spirv-val) DKSH=1 condemned2/tools/build_shaders.sh shaders_dump shaders_out
```

Hasil: `shaders_out/c2_shaders.dslib` dan `shaders_out/shadercache_dksh.bin`. Shader yang gagal dicatat di
`shaders_out/err/`.

## Langkah 6. Kartu SD

```
sdmc:/switch/condemned2-nx/
    condemned2-nx.nro
    c2_shaders.dslib
    shadercache_dksh.bin
    game/   default.xex  default.archcfg  autoexec.cfg  Layer01.Arch02  Layer02.Arch02  game/  $systemupdate/
```

Setelah game berjalan lebih jauh (level baru, efek baru), shader yang belum ada di pustaka tetap direkam ke
`shaders_dump/`. Gabungkan dengan dump lama, lalu ulangi langkah 5.

## Log

Log ada di `sdmc:/switch/condemned2-nx/logs/`. Baris yang perlu diperhatikan:

- `[shaders] library ...: N shaders` berarti pustaka terbaca.
- `object ... no pointer to its microcode` berarti struktur objek shader berbeda dari dugaan; kirim lognya.
- `Call to invalid or unregistered function at guest address 0x...` berarti ada fungsi yang belum dikenali
  codegen. Tambahkan alamatnya ke `recomp/condemned2_functions.toml`, lalu ulangi langkah 2 dan 3.

## Menu opsi grafis

Tekan **ZL + ZR + Minus** saat bermain. Atas/Bawah memilih baris, Kiri/Kanan mengubah nilai, A menyimpan ke
`c2_opciones.toml` di samping NRO, B menutup menu. Baris bertanda `!` baru berlaku setelah game dijalankan ulang.

| Baris | Isi |
|---|---|
| CPU | STOCK, 1224, 1428, 1581, 1785 MHz (bawaan 1785, laju boost resmi konsol). Langsung berlaku. |
| GPU (HANDHELD) | 307, 384, 460 MHz lewat profil Horizon; 614 dan 768 MHz lewat sys-clk / Horizon OC (bawaan 768). Tanpa sysmodule tersebut tetap 460 MHz. Di dock selalu 768 MHz. |
| TEXTURE FILTERING | Trilinear sampai 16x anisotropik. |
| ANTI-ALIASING | Mati atau FXAA. |
| FILM GRAIN | Butiran film dari pass komposit game. |
| DEPTH OF FIELD | Blur kedalaman bidang dari game. |
| SHARPEN | Pass penajaman dari game. |
| BRIGHTNESS, GAMMA | Koreksi di pass keluaran. |
| VIBRATION, FPS COUNTER | Getaran dan penghitung FPS. |

Efek FILM GRAIN, DEPTH OF FIELD, dan SHARPEN dimatikan dengan menolkan konstanta kekuatannya di shader game
sendiri (dicari per shader dari tabel konstantanya), sehingga pass lain tidak tersentuh.

Catatan clock: saat bermain di tangan, game ini dibatasi GPU (GPU sibuk sekitar 95% tiap frame pada 460 MHz). Karena
itu GPU 768 MHz adalah pengaturan yang paling menaikkan FPS tanpa menurunkan kualitas gambar, dengan konsekuensi baterai
lebih cepat habis dan konsol lebih hangat. Bila sys-clk / Horizon OC terpasang, permintaan CPU dan GPU dikirim lewat
override sysmodule itu dan dilepas saat game ditutup normal. Bila game tertutup paksa, override bisa tertinggal sampai
diubah dari overlay atau konsol dinyalakan ulang.

## Status dan batasan

Dunia 3D, video Bink, dan langit sudah tampil benar di konsol. Bug terakhir yang diketahui (separuh kanan layar lebih
gelap saat bermain) berasal dari resolve depth tile kanan pada predicated tiling dan diperbaiki oleh
`sdk-patches/0028`. Laporkan bila masih ada efek yang salah tampil, sertakan log serta hasil ZL+ZR+Kiri (dump frame) dan
ZL+ZR+Bawah (tangkapan layar) dengan `switch_atajos_depuracion = true` di `c2.toml`.
