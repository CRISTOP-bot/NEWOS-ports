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

/* --- compact 5x7 font for the startup splash (when no game data) ------- */
/* Glyphs in ASCII order 32..90, 7 bytes each (5 bits per row). */
static const unsigned char q2_font[] = {
 /* ' ' */ 0,0,0,0,0,0,0,
 /* '!' */ 4,4,4,4,4,0,4,
 /* '"' */ 0,0,0,0,0,0,0,
 /* '#' */ 0,0,0,0,0,0,0,
 /* '$' */ 0,0,0,0,0,0,0,
 /* '%' */ 0,0,0,0,0,0,0,
 /* '&' */ 0,0,0,0,0,0,0,
 /* '\'' */ 0,0,0,0,0,0,0,
 /* '(' */ 2,4,8,8,8,4,2,
 /* ')' */ 8,4,2,2,2,4,8,
 /* '*' */ 0,0,0,0,0,0,0,
 /* '+' */ 0,4,4,31,4,4,0,
 /* ',' */ 0,0,0,0,12,12,8,
 /* '-' */ 0,0,0,31,0,0,0,
 /* '.' */ 0,0,0,0,0,12,12,
 /* '/' */ 1,1,2,4,8,16,16,
 /* 0..9 */ 14,17,19,21,25,17,14,  4,12,4,4,4,4,14,
          14,17,1,2,4,8,31,  30,1,1,14,1,1,30,
           2,6,10,18,31,2,2,  31,16,16,30,1,1,30,
          14,16,16,30,17,17,14, 31,1,2,4,8,8,8,
          14,17,17,14,17,17,14, 14,17,17,15,1,1,14,
 /* : ; < = > ? @ */ 0,12,12,0,12,12,0,  0,12,12,0,12,12,8,
                     2,4,8,16,8,4,2,  0,0,31,0,31,0,0,
                     8,4,2,1,2,4,8,  14,17,1,2,4,0,4,  0,0,0,0,0,0,0,
 /* A..Z */ 14,17,17,31,17,17,17, 30,17,17,30,17,17,30,
          14,17,16,16,16,17,14, 30,17,17,17,17,17,30,
          31,16,16,30,16,16,31, 31,16,16,30,16,16,16,
          14,17,16,23,17,17,15, 17,17,17,31,17,17,17,
          14,4,4,4,4,4,14,      7,2,2,2,18,18,12,
          17,18,20,24,20,18,17, 16,16,16,16,16,16,31,
          17,27,21,21,17,17,17, 17,25,21,19,17,17,17,
          14,17,17,17,17,17,14, 30,17,17,30,16,16,16,
          14,17,17,17,21,18,13, 30,17,17,30,20,18,17,
          15,16,16,14,1,1,30,   31,4,4,4,4,4,4,
          17,17,17,17,17,17,14, 17,17,17,17,17,10,4,
          17,17,17,21,21,21,10, 17,17,10,4,10,17,17,
          17,17,10,4,4,4,4,     31,1,2,4,8,16,31,
};

static const unsigned char *q2_glyph(int c)
{
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c < 32 || c > 90) c = ' ';
    return &q2_font[(c - 32) * 7];
}

/* Render 5x7 glyph text into a 2D XRGB8888 buffer (width 200 px).
 * buf[y*W + x], W=200, H=28 (enough for one line at scale 4). */
#define Q2_TXT_W 400
#define Q2_TXT_H 32
static void q2_text_2d(u32 *buf, const char *text, u32 color)
{
    int x = 0;
    const char *p;
    for (p = text; *p && x + 6 <= Q2_TXT_W/4; p++) {
        const unsigned char *g = q2_glyph(*p);   /* g[row], bit 4..0 = cols */
        int row, col, sy, sx;
        for (row = 0; row < 7; row++) {
            unsigned char bits = g[row];
            for (col = 0; col < 5; col++) {
                if (bits & (1u << (4 - col))) {
                    for (sy = 0; sy < 4; sy++)
                        for (sx = 0; sx < 4; sx++) {
                            int px = x*4 + col*4 + sx;
                            int py = row*4 + sy;
                            if (px < Q2_TXT_W && py < Q2_TXT_H)
                                buf[py*Q2_TXT_W + px] = color;
                        }
                }
            }
        }
        x += 6;
    }
}

void q2_show_splash(const char *line1, const char *line2)
{
    struct nsh_fbinfo fi;
    u32 row[2048];
    struct nsh_fbwrite rq;
    int y, x;

    if (sys_fbinfo(&fi) != 0 || !fi.present || fi.bpp != 32)
        return;
    {
        unsigned int fw = (unsigned int)(fi.width < 2048 ? fi.width : 2048);
        unsigned int fh = (unsigned int)(fi.height < 800 ? fi.height : 800);
        for (y = 0; y < (int)fh; y++) {
            for (x = 0; x < (int)fw; x++) row[x] = 0xff102040u;
            rq.x = 0; rq.y = (u64)y; rq.w = fw; rq.h = 1;
            rq.pixels = (u64)(unsigned long)row;
            rq.pixlen = (u64)fw * 4u;
            if (sys_fbwrite(&rq) != 0) {
                putf("Q2 fbwrite FAIL y=%d w=%d len=%d\n",
                     (int)rq.y, (int)rq.w, (int)rq.pixlen);
                break;
            }
        }
    }
    for (y = 120; y < 168; y++) {
        for (x = 40; x < 984; x++) row[x] = 0xff2050a0u;
        rq.x = 0; rq.y = (u64)y; rq.w = 1024; rq.h = 1;
        rq.pixels = (u64)(unsigned long)row; rq.pixlen = 1024u * 4u;
        sys_fbwrite(&rq);
    }
    {
        u32 txt[Q2_TXT_W * Q2_TXT_H];
        int ty;
        /* Title */
        for (ty = 0; ty < Q2_TXT_H*Q2_TXT_W; ty++) txt[ty] = 0xff102040u;
        q2_text_2d(txt, line1, 0xffffffffu);
        for (ty = 0; ty < Q2_TXT_H; ty++) {
            rq.x = 48; rq.y = (u64)(124 + ty); rq.w = Q2_TXT_W; rq.h = 1;
            rq.pixels = (u64)(unsigned long)(txt + ty*Q2_TXT_W);
            rq.pixlen = Q2_TXT_W * 4u;
            sys_fbwrite(&rq);
        }
        /* Subtitle */
        for (ty = 0; ty < Q2_TXT_H*Q2_TXT_W; ty++) txt[ty] = 0xff102040u;
        q2_text_2d(txt, line2, 0xffccccccu);
        for (ty = 0; ty < Q2_TXT_H; ty++) {
            rq.x = 48; rq.y = (u64)(200 + ty); rq.w = Q2_TXT_W; rq.h = 1;
            rq.pixels = (u64)(unsigned long)(txt + ty*Q2_TXT_W);
            rq.pixlen = Q2_TXT_W * 4u;
            sys_fbwrite(&rq);
        }
    }
}

void SWimp_BeginFrame(float camera_separation)
{
    (void)camera_separation;
}

void SWimp_EndFrame(void)
{
    struct nsh_fbinfo fi;

    if (sys_fbinfo(&fi) != 0 || !fi.present)
        return;
    if (fi.bpp == 32 && fi.width && fi.height && vid.buffer) {
        /* Fullscreen: stretch the software renderer's output to fill the
         * whole framebuffer (no centered letterbox). Nearest-neighbour
         * scaling keeps it integer-only; q2_row caps the width at 1024,
         * which matches the BGA 1024x768 mode. */
        int sw = (int)vid.width, sh = (int)vid.height;
        int dw = (int)(fi.width < 1024u ? fi.width : 1024u);
        int dh = (int)fi.height;
        int dy;

        for (dy = 0; dy < dh; dy++) {
            int sy = (sh > 0) ? (dy * sh / dh) : 0;
            const unsigned char *src = vid.buffer + (u64)sy * vid.rowbytes;
            int dx;
            for (dx = 0; dx < dw; dx++) {
                int sx = (sw > 0) ? (dx * sw / dw) : 0;
                const unsigned char *c = &q2_palette[src[sx] * 3];
                q2_row[dx] = 0xff000000u | ((u32)c[0] << 16) |
                             ((u32)c[1] << 8) | c[2];
            }
            struct nsh_fbwrite rq;
            rq.x = 0; rq.y = (u64)dy;
            rq.w = (u64)dw; rq.h = 1;
            rq.pixels = (u64)(unsigned long)q2_row;
            rq.pixlen = (u64)dw * 4u;
            sys_fbwrite(&rq);
        }
    }
}

int SWimp_Init(void *hInstance, void *wndProc)
{
    struct nsh_fbinfo fi;
    (void)hInstance; (void)wndProc;

    /* Stop fbcon from painting kernel logs over our framebuffer. */
    sys_console_gui(1);

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
    /* Draw a visible banner: the renderer only reaches SetMode once the
     * engine starts, and with no PAK data nothing else would be drawn. */
    q2_show_splash("QUAKE 2 FOR NEWOS", "NEEDS PAK0.PAK - SEE README");
    return rserr_ok;
}

void SWimp_AppActivate(qboolean active)
{
    (void)active;
}
