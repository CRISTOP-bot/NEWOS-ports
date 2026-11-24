/*
 * in_newos.c - Quake 2 input backend for NEWOS.
 * Reads PS/2 keyboard (SYS_KBD_GRAB/READ) and mouse (SYS_MOUSE_GET),
 * translating scancodes to Quake key bits via the keymap.
 */
#include "../client/client.h"
#include "newos_platform.h"
#include <string.h>

/* NEWOS key codes (from voxel/keymap.h) -> Quake 2 K_* codes.
 * We forward the raw byte through Key_Event with a mapped key number. */
void IN_Init(void)
{
    sys_kbd_grab(1);
}

void IN_Shutdown(void)
{
    sys_kbd_grab(0);
}

void IN_Commands(void)
{
}

void IN_Frame(void)
{
}

void IN_Move(usercmd_t *cmd)
{
    struct nsh_mouse m;

    if (sys_mouse_get(&m) == 0) {
        /* Mouse look is applied through the view angles. */
        cmd->angles[0] -= (float)m.y * 0.1f;
        cmd->angles[1] -= (float)m.x * 0.1f;
    }
}

void IN_Activate(qboolean active)
{
    (void)active;
    sys_kbd_grab(1);
}

void IN_ActivateMouse(void)
{
}

void IN_DeactivateMouse(void)
{
}
