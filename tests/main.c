/*
 * Copyright (c) 2014 Nicolas Martyanoff
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "buffer.h"

static void die(const char *, ...)
    __attribute__((format(printf, 1, 2)));
static void usage(const char *, int);

#define TEST_DEFINE(name_) \
    int test_case_##name_()

#define TEST_SUCCEED() \
    return 1

#define TEST_FAIL(fmt_, ...)                  \
    do {                                      \
        fprintf(stderr, fmt_"\n", ##__VA_ARGS__); \
        return 0;                             \
    } while (0)

#define TEST_ERROR(fmt_, ...)                 \
    do {                                      \
        fprintf(stderr, fmt_"\n", ##__VA_ARGS__); \
        return -1;                            \
    } while (0)

#define TEST_ASSERT(cond_, fmt_, ...)                                     \
    if (!(cond_)) {                                                       \
        TEST_FAIL("assertion '" #cond_ "' failed: " fmt_, ##__VA_ARGS__); \
    }


TEST_DEFINE(append) {
    TEST_ASSERT(0 == 0, "TODO");

    TEST_SUCCEED();
}


#define TEST_CASE(name_) {.name = #name_, .test_func = test_case_##name_}

static struct {
    const char *name;
    int (*test_func)();
} test_cases[] = {
    TEST_CASE(append),
};

#undef TEST_CASE

int
main(int argc, char **argv) {
    size_t nb_tests, nb_passed;
    const char *color_esc;
    int opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, "hn:")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0], 0);
                break;

            case '?':
                usage(argv[0], 1);
        }
    }

    nb_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    nb_passed = 0;

    for (size_t i = 0; i < nb_tests; i++) {
        int ret;

        printf("\e[34m---- %-30s ----\e[0m\n", test_cases[i].name);

        ret = test_cases[i].test_func();

        if (ret == -1) {
            printf("\e[31;1merror\e[0m\n");
        } else if (ret == 0) {
            printf("\e[31mfailure\e[0m\n");
        } else {
            printf("\e[32mok\e[0m\n");
            nb_passed++;
        }
    }

    printf("\e[34m----------------------------------------\e[0m\n");

    if (nb_passed == 0) {
        color_esc = "\e[31m";
    } else if (nb_passed == nb_tests) {
        color_esc = "\e[32m";
    } else {
        color_esc = "\e[33m";
    }

    printf("%s%zu/%zu tests passed (%.0f%%)\e[0m\n",
           color_esc, nb_passed, nb_tests,
           (double)nb_passed * 100.0 / nb_tests);

    return (nb_passed == nb_tests) ? 0 : 1;
}

static void
usage(const char *argv0, int exit_code) {
    printf("Usage: %s [-hn]\n"
            "\n"
            "Options:\n"
            "  -h         display help\n",
            argv0);
    exit(exit_code);
}

static void
die(const char *fmt, ...) {
    va_list ap;

    fprintf(stderr, "fatal error: ");

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    putc('\n', stderr);
    exit(1);
}
