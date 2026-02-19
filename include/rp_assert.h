#ifndef ROSE_PETAL_ASSERT_H
#define ROSE_PETAL_ASSERT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/*
 * A custom assertion macro that, if `cond` is false,
 * calls the stdlib `abort()` function to end program.
 */
#define RP_ASSERT(cond)                                                        \
        do {                                                                   \
                if (!!(cond))                                                  \
                        break;                                                 \
                                                                               \
                fprintf(stderr,                                                \
                        "\nASSERTION (%s) FAILED @ %s:%d in `%s()`\n\n",       \
                        #cond,                                                 \
                        __FILE__,                                              \
                        __LINE__,                                              \
                        __func__);                                             \
                abort();                                                       \
        } while (0)

/*
 * A custom assertion macro that prints out a
 * user-specified message upon `cond` being false.
 *
 * NOTE: This can only handle a single string with no
 * format specifiers. For that, use `RP_ASSERTF()`.
 */
#define RP_ASSERTM(cond, msg)                                                  \
        do {                                                                   \
                if (!!(cond))                                                  \
                        break;                                                 \
                                                                               \
                fprintf(stderr,                                                \
                        "\nASSERTION (%s) FAILED @ %s:%d in `%s()`\n\t%s\n\n", \
                        #cond,                                                 \
                        __FILE__,                                              \
                        __LINE__,                                              \
                        __func__,                                              \
                        msg);                                                  \
                abort();                                                       \
        } while (0)

/*
 * A custom assertion macro that prints out a user-specified
 * formatted string upon `cond` being false.
 *
 * NOTE: This will not work with just a single message string
 * with no parameters. For that, use `RP_ASSERTM()`.
 */
#define RP_ASSERTF(cond, fmt, ...)                                             \
        do {                                                                   \
                if (!!(cond))                                                  \
                        break;                                                 \
                                                                               \
                rp_internal_assertf(#cond,                                     \
                                    __FILE__,                                  \
                                    __LINE__,                                  \
                                    __func__,                                  \
                                    fmt,                                       \
                                    __VA_ARGS__);                              \
                abort();                                                       \
        } while (0)

static __inline void rp_internal_assertf(const char *cond_str,
                                         const char *file,
                                         const int   line,
                                         const char *func,
                                         const char *fmt,
                                         ...)
{
        va_list args;

        va_start(args, fmt);
        (void)fprintf(stderr,
                      "\nASSERTION (%s) FAILED @ %s:%d in `%s()`\n\t",
                      cond_str,
                      file,
                      line,
                      func);
        (void)vfprintf(stderr, fmt, args);
        (void)fprintf(stderr, "\n\n");
        va_end(args);
}

#endif /* #ifndef ROSE_PETAL_ASSERT_H */
