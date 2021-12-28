#include "htmll.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SVDEF static inline
#define SV_IMPLEMENTATION
#include <sv.h>

#include "util.h"
#include "tmp_buf.h"

void read_file_into_buffer(Cstr file_path, Buffer *buf, size_t *out_len)
{
    FILE *infile = fopen(file_path, "rb");

    if (infile == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n",
            file_path, strerror(errno));
        exit(1);
    }

    if (fseek(infile, 0, SEEK_END) < 0) {
        fprintf(stderr, "ERROR: could not seek in file %s: %s\n",
            file_path, strerror(errno));
        fclose(infile);
        exit(1);
    }
    size_t len = ftell(infile);

    if (len < 0) {
        fprintf(stderr, "ERROR: Could not obtain file size for %s: %s\n",
            file_path, strerror(errno));

        fclose(infile);
        exit(1);
    }

    if (out_len) *out_len = len;

    if (fseek(infile, 0, SEEK_SET) < 0) {
        fprintf(stderr, "ERROR: could not seek in file %s: %s\n",
            file_path, strerror(errno));
        fclose(infile);
        exit(1);
    }

    char *contents = NOTNULL(malloc((len) * sizeof(char)));
    fread(contents, sizeof(char), len, infile);
    if (ferror(infile)) {
        fprintf(stderr, "ERROR: Could not read file %s: %s\n",
            file_path, strerror(errno));

        fclose(infile);
        free(contents);
        exit(1);
    }

    buffer_append_str(buf, contents, len);

    free(contents);
    fclose(infile);
}

void htmll(const struct Arguments *args, Buffer *buf)
{
    size_t orig_sz = buf->size;

    assert(args);
    assert(buf);

    assert(args->input_file);
    size_t input_len;
    read_file_into_buffer(args->input_file, buf, &input_len);

    buffer_rewind(buf, orig_sz);
}
