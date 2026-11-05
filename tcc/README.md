# tcc for NEWOS

TinyCC (TCC) is NEWOS's on-board C compiler. This directory is the NEWOS
fork: the upstream release, the NEWOS patches, the host build recipe and the
**checked-in binary** that the image embeds as `/bin/tcc`.

```
ports/tcc/
├── src/          upstream tcc-0.9.27 source + the NEWOS search-path patch
├── patches/      every NEWOS change as a standalone diff against upstream
├── demo/hello.c  canonical copy of the image's /etc/tcc-hello.c
├── tcc           static x86_64-linux-musl ET_EXEC (embedded as /bin/tcc)
├── libtcc1.a     tcc runtime helpers (shipped by the SDK, see below)
├── COPYING       LGPL-2.1 (upstream license, kept verbatim)
├── SHA256SUMS    digests of the binary, the runtime archive and COPYING
└── README.md
```

## Provenance

- Upstream: <https://download.savannah.gnu.org/releases/tinycc/tcc-0.9.27.tar.bz2>
- Upstream release: 0.9.27 (2023-04-01)
- License: **LGPL-2.1-or-later** (`COPYING`); the NEWOS fork keeps the same
  license and records every local change in `patches/`.
- Local changes, recorded as two standalone patches in `patches/`:
  `0001-add-newos-sdk-paths.patch` adds `/usr/lib` to the library search path
  (upstream only looks in `/usr/lib64`-style directories), and
  `0002-resolve-plt32-directly-in-static-links.patch` makes `-static` links
  resolve calls directly instead of emitting a PLT stub that no dynamic
  loader could initialise. See "NEWOS patches" below.
- NEWOS does not copy TCC code into the kernel or its libc. It ships the
  compiler as a userspace program, exactly like `/bin/busybox` and the GNU
  tools under `/usr/bin`.

## How it runs

The binary is built **statically against musl** (no `PT_INTERP`) so the
kernel ELF loader accepts it, and it uses the Linux x86-64 `syscall`
instruction, which NEWOS implements in
`arch/x86_64/interrupts/x86_interrupt.S`. The syscall dispatcher provides the
subset TCC needs: file I/O (`open`/`read`/`write`/`lseek`), `stat`,
`mmap`/`munmap`, `getcwd`, `realpath` (via `readlink` returning `-EINVAL`,
which is the correct "not a symlink" answer on a VFS without symlinks),
`rename`, `unlink`, `chmod`, `fsync`, and the `RT_SIGACTION` no-op stubs.

TCC's x86-64 ELF image base is `0x400000`, which is exactly the address
NEWOS links user programs at, so the objects it emits are load-compatible
with the NEWOS ABI.

## On-board usage

```sh
tcc -v                                      # version and target
tcc -c /etc/tcc-hello.c -o /tmp/hello.o     # compile only (no SDK needed)
tcc -nostdlib -run /etc/tcc-hello.c         # compile and run in this process
```

`-run` works: `tcc_run()` relocates the generated code into the compiler's
own address space and calls `main`, so it needs neither `fork` nor `execve`
(NEWOS deliberately has neither). The demo is freestanding and talks to the
kernel through `int $0x80`.

A full link needs the NEWOS SDK (headers, `crt0.o`, `libc.a`) because TCC's
own startup files and C library are Linux/musl artifacts:

```sh
newpkg install /tmp/newos-sdk-<version>-x86_64.new
tcc -static -nostdlib -o /tmp/hello \
    /usr/lib/crt0.o hello.c /usr/lib/tcc/libtcc1.a \
    /usr/lib/libnsh.a /usr/lib/libc.a
/tmp/hello
```

- `-static` stops tcc from emitting `PT_INTERP`; NEWOS has no dynamic loader.
- `libnsh.a` provides `sys_exit`, called by `crt0.o` after `main` returns.
- `/usr/lib/tcc/libtcc1.a` holds tcc's runtime helpers (64-bit division,
  varargs register save); tcc-generated code calls them.
- `/usr/include` and `/usr/lib` are searched by default (patch 0001), so no
  `-I`/`-L` is needed.
- Calls are resolved directly in static links (patch 0002): a PLT stub would
  have no dynamic loader to fill its GOT slot and would jump to 0.

The *default* tcc link (without `-nostdlib`) expects Linux
`crt1.o`/`crti.o`/`crtn.o` and shared libraries, which NEWOS does not ship:
use the `-nostdlib` recipe above.

## NEWOS patches

- `0001-add-newos-sdk-paths.patch` — adds `/usr/lib` to the library search
  path. It must run after `tccpp_new()`: the path helpers use the CString
  allocator that `tccpp_new()` initialises (upstream only calls them after
  `tcc_new()` returns). `<stdio.h>` needs no extra path because
  `CONFIG_TCC_SYSINCLUDEPATHS` already contains `/usr/include`.
- `0002-resolve-plt32-directly-in-static-links.patch` — on x86-64 every call
  is an `R_X86_64_PLT32` relocation, so tcc builds a PLT entry even for
  defined functions. With `-static` there is no dynamic loader to fill the
  PLT GOT slot and `relocate_plt()` is only called for dynamic output, so the
  stub jumped through unrelocated data (observable as a fault at a bogus RIP
  such as `0x2825ffffff`). Static links now rewrite `PLT32` to a direct
  PC-relative `PC32`, which is what a static executable needs.

## Rebuilding the binary (host)

The compiler is built on the host, never inside NEWOS:

```sh
MUSL_CROSS=/path/to/x86_64-linux-musl-cross bash scripts/build-tcc.sh
```

The prefix comes from <https://musl.cc/x86_64-linux-musl-cross.tgz>; the
script defaults to `ports/build/x86_64-linux-musl-cross` (a git-ignored build
cache). It writes the stripped `tcc` and, from `src/lib/`, `libtcc1.a`. Both
are checked in so a normal `make` stays offline and root-free.
