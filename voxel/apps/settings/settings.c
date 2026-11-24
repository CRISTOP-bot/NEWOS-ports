#include "voxel/app.h"
#include "voxel/classic_ui.h"

/* Display settings now persist in an 8.3-safe file on the FAT32 disk. The
 * payload is two hex lines (<64 bytes), which keeps every SYS_WRITE call
 * inside the kernel's 512-byte bounce buffer. Without a disk the app stays
 * session-local and says so on screen. */
#define VOXEL_CFG_PATH "/fat/VOXEL.CFG"

struct settings_state { int loaded; int saved; int scale; };
static struct settings_state *st(struct voxel_app_context *a)
{
    return (struct settings_state *)a->state;
}

static uint32_t hex_digit(char c)
{
    if (c >= '0' && c <= '9') return (uint32_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint32_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (uint32_t)(c - 'A' + 10);
    return 0xffffffffu;
}

static uint32_t hex_read(const char *s, unsigned count)
{
    uint32_t v = 0;
    unsigned i;
    for (i = 0; i < count; i++) {
        uint32_t d = hex_digit(s[i]);
        if (d == 0xffffffffu) return 0xffffffffu;
        v = (v << 4) | d;
    }
    return v;
}

static void hex_write(char *dst, uint32_t v)
{
    static const char digits[] = "0123456789ABCDEF";
    int i;
    for (i = 7; i >= 0; i--) {
        dst[i] = digits[v & 0xFu];
        v >>= 4;
    }
    dst[8] = 0;
}

static const char *cfg_value(const char *buf, uint32_t len, const char *key)
{
    uint32_t i = 0, klen = 0;
    while (key[klen]) klen++;
    while (i < len) {
        uint32_t j = 0;
        int match = 1;
        while (j < klen && i + j < len) {
            if (buf[i + j] != key[j]) { match = 0; break; }
            j++;
        }
        if (match && i + klen + 8 <= len && buf[i + klen] == '=')
            return buf + i + klen + 1;
        while (i < len && buf[i] != '\n') i++;
        i++;
    }
    return 0;
}

static int settings_save(struct voxel_app_context *a)
{
    struct voxel_theme_state *t = a->host ? a->host->theme : 0;
    struct settings_state *s = st(a);
    char buf[64];
    uint32_t n = 0;

    if (!t || !a->host || !a->host->write_file) return -1;
    buf[n++] = 'd'; buf[n++] = 'e'; buf[n++] = 's'; buf[n++] = 'k';
    buf[n++] = 't'; buf[n++] = 'o'; buf[n++] = 'p'; buf[n++] = '=';
    hex_write(buf + n, t->desktop); n += 8;
    buf[n++] = '\n';
    buf[n++] = 'f'; buf[n++] = 'a'; buf[n++] = 'c'; buf[n++] = 'e';
    buf[n++] = '=';
    hex_write(buf + n, t->face); n += 8;
    buf[n++] = '\n';
    buf[n++] = 't'; buf[n++] = 'i'; buf[n++] = 't'; buf[n++] = 'l';
    buf[n++] = 'e'; buf[n++] = '=';
    hex_write(buf + n, t->title); n += 8;
    buf[n++] = '\n';
    buf[n++] = 's'; buf[n++] = 'c'; buf[n++] = 'a'; buf[n++] = 'l';
    buf[n++] = 'e'; buf[n++] = '='; buf[n++] = s->scale ? '1' : '0';
    buf[n++] = '\n';
    if (a->host->write_file(a->host->userdata, VOXEL_CFG_PATH, buf, n)) {
        s->saved = -1;
        return -1;
    }
    s->saved = 1;
    return 0;
}

void voxel_settings_init(struct voxel_app_context *a)
{
    struct settings_state *s = st(a);
    char buf[64];
    uint32_t n = 0;
    const char *v;

    s->loaded = 0;
    s->saved = 0;
    s->scale = 0;
    if (!a->host || !a->host->read_file || !a->host->theme) return;
    if (a->host->read_file(a->host->userdata, VOXEL_CFG_PATH, buf,
                           sizeof(buf), &n)) return;
    v = cfg_value(buf, n, "desktop");
    if (v) {
        uint32_t color = hex_read(v, 8);
        if (color != 0xffffffffu) a->host->theme->desktop = color;
    }
    v = cfg_value(buf, n, "face");
    if (v) {
        uint32_t color = hex_read(v, 8);
        if (color != 0xffffffffu) a->host->theme->face = color;
    }
    v = cfg_value(buf, n, "title");
    if (v) {
        uint32_t color = hex_read(v, 8);
        if (color != 0xffffffffu) a->host->theme->title = color;
    }
    {
        const char *p;
        for (p = buf; (uint32_t)(p - buf) + 7u < n; p++) {
            if (p[0]=='s' && p[1]=='c' && p[2]=='a' && p[3]=='l' &&
                p[4]=='e' && p[5]=='=' && p[6]=='1')
                s->scale = 1;
        }
    }
    s->loaded = 1;
    a->host->theme->desktop_saved = a->host->theme->desktop;
    a->host->theme->face_saved = a->host->theme->face;
}

void voxel_settings_draw(struct voxel_app_context *a)
{
    struct voxel_theme_state *t = a->host ? a->host->theme : 0;
    struct settings_state *s = st(a);
    char hex[9];

    voxel_surface_clear(a->surface, VOXEL_UI_FACE);
    voxel_ui_window_frame(a->surface, (struct voxel_ui_rect){2,2,(int32_t)a->surface->width-4,(int32_t)a->surface->height-4}, "Display Properties", 1);
    voxel_ui_text(a->surface, 16, 38, "DESKTOP COLOR", VOXEL_UI_TEXT);
    voxel_ui_checkbox(a->surface, (struct voxel_ui_rect){16, 54, 130, 16}, "TEAL CLASSIC", t && t->desktop == VOXEL_UI_DESKTOP);
    voxel_ui_checkbox(a->surface, (struct voxel_ui_rect){16, 74, 130, 16}, "FOREST GREEN", t && t->desktop == 0x00008000u);
    voxel_ui_text(a->surface, 16, 102, "WINDOW THEME", VOXEL_UI_TEXT);
    voxel_ui_radio(a->surface, (struct voxel_ui_rect){16, 118, 130, 16}, "GRAY / NAVY", t && t->face == VOXEL_UI_FACE);
    voxel_ui_radio(a->surface, (struct voxel_ui_rect){16, 138, 130, 16}, "LIGHT GRAY", t && t->face == 0x00a8a8a8u);
    voxel_ui_radio(a->surface, (struct voxel_ui_rect){16, 158, 130, 16}, "SLATE BLUE", t && t->face == 0x00c0d0e0u);
    voxel_ui_checkbox(a->surface, (struct voxel_ui_rect){16, 178, 180, 16}, "FULLSCREEN (RESTART)", s->scale);
    voxel_ui_text(a->surface, 16, 202, "SYSTEM LOAD", VOXEL_UI_TEXT);
    voxel_ui_progress(a->surface, (struct voxel_ui_rect){16, 218, (int32_t)a->surface->width-32, 14}, 35, 100);
    if (s->saved < 0)
        voxel_ui_text(a->surface, 16, 240, "NO DISK: SESSION ONLY", VOXEL_UI_TEXT);
    else if (s->saved)
        voxel_ui_text(a->surface, 16, 240, "SAVED TO /FAT/VOXEL.CFG", VOXEL_UI_TEXT);
    else if (s->loaded)
        voxel_ui_text(a->surface, 16, 240, "LOADED FROM /FAT/VOXEL.CFG", VOXEL_UI_TEXT);
    else
        voxel_ui_text(a->surface, 16, 240, "PRESS OK TO SAVE TO /FAT", VOXEL_UI_TEXT);
    if (t) {
        hex_write(hex, t->desktop);
        voxel_ui_text(a->surface, 160, 58, hex, VOXEL_UI_TEXT);
        hex_write(hex, t->face);
        voxel_ui_text(a->surface, 160, 110, hex, VOXEL_UI_TEXT);
        hex_write(hex, t->title);
        voxel_ui_text(a->surface, 160, 160, hex, VOXEL_UI_TEXT);
    }
    voxel_ui_button(a->surface, (struct voxel_ui_rect){(int32_t)a->surface->width-86, (int32_t)a->surface->height-34, 72, 22}, "OK", s->saved < 0);
}

int voxel_settings_pointer(struct voxel_app_context *a, int32_t x, int32_t y)
{
    struct voxel_theme_state *t = a->host ? a->host->theme : 0;
    struct settings_state *s = st(a);

    if (!t) return 0;
    if (y >= (int32_t)a->surface->height-34 && y < (int32_t)a->surface->height-12 &&
        x >= (int32_t)a->surface->width-86 && x < (int32_t)a->surface->width-14) {
        settings_save(a);
        return 1;
    }
    if (y >= 54 && y < 70) { t->desktop = VOXEL_UI_DESKTOP; s->saved = 0; return 1; }
    if (y >= 74 && y < 90) { t->desktop = 0x00008000u; s->saved = 0; return 1; }
    if (y >= 118 && y < 134) { t->face = VOXEL_UI_FACE; t->title = VOXEL_UI_NAVY; s->saved = 0; return 1; }
    if (y >= 138 && y < 154) { t->face = 0x00a8a8a8u; t->title = 0x00008080u; s->saved = 0; return 1; }
    if (y >= 158 && y < 174) { t->face = 0x00c0d0e0u; t->title = 0x00003070u; s->saved = 0; return 1; }
    if (y >= 178 && y < 194 && x >= 16 && x < 196) {
        s->scale = !s->scale; s->saved = 0; return 1;
    }
    return 0;
}
