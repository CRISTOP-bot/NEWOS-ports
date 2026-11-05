/*
 * NEWOS tcc demo: a freestanding C program with no headers at all.
 *
 * The image installs this file as /etc/tcc-hello.c. TinyCC compiles and runs
 * it in place (no fork/exec: tcc -run relocates the code into this process
 * and calls main):
 *
 *     tcc -nostdlib -run /etc/tcc-hello.c
 *
 * The int $0x80 calls below are the native NEWOS ABI (rax = number,
 * rdi/rsi/rdx = arguments), not Linux's syscall instruction.
 */
static long newos_syscall3(long nr, long a, long b, long c)
{
    long ret;

    __asm__ volatile("int $0x80"
                     : "=a"(ret)
                     : "a"(nr), "D"(a), "S"(b), "d"(c)
                     : "rcx", "r11", "memory");
    return ret;
}

static unsigned long text_len(const char *s)
{
    unsigned long n = 0;
    while (s[n])
        n++;
    return n;
}

int main(void)
{
    const char msg[] = "Hello from tcc on NEWOS!\n";

    newos_syscall3(1, 1, (long)msg, (long)text_len(msg));  /* SYS_WRITE */
    return 0;
}
