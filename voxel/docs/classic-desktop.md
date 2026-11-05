# Classic desktop widgets and demo

VOXEL now includes a reusable, CPU-rendered Win95/98-inspired widget layer.
It is intentionally built on the project's existing `voxel_surface` and
compositor APIs. It now also uses NEWOS `SYS_KBD_GRAB`/`SYS_KBD_READ` and
`SYS_CONSOLE_GUI` for keyboard and console ownership, but remains a
single-process demo with no window protocol.

## Components

- `include/voxel/classic_ui.h`, `src/theme/classic.c`: clipped 5×7 bitmap text,
  neutral gray/navy color constants, raised/sunken bevels, square buttons,
  active/inactive title bars, window frames, compact menu bars and popup menus,
  three pixel-art icons, Start menu, taskbar, a dialog helper, plus checkbox,
  radio button, progress bar, scrollbar and text field widgets.
- `demo/voxel_demo.c`: teal desktop with desktop shortcuts, My Computer,
  Settings and About windows, File/View/Help menus, task buttons, Start menu,
  modal About dialog, draggable title bars, and close/minimize/maximize controls.
- `tests/test_core.c`: pixel assertions for raised/sunken button edges, navy
  titlebar, menu layout and clipping at the surface edge.

The 3D bevels use flat pixels (white/light edge and medium/dark edge), not
shadows, gradient fills, transparency effects or rounded corners. Surface
buffers are caller-owned as before; VOXEL remains a single-process demo, not a
persistent window manager. The Settings window is illustrative and does not
change kernel settings. Keyboard shortcuts: `q` or Escape exits the demo. Text entry still uses the
on-screen keyboard for complex input.

## Controls

- Click **Start** to open the classic menu. Programs/Documents opens My Computer;
  Settings opens Display Properties; About VOXEL opens an information dialog.
- Click a taskbar button to activate its window; click the active button again
  to minimize it.
- Drag a titlebar to move a window. The caption controls minimize, maximize /
  restore and close. The File menu includes Open, Close and About VOXEL.
- Click desktop shortcuts to open their corresponding demo window.

The existing NEWOS framebuffer and mouse syscalls remain the only target
backend. This code still needs actual cross-build and QEMU visual/interactive
validation before any claims about appearance or runtime on the target.
