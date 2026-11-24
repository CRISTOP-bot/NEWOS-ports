/*
 * NEWOS tcc test program — full-libc edition.
 *
 * Unlike /etc/tcc-hello.c (freestanding, int $0x80 only), this one uses the
 * NEWOS SDK's stdio facade. It is the canonical smoke test for the complete
 * on-board tcc toolchain: header search, libc linking and crt0 startup.
 *
 * Compile and run inside NEWOS (after installing the SDK package):
 *
 *   newpkg install /tmp/newos-sdk.new
 *   tcc -static -nostdlib -o /tmp/tccmain \
 *       /usr/lib/crt0.o /etc/tcc-main.c \
 *       /usr/lib/tcc/libtcc1.a /usr/lib/libnsh.a /usr/lib/libc.a
 *   /tmp/tccmain
 *
 * Expected output:
 *   puts:  TCC libc OK (puts)
 *   printf: TCC libc OK: 6 + 7 = 13, 0x2a = 42
 *   argv[0] = /tmp/tccmain
 */
#include <stdio.h>

int main(int argc, char **argv)
{
    puts("TCC libc OK (puts)");
    printf("TCC libc OK: %d + %d = %d, 0x%02x = %d\n",
           6, 7, 6 + 7, 0x2a, 42);
    if (argc > 0)
        printf("argv[0] = %s\n", argv[0]);
    return 0;
}
