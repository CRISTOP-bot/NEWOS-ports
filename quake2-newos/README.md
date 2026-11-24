# Quake 2 for NEWOS

A native port of id Software's Quake 2 (GPL-2.0, source release 3.21) to
NEWOS. The engine runs with the **software renderer** (`ref_soft`) and uses
NEWOS-specific backends instead of X11/OpenGL/SDL/OSS:

| Backend | File | NEWOS facility |
|---|---|---|
| System / timing | `null/sys_newos.c` | `CLOCK_MONOTONIC`, `malloc`, `SYS_Mkdir` |
| Input (keyboard/mouse) | `null/in_newos.c` | `SYS_KBD_*`, `SYS_MOUSE_GET` |
| Video / mode | `null/vid_null.c` | (shared glue; renderer via swimp below) |
| Framebuffer blit | `null/swimp_newos.c` | `SYS_FBINFO` / `SYS_FBWRITE`, 8-bit palette -> XRGB8888 |
| Sound (PCM DMA) | `null/snddma_newos.c` | **`SYS_AUDIO_*`** -> Intel HDA driver |
| Network | `null/net_newos.c` | no-op (single-player; no sockets needed) |

## What works

- Quake 2 3.21 engine (client + server + game + ref_soft) compiles and links
  as a static NEWOS ELF with FPU enabled (same treatment as DOOM/glxgears).
- `Qcommon_Init()` completes: filesystem, cvars, console, commands, network
  glue, and the software renderer all initialize. The frame loop runs.
- Audio: the `snddma_newos.c` backend feeds the portable Quake mixer an 8-bit
  mono DMA buffer, expands it to 16-bit stereo on `Submit`, and plays it via
  the NEWOS audio syscalls — the same path the `beep` demo uses, verified to
  produce a real 440 Hz tone through QEMU's `intel-hda` codec.

## Game data (required)

Quake 2's data (`pak0.pak`) is **not freely distributable** and is therefore
not included (unlike DOOM's shareware `doom1.wad`). To play:

1. Obtain the Quake 2 shareware/full game `pak0.pak` (from id Software).
2. Install it on the NEWOS image under the game directory (`baseq2/`), e.g.
   stage it in the initramfs or copy it at runtime.
3. The engine's `FS_InitFilesystem` will find it and load maps/sounds.

Without `pak0.pak`, the engine starts and shows `FILE NOT FOUND` in its
in-game console (drawn to the framebuffer) — the serial console stays quiet,
which is expected because `Com_Printf` goes to the in-game console.

## Build

```sh
make -C ports/quake2-newos -f Makefile.newos ROOT=/path/to/newos
```

The root Makefile builds and embeds it automatically (`Q2_BIN`/`Q2_EMBED`),
installing `/bin/quake2` through `initramfs.c`.

## License

Quake 2 source: GPL-2.0, Copyright (C) 1997 id Software. See upstream
`gnu.txt` / `readme.txt` in this directory. The NEWOS backends (this port's
own code) are MIT. No id Software game assets are included.
