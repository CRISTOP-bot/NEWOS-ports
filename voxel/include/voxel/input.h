#ifndef VOXEL_INPUT_H
#define VOXEL_INPUT_H

#include <stdint.h>
#include "voxel/abi.h"

struct voxel_pointer {
    int32_t x, y;
    uint32_t buttons;
    int32_t wheel;
    uint64_t sequence;
};

/* Polls the kernel's latest PS/2 pointer snapshot. sequence changes per packet. */
int voxel_pointer_poll(struct voxel_pointer *pointer);

/* Waits for a duration using the implemented SYS_SLEEP call. */
int voxel_sleep_ms(uint32_t milliseconds);



/* Claims/releases the framebuffer text console for a compositor. While claimed,
 * kernel fbcon and the legacy cursor do not paint over compositor pixels. */
int voxel_console_gui_claim(void);
int voxel_console_gui_release(void);

/* Foreground PS/2 keyboard ownership for a GUI session. Grabbing redirects
 * decoded PS/2 bytes to this process's private queue; serial input keeps
 * working. Teardown releases automatically. */
int voxel_keyboard_grab(void);
int voxel_keyboard_release(void);

/* Non-blocking read of up to `capacity` decoded bytes. Returns the number of
 * bytes read (0 when the queue is empty, -1 on error). */
int voxel_keyboard_read(char *buffer, uint32_t capacity);

#endif
