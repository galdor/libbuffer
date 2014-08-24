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
   void *(*malloc)(size_t);
   void (*free)(void *);
   void *(*calloc)(size_t, size_t);
   void *(*realloc)(void *, size_t);
};

extern struct bf_memory_allocator *bf_default_memory_allocator;

const char *bf_version(void);
const char *bf_build_id(void);

const char *bf_get_error(void);

void bf_set_memory_allocator(const struct bf_memory_allocator *);

void *bf_malloc(size_t);
void bf_free(void *);
void *bf_calloc(size_t, size_t);
void *bf_realloc(void *, size_t);

struct bf_buffer *bf_buffer_new(size_t);
void bf_buffer_delete(struct bf_buffer *);

void *bf_buffer_data(const struct bf_buffer *);
size_t bf_buffer_length(const struct bf_buffer *);
size_t bf_buffer_size(const struct bf_buffer *);
size_t bf_buffer_free_space(const struct bf_buffer *);

void bf_buffer_reset(struct bf_buffer *);
void bf_buffer_clear(struct bf_buffer *);
void bf_buffer_truncate(struct bf_buffer *, size_t);

void *bf_buffer_reserve(struct bf_buffer *, size_t);
int bf_buffer_increase_length(struct bf_buffer *, size_t);
int bf_buffer_insert(struct bf_buffer *, size_t, const void *, size_t);
int bf_buffer_add(struct bf_buffer *, const void *, size_t);
int bf_buffer_add_buffer(struct bf_buffer *, const struct bf_buffer *);
int bf_buffer_add_string(struct bf_buffer *, const char *);
int bf_buffer_add_vprintf(struct bf_buffer *, const char *, va_list);
int bf_buffer_add_printf(struct bf_buffer *, const char *, ...)
    __attribute__((format(printf, 2, 3)));

void bf_buffer_skip(struct bf_buffer *, size_t);
size_t bf_buffer_remove_before(struct bf_buffer *, size_t, size_t);
size_t bf_buffer_remove_after(struct bf_buffer *, size_t, size_t);
size_t bf_buffer_remove(struct bf_buffer *, size_t);

void *bf_buffer_extract(struct bf_buffer *, size_t *);
void *bf_buffer_dup(const struct bf_buffer *);
char *bf_buffer_dup_string(const struct bf_buffer *);

ssize_t bf_buffer_read(struct bf_buffer *, int, size_t);
ssize_t bf_buffer_write(struct bf_buffer *, int);

#endif
