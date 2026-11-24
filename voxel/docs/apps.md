# VOXEL applications: implementation status

The application modules are hosted in-process by the VOXEL demonstration. They are not independent binaries or isolated processes. The widget layer now follows the Windows 95 look closely: solid teal desktop, gray 3D bevels, a horizontal navy-to-blue active title gradient with drawn minimize/maximize/close boxes, gray inactive title, a four-color Start logo with bold label, sunken taskbar buttons and blue selection highlights. The default presentation is the original centered 640×480 view with 1:1 pointer coordinates. `Display Properties → FULLSCREEN` stores `scale=1` in `/fat/VOXEL.CFG`; on the next boot the compositor scales the view to the real panel (nearest-neighbour) and pointer coordinates are mapped back into view space. `Display Properties` edits a live theme state that the widget layer reads through `voxel_ui_set_palette()`, so color changes apply immediately and persist in the same config file.

| Module | Implemented | Limits |
|---|---|---|
| About | Reports VOXEL identity and implementation scope. | Static view. |
| Explorer | Reads real VFS directory entries, enters directories and parent, shows sizes/selected row, creates directories (`NEW` or `N`), deletes files (`DEL` or Delete key), and accepts a typed path (`GO` or `P`) with the on-screen keyboard. | Eight visible rows (no scrolling); rename on `/fat` is refused by the kernel VFS; only regular files can be deleted. |
| Settings | Toggles desktop and window palette and saves them to `/fat/VOXEL.CFG` (or loads them at startup when a FAT32 disk is attached). | Without a disk it stays session-local; payload is kept under 512 bytes for `SYS_WRITE`. |
| Run | On-screen mouse keyboard enters an executable path and launches via NEWOS `SPAWN2`. | Path only; no arguments or child-output capture. |
| Console | On-screen keyboard and bounded history; can launch an absolute executable path. | Not a shell, no pipes/command parser/output redirection. |
| Hello World | Renders a small app view. | In-process demo. |
| Editor | Bounded text buffer, on-screen key input and NEWOS read/write callbacks. | Fixed-size buffer; Save creates `/notes.txt` initially; Open reads current path or `/etc/version`. |
| Image Viewer | Loads and renders BMP via a bounded decoder. | Strict uncompressed 24-bit BMP, maximum decoded storage 64×48. |

The pinned NEWOS ABI also provides GUI keyboard read/grab
(`SYS_KBD_GRAB`/`SYS_KBD_READ`, 460/461) and console ownership
(`SYS_CONSOLE_GUI`, 462); the demo claims both. `voxel/keymap.h` decodes raw
bytes (including arrow/Delete escape sequences) into `VOXEL_KEY_*` codes and
routes them to the focused app's `key()` handler, so Run, Console and Editor
accept real typing, Backspace and Enter. The on-screen keyboard remains as an
alternative. Only unconsumed `q`/Escape exit the desktop, so typing `q` into
a text field no longer quits VOXEL. The app registry is built into the VOXEL
process; separate
`.new` packages, a real terminal session/window service, process output
redirection, and desktop session integration remain unimplemented.
