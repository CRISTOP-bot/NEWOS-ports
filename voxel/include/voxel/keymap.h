#ifndef VOXEL_KEYMAP_H
#define VOXEL_KEYMAP_H

#include <stdint.h>

/* Streaming decoder for the decoded ASCII bytes delivered by
 * SYS_KBD_READ. Turns single bytes and xterm-style escape sequences into
 * the portable VOXEL_KEY_* codes used by app key handlers. */
struct voxel_key_decoder {
    uint8_t esc;        /* saw ESC, waiting for '[' or a plain key */
    uint8_t bracket;    /* inside ESC [ ... */
    uint8_t delete_seq; /* inside ESC [ 3 ~ */
    uint8_t pending;    /* plain byte to emit after a standalone ESC */
};

void voxel_key_decoder_init(struct voxel_key_decoder *decoder);
/* Returns 0 when the byte completes no key (sequence still in progress).
 * Otherwise returns an ASCII byte or a VOXEL_KEY_* constant. */
uint32_t voxel_key_decode(struct voxel_key_decoder *decoder,
                          unsigned char byte);

#endif
