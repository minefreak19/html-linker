#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "htmll.h"

void usage(FILE *stream, Cstr program_name)
{
    fprintf(stream, "USAGE: %s [options...] <input_file> -o <output_file>\n",
        program_name);
    fprintf(stream,
        "OPTIONS: \n"
        "   --no-mention-source     Remove comments that mention when something was inlined.\n"
        "   --ignore-whitespace     Ignore whitespace in HTML content.\n"
        "   --include-comments      Include HTML comments in output.\n");
}

#define streq(a, b) (strcmp(a, b) == 0)

struct HTML_Linker_Args parse_html_linker_args(int *argc, Cstr *argv[])
{
    struct HTML_Linker_Args ret = DEFAULT_HTML_LINKER_ARGS;
    ret.program_name = shift_arg(argc, argv);

    Cstr arg;

    while (*argc > 0) {
        arg = shift_arg(argc, argv);

        if (streq(arg, "-o")) {
            if (*argc == 0) {
                usage(stderr, ret.program_name);
                fprintf(stderr, "ERROR: `-o` without output file\n");
                exit(1);
            }

            ret.output_file = shift_arg(argc, argv);
        } else if (streq(arg, "--no-mention-source")) {
            ret.mention_source = false;
        } else if (streq(arg, "--ignore-whitespace")) {
            ret.ignore_whitespace = true;
        } else if (streq(arg, "--include-comments")) {
            ret.include_comments = true;
        } else {
            ret.input_file = arg;
        }
    }

    if (ret.input_file == NULL) {
        usage(stderr, ret.program_name);
        fprintf(stderr, "ERROR: no input file provided\n");
        exit(1);
    }

    if (ret.output_file == NULL) {
        usage(stderr, ret.program_name);
        fprintf(stderr, "ERROR: no output file provided\n");
        exit(1);
    }

    return ret;
}

int main(int argc, const char *argv[])
{
    struct HTML_Linker_Args args = parse_html_linker_args(&argc, &argv);

    htmll(&args);
    printf("Finished!\n");
    return 0;
}
