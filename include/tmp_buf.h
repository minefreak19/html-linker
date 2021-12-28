/*
Created 28 December 2021
 */

#ifndef TMP_BUF_H_
#define TMP_BUF_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define DEFAULT_TMP_BUF_CAP 1024

typedef struct Buffer {
    char *data;
    size_t size;
    size_t cap;
} Buffer;

Buffer *new_buffer(size_t cap);

void buffer_clear(Buffer *);
void buffer_free(Buffer *);

void buffer_append_char(Buffer *, char);
void buffer_append_str(Buffer *, const char *str, size_t len);
void buffer_append_cstr(Buffer *, const char *cstr);

void buffer_rewind(Buffer *, size_t prev_sz);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TMP_BUF_H_ */
