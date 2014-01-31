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

#ifndef LIBBUFFER_BUFFER_H
#define LIBBUFFER_BUFFER_H

#include <stdarg.h>
#include <stdlib.h>

struct bf_memory_allocator {
   void *(*malloc)(size_t sz);
   void (*free)(void *ptr);
   void *(*calloc)(size_t nb, size_t sz);
   void *(*realloc)(void *ptr, size_t sz);
};

extern struct bf_memory_allocator *bf_default_memory_allocator;


const char *bf_get_error(void);

void bf_set_memory_allocator(const struct bf_memory_allocator *allocator);


struct bf_buffer *bf_buffer_new(size_t initial_size);
void bf_buffer_delete(struct bf_buffer *buf);

char *bf_buffer_data(const struct bf_buffer *buf);
size_t bf_buffer_length(const struct bf_buffer *buf);
void bf_buffer_clear(struct bf_buffer *buf);

char *bf_buffer_reserve(struct bf_buffer *buf, size_t sz);
int bf_buffer_insert(struct bf_buffer *buf, size_t offset, const char *data,
                     size_t sz);
int bf_buffer_add(struct bf_buffer *buf, const char *data, size_t sz);
int bf_buffer_add_buffer(struct bf_buffer *buf, const struct bf_buffer *src);
int bf_buffer_add_string(struct bf_buffer *buf, const char *str);
int bf_buffer_add_vprintf(struct bf_buffer *buf, const char *fmt, va_list ap);
int bf_buffer_add_printf(struct bf_buffer *buf, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

void bf_buffer_skip(struct bf_buffer *buf, size_t n);
size_t bf_buffer_remove_before(struct bf_buffer *buf, size_t offset, size_t n);
size_t bf_buffer_remove_after(struct bf_buffer *buf, size_t offset, size_t n);
size_t bf_buffer_remove(struct bf_buffer *buf, size_t n);

char *bf_buffer_dup(const struct bf_buffer *buf);
char *bf_buffer_dup_string(const struct bf_buffer *buf);

ssize_t bf_buffer_read(struct bf_buffer *buf, int fd, size_t n);
ssize_t bf_buffer_write(struct bf_buffer *buf, int fd);

#endif
