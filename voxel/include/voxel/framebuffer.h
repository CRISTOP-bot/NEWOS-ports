#ifndef VOXEL_FRAMEBUFFER_H
#define VOXEL_FRAMEBUFFER_H

#include <stddef.h>
#include <stdint.h>
#include "voxel/abi.h"

struct voxel_framebuffer {
    struct voxel_fbinfo info;
};

/* Fetch the kernel's real panel geometry; rejects unsupported/inconsistent modes. */
int voxel_framebuffer_open(struct voxel_framebuffer *fb);

/* Present a contiguous XRGB8888 rectangle. The kernel converts to its native
 * 24- or 32-bit RGB pixel layout and validates user memory and screen bounds. */
int voxel_framebuffer_blit(struct voxel_framebuffer *fb,
                           uint32_t x, uint32_t y,
                           uint32_t width, uint32_t height,
                           const uint32_t *pixels, size_t stride_pixels);

/* Nearest-neighbour upscale of a view rectangle to the real panel. `vw`/`vh`
 * are the internal view dimensions; the destination is the whole panel. */
int voxel_framebuffer_blit_scaled(struct voxel_framebuffer *fb,
                                  uint32_t sx, uint32_t sy,
                                  uint32_t sw, uint32_t sh,
                                  uint32_t vw, uint32_t vh,
                                  const uint32_t *pixels, size_t stride_pixels);

#endif
