# NEWOS ports

This component stores ports integration assets and the staged GNU utility payload for NEWOS. Staged binaries and the source distfile are retained byte-for-byte in `assets/stage.tar.gz` and `assets/distfiles.tar.gz`; `scripts/restore-assets.sh` expands them before packaging.

Vendored source forks that ship as checked-in `/bin` binaries:

- `busybox/` — official static BusyBox 1.35.0 (402 applets), GPLv2.
- `nano/` — GNU nano 8.4 + static ncursesw 6.5, GPL-3.0-or-later
  (`scripts/build-nano.sh`).
- `tcc/` — TinyCC 0.9.27, LGPL-2.1-or-later (`scripts/build-tcc.sh`), plus
  the `/etc/tcc-hello.c` demo and the NEWOS SDK package definition.
- `voxel/` — MIT software graphics stack + classic desktop demo
  (`scripts/build-voxel.sh`), NEWOS-native source shipped as `/bin/voxel`.

Each directory carries the upstream license, a README with provenance and
limits, and `SHA256SUMS` for the embedded binary. `voxel/` is built from
source on every image build, so it has no checked-in binary or digests.

`glxgears-newos` is retained as an optional software-rendered graphics application/demo, not the NEWOS desktop environment. The built-in desktop is NEWOS code (`user/programs/desktop`); the MIT [VOXEL](https://github.com/CRISTOP-bot/voxel) stack is vendored under `ports/voxel` and shipped as `/bin/voxel`, the standalone classic-desktop demo.

Source snapshot imported from `CRISTOP-bot/NEWOS` at `c714f0b204099ac9a95c65489677527377760f57`; original monorepo history was not rewritten into this component.

OpenRC is being evaluated as a real service manager for NEWOS and is pinned as `openrc-newos` at the `CRISTOP-bot/openrc` fork's `newos-port` branch. This is a source integration point, **not yet a runnable NEWOS port**: the current kernel/libc lack process-lifecycle and POSIX interfaces required by OpenRC. See `openrc-newos/PORTING-NEWOS.md` for the verified gaps and acceptance gates. The ports CI verifies the pin and recursive checkout; it does not claim an OpenRC build or NEWOS boot test.
