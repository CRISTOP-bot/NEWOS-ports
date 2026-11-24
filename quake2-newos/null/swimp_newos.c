/*
 * swimp_newos.c - Quake 2 software-renderer framebuffer backend for NEWOS.
 * Replaces null/swimp_null.c. Draws the 8-bit paletized vid.buffer to the
 * NEWOS framebuffer via SYS_FBWRITE, converting through the 256-entry palette.
 */
#include "../ref_soft/r_local.h"
#include "newos_platform.h"
#include <stdlib.h>
#include <string.h>

u64  q2_fb_width, q2_fb_height, q2_fb_pitch;
u32  q2_fb_bpp;
unsigned char q2_palette[256 * 3];

/* One expanded row at a time (fits the syscall bounce buffer). */
static u32 q2_row[1024];

void q2_expand_row(const unsigned char *src, u32 *dst, int width)
{
    int i;
    for (i = 0; i < width; i++) {
        const unsigned char *c = &q2_palette[src[i] * 3];
        dst[i] = 0xff000000u | ((u32)c[0] << 16) | ((u32)c[1] << 8) | c[2];
    }
}

void SWimp_BeginFrame(float camera_separation)
{
    (void)camera_separation;
}

void SWimp_EndFrame(void)
{
    struct nsh_fbinfo fi;
    int y;

    if (sys_fbinfo(&fi) != 0 || !fi.present)
        return;
    if (fi.bpp == 32 && fi.width && fi.height && vid.buffer) {
        int w = (int)((u64)vid.width < fi.width ? (u64)vid.width : fi.width);
        int h = (int)((u64)vid.height < fi.height ? (u64)vid.height : fi.height);
        int xo = (int)((fi.width  - (u64)vid.width)  / 2);
        int yo = (int)((fi.height - (u64)vid.height) / 2);

        for (y = 0; y < h; y++) {
            const unsigned char *src = vid.buffer + y * vid.rowbytes;
            q2_expand_row(src, q2_row, w);
            struct nsh_fbwrite rq;
            rq.x = (u64)xo; rq.y = (u64)(yo + y);
            rq.w = (u64)w; rq.h = 1;
            rq.pixels = (u64)(u32)(unsigned long)q2_row;
            rq.pixlen = (u64)w * 4u;
            sys_fbwrite(&rq);
        }
    }
}

int SWimp_Init(void *hInstance, void *wndProc)
{
    struct nsh_fbinfo fi;
    (void)hInstance; (void)wndProc;

    if (sys_fbinfo(&fi) == 0 && fi.present) {
        q2_fb_width = fi.width;
        q2_fb_height = fi.height;
        q2_fb_pitch = fi.pitch;
        q2_fb_bpp = fi.bpp;
    }
    return 1;
}

void SWimp_SetPalette(const unsigned char *palette)
{
    if (palette)
        memcpy(q2_palette, palette, sizeof(q2_palette));
}

void SWimp_Shutdown(void)
{
}

rserr_t SWimp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen)
{
    (void)fullscreen; (void)mode;
    /* 320x240 software mode: 8-bit buffer, blitted to the native FB. */
    if (pwidth && pheight) {
        *pwidth = 320;
        *pheight = 240;
    }
    return rserr_ok;
}

void SWimp_AppActivate(qboolean active)
{
    (void)active;
}
