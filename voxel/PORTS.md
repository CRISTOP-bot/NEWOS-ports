# VOXEL for NEWOS

VOXEL is NEWOS's software graphics stack and classic-desktop demo. It is a
separate MIT repository, vendored here so the image can build offline.

```
ports/voxel/
├── include/voxel/   public surface, compositor, framebuffer, input, widgets
├── src/             surfaces, compositor, classic theme, NEWOS backend
├── apps/            in-process apps (Explorer, Editor, Terminal, ...)
├── demo/            voxel_demo.c: the classic desktop (entry point)
├── tests/           portable core/app tests (host only)
├── packages/        voxel.newspec for `newpkg`
├── patches/         NEWOS changes as a diff against upstream HEAD
├── LICENSE          MIT (kept verbatim)
└── README.md        upstream documentation
```

## Provenance

- Upstream: <https://github.com/CRISTOP-bot/voxel> (MIT, CRISTOP-bot)
- Vendored at commit `a37c3594fdad01b20e4a8362d7ef4c85eee131f0` (`main`,
  2026-10-27, "Correct BMP decoder test fixture").
- `patches/0001-host-gcc-werror-fixes.patch` is the only local change: it
  removes two unused variables, a leftover `file_menu_open=0;` in the demo's
  click handler, fixes `open_path` to match the `void` hook in
  `include/voxel/app.h`, and splits the dense one-line statements in
  `src/platform/newos/apps.c` that GCC 16 rejects under `-Werror`
  (`-Wmisleading-indentation`, `-Wincompatible-pointer-types`).

## Building

VOXEL upstream drives `toolchain/newos-gcc`; this tree instead uses the same
host-compiler recipe as the rest of NEWOS userland:

```sh
make sdk          # sysroot/usr/lib/{crt0.o,app.ld,libc.a,libnsh.a}
bash scripts/build-voxel.sh build/voxel.elf
```

The image build runs that script automatically and embeds the result as
`/bin/voxel` (see `VOXEL_BIN` in the top-level `Makefile`). Upstream's
`make test` still runs on the host; `TINYGL_ROOT` is **not** vendored here
(the TinyGL demo needs the separate `vendor/tinygl` submodule).

## Running on NEWOS

```sh
voxel                # interactive classic desktop (mouse)
voxel --frames 60    # render 60 frames, print the completion marker, exit
```

The demo needs a framebuffer (`SYS_FBINFO`/`SYS_FBWRITE`, 453/454) and reads
the pointer through `SYS_MOUSE_GET` (455). It has no keyboard API: text entry
uses the on-screen keyboard, because the kernel does not expose GUI key
events to userland.

## Scope

VOXEL is a compositor and classic-widget toolkit plus a demo desktop, not a
window server. The NEWOS session shell and its keyboard-driven desktop live
in `userland/user/programs/desktop`; VOXEL is the documented direction for
the graphics stack, and the two coexist as separate programs.
