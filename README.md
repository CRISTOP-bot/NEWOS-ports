# NEWOS ports

This component stores ports integration assets and the staged GNU utility payload for NEWOS. Staged binaries and the source distfile are retained byte-for-byte in `assets/stage.tar.gz` and `assets/distfiles.tar.gz`; `scripts/restore-assets.sh` expands them before packaging.

`glxgears-newos` is retained as an optional software-rendered graphics application/demo, not the NEWOS desktop environment. Nova is the selected upstream desktop/compositor source for a future NEWOS port, pinned at `d586f1bfc4687172ca097298ba3c91d6f7d40069`. It is GPL-3.0 and is currently source-only: its BoredOS-specific compiler/SDK, Linux framebuffer/VT ioctls, POSIX sockets/shared memory/threads, and dynamic-linker assumptions are not provided by NEWOS, so it is not yet buildable or runnable on NEWOS. See the superproject `docs/nova-porting.md` before enabling it.

Source snapshot imported from `CRISTOP-bot/NEWOS` at `c714f0b204099ac9a95c65489677527377760f57`; original monorepo history was not rewritten into this component.
