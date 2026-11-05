#include "voxel/input.h"
#include "syscall.h"

#include <stdint.h>
#include <stddef.h>


int voxel_pointer_poll(struct voxel_pointer *pointer)
{
    struct voxel_mouse_state state;
    if (!pointer || voxel_newos_syscall1(VOXEL_SYS_MOUSE_GET,
                                   (long)(uintptr_t)&state) != 0)
        return -1;
    if (state.x < INT32_MIN || state.x > INT32_MAX ||
        state.y < INT32_MIN || state.y > INT32_MAX)
        return -1;
    pointer->x = (int32_t)state.x;
    pointer->y = (int32_t)state.y;
    pointer->buttons = state.buttons;
    pointer->wheel = state.wheel;
    pointer->sequence = state.seq;
    return 0;
}

int voxel_sleep_ms(uint32_t milliseconds)
{
    struct voxel_timespec delay;
    delay.tv_sec = (int64_t)(milliseconds / 1000u);
    delay.tv_nsec = (int64_t)(milliseconds % 1000u) * 1000000;
    return (int)voxel_newos_syscall1(VOXEL_SYS_SLEEP,
                               (long)(uintptr_t)&delay);
}

int voxel_console_gui_claim(void)
{
    return (int)voxel_newos_syscall1(VOXEL_SYS_CONSOLE_GUI, 1);
}

int voxel_console_gui_release(void)
{
    return (int)voxel_newos_syscall1(VOXEL_SYS_CONSOLE_GUI, 0);
}

int voxel_keyboard_grab(void)
{
    return (int)voxel_newos_syscall1(VOXEL_SYS_KBD_GRAB, 1);
}

int voxel_keyboard_release(void)
{
    return (int)voxel_newos_syscall1(VOXEL_SYS_KBD_GRAB, 0);
}

int voxel_keyboard_read(char *buffer, uint32_t capacity)
{
    if (!buffer || capacity == 0)
        return -1;
    if (capacity > 64u)
        capacity = 64u;
    return (int)voxel_newos_syscall2(VOXEL_SYS_KBD_READ,
                                     (long)(uintptr_t)buffer,
                                     (long)capacity);
}
