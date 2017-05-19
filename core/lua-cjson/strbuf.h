/* Lstrbuf - String buffer routines
 *
 * Copyright (c) 2010-2012  Mark Pulford <mark@kyne.com.au>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdarg.h>

/* Size: Total bytes allocated to *buf
 * Length: String length, excluding optional NULL terminator.
 * Increment: Allocation increments when resizing the string buffer.
 * Dynamic: True if created via Lstrbuf_new()
 */

typedef struct {
    char *buf;
    int size;
    int length;
    int increment;
    int dynamic;
    int reallocs;
    int debug;
} Lstrbuf_t;

#ifndef Lstrbuf_DEFAULT_SIZE
#define Lstrbuf_DEFAULT_SIZE 1023
#endif
#ifndef Lstrbuf_DEFAULT_INCREMENT
#define Lstrbuf_DEFAULT_INCREMENT -2
#endif

/* Initialise */
extern Lstrbuf_t *Lstrbuf_new(int len);
extern void Lstrbuf_init(Lstrbuf_t *s, int len);
extern void Lstrbuf_set_increment(Lstrbuf_t *s, int increment);

/* Release */
extern void Lstrbuf_free(Lstrbuf_t *s);
extern char *Lstrbuf_free_to_string(Lstrbuf_t *s, int *len);

/* Management */
extern void Lstrbuf_resize(Lstrbuf_t *s, int len);
static int Lstrbuf_empty_length(Lstrbuf_t *s);
static int Lstrbuf_length(Lstrbuf_t *s);
static char *Lstrbuf_string(Lstrbuf_t *s, int *len);
static void Lstrbuf_ensure_empty_length(Lstrbuf_t *s, int len);
static char *Lstrbuf_empty_ptr(Lstrbuf_t *s);
static void Lstrbuf_extend_length(Lstrbuf_t *s, int len);

/* Update */
extern void Lstrbuf_append_fmt(Lstrbuf_t *s, int len, const char *fmt, ...);
extern void Lstrbuf_append_fmt_retry(Lstrbuf_t *s, const char *format, ...);
static void Lstrbuf_append_mem(Lstrbuf_t *s, const char *c, int len);
extern void Lstrbuf_append_string(Lstrbuf_t *s, const char *str);
static void Lstrbuf_append_char(Lstrbuf_t *s, const char c);
static void Lstrbuf_ensure_null(Lstrbuf_t *s);

/* Reset string for before use */
static inline void Lstrbuf_reset(Lstrbuf_t *s)
{
    s->length = 0;
}

static inline int Lstrbuf_allocated(Lstrbuf_t *s)
{
    return s->buf != NULL;
}

/* Return bytes remaining in the string buffer
 * Ensure there is space for a NULL terminator. */
static inline int Lstrbuf_empty_length(Lstrbuf_t *s)
{
    return s->size - s->length - 1;
}

static inline void Lstrbuf_ensure_empty_length(Lstrbuf_t *s, int len)
{
    if (len > Lstrbuf_empty_length(s))
        Lstrbuf_resize(s, s->length + len);
}

static inline char *Lstrbuf_empty_ptr(Lstrbuf_t *s)
{
    return s->buf + s->length;
}

static inline void Lstrbuf_extend_length(Lstrbuf_t *s, int len)
{
    s->length += len;
}

static inline int Lstrbuf_length(Lstrbuf_t *s)
{
    return s->length;
}

static inline void Lstrbuf_append_char(Lstrbuf_t *s, const char c)
{
    Lstrbuf_ensure_empty_length(s, 1);
    s->buf[s->length++] = c;
}

static inline void Lstrbuf_append_char_unsafe(Lstrbuf_t *s, const char c)
{
    s->buf[s->length++] = c;
}

static inline void Lstrbuf_append_mem(Lstrbuf_t *s, const char *c, int len)
{
    Lstrbuf_ensure_empty_length(s, len);
    memcpy(s->buf + s->length, c, len);
    s->length += len;
}

static inline void Lstrbuf_append_mem_unsafe(Lstrbuf_t *s, const char *c, int len)
{
    memcpy(s->buf + s->length, c, len);
    s->length += len;
}

static inline void Lstrbuf_ensure_null(Lstrbuf_t *s)
{
    s->buf[s->length] = 0;
}

static inline char *Lstrbuf_string(Lstrbuf_t *s, int *len)
{
    if (len)
        *len = s->length;

    return s->buf;
}

/* vi:ai et sw=4 ts=4:
 */
