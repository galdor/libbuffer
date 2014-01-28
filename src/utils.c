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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "buffer.h"

#define BF_ERROR_BUFSZ 1024U

static __thread char bf_error_buf[BF_ERROR_BUFSZ];

#define BF_DEFAULT_ALLOCATOR \
    {                        \
        .malloc = malloc,    \
        .free = free,        \
        .calloc = calloc,    \
        .realloc = realloc   \
    }


static const struct bf_memory_allocator bf_default_allocator =
    BF_DEFAULT_ALLOCATOR;

static struct bf_memory_allocator bf_allocator = BF_DEFAULT_ALLOCATOR;

struct bf_memory_allocator *bf_default_memory_allocator;

const char *
bf_get_error(void) {
    return bf_error_buf;
}

void
bf_set_error(const char *fmt, ...) {
    char buf[BF_ERROR_BUFSZ];
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsnprintf(buf, BF_ERROR_BUFSZ, fmt, ap);
    va_end(ap);

    if ((size_t)ret >= BF_ERROR_BUFSZ) {
        memcpy(bf_error_buf, buf, BF_ERROR_BUFSZ);
        bf_error_buf[BF_ERROR_BUFSZ - 1] = '\0';
        return;
    }

    strncpy(bf_error_buf, buf, (size_t)ret + 1);
    bf_error_buf[ret] = '\0';
}

void
bf_set_memory_allocator(const struct bf_memory_allocator *allocator) {
    if (allocator) {
        bf_allocator = *allocator;
    } else {
        bf_allocator = bf_default_allocator;
    }
}

void *
bf_malloc(size_t sz) {
    return bf_allocator.malloc(sz);
}

void
bf_free(void *ptr) {
    bf_allocator.free(ptr);
}

void *
bf_calloc(size_t nb, size_t sz) {
    return bf_allocator.calloc(nb, sz);
}

void *
bf_realloc(void *ptr, size_t sz) {
    return bf_allocator.realloc(ptr, sz);
}
