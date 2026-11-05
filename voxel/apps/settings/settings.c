#include "voxel/app.h"
#include "voxel/classic_ui.h"

void voxel_settings_draw(struct voxel_app_context *a)
{
    struct voxel_theme_state *t = a->host ? a->host->theme : 0;
    voxel_surface_clear(a->surface, VOXEL_UI_FACE);
    voxel_ui_window_frame(a->surface, (struct voxel_ui_rect){2,2,(int32_t)a->surface->width-4,(int32_t)a->surface->height-4}, "Display Properties", 1);
    voxel_ui_text(a->surface, 16, 38, "DESKTOP COLOR", VOXEL_UI_TEXT);
    voxel_ui_checkbox(a->surface, (struct voxel_ui_rect){16, 54, 130, 16}, "TEAL CLASSIC", t && t->desktop == VOXEL_UI_DESKTOP);
    voxel_ui_checkbox(a->surface, (struct voxel_ui_rect){16, 74, 130, 16}, "FOREST GREEN", t && t->desktop == 0x00008000u);
    voxel_ui_text(a->surface, 16, 102, "WINDOW THEME", VOXEL_UI_TEXT);
    voxel_ui_radio(a->surface, (struct voxel_ui_rect){16, 118, 130, 16}, "GRAY / NAVY", t && t->face == VOXEL_UI_FACE);
    voxel_ui_radio(a->surface, (struct voxel_ui_rect){16, 138, 130, 16}, "LIGHT GRAY", t && t->face == 0x00a8a8a8u);
    voxel_ui_text(a->surface, 16, 166, "SYSTEM LOAD", VOXEL_UI_TEXT);
    voxel_ui_progress(a->surface, (struct voxel_ui_rect){16, 182, (int32_t)a->surface->width-32, 14}, 35, 100);
    voxel_ui_text(a->surface, 16, 204, "Settings are session-local.", VOXEL_UI_TEXT);
    if (t) {
        voxel_ui_text(a->surface, 160, 58, "DESKTOP", t->desktop);
        voxel_ui_text(a->surface, 160, 110, "WINDOWS", t->face);
    }
    voxel_ui_button(a->surface, (struct voxel_ui_rect){(int32_t)a->surface->width-86, (int32_t)a->surface->height-34, 72, 22}, "OK", 0);
}

int voxel_settings_pointer(struct voxel_app_context *a, int32_t x, int32_t y)
{
    struct voxel_theme_state *t = a->host ? a->host->theme : 0;
    (void)x;
    if (!t) return 0;
    if (y >= 54 && y < 70) { t->desktop = VOXEL_UI_DESKTOP; return 1; }
    if (y >= 74 && y < 90) { t->desktop = 0x00008000u; return 1; }
    if (y >= 118 && y < 134) { t->face = VOXEL_UI_FACE; return 1; }
    if (y >= 138 && y < 154) { t->face = 0x00a8a8a8u; return 1; }
    return 0;
}
