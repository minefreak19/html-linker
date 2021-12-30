#include "htmll.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SVDEF static inline
#define SV_IMPLEMENTATION
#include <sv.h>

#include "util.h"
#include "tmp_buf.h"

static void read_file_into_buffer(Cstr file_path, Buffer *buf)
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

// very rudimentary and non-extensible because we only care about very specific types of tags
typedef enum {
    HTML_TAG_TYPE_LINK = 0,
    HTML_TAG_TYPE_END_LINK,
    HTML_TAG_TYPE_SCRIPT,
    HTML_TAG_TYPE_END_SCRIPT,
    HTML_TAG_TYPE_BODY,
    HTML_TAG_TYPE_OTHER,

    COUNT_HTML_TAG_TYPES
} HTML_Tag_Type;

const char *const const HTML_TAG_TYPE_NAMES[COUNT_HTML_TAG_TYPES] = {
    [HTML_TAG_TYPE_LINK] = "LINK",
    [HTML_TAG_TYPE_SCRIPT] = "SCRIPT",
    [HTML_TAG_TYPE_BODY] = "BODY",
    [HTML_TAG_TYPE_OTHER] = "OTHER",
};

typedef struct {
    String_View href;
    String_View rel;
} HTML_Link_Tag;

typedef struct {
    bool deferred;
    String_View src;
} HTML_Script_Tag;

typedef union {
    HTML_Link_Tag link;
    HTML_Script_Tag script;
} HTML_Tag_As;

typedef struct {
    HTML_Tag_Type type;
    HTML_Tag_As as;
    String_View name;
} HTML_Tag;

static void parse_html_link_tag(String_View *source, HTML_Tag *out)
{
    assert(source->data);
    HTML_Tag result;

    result.type = HTML_TAG_TYPE_LINK;

    {
        String_View attr, attr_name, attr_value;
        while (source->count > 0) {
            attr = sv_chop_by_delim(source, ' ');
            if (attr.count == 0) continue;

            attr_name = sv_chop_by_delim(&attr, '=');
            attr_value = sv_trim(attr);

            // get rid of quotes
            sv_chop_left(&attr_value, 1);
            sv_chop_right(&attr_value, 1);

            if (sv_eq(attr_name, SV("href"))) {
                result.as.link.href = attr_value;
            } else if (sv_eq(attr_name, SV("rel"))) {
                result.as.link.rel = attr_value;
            }
            *source = sv_trim(*source);
        }
    }

    if (out) *out = result;
}

static void parse_html_script_tag(String_View *source, HTML_Tag *out)
{
    assert(source->data != NULL);
    HTML_Tag result;

    result.type = HTML_TAG_TYPE_SCRIPT;

    {
        String_View attr, attr_name, attr_value;

        while (source->count > 0) {
            attr = sv_chop_by_delim(source, ' ');
            if (attr.count == 0) continue;

            attr_name = sv_chop_by_delim(&attr, '=');
            attr_value = sv_trim(attr);

            // get rid of quotes
            sv_chop_left(&attr_value, 1);
            sv_chop_right(&attr_value, 1);

            if (sv_eq(attr_name, (String_View) SV_STATIC("src"))) {
                result.as.script.src = attr_value;
            } else if (sv_eq(attr_name, (String_View) SV_STATIC("defer"))) {
                result.as.script.deferred = true;
            }
            *source = sv_trim(*source);
        }
    }

    if (out) *out = result;
}

static bool parse_html_tag(String_View *source, HTML_Tag *out)
{
    *source = sv_trim_left(*source);
    if (sv_starts_with(*source, (String_View) SV_STATIC("<!DOCTYPE"))) {
        sv_chop_by_delim(source, '>');
        if (sv_trim(*source).count == 0) {
            fprintf(stderr, "ERROR: Expected HTML Tag, but reached EOF\n");
            exit(1);
        }

        return parse_html_tag(source, out);
    }

    if (sv_starts_with(*source, (String_View) SV_STATIC("<!--"))) {
        sv_chop_by_sv(source, SV("-->"));
        if (sv_trim(*source).count == 0) {
            return false;
        }
    }

    if (!(sv_starts_with(*source, (String_View) SV_STATIC("<")))) {
        fprintf(stderr, "ERROR: Unexpected character `%c` in HTML tag\n", *source->data);
        exit(1);
    }

    {
        sv_chop_left(source, 1);
        String_View tag = sv_trim(sv_chop_by_delim(source, '>'));
        if (tag.count == 0) {
            fprintf(stderr, "ERROR: Empty HTML tag\n");
        }

        HTML_Tag result;

        String_View name = sv_trim(sv_chop_by_delim(&tag, ' '));

        if (sv_eq(name, SV("link"))) {
            parse_html_link_tag(&tag, &result);
        } else if (sv_eq(name, SV("script"))) {
            parse_html_script_tag(&tag, &result);
        } else if (sv_eq(name, SV("body"))) {
            assert(false && "Unimplemented");
        } else {
            result.type = HTML_TAG_TYPE_OTHER;
        }

        result.name = name;

        if (out) *out = result;
        return true;
    }
}

static void print_html_link_tag(FILE *stream, HTML_Link_Tag tag)
{
    fprintf(stream, "       HTML_Link_Tag {\n"
                    "           href = `"SV_Fmt"`,\n"
                    "           rel = `"SV_Fmt"`,\n"
                    "       }\n",
            SV_Arg(tag.href),
            SV_Arg(tag.rel));
}

static void print_html_tag(FILE *stream, HTML_Tag tag)
{
    fprintf(stream, "HTML_Tag {\n"
                    "   name  = "SV_Fmt",\n"
                    "   type  = %s,\n"
                    "   value = \n",
            SV_Arg(tag.name),
            HTML_TAG_TYPE_NAMES[tag.type]);
    switch (tag.type) {
        case HTML_TAG_TYPE_LINK: {
            print_html_link_tag(stream, tag.as.link);
        } break;
    }

    fprintf(stream, "}\n");
}

// TODOOOO: ECMAScript modules not supported, only includes files directly in html
void htmll(const struct Arguments *args)
{
    Buffer *input_buf = new_buffer(0);

    assert(args);

    assert(args->input_file);
    read_file_into_buffer(args->input_file, input_buf);
    String_View contents = sv_from_parts(input_buf->data, input_buf->size);

    HTML_Tag tag;
    bool success;
    while (success = parse_html_tag(&contents, &tag)) {
        if (success) {
            print_html_tag(stdout, tag);
        }
        else printf("Could not parse an HTML Tag\n");
    }

    buffer_clear(input_buf);
    buffer_free(input_buf);
}
