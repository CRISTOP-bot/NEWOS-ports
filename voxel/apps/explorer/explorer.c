#include "voxel/app.h"
#include "voxel/classic_ui.h"
#include "../common/virtual_keyboard.h"

/* File manager backed by the real NEWOS VFS through the host callbacks.
 * Adds creation/deletion, a typed path prompt, selection and sizes on top of
 * the original listing/enter/parent browser. */
struct explorer_state {
    char path[VOXEL_APP_PATH_MAX];
    char prompt[VOXEL_APP_PATH_MAX];
    char status[VOXEL_APP_PATH_MAX];
    struct voxel_fs_entry entries[8];
    int count;
    int selected;
    unsigned prompt_len;
    int mode;                       /* 0 = browse, 1 = mkdir name, 2 = path */
};

static struct explorer_state *st(struct voxel_app_context *a)
{
    return (struct explorer_state *)a->state;
}

static void status_set(struct explorer_state *s, const char *msg)
{
    unsigned i = 0;
    while (msg[i] && i < VOXEL_APP_PATH_MAX - 1) { s->status[i] = msg[i]; i++; }
    s->status[i] = 0;
}

static void refresh(struct voxel_app_context *a)
{
    struct explorer_state *s = st(a);
    s->count = -1;
    s->selected = 0;
    if (a->host && a->host->list_dir)
        s->count = a->host->list_dir(a->host->userdata, s->path,
                                     s->entries, 8u);
}

void voxel_explorer_init(struct voxel_app_context *a)
{
    struct explorer_state *s = st(a);
    s->path[0] = '/'; s->path[1] = 0;
    s->prompt[0] = 0; s->prompt_len = 0;
    s->mode = 0;
    status_set(s, "N: NEW DIR   P: TYPE PATH   DEL: DELETE");
    refresh(a);
}

static void full_path(struct explorer_state *s, const char *name, char *out)
{
    unsigned b = 0, n = 0;
    while (s->path[b] && b < VOXEL_APP_PATH_MAX - 2) { out[b] = s->path[b]; b++; }
    if (b > 1 && out[b-1] != '/') out[b++] = '/';
    while (name[n] && n < VOXEL_FS_NAME_MAX - 1 &&
           b < VOXEL_APP_PATH_MAX - 1) out[b++] = name[n++];
    out[b] = 0;
}

int voxel_explorer_parent(struct voxel_app_context *a)
{
    struct explorer_state *s = st(a);
    unsigned n = 0;
    if (!s->path[0] || (s->path[0] == '/' && !s->path[1])) return 0;
    while (s->path[n] && n < VOXEL_APP_PATH_MAX - 1) n++;
    if (n > 1 && s->path[n-1] == '/') --n;
    while (n > 1 && s->path[n-1] != '/') --n;
    if (n > 1) --n;
    s->path[n] = 0;
    status_set(s, "PARENT DIRECTORY");
    refresh(a);
    return 1;
}

int voxel_explorer_open_selected(struct voxel_app_context *a, int row)
{
    struct explorer_state *s = st(a);
    char full[VOXEL_APP_PATH_MAX];
    unsigned b = 0;

    if (row < 0 || row >= s->count || row >= 8) return -1;
    s->selected = row;
    full_path(s, s->entries[row].name, full);
    if (s->entries[row].mode & VOXEL_FS_MODE_DIR) {
        for (b = 0; b < VOXEL_APP_PATH_MAX - 1 && full[b]; ++b)
            s->path[b] = full[b];
        s->path[b] = 0;
        status_set(s, "ENTERED DIRECTORY");
        refresh(a);
        return 1;
    }
    status_set(s, "OPENING FILE");
    if (a->host && a->host->open_image)
        if (a->host->open_image(a->host->userdata, full) == 0) return 1;
    if (a->host && a->host->open_path) {
        a->host->open_path(a->host->userdata, full);
        return 1;
    }
    return -1;
}

static int start_mode(struct explorer_state *s, int mode, const char *seed)
{
    unsigned i = 0;
    s->mode = mode;
    while (seed[i] && i < VOXEL_APP_PATH_MAX - 1) { s->prompt[i] = seed[i]; i++; }
    s->prompt[i] = 0;
    s->prompt_len = i;
    return 1;
}

static int commit_prompt(struct voxel_app_context *a)
{
    struct explorer_state *s = st(a);
    char full[VOXEL_APP_PATH_MAX];

    if (!s->prompt_len) { s->mode = 0; return 1; }
    if (s->mode == 1) {
        full_path(s, s->prompt, full);
        if (a->host && a->host->mkdir &&
            a->host->mkdir(a->host->userdata, full) == 0) {
            status_set(s, "DIRECTORY CREATED");
            refresh(a);
        } else {
            status_set(s, "MKDIR FAILED");
        }
    } else if (s->mode == 2) {
        unsigned i;
        for (i = 0; i < VOXEL_APP_PATH_MAX - 1 && s->prompt[i]; i++)
            s->path[i] = s->prompt[i];
        s->path[i] = 0;
        status_set(s, "PATH CHANGED");
        refresh(a);
    }
    s->mode = 0;
    s->prompt[0] = 0;
    s->prompt_len = 0;
    return 1;
}

static int delete_selected(struct voxel_app_context *a)
{
    struct explorer_state *s = st(a);
    char full[VOXEL_APP_PATH_MAX];

    if (!a->host || !a->host->unlink || s->count <= 0 ||
        s->selected >= s->count || s->selected >= 8) return 0;
    if (s->entries[s->selected].mode & VOXEL_FS_MODE_DIR) {
        status_set(s, "REMOVE EMPTY DIRS WITH RMDIR");
        return 1;
    }
    full_path(s, s->entries[s->selected].name, full);
    if (a->host->unlink(a->host->userdata, full) == 0) {
        status_set(s, "FILE DELETED");
        refresh(a);
    } else {
        status_set(s, "DELETE FAILED");
    }
    return 1;
}

void voxel_explorer_draw(struct voxel_app_context *a)
{
    struct explorer_state *s = st(a);
    int i;
    int32_t h = (int32_t)a->surface->height;

    voxel_surface_clear(a->surface, VOXEL_UI_FACE);
    voxel_ui_window_frame(a->surface, (struct voxel_ui_rect){2,2,(int32_t)a->surface->width-4,h-4}, "File Manager (VFS)", 1);
    voxel_ui_button(a->surface, (struct voxel_ui_rect){10,30,56,18}, "UP", 0);
    voxel_ui_button(a->surface, (struct voxel_ui_rect){72,30,56,18}, "NEW", 0);
    voxel_ui_button(a->surface, (struct voxel_ui_rect){134,30,56,18}, "DEL", 0);
    voxel_ui_button(a->surface, (struct voxel_ui_rect){196,30,44,18}, "GO", 0);
    voxel_ui_text(a->surface, 248, 34, s->path, VOXEL_UI_TEXT);
    voxel_ui_bevel(a->surface, (struct voxel_ui_rect){10,52,(int32_t)a->surface->width-20,h-158},1);

    if (s->count < 0) {
        voxel_ui_text(a->surface,18,60,"DIRECTORY LIST UNAVAILABLE",VOXEL_UI_TEXT);
    } else if (!s->count) {
        voxel_ui_text(a->surface,18,60,"THIS DIRECTORY IS EMPTY",VOXEL_UI_TEXT);
    } else {
        for (i = 0; i < s->count && i < 8; i++) {
            char label[VOXEL_FS_NAME_MAX + 6];
            unsigned j = 0, k = 2;
            label[0] = (s->entries[i].mode & VOXEL_FS_MODE_DIR) ? '[' : ' ';
            label[1] = ' ';
            for (j = 0; j < VOXEL_FS_NAME_MAX - 4 && s->entries[i].name[j]; j++)
                label[k++] = s->entries[i].name[j];
            label[k] = 0;
            if (i == s->selected)
                voxel_surface_fill_rect(a->surface, 13, 54+i*18,
                    (int32_t)a->surface->width-26, 17, VOXEL_UI_SELECTION);
            voxel_ui_text(a->surface,18,58+i*18,label,VOXEL_UI_TEXT);
        }
    }
    voxel_ui_text(a->surface, 16, h-116, s->status, VOXEL_UI_TEXT);
    if (s->mode)
        voxel_ui_text_field(a->surface,
            (struct voxel_ui_rect){10,h-112,(int32_t)a->surface->width-20,22},
            s->prompt, s->prompt_len);
    voxel_vk_draw(a->surface, h-90);
}

int voxel_explorer_pointer(struct voxel_app_context *a, int32_t x, int32_t y)
{
    struct explorer_state *s = st(a);
    int32_t h = (int32_t)a->surface->height;
    int row;
    uint32_t key = 0;

    if (y >= h-90 && s->mode &&
        voxel_vk_pointer(a->surface, x, y, h-90, s->prompt,
                         sizeof(s->prompt), &s->prompt_len, &key)) {
        if (key == 13u) return commit_prompt(a);   /* virtual OK key */
        return 1;
    }
    if (y >= 30 && y < 48) {
        if (x >= 10 && x < 66) return voxel_explorer_parent(a);
        if (x >= 72 && x < 128) return start_mode(s, 1, "");
        if (x >= 134 && x < 190) return delete_selected(a);
        if (x >= 196 && x < 240) return start_mode(s, 2, s->path);
        return 0;
    }
    if (x < 12 || x > (int32_t)a->surface->width-12 || y < 54 || y > h-124)
        return 0;
    row = (y-54)/18;
    return voxel_explorer_open_selected(a, row);
}

int voxel_explorer_key(struct voxel_app_context *a, uint32_t k)
{
    struct explorer_state *s = st(a);

    if (s->mode) {
        if (k == VOXEL_KEY_ENTER) return commit_prompt(a);
        if (k == VOXEL_KEY_BACKSPACE) {
            if (s->prompt_len) s->prompt[--s->prompt_len] = 0;
            return 1;
        }
        if (k >= 32 && k < 127 && s->prompt_len + 1 < VOXEL_APP_PATH_MAX) {
            s->prompt[s->prompt_len++] = (char)k;
            s->prompt[s->prompt_len] = 0;
            return 1;
        }
        return 1;                    /* swallow other keys while editing */
    }
    if (k == VOXEL_KEY_ENTER) return voxel_explorer_open_selected(a, s->selected);
    if (k == VOXEL_KEY_BACKSPACE) return voxel_explorer_parent(a);
    if (k == VOXEL_KEY_DELETE) return delete_selected(a);
    if (k == 'n' || k == 'N') return start_mode(s, 1, "");
    if (k == 'p' || k == 'P') return start_mode(s, 2, s->path);
    return 0;
}
