#include "voxel/app.h"
#include "voxel/classic_ui.h"

void voxel_about_draw(struct voxel_app_context *a)
{
    voxel_surface_clear(a->surface, VOXEL_UI_FACE);
    voxel_ui_window_frame(a->surface, (struct voxel_ui_rect){2,2,(int32_t)a->surface->width-4,(int32_t)a->surface->height-4}, "About VOXEL", 1);
    voxel_ui_icon(a->surface, 16, 44, 0, "VOXEL", 0);
    voxel_ui_text(a->surface, 90, 52, "VOXEL CLASSIC DESKTOP", VOXEL_UI_TEXT);
    voxel_ui_text(a->surface, 90, 68, "SOFTWARE UI / NEWOS", VOXEL_UI_TEXT);
    voxel_ui_bevel(a->surface, (struct voxel_ui_rect){16, 92, (int32_t)a->surface->width-32, 1}, 1);
    voxel_ui_text(a->surface, 16, 104, "Version 0.3.0", VOXEL_UI_TEXT);
    voxel_ui_text(a->surface, 16, 120, "Independent graphical shell demo", VOXEL_UI_TEXT);
    voxel_ui_text(a->surface, 16, 136, "Built with the VOXEL surface compositor", VOXEL_UI_TEXT);
    voxel_ui_text(a->surface, 16, 152, "MIT licensed, NEWOS-native ELF", VOXEL_UI_TEXT);
    voxel_ui_progress(a->surface, (struct voxel_ui_rect){16, 176, (int32_t)a->surface->width-32, 14}, 72, 100);
    voxel_ui_text(a->surface, 16, 196, "SYSTEM LOAD", VOXEL_UI_TEXT);
    voxel_ui_button(a->surface, (struct voxel_ui_rect){(int32_t)a->surface->width-86, (int32_t)a->surface->height-34, 72, 22}, "OK", 0);
}
