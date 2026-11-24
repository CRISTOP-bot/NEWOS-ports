/*
 * sys_newos.c - Quake 2 system backend for NEWOS.
 * Replaces null/sys_null.c with real timing (SYS_GETTIME), hunk memory
 * (malloc), directory creation and an error path. Filesystem PAK loading
 * uses the stdio shim from the NEWOS libc.
 */
#include "../qcommon/qcommon.h"
#include "newos_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

int   curtime;
unsigned sys_frame_time;

void Sys_Mkdir(char *path)
{
    sys_mkdir(path);
}

void Sys_Error(char *error, ...)
{
    va_list argptr;
    char msg[1024];

    va_start(argptr, error);
    vsnprintf(msg, sizeof(msg), error, argptr);
    va_end(argptr);
    puts("Sys_Error: ");
    puts(msg);
    puts("\n");
    sys_exit(1);
}

void Sys_Quit(void)
{
    sys_exit(0);
}

void Sys_UnloadGame(void)
{
}

void *Sys_GetGameAPI(void *parms)
{
    (void)parms;
    return NULL;            /* no dynamically loaded game DLL on NEWOS */
}

char *Sys_ConsoleInput(void)
{
    return NULL;
}

void Sys_ConsoleOutput(char *string)
{
    (void)string;
}

extern void Key_Event(int key, int down, unsigned time);

void Sys_SendKeyEvents(void)
{
    unsigned char buf[64];
    long n, i;

    for (;;) {
        n = sys_kbd_read(buf, sizeof(buf));
        if (n <= 0)
            break;
        for (i = 0; i < n; i++) {
            int k = (buf[i] >= 'a' && buf[i] <= 'z') ? (buf[i] - 'a' + 'A')
                                                     : (int)buf[i];
            Key_Event(k, 1, (unsigned)Sys_Milliseconds());
            Key_Event(k, 0, (unsigned)Sys_Milliseconds());
        }
    }
}

void Sys_AppActivate(void)
{
}

void Sys_CopyProtect(void)
{
}

char *Sys_GetClipboardData(void)
{
    return NULL;
}

/* Hunk memory: a simple arena on top of malloc. */
static byte *hunk_base;
static int   hunk_size;
static int   hunk_used;

void *Hunk_Begin(int maxsize)
{
    hunk_base = (byte *)malloc((size_t)maxsize);
    hunk_size = hunk_base ? maxsize : 0;
    hunk_used = 0;
    return hunk_base;
}

void *Hunk_Alloc(int size)
{
    void *p;
    if (!hunk_base || hunk_used + size > hunk_size)
        return NULL;
    p = hunk_base + hunk_used;
    hunk_used += (size + 3) & ~3;
    return p;
}

void Hunk_Free(void *buf)
{
    if (buf == hunk_base) {
        free(hunk_base);
        hunk_base = NULL;
    }
}

int Hunk_End(void)
{
    return hunk_used;
}

/* Milliseconds via the CMOS clock (CLOCK_MONOTONIC in the NEWOS libc). */
int Sys_Milliseconds(void)
{
    /* Use the monotonic clock exposed by the NEWOS libc (CLOCK_MONOTONIC),
     * same source DOOM's I_GetTime uses. */
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
        return (int)((long long)ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL);
    return 0;
}

/* Filesystem search stubs: no directory enumeration for PAKs needed when
 * the game data is embedded (FS_LoadFile reads from the VFS). */
char *Sys_FindFirst(char *path, unsigned musthave, unsigned canthave)
{
    (void)path; (void)musthave; (void)canthave;
    return NULL;
}

char *Sys_FindNext(unsigned musthave, unsigned canthave)
{
    (void)musthave; (void)canthave;
    return NULL;
}

void Sys_FindClose(void)
{
}

void Sys_Init(void)
{
}

void Sys_Printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

/* Real entry point: startup with the standard Quame argument convention. */
int main(int argc, char **argv)
{
    /* Claim the framebuffer and show a banner immediately, so running
     * `quake2` visibly opens even before the engine (or game data) loads. */
    {
        long rc = sys_console_gui(1);
        if (rc != 0) {
            char m[] = "Q2: console_gui FAILED\n";
            sys_write(1, m, sizeof(m)-1);
        }
    }
    q2_show_splash("QUAKE 2 FOR NEWOS", "LOADING DEMO...");
    Qcommon_Init(argc, argv);
    for (;;)
        Qcommon_Frame(0.1f);
    return 0;
}
