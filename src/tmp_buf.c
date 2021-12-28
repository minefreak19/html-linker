#include "tmp_buf.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

static void buffer_ensure(Buffer *buf, size_t req)
{
    if (buf->cap >= req) return;

    buf->data = NOTNULL(realloc(buf->data, req));
}

Buffer *new_buffer(size_t cap)
{
    if (cap == 0) cap = DEFAULT_TMP_BUF_CAP;

    Buffer *result = NOTNULL(malloc(sizeof(Buffer)));

    *result = (Buffer) {
        .cap = cap,
        .data = NOTNULL(calloc(cap, sizeof(char))),
        .size = 0,
    };

    return result;
}

void buffer_clear(Buffer *buf) 
{
    assert(buf != NULL);
    buf->size = 0;
}

void buffer_free(Buffer *buf)
{
    assert(buf != NULL);

    if (buf->size > 0) {
        PANIC("Attempted to free non-empty buffer. buffer_clear() should be called first.\n");
    }

    free(buf->data);
    free(buf);
}

void buffer_append_char(Buffer *buf, char c)
{
    buffer_ensure(buf, buf->size + 1);
    buf->data[buf->size++] = c;
}

void buffer_append_str(Buffer *buf, const char *str, size_t len)
{
    assert(str != NULL);
    assert(buf != NULL);

    if (len == 0) return;
    
    buffer_ensure(buf, buf->size + len);

    memcpy(&buf->data[buf->size], str, len);
    buf->size += len;
}

void buffer_append_cstr(Buffer *buf, const char *cstr)
{
    assert(cstr != NULL);
    buffer_append_str(buf, cstr, strlen(cstr));
}

void buffer_rewind(Buffer * buf, size_t prev_sz)
{
    assert(buf != NULL);
    assert(prev_sz <= buf->size);
    
    buf->size = prev_sz;
}
