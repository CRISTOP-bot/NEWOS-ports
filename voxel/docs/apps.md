# VOXEL applications: implementation status

The application modules are hosted in-process by the VOXEL demonstration. They are not independent binaries or isolated processes.

| Module | Implemented | Limits |
|---|---|---|
| About | Reports VOXEL identity and implementation scope. | Static view. |
| Explorer | Reads real directory entries, enters directories, and goes to parent. | Eight rows; files are not opened automatically by the desktop yet. |
| Settings | Toggles desktop and window palette in memory. | Session-only; no config persistence. |
| Run | On-screen mouse keyboard enters an executable path and launches via NEWOS `SPAWN2`. | Path only; no arguments or child-output capture. |
| Console | On-screen keyboard and bounded history; can launch an absolute executable path. | Not a shell, no pipes/command parser/output redirection. |
| Hello World | Renders a small app view. | In-process demo. |
| Editor | Bounded text buffer, on-screen key input and NEWOS read/write callbacks. | Fixed-size buffer; Save creates `/notes.txt` initially; Open reads current path or `/etc/version`. |
| Image Viewer | Loads and renders BMP via a bounded decoder. | Strict uncompressed 24-bit BMP, maximum decoded storage 64×48. |

The pinned NEWOS ABI now also provides GUI keyboard read/grab
(`SYS_KBD_GRAB`/`SYS_KBD_READ`, 460/461) and console ownership
(`SYS_CONSOLE_GUI`, 462); the demo claims both. Text entry still uses the
on-screen keyboard for complex input, while single-key shortcuts work through
the new backend. The app registry is built into the VOXEL process; separate
`.new` packages, a real terminal session/window service, process output
redirection, and desktop session integration remain unimplemented.
