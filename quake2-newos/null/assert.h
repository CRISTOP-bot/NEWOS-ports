/* Stub assert.h for Quake 2: no-op asserts (NEWOS libc's version uses a
 * statement-style __assert_fail in an expression, which gnu89 rejects). */
#ifndef Q2_ASSERT_H
#define Q2_ASSERT_H
#define assert(x) ((void)0)
#endif
