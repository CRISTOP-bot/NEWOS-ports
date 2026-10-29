# BusyBox for NEWOS

This directory contains the **official upstream static BusyBox 1.35.0**
`x86_64-linux-musl` build. It is not a reduced or reimplemented applet set:
`busybox --list` reports 402 applets.

## Provenance

- Binary URL:
  <https://busybox.net/downloads/binaries/1.35.0-x86_64-linux-musl/busybox>
- Source tarball:
  <https://busybox.net/downloads/busybox-1.35.0.tar.bz2>
- SHA-256 of `busybox`:
  `6e123e7f3202a8c1e9b1f94d8941580a25135382b99e8d3e34fb858bba311348`
- License: GPLv2 only. See `LICENSE` in this directory.

The binary is checked in so the image build is reproducible and does not need
network access. The source tarball is not vendored here; a distributor that
ships this image must also make the corresponding BusyBox 1.35.0 source
available (the URL above is the exact upstream release).

## How NEWOS runs it

The binary is a static ET_EXEC that uses the Linux x86-64 `syscall`
instruction. NEWOS implements that entry point in
`kernel/arch/x86_64/interrupts/x86_interrupt.S` and maps the Linux syscall
numbers in `syscall/syscall_dispatch.c` / `syscall/syscall_posix.c`. It is
copied to `/bin/busybox` in the initramfs.

Run an applet directly:

```sh
busybox --list
busybox echo hello
busybox ls /
busybox sh -c 'echo from busybox sh'
```

`nsh` also falls back to `/bin/busybox <applet>` for any command that is not a
native `/bin` tool or an installed GNU tool, so unshadowed names such as
`awk`, `sed`, `vi`, `wget`, `tar` and `xz` work without symlinks (the NEWOS
VFS does not implement symlinks yet).

## Limits

BusyBox applets that need missing kernel facilities (for example `mount` of
filesystems NEWOS does not implement, `ip`/netlink, `mdev`, or applets that
require a Linux `/proc` layout different from NEWOS's) may return errors. The
402 applets are present and executable; per-applet kernel feature gaps are a
separate, documented porting issue rather than a stripped-down build.
