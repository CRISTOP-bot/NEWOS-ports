#include "voxel/classic_ui.h"
#include "voxel/app.h"

#include <stddef.h>

static struct voxel_theme_state *g_palette;

void voxel_ui_set_palette(struct voxel_theme_state *theme)
{
    g_palette = theme;
}

static uint32_t ui_face(void)
{
    return g_palette && g_palette->face ? g_palette->face : VOXEL_UI_FACE;
}

static uint32_t ui_title(void)
{
    return g_palette && g_palette->title ? g_palette->title : VOXEL_UI_NAVY;
}

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define TITLE_HEIGHT 22
#define TASKBAR_HEIGHT 30

static int valid(const struct voxel_surface *s)
{
    return s && s->pixels && s->width && s->height &&
           s->stride_pixels >= s->width &&
           s->stride_pixels <= SIZE_MAX / s->height &&
           s->stride_pixels * s->height <= s->capacity_pixels;
}

static uint32_t surface_color(const struct voxel_surface *s, uint32_t color)
{
    return s && s->format == VOXEL_ARGB8888 ? color | 0xff000000u : color;
}

static void rect(struct voxel_surface *s, int32_t x, int32_t y,
                 int32_t width, int32_t height, uint32_t color)
{
    if (valid(s) && width > 0 && height > 0)
        (void)voxel_surface_fill_rect(s, x, y, width, height,
                                      surface_color(s, color));
}

static uint8_t row_bits(char ch, int row)
{
    static const uint8_t alpha[26][7] = {
        {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},
        {30,17,17,17,17,17,30},{31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
        {14,17,16,23,17,17,15},{17,17,17,31,17,17,17},{14,4,4,4,4,4,14},
        {7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
        {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},
        {30,17,17,30,16,16,16},{14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
        {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},{17,17,17,17,17,17,14},
        {17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
        {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}
    };
    static const uint8_t numbers[10][7] = {
        {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},
        {30,1,1,14,1,1,30},{2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
        {14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},
        {14,17,17,15,1,1,14}
    };
    if (row < 0 || row > 6) return 0;
    if (ch >= 'a' && ch <= 'z') ch = (char)(ch - 'a' + 'A');
    if (ch >= 'A' && ch <= 'Z') return alpha[ch - 'A'][row];
    if (ch >= '0' && ch <= '9') return numbers[ch - '0'][row];
    switch (ch) {
    case ' ': return 0;
    case '.': return row == 6 ? 4 : 0;
    case ',': return row == 5 ? 4 : (row == 6 ? 8 : 0);
    case ':': return (row == 2 || row == 5) ? 4 : 0;
    case '-': return row == 3 ? 14 : 0;
    case '_': return row == 6 ? 31 : 0;
    case '!': return row < 5 ? 4 : (row == 6 ? 4 : 0);
    case '?': { static const uint8_t q[7]={14,17,1,2,4,0,4}; return q[row]; }
    case '+': return row == 3 ? 14 : ((row == 2 || row == 4) ? 4 : 0);
    case '(': return row < 3 ? 2 : (row == 3 ? 4 : 2);
    case ')': return row < 3 ? 8 : (row == 3 ? 4 : 8);
    case '/': return (uint8_t)(1u << (row > 4 ? 0 : 4-row));
    default: return 0;
    }
}

void voxel_ui_text(struct voxel_surface *s, int32_t x, int32_t y,
                   const char *str, uint32_t color)
{
    size_t i;
    int row, col;
    if (!valid(s) || !str) return;
    for (i = 0; str[i]; ++i) {
        for (row = 0; row < 7; ++row) {
            uint8_t bits = row_bits(str[i], row);
            for (col = 0; col < 5; ++col)
                if (bits & (1u << (4-col)))
                    (void)voxel_surface_put_pixel(s, x+(int32_t)i*6+col,
                                                  y+row, surface_color(s, color));
        }
    }
}

/* Draw text rotated 90 degrees counter-clockwise (reads bottom-to-top),
 * used by the Start menu's classic vertical banner. */
static void text_rotated(struct voxel_surface *s, int32_t x, int32_t y,
                         const char *str, uint32_t color)
{
    int32_t cy = y;
    const char *p;
    if (!valid(s) || !str) return;
    for (p = str; *p; ++p) {
        int row, col;
        for (row = 0; row < 7; ++row) {
            uint8_t bits = row_bits(*p, row);
            for (col = 0; col < 5; ++col)
                if (bits & (1u << (4 - col)))
                    rect(s, x + row, cy - col, 1, 1, color);
        }
        cy -= 8;
    }
}

void voxel_ui_bevel(struct voxel_surface *s, struct voxel_ui_rect r, int pressed)
{
    uint32_t hi = pressed ? VOXEL_UI_DARK : VOXEL_UI_LIGHT;
    uint32_t hi2 = pressed ? VOXEL_UI_MID : VOXEL_UI_WHITE;
    uint32_t lo = pressed ? VOXEL_UI_WHITE : VOXEL_UI_DARK;
    uint32_t lo2 = pressed ? ui_face() : VOXEL_UI_MID;
    if (!valid(s) || r.width < 2 || r.height < 2) return;
    rect(s,r.x,r.y,r.width,1,hi); rect(s,r.x,r.y,1,r.height,hi);
    rect(s,r.x+1,r.y+1,r.width-2,1,hi2); rect(s,r.x+1,r.y+1,1,r.height-2,hi2);
    rect(s,r.x,r.y+r.height-1,r.width,1,lo); rect(s,r.x+r.width-1,r.y,1,r.height,lo);
    if (r.width>3 && r.height>3) {
        rect(s,r.x+1,r.y+r.height-2,r.width-2,1,lo2);
        rect(s,r.x+r.width-2,r.y+1,1,r.height-2,lo2);
    }
}

void voxel_ui_button(struct voxel_surface *s, struct voxel_ui_rect r,
                     const char *label, int pressed)
{
    size_t n=0;
    int32_t tx,ty;
    if (!valid(s)) return;
    if (label) while (label[n]) ++n;
    rect(s,r.x,r.y,r.width,r.height,ui_face());
    voxel_ui_bevel(s,r,pressed);
    tx=r.x+MAX(4,(r.width-(int32_t)n*6)/2)+(pressed?1:0);
    ty=r.y+MAX(3,(r.height-7)/2)+(pressed?1:0);
    voxel_ui_text(s,tx,ty,label,VOXEL_UI_TEXT);
}

/* Win95-style control boxes: beveled button plus line/square/X glyph drawn
 * from pixels (the 5x7 font has no good maximize glyph). */
static void title_button(struct voxel_surface *s, struct voxel_ui_rect b,
                         int kind)
{
    int32_t cx = b.x + b.width / 2, cy = b.y + b.height / 2, i;

    voxel_ui_button(s, b, "", 0);
    if (kind == 0) {                       /* minimize */
        rect(s, cx - 4, cy + 2, 9, 1, VOXEL_UI_TEXT);
        return;
    }
    if (kind == 1) {                       /* maximize */
        rect(s, cx - 4, cy - 4, 9, 1, VOXEL_UI_TEXT);
        rect(s, cx - 4, cy + 3, 9, 1, VOXEL_UI_TEXT);
        rect(s, cx - 4, cy - 4, 1, 8, VOXEL_UI_TEXT);
        rect(s, cx + 4, cy - 4, 1, 8, VOXEL_UI_TEXT);
        return;
    }
    for (i = 0; i < 7; i++) {              /* close */
        rect(s, cx - 4 + i, cy - 4 + i, 1, 1, VOXEL_UI_TEXT);
        rect(s, cx + 4 - i, cy - 4 + i, 1, 1, VOXEL_UI_TEXT);
    }
}

void voxel_ui_titlebar(struct voxel_surface *s, struct voxel_ui_rect r,
                       const char *title, int active)
{
    uint32_t i;
    struct voxel_ui_rect b;
    uint32_t text_color = active ? VOXEL_UI_WHITE : VOXEL_UI_MID;
    if (!valid(s) || r.width<55 || r.height<14) return;
    /* Win95 active title gradient: dark navy on the left to blue on the right. */
    for (i = 0; i < (uint32_t)r.width; i++) {
        uint32_t t = (uint32_t)((uint64_t)i * 255u / (r.width ? r.width - 1 : 1));
        uint32_t r1, g1, b1;
        if (active) {
            r1 = (uint32_t)(0x00u + (0x10u - 0x00u) * t / 255u);
            g1 = (uint32_t)(0x00u + (0x54u - 0x00u) * t / 255u);
            b1 = (uint32_t)(0x80u + (0xd0u - 0x80u) * t / 255u);
        } else {
            r1 = 0x80u; g1 = 0x80u; b1 = 0x80u;
        }
        rect(s, r.x + (int32_t)i, r.y, 1, r.height,
             (r1 << 16) | (g1 << 8) | b1);
    }
    /* Small application glyph at the left of the title. */
    rect(s, r.x + 4, r.y + MAX(3,(r.height-11)/2), 11, 11, ui_face());
    rect(s, r.x + 5, r.y + MAX(3,(r.height-11)/2) + 1, 9, 9, ui_title());
    rect(s, r.x + 7, r.y + MAX(3,(r.height-11)/2) + 3, 5, 5, VOXEL_UI_WHITE);
    voxel_ui_text(s, r.x + 18, r.y + MAX(2, (r.height - 7) / 2), title,
                  text_color);
    b = (struct voxel_ui_rect){r.x + r.width - 17, r.y + 3, 14, r.height - 6};
    title_button(s, b, 2);                 /* close */
    b.x -= 16; title_button(s, b, 1);      /* maximize */
    b.x -= 16; title_button(s, b, 0);      /* minimize */
}

void voxel_ui_window_frame(struct voxel_surface *s, struct voxel_ui_rect r,
                           const char *title, int active)
{
    if (!valid(s) || r.width<58 || r.height<48) return;
    /* Drop shadow */
    rect(s, r.x + 3, r.y + 3, r.width, r.height, 0x00404040u);
    rect(s, r.x, r.y, r.width, r.height, ui_face());
    voxel_ui_bevel(s, r, 0);
    voxel_ui_titlebar(s, (struct voxel_ui_rect){r.x + 3, r.y + 3, r.width - 6, TITLE_HEIGHT}, title, active);
    /* Diagonal resize grip in the bottom-right corner. */
    {
        int32_t gx = r.x + r.width - 14, gy = r.y + r.height - 14;
        int k;
        for (k = 0; k < 5; ++k) {
            rect(s, gx + k*2, gy + 10, 2, 2, VOXEL_UI_WHITE);
            rect(s, gx + 10 - k*2, gy + k*2, 2, 2, VOXEL_UI_WHITE);
        }
    }
}

void voxel_ui_menu_bar(struct voxel_surface *s,int32_t x,int32_t y,int32_t width,
                       const char *const *labels,uint32_t count)
{
    uint32_t i;
    if (!valid(s)||width<=0) return;
    rect(s,x,y,width,19,ui_face());
    for(i=0;i<count;++i) voxel_ui_text(s,x+8+(int32_t)i*48,y+6,labels[i],VOXEL_UI_TEXT);
    rect(s,x,y+18,width,1,VOXEL_UI_MID);
}

static void icon_computer(struct voxel_surface *s,int32_t x,int32_t y)
{
    /* Monitor with bezel, screen, highlight and stand. */
    rect(s,x+2,y+1,29,21,VOXEL_UI_DARK);
    rect(s,x+4,y+3,25,16,ui_face());
    rect(s,x+6,y+5,21,12,0x00008080u);
    rect(s,x+7,y+5,19,1,0x0000a0a0u);      /* screen sheen */
    rect(s,x+7,y+6,1,10,0x00006060u);
    rect(s,x+13,y+22,7,3,VOXEL_UI_MID);     /* stand */
    rect(s,x+9,y+25,15,3,ui_face());    /* base */
    rect(s,x+9,y+25,15,1,VOXEL_UI_WHITE);
    rect(s,x+9,y+27,15,1,VOXEL_UI_DARK);
    rect(s,x+26,y+18,2,2,0x0000ff00u);     /* LED */
}
static void icon_folder(struct voxel_surface *s,int32_t x,int32_t y)
{
    /* Folder with raised tab, body shading and front lip. */
    rect(s,x+3,y+4,14,3,0x00a07820u);      /* tab back */
    rect(s,x+5,y+3,11,4,0x00e8c75au);      /* tab front */
    rect(s,x+2,y+7,29,20,0x00806010u);     /* body shadow */
    rect(s,x+2,y+7,29,18,0x00b68b25u);     /* body */
    rect(s,x+4,y+10,25,13,0x00f1d675u);    /* front face */
    rect(s,x+4,y+10,25,1,0x00ffe8a0u);     /* top highlight */
    rect(s,x+2,y+25,29,2,0x00907018u);     /* bottom lip */
}
static void icon_gear(struct voxel_surface *s,int32_t x,int32_t y)
{
    static const uint8_t gear[9]={0x18,0x3c,0x7e,0xff,0xff,0xff,0x7e,0x3c,0x18};
    int row,col;
    for(row=0;row<9;++row) for(col=0;col<8;++col)
        if(gear[row]&(1u<<(7-col))) rect(s,x+col*3,y+row*3,3,3,VOXEL_UI_DARK);
    rect(s,x+9,y+9,6,6,ui_face());     /* hub */
    rect(s,x+10,y+10,4,4,VOXEL_UI_MID);
    rect(s,x+11,y+11,2,2,VOXEL_UI_WHITE);
}

void voxel_ui_icon(struct voxel_surface *s,int32_t x,int32_t y,uint32_t kind,
                   const char *label,int selected)
{
    size_t n=0;
    if(!valid(s)) return;
    if(label) while(label[n]) ++n;
    if(selected) rect(s,x,y,MAX(38,(int32_t)n*6+8),42,VOXEL_UI_SELECTION);
    if(kind==0) icon_computer(s,x+3,y);
    else if(kind==1) icon_folder(s,x+3,y);
    else icon_gear(s,x+9,y+1);
    voxel_ui_text(s,x+2,y+31,label,VOXEL_UI_WHITE);
}

void voxel_ui_taskbar(struct voxel_surface *s,int32_t width,int32_t height,int pressed)
{
    struct voxel_ui_rect start, tray;
    if(!valid(s)||width<80||height<TASKBAR_HEIGHT) return;
    rect(s,0,height-TASKBAR_HEIGHT,width,TASKBAR_HEIGHT,ui_face());
    rect(s,0,height-TASKBAR_HEIGHT,width,1,VOXEL_UI_WHITE);
    /* Start button with the four-color Windows-style flag and bold label. */
    start=(struct voxel_ui_rect){3,height-TASKBAR_HEIGHT+3,72,TASKBAR_HEIGHT-6};
    rect(s,start.x,start.y,start.width,start.height,ui_face());
    voxel_ui_bevel(s,start,pressed);
    rect(s,start.x+4,start.y+3,4,4,0x000000ffu);
    rect(s,start.x+8,start.y+3,4,4,0x0000ff00u);
    rect(s,start.x+4,start.y+7,4,4,0x00ff0000u);
    rect(s,start.x+8,start.y+7,4,4,0x0000ffffu);
    {
        int32_t ty=start.y+MAX(3,(start.height-7)/2);
        voxel_ui_text(s,start.x+16,ty,"Start",VOXEL_UI_TEXT);
        voxel_ui_text(s,start.x+17,ty,"Start",VOXEL_UI_TEXT); /* faux bold */
    }
    /* System tray with clock */
    tray=(struct voxel_ui_rect){width-90,height-TASKBAR_HEIGHT+4,85,TASKBAR_HEIGHT-8};
    voxel_ui_bevel(s,tray,1);
    voxel_ui_text(s,tray.x+8,tray.y+6,"12:00 PM",VOXEL_UI_TEXT);
}

void voxel_ui_start_menu(struct voxel_surface *s,int32_t x,int32_t y,int selected)
{
    static const char *const items[]={"Programs","Documents","Settings","About VOXEL"};
    int i;
    struct voxel_ui_rect r={x,y,186,124};
    if(!valid(s)) return;
    rect(s,r.x,r.y,r.width,r.height,ui_face());
    voxel_ui_bevel(s,r,0);
    /* Classic vertical banner on the left */
    rect(s,x+2,y+2,24,r.height-4,ui_title());
    text_rotated(s,x+6,y+r.height-10,"NEWOS 95",VOXEL_UI_WHITE);
    /* Separator after the banner */
    rect(s,x+27,y+2,1,r.height-4,VOXEL_UI_MID);
    for(i=0;i<4;++i) {
        int32_t iy=y+5+i*22;
        if(i==selected) rect(s,x+30,iy,r.width-33,20,VOXEL_UI_SELECTION);
        /* small item glyph */
        rect(s,x+33,iy+5,10,10,i==selected?VOXEL_UI_WHITE:VOXEL_UI_MID);
        voxel_ui_text(s,x+47,iy+6,items[i],i==selected?VOXEL_UI_WHITE:VOXEL_UI_TEXT);
    }
    /* Bottom separator + Shutdown row */
    rect(s,x+30,y+r.height-26,r.width-33,1,VOXEL_UI_MID);
    voxel_ui_text(s,x+47,y+r.height-19,"Shutdown...",VOXEL_UI_TEXT);
}

void voxel_ui_menu(struct voxel_surface *s,struct voxel_ui_rect r,
                   const char *const *items,uint32_t count,int selected)
{
    uint32_t i;
    if(!valid(s)||r.width<8||r.height<8) return;
    rect(s,r.x,r.y,r.width,r.height,ui_face());
    voxel_ui_bevel(s,r,0);
    for(i=0;i<count;++i) {
        int32_t iy=r.y+4+(int32_t)i*20;
        if(iy+19>=r.y+r.height) break;
        if((int)i==selected) rect(s,r.x+3,iy,r.width-6,19,VOXEL_UI_SELECTION);
        voxel_ui_text(s,r.x+10,iy+6,items[i],(int)i==selected?VOXEL_UI_WHITE:VOXEL_UI_TEXT);
    }
}

void voxel_ui_dialog(struct voxel_surface *s,struct voxel_ui_rect r,
                     const char *title,const char *message)
{
    voxel_ui_window_frame(s,r,title,1);
    rect(s,r.x+15,r.y+43,2,18,ui_title());
    voxel_ui_text(s,r.x+25,r.y+48,message,VOXEL_UI_TEXT);
    voxel_ui_button(s,(struct voxel_ui_rect){r.x+r.width-78,r.y+r.height-31,66,23},"OK",0);
}

void voxel_ui_checkbox(struct voxel_surface *s, struct voxel_ui_rect r,
                       const char *label, int checked)
{
    size_t n=0;
    if (!valid(s)) return;
    if (label) while (label[n]) ++n;
    rect(s, r.x, r.y + MAX(0,(r.height-13)/2), 13, 13, ui_face());
    voxel_ui_bevel(s, (struct voxel_ui_rect){r.x, r.y + MAX(0,(r.height-13)/2), 13, 13}, 1);
    if (checked) {
        int32_t cy = r.y + MAX(0,(r.height-13)/2);
        rect(s, r.x + 3, cy + 7, 2, 2, VOXEL_UI_TEXT);
        rect(s, r.x + 5, cy + 9, 2, 2, VOXEL_UI_TEXT);
        rect(s, r.x + 7, cy + 7, 2, 2, VOXEL_UI_TEXT);
        rect(s, r.x + 9, cy + 5, 2, 2, VOXEL_UI_TEXT);
        rect(s, r.x + 11, cy + 3, 2, 2, VOXEL_UI_TEXT);
    }
    voxel_ui_text(s, r.x + 18, r.y + MAX(3,(r.height-7)/2), label, VOXEL_UI_TEXT);
}

void voxel_ui_radio(struct voxel_surface *s, struct voxel_ui_rect r,
                    const char *label, int selected)
{
    size_t n=0;
    int32_t cx, cy;
    if (!valid(s)) return;
    if (label) while (label[n]) ++n;
    cx = r.x + 6;
    cy = r.y + (r.height > 13 ? (r.height-13)/2 : 0) + 6;
    rect(s, cx-6, cy-6, 13, 13, ui_face());
    voxel_ui_bevel(s, (struct voxel_ui_rect){cx-6, cy-6, 13, 13}, 1);
    if (selected) {
        rect(s, cx-3, cy-3, 7, 7, ui_title());
    }
    voxel_ui_text(s, r.x + 18, r.y + MAX(3,(r.height-7)/2), label, VOXEL_UI_TEXT);
}

void voxel_ui_progress(struct voxel_surface *s, struct voxel_ui_rect r,
                       uint32_t value, uint32_t maximum)
{
    uint32_t fill;
    if (!valid(s) || r.width < 4 || r.height < 4) return;
    rect(s, r.x, r.y, r.width, r.height, ui_face());
    voxel_ui_bevel(s, r, 1);
    if (maximum == 0) maximum = 1;
    if (value > maximum) value = maximum;
    fill = (uint32_t)((uint64_t)(r.width - 4) * value / maximum);
    if (fill) {
        int32_t px;
        for (px = 0; px < (int32_t)fill; px += 8)
            rect(s, r.x + 2 + px, r.y + 2,
                 ((int32_t)fill - px < 6) ? (int32_t)fill - px : 6,
                 r.height - 4, VOXEL_UI_SELECTION);
    }
}

void voxel_ui_scrollbar(struct voxel_surface *s, struct voxel_ui_rect r,
                        uint32_t value, uint32_t maximum, int vertical)
{
    uint32_t thumb;
    if (!valid(s) || r.width < 8 || r.height < 8) return;
    rect(s, r.x, r.y, r.width, r.height, ui_face());
    voxel_ui_bevel(s, r, 1);
    if (maximum == 0) maximum = 1;
    if (value > maximum) value = maximum;
    if (vertical) {
        thumb = (uint32_t)((uint64_t)(r.height - 16) * value / maximum);
        if (thumb < 8) thumb = 8;
        /* Up / down arrow buttons */
        rect(s, r.x + 2, r.y + 2, r.width - 4, 12, ui_face());
        voxel_ui_bevel(s, (struct voxel_ui_rect){r.x + 2, r.y + 2, r.width - 4, 12}, 0);
        rect(s, r.x + (r.width-4)/2 - 1, r.y + 6, 3, 1, VOXEL_UI_TEXT);
        rect(s, r.x + (r.width-4)/2 - 2, r.y + 7, 5, 1, VOXEL_UI_TEXT);
        rect(s, r.x + 2, r.y + r.height - 14, r.width - 4, 12, ui_face());
        voxel_ui_bevel(s, (struct voxel_ui_rect){r.x + 2, r.y + r.height - 14, r.width - 4, 12}, 0);
        rect(s, r.x + (r.width-4)/2 - 2, r.y + r.height - 8, 5, 1, VOXEL_UI_TEXT);
        rect(s, r.x + (r.width-4)/2 - 1, r.y + r.height - 9, 3, 1, VOXEL_UI_TEXT);
        /* Thumb */
        rect(s, r.x + 2, r.y + 16 + (int32_t)thumb, r.width - 4, 12, VOXEL_UI_MID);
        voxel_ui_bevel(s, (struct voxel_ui_rect){r.x + 2, r.y + 16 + (int32_t)thumb,
                       r.width - 4, 12}, 0);
    } else {
        thumb = (uint32_t)((uint64_t)(r.width - 16) * value / maximum);
        if (thumb < 8) thumb = 8;
        rect(s, r.x + 2 + (int32_t)thumb, r.y + 2, 12, r.height - 4, VOXEL_UI_MID);
        voxel_ui_bevel(s, (struct voxel_ui_rect){r.x + 2 + (int32_t)thumb,
                       r.y + 2, 12, r.height - 4}, 0);
    }
}

void voxel_ui_text_field(struct voxel_surface *s, struct voxel_ui_rect r,
                         const char *text, uint32_t caret)
{
    size_t n=0;
    if (!valid(s) || r.width < 8 || r.height < 12) return;
    rect(s, r.x, r.y, r.width, r.height, VOXEL_UI_WHITE);
    voxel_ui_bevel(s, r, 1);
    if (text) while (text[n]) ++n;
    voxel_ui_text(s, r.x + 4, r.y + MAX(3,(r.height-7)/2), text ? text : "", VOXEL_UI_TEXT);
    if (caret <= n) {
        int32_t cx = r.x + 4 + (int32_t)caret * 6;
        rect(s, cx, r.y + 3, 1, r.height - 6, VOXEL_UI_TEXT);
    }
}
