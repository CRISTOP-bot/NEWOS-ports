#ifndef VOXEL_NEWOS_SYSCALL_H
#define VOXEL_NEWOS_SYSCALL_H

/* Private NEWOS ABI bridge. Not a general cross-platform syscall interface. */
long voxel_newos_syscall1(long number, long argument);
long voxel_newos_syscall2(long number, long a, long b);
long voxel_newos_syscall3(long number, long a, long b, long c);

#endif
