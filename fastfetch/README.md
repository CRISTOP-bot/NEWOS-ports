# fastfetch for NEWOS

fastfetch is a feature-rich system information tool (a `neofetch` successor)
written in C. This directory is the NEWOS fork: unmodified upstream source,
the NEWOS ASCII logo, a host build recipe and the **checked-in binary**
embedded as `/bin/fastfetch`.

```
ports/fastfetch/
├── src/        upstream fastfetch source (v2.72.0) + NEWOS ASCII logo
├── fastfetch   static x86_64 ET_EXEC (embedded as /bin/fastfetch)
├── COPYING     MIT license (upstream license, kept verbatim)
├── SHA256SUMS  digest of the embedded binary
└── README.md
```

## Provenance

- Upstream: <https://github.com/fastfetch-cli/fastfetch>
- Upstream release: **v2.72.0** (commit `83a6783576f91cbfba52270d2ab673555a88d035`)
- License: **MIT** (`LICENSE` copied as `COPYING`); the NEWOS fork keeps the
  same license.
- Local change: `src/logo/ascii/n/newos.txt` — a NEWOS ASCII logo (registered
  in `src/logo/ascii/n.inc` so `--logo newos` and distro auto-detection both
  find it).

## How it runs

`/bin/fastfetch` is a static ELF64 `ET_EXEC` (Linux ABI) that runs through
NEWOS's Linux-compatible `SYSCALL` entry, the same path used by `/bin/busybox`,
`/bin/nano` and `/bin/tcc`. It is an opaque data blob to the kernel: the
`fastfetch.bin.o` object is linked into the kernel image, and initramfs
installs it as `/bin/fastfetch`.

The build uses the host GCC 14 in C23 mode with `-static`; the bundled yyjson
is compiled in and every optional native graphics/desktop library (X11,
Wayland, DRM, Vulkan, D-Bus, SQLite3, ImageMagick, Lua, …) is disabled at
CMake time. `BINARY_LINK_TYPE=static` also compiles out `dlopen`, so the
binary needs no shared objects at runtime.

## Limitations on NEWOS

fastfetch detects hardware through Linux `/proc` and `/sys`. NEWOS provides
compatible `/proc/meminfo`, `/proc/uptime`, `/proc/version`, `/sys/kernel/*`
and `/sys/bus/pci/devices`, so those modules report real data. Interfaces
NEWOS does not implement leave their module empty or as `Unknown`:

- CPU model/frequency come from CPUID; thermal/hwmon sensors are absent.
- GPU appears as a PCI ID (no DRM/NVML).
- Packages, DE/WM, terminal/shell parent chain, sound, network and disk I/O
  modules depend on interfaces NEWOS does not expose yet.
- `fork`/`execve` are stubs, so modules that spawn external package managers
  or `xrandr` are inactive.
- The image logo is off (`ENABLE_IMAGE_LOGO=OFF`); only ASCII logos work.

## Rebuilding

```sh
bash scripts/build-fastfetch.sh            # writes ports/fastfetch/fastfetch
```
