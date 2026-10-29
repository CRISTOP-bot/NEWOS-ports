# GNU nano for NEWOS

GNU nano is NEWOS's full-screen editor for people who expect `nano`; the
small Kilo editor that predates it was retired. This directory is the NEWOS
fork: upstream source, patches, the
host build recipe and the **checked-in binary** embedded as `/bin/nano`.

```
ports/nano/
├── src/        upstream nano-8.4 source + NEWOS patches (this fork)
├── patches/    every NEWOS change as a standalone diff against upstream
├── nano        static x86_64-linux-musl ET_EXEC (embedded as /bin/nano)
├── COPYING     GPL-3.0-or-later (upstream license, kept verbatim)
├── SHA256SUMS  digest of the embedded binary
└── README.md
```

## Provenance

- Upstream: <https://www.nano-editor.org/dist/v8/nano-8.4.tar.xz>
- Upstream release: 8.4
- License: **GPL-3.0-or-later** (`COPYING`); the NEWOS fork keeps the same
  license. Distribution of the binary is accompanied by this source tree and
  the upstream release URL above (GPL section 6 source offer).
- Local change: `patches/0001-drop-doc-and-po-directories.patch` removes the
  `doc/` and `po/` subdirectories from the build (the image ships no manual
  or translation catalogs, and building them would require gettext tools).
- Local change: `patches/0002-default-to-a-color-terminal.patch` makes nano
  fall back to `TERM=linux` when the shell provides no environment (NEWOS's
  `spawn` has no envp), so ncurses reports colors instead of using the
  monochrome VT220 entry.
- ncurses: **6.5**, MIT-style license, built statically with the `linux`,
  `xterm`, `xterm-256color` and `vt220` terminfo entries compiled in. NEWOS
  ships no terminfo database, so `nano`'s own `TERM=linux` fallback resolves
  to an entry that exists inside the library.
- `/usr/etc/nanorc` (built from `user/rootfs/nanorc`) is installed by
  initramfs: `set autoindent`, `set linenumbers`, plus C/C++, shell,
  Makefile and nanorc syntax highlighting.

## How it runs

Static musl `ET_EXEC` (no dynamic loader) through the Linux x86-64 `syscall`
entry, like `/bin/busybox`. The terminal model is the kernel's console:
`TCGETS`/`TCSETS`/`TIOCGWINSZ` return the fixed 80x25 (or framebuffer) size,
input arrives byte by byte from the serial/PS/2 ring, and output goes through
the serial console and the VGA/framebuffer text backends (which implement the
VT100 escapes nano/ncurses emit).

The kernel gained the readiness and metadata calls nano/ncurses need:
`pselect6`/`select`/`poll` (console and pipe readiness with timeouts),
`fsync`/`fdatasync` (no-op success on the RAM VFS), `chmod`/`fchmod`
(recorded on the inode) and `readlink` (`-EINVAL`, i.e. "not a symlink",
which is what `realpath` expects on a VFS without symlinks).

## On-board usage

```sh
nano /etc/nsh.rc       # edit a file
nano -v                # version
```

Keys are the standard nano bindings: `Ctrl+O` writes out, `Ctrl+X` exits,
`Ctrl+W` searches, `Ctrl+K` cuts a line, `Ctrl+U` pastes, `Ctrl+G` help.

Limits (honest scope): no syntax highlighting database (`/usr/share/nano`
ships no files), no spell checker (`spell` filter is not present), no
`fork`/`exec` for external commands (`^R ^X`, linter, formatter), and history
files stay unset because NEWOS starts every program without an environment
(`HOME` is therefore absent; nano falls back to the passwd entry).

## Rebuilding the binary (host)

```sh
MUSL_CROSS=/path/to/x86_64-linux-musl-cross bash scripts/build-nano.sh
```

Mirrors `scripts/build-tcc.sh`: the musl.cc cross compiler plus a static
ncursesw built into `ports/build/nano-newos/ncurses-prefix`. The ncurses
tarball lives in the git-ignored `ports/distfiles/` (the top-level build
restores it from `assets/distfiles.tar.gz`; the script downloads it only if
it is absent). The stripped `nano` is checked in so `make` never needs
network access.
