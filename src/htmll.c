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
    // HTML_TAG_TYPE_END_LINK,
    HTML_TAG_TYPE_SCRIPT,
    HTML_TAG_TYPE_END_SCRIPT,
    HTML_TAG_TYPE_BODY,
    HTML_TAG_TYPE_END_BODY,

    HTML_TAG_TYPE_CONTENT,

    HTML_TAG_TYPE_OTHER,

    COUNT_HTML_TAG_TYPES
} HTML_Tag_Type;

static_assert(COUNT_HTML_TAG_TYPES == 7, "Exhaustive definition of HTML_TAG_TYPE_NAMES with respect to HTML_Tag_Type's");
const char *const HTML_TAG_TYPE_NAMES[COUNT_HTML_TAG_TYPES] = {
    [HTML_TAG_TYPE_LINK]       = "LINK",
    [HTML_TAG_TYPE_SCRIPT]     = "SCRIPT",
    [HTML_TAG_TYPE_BODY]       = "BODY",

    [HTML_TAG_TYPE_END_BODY]   = "END BODY",
    [HTML_TAG_TYPE_END_SCRIPT] = "END SCRIPT",
    
    [HTML_TAG_TYPE_CONTENT]    = "CONTENT",
    
    [HTML_TAG_TYPE_OTHER]      = "OTHER",
};

typedef struct {
    String_View href;
    String_View rel;
} HTML_Link_Tag;

typedef struct {
    bool deferred;
    bool closed;
    bool inlyne;
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
    String_View text;
} HTML_Tag;

typedef struct {
    bool in_script;  
} Parser;

static void parse_html_link_tag(Parser *parser, String_View *source, HTML_Tag *out)
{
    (void) parser;
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

            if (attr_value.count >= 2) {
                // get rid of quotes
                sv_chop_left(&attr_value, 1);
                sv_chop_right(&attr_value, 1);
            }

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

static void parse_html_script_tag(Parser *parser, String_View *source, HTML_Tag *out)
{
    assert(source->data != NULL);
    HTML_Tag result = {0};
    bool has_src = false;

    result.type = HTML_TAG_TYPE_SCRIPT;

    {
        String_View attr, attr_name, attr_value;

        while (source->count > 0) {
            attr = sv_chop_by_delim(source, ' ');
            if (attr.count == 0) continue;

            attr_name = sv_chop_by_delim(&attr, '=');
            attr_value = sv_trim(attr);

            if (attr_value.count >= 2) {
                // get rid of quotes
                sv_chop_left(&attr_value, 1);
                sv_chop_right(&attr_value, 1);
            }

            if (sv_eq(attr_name, (String_View) SV_STATIC("src"))) {
                result.as.script.src = attr_value;
                has_src = true;
            } else if (sv_eq(attr_name, (String_View) SV_STATIC("defer"))) {
                result.as.script.deferred = true;
            } else if (sv_eq(attr_name, (String_View) SV_STATIC("type"))) {
                if (!(sv_eq_ignorecase(attr_value, SV("text/javascript")))) {
                    // TODO: non-supported script types should be treated as regular HTML and copied across
                    fprintf(stderr, "ERROR: script type `"SV_Fmt"` is not supported.\n",
                            SV_Arg(attr_value));
                    exit(1);
                }
            } else if (sv_eq(attr_name, (String_View) SV_STATIC("/"))) {
                result.as.script.closed = true;
            }
            *source = sv_trim(*source);
        }
    }

    result.as.script.inlyne = !has_src;

    if (!(result.as.script.closed)) {
        parser->in_script = true;
    }

    if (out) *out = result;
}

static bool parse_html_tag(Parser *parser, String_View *source, HTML_Tag *out)
{
    *source = sv_trim_left(*source);
    
    if (source->count == 0) return false;

    if (sv_starts_with(*source, (String_View) SV_STATIC("<!DOCTYPE"))) {
        sv_chop_by_delim(source, '>');
        if (sv_trim(*source).count == 0) {
            fprintf(stderr, "ERROR: Expected HTML Tag, but reached EOF\n");
            exit(1);
        }

        return parse_html_tag(parser, source, out);
    }

    if (sv_starts_with(*source, (String_View) SV_STATIC("<!--"))) {
        sv_chop_by_sv(source, SV("-->"));
        if (sv_trim(*source).count == 0) {
            return false;
        }
    }

    if (!(sv_starts_with(*source, (String_View) SV_STATIC("<")))) {
        if (!(parser->in_script)) {
            // TODO: Add `ignore-whitespace` option to ignore content that is all whitespace
            String_View content = sv_chop_by_delim(source, '<');
        
            HTML_Tag result = {
                .name = SV_STATIC("__content__"),
                .type = HTML_TAG_TYPE_CONTENT,
                .text = content,
                .as = {0},
            };

            if (out) *out = result;

            source->data--;
            source->count++;

            return true;
        } else {
            // TODO: inlined javascript should be handled properly 
            //  current implementation gets triggered by `</script>`
            //  in string literals, comments, etc
            const String_View end_script_tag = SV_STATIC("</script>");
            String_View inlined_script = sv_chop_by_sv(source, end_script_tag);
            source->data  -= end_script_tag.count;
            source->count += end_script_tag.count;
            HTML_Tag result = {
                .name = SV("__inlined_script__"),
                .text = inlined_script,
                .type = HTML_TAG_TYPE_SCRIPT,
                .as   = {
                    .script = {
                        .closed = false,
                        .deferred = false,
                        .inlyne = true,
                        .src = {0},
                    },
                },
            };
            if (out) *out = result;
            return true;
        }
    }

    {
        sv_chop_left(source, 1);
        String_View tag = sv_trim(sv_chop_by_delim(source, '>'));
        
        if (tag.count == 0) {
            fprintf(stderr, "ERROR: Empty HTML tag\n");
        }

        String_View tag_text = tag;
        tag_text.data  -= 1;
        tag_text.count += 2;
       
        HTML_Tag result;

        String_View name = sv_trim(sv_chop_by_delim(&tag, ' '));

        if (sv_eq(name, SV("link"))) {
            parse_html_link_tag(parser, &tag, &result);
        } else if (sv_eq(name, SV("script"))) {
            parse_html_script_tag(parser, &tag, &result);
        } else if (sv_eq(name, SV("/script"))) {
            if (parser->in_script) {
                parser->in_script = false;
                result.type = HTML_TAG_TYPE_END_SCRIPT;
            } else {
                return false;
            }
        } else if (sv_eq(name, SV("body"))) {
            result.type = HTML_TAG_TYPE_BODY;
        } else if (sv_eq(name, SV("/body"))) {
            result.type = HTML_TAG_TYPE_END_BODY;
        } else {
            result.type = HTML_TAG_TYPE_OTHER;
        }

        result.name = name;
        result.text = tag_text;

        if (out) *out = result;
        return true;
    }
}

static void print_html_link_tag(FILE *stream, HTML_Tag tag)
{
    assert(tag.type == HTML_TAG_TYPE_LINK);
    fprintf(stream, "       HTML_Link_tag.as.link {\n"
                    "           href = `"SV_Fmt"`,\n"
                    "           rel = `"SV_Fmt"`,\n"
                    "       }\n",
            SV_Arg(tag.as.link.href),
            SV_Arg(tag.as.link.rel));
}

#define BOOL_TO_STR(b) (b ? "true" : "false")
static void print_html_script_tag(FILE *stream, HTML_Tag tag)
{
    assert(tag.type == HTML_TAG_TYPE_SCRIPT);
    if (!tag.as.script.inlyne) {
        fprintf(stream, "       HTML_Script_Tag {\n"
                        "           inlined = %s,\n"
                        "           src = `"SV_Fmt"`,\n"
                        "           closed = %s,\n"
                        "           deferred = %s,\n"
                        "       }\n",
                BOOL_TO_STR(tag.as.script.inlyne),
                SV_Arg(tag.as.script.src),
                BOOL_TO_STR(tag.as.script.closed),
                BOOL_TO_STR(tag.as.script.deferred));
    } else {
        fprintf(stream, "       HTML_Script_Tag {\n"
                        "           inlined = %s,\n"
                        "           closed = %s,\n"
                        "           deferred = %s,\n",
                BOOL_TO_STR(tag.as.script.inlyne),
                BOOL_TO_STR(tag.as.script.closed),
                BOOL_TO_STR(tag.as.script.deferred));
        if (!tag.as.script.closed)
            fprintf(stream, 
                        "           text = `"SV_Fmt"`,\n",
                        SV_Arg(tag.text));
        fprintf(stream, "       }\n");
    }
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
            print_html_link_tag(stream, tag);
        } break;
        
        case HTML_TAG_TYPE_SCRIPT: {
            print_html_script_tag(stream, tag);
        } break;

        default: {};
    }

    if (tag.text.count > 0) {
        fprintf(stream, "   text = `"SV_Fmt"`\n",
                    SV_Arg(tag.text));
    }

    fprintf(stream, "}\n");
}

typedef struct {
    Buffer *output;
    Buffer *after_body;
} HTML_Linker;

static void process_html_tag(HTML_Linker *linker, HTML_Tag tag)
{
    switch (tag.type) {
        case HTML_TAG_TYPE_SCRIPT: {
            // TODOOOOOOOOOOOOOOOOOOOO: process_html_tag does not grab scripts from files
            if (tag.as.script.inlyne) {
                buffer_append_str(linker->output, tag.text.data, tag.text.count);
            } else {
                if (tag.as.script.deferred) {
                    printf("appending `"SV_Fmt"` to linker->after_body\n", SV_Arg(tag.text));
                    buffer_append_str(linker->after_body, tag.text.data, tag.text.count);
                } else {
                    buffer_append_str(linker->output, tag.text.data, tag.text.count);
                }
            }
        } break;

        case HTML_TAG_TYPE_END_BODY: {
            buffer_append_str(linker->output, tag.text.data, tag.text.count);
            buffer_append_str(linker->output, linker->after_body->data, linker->after_body->size);
        } break;

        default: {
            buffer_append_str(linker->output, tag.text.data, tag.text.count);
        }
    }
}

// TODOOOO: ECMAScript modules not supported, only includes files directly in html
void htmll(const struct Arguments *args)
{
    Buffer *input_buf = new_buffer(0);
    Buffer *output_buf = new_buffer(0);

    assert(args);

    assert(args->input_file);
    assert(args->output_file);
    read_file_into_buffer(args->input_file, input_buf);
    String_View contents = sv_from_parts(input_buf->data, input_buf->size);

    HTML_Tag tag;
    bool success;
    Parser parser = {0};
    HTML_Linker *linker = &(HTML_Linker) {
        .output = output_buf,
        .after_body = new_buffer(0),
    };
    while ((success = parse_html_tag(&parser, &contents, &tag))) {
        if (success) {
            print_html_tag(stdout, tag);
            process_html_tag(linker, tag);
        }
    }

    buffer_clear(input_buf);
    buffer_free(input_buf);

    FILE *outfile = fopen(args->output_file, "wb");
    if (outfile == NULL) {
        fprintf(stderr, "ERROR: Could not open file %s: %s\n", 
                args->output_file, strerror(errno));
        exit(1);
    }

    fprintf(outfile, "%s\n", "<!DOCTYPE html>");
    fprintf(outfile, "%.*s", (int) output_buf->size, output_buf->data);

    fclose(outfile);

    buffer_clear(linker->after_body);
    buffer_free(linker->after_body);

    buffer_clear(output_buf);
    buffer_free(output_buf);
}
