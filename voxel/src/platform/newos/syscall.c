#include "syscall.h"

long voxel_newos_syscall1(long number, long argument)
{
    long result;
    __asm__ __volatile__("int $0x80"
                         : "=a"(result)
                         : "a"(number), "D"(argument)
                         : "memory");
    return result;
}

long voxel_newos_syscall2(long number, long a, long b)
{
    long result;
    __asm__ __volatile__("int $0x80"
                         : "=a"(result)
                         : "a"(number), "D"(a), "S"(b)
                         : "memory");
    return result;
}

long voxel_newos_syscall3(long number, long a, long b, long c)
{
    long result;
    __asm__ __volatile__("int $0x80"
                         : "=a"(result)
                         : "a"(number), "D"(a), "S"(b), "d"(c)
                         : "memory");
    return result;
}
