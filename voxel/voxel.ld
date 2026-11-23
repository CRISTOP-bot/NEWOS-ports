/* VOXEL static ELF64 layout for NEWOS.
 *
 * Same load address and section order as NEWOS's user/lib/app.ld, but with
 * explicit PHDRS so the loader sees two PT_LOAD segments (R+X and R+W)
 * instead of one RWX segment. The stock script's single RWX load segment is
 * functional on NEWOS but makes `ld` emit a noisy warning on every build.
 *
 * Keep this in sync with user/lib/app.ld when the userland ABI changes.
 */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);   /* PF_R | PF_X */
    data PT_LOAD FLAGS(6);   /* PF_R | PF_W */
}

SECTIONS
{
    . = 0x400000;

    .text : { *(.text*) } :text
    .rodata : { *(.rodata*) } :text

    /* Separate pages so the code and data segments never overlap. */
    . = ALIGN(0x1000);
    .data : { *(.data*) } :data
    .bss : { *(.bss*) } :data

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
