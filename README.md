# NEWOS ports

Ports integration assets and the staged GNU utility payload for NEWOS. Staged binaries and the source distfile are retained byte-for-byte in `assets/stage.tar.gz` and `assets/distfiles.tar.gz`; `scripts/restore-assets.sh` expands them before packaging. These are generated/downloadable artifacts, not kernel or userland sources. `glxgears-newos` is a pinned nested submodule; TinyGL is nested below that port.

Source snapshot imported from `CRISTOP-bot/NEWOS` at `c714f0b204099ac9a95c65489677527377760f57`; the original monorepo commit history was not rewritten into this component.
