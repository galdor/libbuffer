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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "internal.h"
#include "buffer.h"

static void bf_buffer_repack(struct bf_buffer *);
static int bf_buffer_resize(struct bf_buffer *, size_t);
static int bf_buffer_grow(struct bf_buffer *, size_t);
static int bf_buffer_ensure_free_space(struct bf_buffer *, size_t);

/*
 *                       sz
 *   <----------------------------------------->
 *
 *    skip             len
 *   <----> <------------------------>
 *
 *  +------+--------------------------+--------+
 *  |      |         content          |        |
 *  +------+--------------------------+--------+
 */

struct bf_buffer {
    char *data;
    size_t sz;
    size_t skip;
    size_t len;
};

struct bf_buffer *
bf_buffer_new(size_t initial_size) {
    struct bf_buffer *buf;

    buf = bf_malloc(sizeof(struct bf_buffer));
    if (!buf)
        return NULL;

    memset(buf, 0, sizeof(struct bf_buffer));

    if (initial_size > 0) {
        if (bf_buffer_resize(buf, initial_size) == -1) {
            bf_free(buf);
            return NULL;
        }
    }

    return buf;
}

void
bf_buffer_delete(struct bf_buffer *buf) {
    if (!buf)
        return;

    bf_free(buf->data);
    buf->data = NULL;

    bf_free(buf);
}

void *
bf_buffer_data(const struct bf_buffer *buf) {
    return buf->data + buf->skip;
}

size_t
bf_buffer_length(const struct bf_buffer *buf) {
    return buf->len;
}

size_t
bf_buffer_size(const struct bf_buffer *buf) {
    return buf->sz;
}

size_t
bf_buffer_free_space(const struct bf_buffer *buf) {
    return buf->sz - buf->len - buf->skip;
}

void
bf_buffer_reset(struct bf_buffer *buf) {
    bf_free(buf->data);
    buf->data = NULL;

    buf->sz = 0;
    buf->skip = 0;
    buf->len = 0;
}

void
bf_buffer_clear(struct bf_buffer *buf) {
    buf->skip = 0;
    buf->len = 0;
}

void
bf_buffer_truncate(struct bf_buffer *buf, size_t sz) {
    if (sz > buf->len)
        sz = buf->len;

    buf->len = sz;

    if (buf->len == 0)
        buf->skip = 0;
}

void *
bf_buffer_reserve(struct bf_buffer *buf, size_t sz) {
    if (bf_buffer_ensure_free_space(buf, sz) == -1)
        return NULL;

    return buf->data + buf->skip + buf->len;
}

int
bf_buffer_increase_length(struct bf_buffer *buf, size_t n) {
    if (n > buf->sz - buf->len - buf->skip) {
        bf_set_error("length increment too large");
        return -1;
    }

    buf->len += n;
    return 0;
}

int
bf_buffer_insert(struct bf_buffer *buf, size_t offset, const void *data,
                 size_t sz) {
    char *ndata;
    size_t nsz;

    if (sz == 0)
        return 0;

    if (offset > buf->len) {
        bf_set_error("invalid offset");
        return -1;
    }

    if (!buf->data) {
        nsz = sz;
        if (nsz < 32)
            nsz = 32;

        buf->sz = sz;
        buf->data = bf_malloc(sz);
        if (!buf->data)
            return -1;
    } else if (bf_buffer_free_space(buf) < sz) {
        bf_buffer_repack(buf);

        if (bf_buffer_free_space(buf) < sz) {
            if (sz > buf->sz) {
                nsz = buf->sz + sz;
            } else {
                nsz = buf->sz * 2;
            }

            if (bf_buffer_resize(buf, nsz) == -1)
                return -1;
        }
    }

    ndata = buf->data + buf->skip + offset;

    if (offset < buf->len)
        memmove(ndata + sz, ndata, buf->len - offset);
    memcpy(ndata, data, sz);

    buf->len += sz;
    return 0;
}

int
bf_buffer_add(struct bf_buffer *buf, const void *data, size_t sz) {
    return bf_buffer_insert(buf, buf->len, data, sz);
}

int
bf_buffer_add_buffer(struct bf_buffer *buf, const struct bf_buffer *src) {
    return bf_buffer_add(buf, src->data + src->skip, src->len);
}

int
bf_buffer_add_string(struct bf_buffer *buf, const char *str) {
    return bf_buffer_insert(buf, buf->len, str, strlen(str));
}

int
bf_buffer_add_vprintf(struct bf_buffer *buf, const char *fmt, va_list ap) {
    size_t fmt_len, free_space;
    char *ptr;

    fmt_len = strlen(fmt);
    if (fmt_len == 0) {
        /* If there is no free space in the buffer after its content and if
         * the format string is empty, the pointer to this free space will be
         * invalid. We may as well return right now. */
        bf_set_error("empty format string");
        return -1;
    }

    /* We need to make space for \0 because vsnprintf() needs it, even
     * though we will ignore it. */
    bf_buffer_ensure_free_space(buf, fmt_len + 1);

    for (;;) {
        int ret;
        va_list local_ap;

        ptr = buf->data + buf->skip + buf->len;
        free_space = bf_buffer_free_space(buf);

        va_copy(local_ap, ap);
        ret = vsnprintf(ptr, free_space, fmt, local_ap);
        va_end(local_ap);

        if (ret == -1) {
            bf_set_error("cannot format string: %s", strerror(errno));
            return -1;
        }

        if ((size_t)ret < free_space) {
            buf->len += (size_t)ret;
            return 0;
        }

        bf_buffer_ensure_free_space(buf, (size_t)ret + 1);
    }
}

int
bf_buffer_add_printf(struct bf_buffer *buf, const char *fmt, ...) {
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = bf_buffer_add_vprintf(buf, fmt, ap);
    va_end(ap);

    return ret;
}

void
bf_buffer_skip(struct bf_buffer *buf, size_t n) {
    if (n > buf->len)
        n = buf->len;

    buf->len -= n;
    buf->skip += n;

    if (buf->len == 0)
        buf->skip = 0;
}

size_t
bf_buffer_remove_before(struct bf_buffer *buf, size_t offset, size_t n) {
    if (offset > buf->len)
        offset = buf->len;

    if (n > offset)
        n = offset;

    if (n == 0)
        return 0;

    if (offset < buf->len) {
        char *ptr;

        ptr = buf->data + buf->skip + offset;
        memmove(ptr - n, ptr, buf->len - offset);
    }

    buf->len -= n;

    if (buf->len == 0)
        buf->skip = 0;

    return n;
}

size_t
bf_buffer_remove_after(struct bf_buffer *buf, size_t offset, size_t n) {
    char *ptr;

    if (offset > buf->len)
        offset = buf->len;

    if (n > buf->len - offset)
        n = buf->len - offset;

    if (n == 0)
        return 0;

    ptr = buf->data + buf->skip + offset;
    memmove(ptr, ptr + n, buf->len - offset - n);

    buf->len -= n;

    if (buf->len == 0)
        buf->skip = 0;

    return n;
}

size_t
bf_buffer_remove(struct bf_buffer *buf, size_t n) {
    return bf_buffer_remove_before(buf, buf->len, n);
}

void *
bf_buffer_extract(struct bf_buffer *buf, size_t *plen) {
    void *data;

    if (!buf->data || buf->len == 0) {
        bf_set_error("cannot extract content from an empty buffer");
        return NULL;
    }

    bf_buffer_repack(buf);

    data = bf_realloc(buf->data, buf->len);
    if (!data)
        return NULL;

    if (plen)
        *plen = buf->len;

    buf->data = NULL;
    buf->sz = 0;
    buf->len = 0;

    return data;
}

char *
bf_buffer_extract_string(struct bf_buffer *buf, size_t *plen) {
    bf_buffer_add(buf, "\0", 1);
    return bf_buffer_extract(buf, plen);
}

void *
bf_buffer_dup(const struct bf_buffer *buf) {
    char *tmp;

    if (!buf->data || buf->len == 0) {
        bf_set_error("cannot duplicate an empty buffer");
        return NULL;
    }

    tmp = bf_malloc(buf->len);
    if (!tmp)
        return NULL;

    memcpy(tmp, buf->data + buf->skip, buf->len);
    return tmp;
}

char *
bf_buffer_dup_string(const struct bf_buffer *buf) {
    char *str;

    str = bf_malloc(buf->len + 1);
    if (!str)
        return NULL;

    if (buf->data)
        memcpy(str, buf->data + buf->skip, buf->len);
    str[buf->len] = '\0';

    return str;
}

ssize_t
bf_buffer_read(struct bf_buffer *buf, int fd, size_t n) {
    ssize_t ret;
    char *ptr;

    ptr = bf_buffer_reserve(buf, n);
    if (!ptr)
        return -1;

    ret = read(fd, ptr, n);
    if (ret > 0)
        buf->len += (size_t)ret;

    return ret;
}

ssize_t
bf_buffer_write(struct bf_buffer *buf, int fd) {
    ssize_t ret;

    ret = write(fd, buf->data + buf->skip, buf->len);
    if (ret > 0) {
        buf->len -= (size_t)ret;

        if (buf->len == 0)
            buf->skip = 0;
    }

    return ret;
}

static void
bf_buffer_repack(struct bf_buffer *buf) {
    if (buf->skip == 0)
        return;

    memmove(buf->data, buf->data + buf->skip, buf->len);
    buf->skip = 0;
}

static int
bf_buffer_resize(struct bf_buffer *buf, size_t sz) {
    char *ndata;

    if (buf->data) {
        ndata = bf_realloc(buf->data, sz);
    } else {
        ndata = bf_malloc(sz);
    }

    if (!ndata)
        return -1;

    buf->data = ndata;
    buf->sz = sz;
    return 0;
}

static int
bf_buffer_grow(struct bf_buffer *buf, size_t sz) {
    return bf_buffer_resize(buf, buf->sz + sz);
}

static int
bf_buffer_ensure_free_space(struct bf_buffer *buf, size_t sz) {
    size_t free_space;

    free_space = bf_buffer_free_space(buf);
    if (free_space < sz)
        return bf_buffer_grow(buf, sz - free_space);

    return 0;
}
