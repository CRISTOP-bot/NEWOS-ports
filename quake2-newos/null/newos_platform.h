/*
 * newos_platform.h - shared declarations for the NEWOS Quake 2 backends.
 *
 * The NEWOS framebuffer is XRGB8888 and SYS_FBWRITE consumes XRGB words.
 * Quake 2's software renderer draws 8-bit paletized pixels into vid.buffer
 * and hands us the 256-entry RGB palette via SWimp_SetPalette. On each
 * EndFrame we expand the 8-bit buffer to a 32-bit row and blit it.
 */
#ifndef NEWOS_PLATFORM_H
#define NEWOS_PLATFORM_H

#include "../../../user/lib/nshlib.h"

/* Framebuffer geometry (filled by SWimp_Init via SYS_FBINFO). */
extern u64  q2_fb_width, q2_fb_height, q2_fb_pitch;
extern u32  q2_fb_bpp;

/* Palette + translation table set by SWimp_SetPalette. */
extern unsigned char q2_palette[256 * 3];

/* Expand one row of 8-bit paletized pixels to XRGB8888 words. */
void q2_expand_row(const unsigned char *src, u32 *dst, int width);
void q2_show_splash(const char *line1, const char *line2);

#endif
