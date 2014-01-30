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


TEST_DEFINE(initialization) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "a buffer with an initial size of 0 is empty");
    TEST_ASSERT(bf_buffer_data(buf) == NULL,
                "a buffer with an initial size of 0 does not contain any data");
    bf_buffer_delete(buf);

    buf = bf_buffer_new(32);
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "a buffer with an initial size of 32 is empty");
    TEST_ASSERT(bf_buffer_data(buf) != NULL,
                "a buffer with an initial size of 0 contain data");
    bf_buffer_delete(buf);

    TEST_SUCCEED();
}

TEST_DEFINE(insert) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    bf_buffer_insert(buf, 0, "abc", 3);
    TEST_ASSERT(bf_buffer_length(buf) == 3,
                "the length is right after inserting data for the first time");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "abc", 3) == 0,
                "data are correct after inserting data for the first time");

    bf_buffer_insert(buf, 2, "123", 3);
    TEST_ASSERT(bf_buffer_length(buf) == 6,
                "the length is right after inserting data");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "ab123c", 6) == 0,
                "data are correct after inserting data for the first time");

    bf_buffer_insert(buf, 6, "de", 2);
    TEST_ASSERT(bf_buffer_length(buf) == 8,
                "the length is right after inserting data at the end");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "ab123cde", 8) == 0,
                "data are correct after inserting data at the end");

    bf_buffer_delete(buf);

    TEST_SUCCEED();
}

TEST_DEFINE(add) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    bf_buffer_add_string(buf, "abc");
    TEST_ASSERT(bf_buffer_length(buf) == 3,
                "the length is right after adding data for the first time");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "abc", 3) == 0,
                "data are correct after adding data for the first time");

    bf_buffer_add_string(buf, "defgh");
    TEST_ASSERT(bf_buffer_length(buf) == 8,
                "the length is right after adding data for the second time");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "abcdefgh", 8) == 0,
                "data are correct after adding data for the second time");

    bf_buffer_clear(buf);
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "the length is right after clearing the buffer");

    bf_buffer_add_printf(buf, "hello: %d", 42);
    TEST_ASSERT(bf_buffer_length(buf) == 9,
                "the length is right after adding formatted data");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "hello: 42", 2) == 0,
                "data are correct after adding formatted data");

    bf_buffer_delete(buf);

    TEST_SUCCEED();
}

TEST_DEFINE(skip) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    bf_buffer_skip(buf, 3);
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "the length is correct after skipping on an empty buffer");

    bf_buffer_add_string(buf, "abcde");
    bf_buffer_skip(buf, 2);
    TEST_ASSERT(bf_buffer_length(buf) == 3,
                "the length is correct after skipping");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "cde", 3) == 0,
                "the content is correct after skipping");

    bf_buffer_skip(buf, 3);
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "the length is correct after skipping the whole content");

    bf_buffer_add_string(buf, "fgh");
    bf_buffer_skip(buf, 6);
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "the length is correct after skipping more bytes than there are");

    bf_buffer_delete(buf);

    TEST_SUCCEED();
}

TEST_DEFINE(remove) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    TEST_ASSERT(bf_buffer_remove(buf, 2) == 0,
                "bf_buffer_remove() returns the right value for an empty buffer");
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "the length is correct after removing from an empty buffer");

    bf_buffer_add_string(buf, "abcde");

    TEST_ASSERT(bf_buffer_remove(buf, 2) == 2,
                "bf_buffer_remove() returns the right value");
    TEST_ASSERT(bf_buffer_length(buf) == 3,
                "the length is correct after removing");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "abc", 3) == 0,
                "the content is correct after removing");

    TEST_ASSERT(bf_buffer_remove(buf, 5) == 3,
                "bf_buffer_remove() returns the right value when n is larger "
                "than the length of the buffer");
    TEST_ASSERT(bf_buffer_length(buf) == 0,
                "the length is correct after removing with n being larger "
                "than the length of the buffer");

    bf_buffer_add_string(buf, "abcde");

    TEST_ASSERT(bf_buffer_remove_before(buf, 3, 2) == 2,
                "bf_buffer_remove_before() returns the right value");
    TEST_ASSERT(bf_buffer_length(buf) == 3,
                "the length is correct after removing before an offset");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "ade", 3) == 0,
                "the content is correct after removing before an offset");

    TEST_ASSERT(bf_buffer_remove_before(buf, 1, 3) == 1,
                "bf_buffer_remove_before() returns the right value when "
                "offset - n < 0");
    TEST_ASSERT(bf_buffer_length(buf) == 2,
                "the length is correct after removing before an offset with "
                "offset - n < 0");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "de", 2) == 0,
                "the content is correct after removing before an offset with "
                "offset - n < 0");

    bf_buffer_clear(buf);
    bf_buffer_add_string(buf, "abcde");
    TEST_ASSERT(bf_buffer_remove_after(buf, 0, 2) == 2,
                "bf_buffer_remove_after() returns the right value for "
                "a zero offset");
    TEST_ASSERT(bf_buffer_length(buf) == 3,
                "the length is correct after removing after a zero offset");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "cde", 3) == 0,
                "the content is correct after removing after a zero offset");

    bf_buffer_clear(buf);
    bf_buffer_add_string(buf, "abcde");
    TEST_ASSERT(bf_buffer_remove_after(buf, 1, 2) == 2,
                "bf_buffer_remove_after() returns the right value");
    TEST_ASSERT(bf_buffer_length(buf) == 3,
                "the length is correct after removing after an offset");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "ade", 3) == 0,
                "the content is correct after removing after an offset");

    bf_buffer_clear(buf);
    bf_buffer_add_string(buf, "abcde");
    TEST_ASSERT(bf_buffer_remove_after(buf, 4, 2) == 1,
                "bf_buffer_remove_after() returns the right value when "
                "offset + n >= len");
    TEST_ASSERT(bf_buffer_length(buf) == 4,
                "the length is correct after removing after an offset with "
                "offset + n >= len");
    TEST_ASSERT(memcmp(bf_buffer_data(buf), "abcd", 4) == 0,
                "the content is correct after removing after an offset with "
                "offset + n >= len");

    bf_buffer_delete(buf);

    TEST_SUCCEED();
}

TEST_DEFINE(dup) {
    struct bf_buffer *buf;
    char *tmp;

    buf = bf_buffer_new(0);

    TEST_ASSERT(bf_buffer_dup(buf) == NULL,
                "an empty buffer cannot be duplicated");

    bf_buffer_add_string(buf, "abcde");
    tmp = bf_buffer_dup(buf);
    TEST_ASSERT(memcmp(tmp, "abcde", 5) == 0,
                "duplicating a buffer yields the same data");
    free(tmp);

    tmp = bf_buffer_dup_string(buf);
    TEST_ASSERT(memcmp(tmp, "abcde\0", 6) == 0,
                "duplicating a buffer as a string yields the same data");
    free(tmp);

    bf_buffer_clear(buf);
    tmp = bf_buffer_dup_string(buf);
    TEST_ASSERT(memcmp(tmp, "\0", 1) == 0,
                "duplicating an empty buffer as a string yields an empty "
                "string");
    free(tmp);

    bf_buffer_delete(buf);

    TEST_SUCCEED();
}


#define TEST_CASE(name_) {.name = #name_, .test_func = test_case_##name_}

static struct {
    const char *name;
    int (*test_func)();
} test_cases[] = {
    TEST_CASE(initialization),
    TEST_CASE(insert),
    TEST_CASE(add),
    TEST_CASE(skip),
    TEST_CASE(remove),
    TEST_CASE(dup),
};

#undef TEST_CASE

int
main(int argc, char **argv) {
    size_t nb_tests, nb_passed;
    const char *color_esc;
    int opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, "h")) != -1) {
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
    printf("Usage: %s [-h]\n"
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
