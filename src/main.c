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

}


struct Arguments parse_args(int *argc, Cstr *argv[])
{
    struct Arguments ret = {0};
    ret.program_name = shift_arg(argc, argv);

    Cstr arg;

    while (*argc > 0) {
        arg = shift_arg(argc, argv);

        if (strcmp(arg, "-o") == 0) {
            if (*argc == 0) {
                usage(stderr, ret.program_name);
                fprintf(stderr, "ERROR: `-o` without output file\n");
                exit(1);
            }

            ret.output_file = shift_arg(argc, argv);
        } 
        else {
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
    }

    return ret;
}

int main(int argc, const char *argv[])
{
    struct Arguments args = parse_args(&argc, &argv);
    Buffer *buffer = new_buffer(0);

    htmll(&args);
    printf("Finished!\n");
    return 0;
}
