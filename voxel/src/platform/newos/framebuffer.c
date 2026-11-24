#include "voxel/framebuffer.h"
#include "syscall.h"

#include <stdint.h>


int voxel_framebuffer_open(struct voxel_framebuffer *fb)
{
    uint64_t min_pitch;
    if (!fb)
        return -1;
    fb->info.present = 0;
    fb->info.width = fb->info.height = fb->info.pitch = 0;
    fb->info.bpp = fb->info._pad = 0;
    if (voxel_newos_syscall1(VOXEL_SYS_FBINFO, (long)(uintptr_t)&fb->info) != 0)
        return -1;
    if (!fb->info.present || !fb->info.width || !fb->info.height ||
        fb->info.width > INT32_MAX || fb->info.height > INT32_MAX ||
        (fb->info.bpp != 24u && fb->info.bpp != 32u))
        return -1;
    if (fb->info.width > UINT64_MAX / (fb->info.bpp / 8u))
        return -1;
    min_pitch = fb->info.width * (fb->info.bpp / 8u);
    if (fb->info.pitch < min_pitch)
        return -1;
    return 0;
}

int voxel_framebuffer_blit(struct voxel_framebuffer *fb,
                           uint32_t x, uint32_t y,
                           uint32_t width, uint32_t height,
                           const uint32_t *pixels, size_t stride_pixels)
{
    uint32_t row;
    if (!fb || !pixels || !width || !height || stride_pixels < width ||
        width > 4096u || height > 4096u || !fb->info.present ||
        x > fb->info.width || y > fb->info.height ||
        width > fb->info.width - x || height > fb->info.height - y ||
        stride_pixels > SIZE_MAX / height ||
        stride_pixels * height > SIZE_MAX / sizeof(*pixels))
        return -1;

    /* SYS_FBWRITE consumes XRGB8888 words; the kernel converts to native
     * 24/32-bpp RGB masks. Panel pitch does not describe this source buffer. */
    for (row = 0; row < height; ++row) {
        struct voxel_fbwrite request;
        request.x = x;
        request.y = (uint64_t)y + row;
        request.w = width;
        request.h = 1;
        request.pixels = (uint64_t)(uintptr_t)(pixels + (size_t)row * stride_pixels);
        request.pixlen = (uint64_t)width * sizeof(*pixels);
        if (voxel_newos_syscall1(VOXEL_SYS_FBWRITE,
                           (long)(uintptr_t)&request) != 0)
            return -1;
    }
    return 0;
}

int voxel_framebuffer_blit_scaled(struct voxel_framebuffer *fb,
                                  uint32_t sx, uint32_t sy,
                                  uint32_t sw, uint32_t sh,
                                  uint32_t vw, uint32_t vh,
                                  const uint32_t *pixels, size_t stride_pixels)
{
    /* One scaled row at a time keeps the staging buffer small (16 KiB) while
     * still filling the real panel: SYS_FBWRITE takes XRGB8888 words only. */
    static uint32_t row[4096];
    uint32_t dx0, dx1, dy0, dy1, dx, dy;

    if (!fb || !pixels || !vw || !vh || !sw || !sh || !fb->info.present ||
        sx >= vw || sy >= vh || sw > vw - sx || sh > vh - sy ||
        stride_pixels < sx + sw)
        return -1;
    dx0 = (uint32_t)(((uint64_t)sx * fb->info.width) / vw);
    dx1 = (uint32_t)(((uint64_t)(sx + sw) * fb->info.width + vw - 1u) / vw);
    dy0 = (uint32_t)(((uint64_t)sy * fb->info.height) / vh);
    dy1 = (uint32_t)(((uint64_t)(sy + sh) * fb->info.height + vh - 1u) / vh);
    if (!dx0 && !dx1) dx1 = (uint32_t)fb->info.width;
    if (!dy0 && !dy1) dy1 = (uint32_t)fb->info.height;
    if (dx1 > fb->info.width) dx1 = (uint32_t)fb->info.width;
    if (dy1 > fb->info.height) dy1 = (uint32_t)fb->info.height;
    if (dx1 <= dx0 || dy1 <= dy0 || dx1 - dx0 > 4096u)
        return -1;

    for (dy = dy0; dy < dy1; dy++) {
        uint32_t src_y = sy + (uint32_t)(((uint64_t)(dy - dy0) * sh) / (dy1 - dy0));
        if (src_y >= sy + sh) src_y = sy + sh - 1u;
        for (dx = dx0; dx < dx1; dx++) {
            uint32_t src_x = sx + (uint32_t)(((uint64_t)(dx - dx0) * sw) / (dx1 - dx0));
            if (src_x >= sx + sw) src_x = sx + sw - 1u;
            row[dx - dx0] = pixels[(size_t)src_y * stride_pixels + src_x];
        }
        if (voxel_framebuffer_blit(fb, dx0, dy, dx1 - dx0, 1u, row,
                                   dx1 - dx0) != 0)
            return -1;
    }
    return 0;
}
