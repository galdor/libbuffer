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

#include <utest.h>

#include "buffer.h"

#define BFT_BUFFER_EMPTY(buf_) \
    TEST_UINT_EQ(bf_buffer_length(buf_), 0)

#define BFT_BUFFER_EQ(buf_, data_, sz_)                           \
    do {                                                          \
        TEST_UINT_EQ(bf_buffer_length(buf_), sz_);                \
        TEST_MEM_EQ(bf_buffer_data(buf_), bf_buffer_length(buf_), \
                    data_, sz_);                                  \
    } while (0)

TEST(initialization) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);
    BFT_BUFFER_EMPTY(buf);
    bf_buffer_delete(buf);

    buf = bf_buffer_new(32);
    BFT_BUFFER_EMPTY(buf);
    bf_buffer_delete(buf);
}

TEST(insert) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    bf_buffer_insert(buf, 0, "abc", 3);
    BFT_BUFFER_EQ(buf, "abc", 3);

    bf_buffer_insert(buf, 2, "123", 3);
    BFT_BUFFER_EQ(buf, "ab123c", 6);

    bf_buffer_insert(buf, 6, "de", 2);
    BFT_BUFFER_EQ(buf, "ab123cde", 8);

    bf_buffer_delete(buf);
}

TEST(add) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    bf_buffer_add_string(buf, "abc");
    BFT_BUFFER_EQ(buf, "abc", 3);

    bf_buffer_add_string(buf, "defgh");
    BFT_BUFFER_EQ(buf, "abcdefgh", 8);

    bf_buffer_clear(buf);
    BFT_BUFFER_EMPTY(buf);

    bf_buffer_add_printf(buf, "hello: %d", 42);
    BFT_BUFFER_EQ(buf, "hello: 42", 9);

    bf_buffer_delete(buf);
}

TEST(skip) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    bf_buffer_skip(buf, 3);
    BFT_BUFFER_EMPTY(buf);

    bf_buffer_add_string(buf, "abcde");
    bf_buffer_skip(buf, 2);
    BFT_BUFFER_EQ(buf, "cde", 3);

    bf_buffer_skip(buf, 3);
    BFT_BUFFER_EMPTY(buf);

    bf_buffer_add_string(buf, "fgh");
    bf_buffer_skip(buf, 6);
    BFT_BUFFER_EMPTY(buf);

    bf_buffer_delete(buf);
}

TEST(remove) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(0);

    TEST_UINT_EQ(bf_buffer_remove(buf, 2), 0);
    BFT_BUFFER_EMPTY(buf);

    bf_buffer_add_string(buf, "abcde");
    TEST_UINT_EQ(bf_buffer_remove(buf, 2), 2);
    BFT_BUFFER_EQ(buf, "abc", 3);
    TEST_UINT_EQ(bf_buffer_remove(buf, 5), 3);
    BFT_BUFFER_EMPTY(buf);

    bf_buffer_add_string(buf, "abcde");
    TEST_UINT_EQ(bf_buffer_remove_before(buf, 3, 2), 2);
    BFT_BUFFER_EQ(buf, "ade", 3);
    TEST_UINT_EQ(bf_buffer_remove_before(buf, 1, 3), 1);
    BFT_BUFFER_EQ(buf, "de", 2);

    bf_buffer_clear(buf);
    bf_buffer_add_string(buf, "abcde");
    TEST_UINT_EQ(bf_buffer_remove_after(buf, 0, 2), 2);
    BFT_BUFFER_EQ(buf, "cde", 3);

    bf_buffer_clear(buf);
    bf_buffer_add_string(buf, "abcde");
    TEST_UINT_EQ(bf_buffer_remove_after(buf, 1, 2), 2);
    BFT_BUFFER_EQ(buf, "ade", 3);

    bf_buffer_clear(buf);
    bf_buffer_add_string(buf, "abcde");
    TEST_UINT_EQ(bf_buffer_remove_after(buf, 4, 2), 1);
    BFT_BUFFER_EQ(buf, "abcd", 4);

    bf_buffer_delete(buf);
}

TEST(dup) {
    struct bf_buffer *buf;
    char *tmp;

    buf = bf_buffer_new(0);

    TEST_PTR_NULL(bf_buffer_dup(buf));

    bf_buffer_add_string(buf, "abcde");
    tmp = bf_buffer_dup(buf);
    TEST_MEM_EQ(tmp, 5, "abcde", 5);
    free(tmp);

    tmp = bf_buffer_dup_string(buf);
    TEST_MEM_EQ(tmp, 6, "abcde\0", 6);
    free(tmp);

    bf_buffer_clear(buf);
    tmp = bf_buffer_dup_string(buf);
    TEST_MEM_EQ(tmp, 1, "\0", 1);
    free(tmp);

    bf_buffer_delete(buf);
}

TEST(free_space_after_skip) {
    struct bf_buffer *buf;

    buf = bf_buffer_new(8);

    bf_buffer_add_string(buf, "hello");
    bf_buffer_skip(buf, 5);
    TEST_UINT_EQ(bf_buffer_free_space(buf), 8);

    bf_buffer_add_string(buf, "hello");
    bf_buffer_skip(buf, 3);
    bf_buffer_skip(buf, 2);
    TEST_UINT_EQ(bf_buffer_free_space(buf), 8);

    bf_buffer_add_string(buf, "hello");
    bf_buffer_skip(buf, 2);
    bf_buffer_remove_before(buf, 3, 3);
    TEST_UINT_EQ(bf_buffer_free_space(buf), 8);

    bf_buffer_add_string(buf, "hello");
    bf_buffer_skip(buf, 2);
    bf_buffer_remove_after(buf, 0, 3);
    TEST_UINT_EQ(bf_buffer_free_space(buf), 8);
}

int
main(int argc, char **argv) {
    struct test_suite *suite;

    suite = test_suite_new("buffer");
    test_suite_initialize_from_args(suite, argc, argv);

    test_suite_start(suite);

    TEST_RUN(suite, initialization);
    TEST_RUN(suite, insert);
    TEST_RUN(suite, add);
    TEST_RUN(suite, remove);
    TEST_RUN(suite, dup);
    TEST_RUN(suite, free_space_after_skip);

    test_suite_print_results_and_exit(suite);
}
