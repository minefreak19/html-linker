#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#define _GNU_SOURCE 1

#include <string.h>
#include <math.h>

// define it up here so util functions can access it
struct arguments_struct
{
    char *input_file;
    char *out_file;
    bool verbose;
} args;
typedef struct arguments_struct arguments;

// utils
#define streq(a, b) (strcmp(a, b) == 0)
#define strcontains(a, b, idx) (strstr(a + idx, b) != NULL)
#define str_startswith(a, b) ((strncmp(a, b, strlen(b))) == 0)

int logv(const char *format, ...)
{
    va_list va_args;
    va_start(va_args, format);
    if (args.verbose)
    {
        printf("[DEBUG] ");
        return vprintf(format, va_args);
    }
    else
        return -1;
}

void error(const char *message)
{
    fprintf(stderr, "\033[38;2;128;0;0mERROR: %s\n", message);
    exit(1);
}

bool str_arr_contains(char *str, int arrlen, char *arr[])
{
    for (int i = 0; i < arrlen; i++)
    {
        if (!streq(str, arr[i]))
            return false;
    }

    return true;
}

#define MODE_READ "r"
#define MODE_WRITE "w"

#define VERBOSE_DEFAULT false

void print_args(arguments *args)
{
    printf("Arguments parsed: \n");
    printf("\tInput file: %s\n", args->input_file);
    printf("\tOutput file: %s\n", args->out_file);

    printf("\n\tFLAGS:\n");
    printf("\tverbose: %s\n", args->verbose ? "true" : "false");

    puts(""); // insert extra newline
}

arguments parse_args(int argc, char *args[])
{
    arguments res;

    res.verbose = VERBOSE_DEFAULT;

    if (argc <= 3)
        error("Too few arguments.");
    char *arg;
    for (int i = 1 /*ignore executable filename*/; i < argc; i++)
    {
        arg = args[i];

        // printf("i: %d\n", i);
        // printf("arg: %s\n", arg);
        // printf("args[i]: %s\n", args[i]);

        if (streq(arg, "-o"))
        {
            res.out_file = args[++i];
            continue;
        }

        // check for flags
        if (streq(arg, "-v") || streq(arg, "--verbose"))
        {
            res.verbose = true;
            continue;
        }

        // the only argument left after all this should be the input file
        res.input_file = arg;
        // strcpy(res.input_file, arg);
    }
    return res;
}

char *inline_scripts_in_html(long len, char *source, arguments args)
{
    char *ret = malloc(len * sizeof(char));
    strcpy(ret, source);

    int *token_indexes = NULL, index_count = 0, cursor = 0;

    char c_val, skip_until = '\0';

    do
    {
        c_val = source[cursor];
        logv("c_val = %c\n", c_val);
        logv("next character = %c\n", source[cursor + 1]);
        logv("cursor = %d\n", cursor);
        logv("currently skipping until: %d\n", skip_until);

        logv("Comparing %c with %c...\n", c_val, skip_until);
        if (c_val == skip_until)
        {
            logv("\t...they are equal.\n");
            skip_until = '\0';
        }

        if (skip_until != '\0')
        {
            logv("skipping...\n");
            goto cont;
        }

        if (c_val == '<')
        {

            if (source[cursor + 1] == '!') // doctype declaration or comment
            {
                skip_until = '>';
                goto cont;
            }

            if (!token_indexes)
                token_indexes = malloc(++index_count * sizeof(int));
            else
                token_indexes = realloc(token_indexes, ++index_count * sizeof(token_indexes[0]));
            token_indexes[index_count - 1] = cursor;
        }

    cont:
        cursor++;
    } while (c_val != '\0');

    logv("index_count = %d\n", index_count);

    for (int i = 0; i < index_count - 1; i++)
    {
        logv("Token %d: \n", i);
        cursor = token_indexes[i];
        logv("Cursor: %d\n", cursor);
        logv("next token index: %d\n", token_indexes[i + 1]);

        // token_indexes[i] is the location where the current token starts
        //  (the previous do-while loop modified the `cursor` variable)

        // this should ignore all script tags
        if (str_startswith(source + token_indexes[i], "<script"))
        {
            logv("encountered script tag\n");
            continue;
        }
        else if (str_startswith(source + token_indexes[i], "</script>"))
        {
            logv("encountered end script tag\n");
            continue;
        }

        do
        {
            c_val = source[cursor++];
            printf("%c", c_val);
        } while (cursor < token_indexes[i + 1]);
        printf("\n");

        puts("\n"); // \n\n
    }

    return ret;
}

int main(int argc, char *cmdargs[])
{
    puts("parsing command-line arguments..."); // TODO remove
    args = parse_args(argc, cmdargs);
    FILE *input_file, *output_file;
    char *contents;
    long int bytes_input, bytes_output;

    if (args.verbose)
        puts("Verbose mode ENABLED");

    // if (args.verbose)
    print_args(&args);

    // read input file
    logv("Opening input file...\n");
    logv("args.input_file = %s\n", args.input_file);
    input_file = fopen(args.input_file, MODE_READ);
    logv("input_file points to %p\n", input_file);

    fseek(input_file, 0l, SEEK_END); // go to end of input_file
    bytes_input = ftell(input_file); // figure out file size
    logv("Input file is %d bytes long.\n", bytes_input);

    fseek(input_file, 0l, SEEK_SET); // go to beginning of input_file

    contents = (char *)calloc(bytes_input, sizeof(char));
    logv("Allocated ptr 0x%p to contents\n", contents);

    if (contents == NULL)
        error("Memory error: calloc() returned NULL");

    logv("Reading input file...\n");
    fread(contents, sizeof(char), bytes_input, input_file);
    logv("Read: \n%s\n\n", contents);

    char *to_parse = (char *)malloc(bytes_input * sizeof(char));
    logv("allocated ptr %p to variable to_parse\n", to_parse);

    logv("Copying contents of input file to variable to_parse...\n");
    strcpy(to_parse, contents);
    logv("copied input contents to to_parse\n");

    logv("calling inline_scripts_in_html...\n");

    char *to_write = inline_scripts_in_html(bytes_input, to_parse, args);

    // write to output file
    logv("opening output file in write mode...\n");
    output_file = fopen(args.out_file, MODE_WRITE);

    fseek(output_file, 0l, SEEK_SET);

    logv("Writing %d bytes to %s\n", bytes_input, args.out_file);
    fprintf(output_file, "%s\n", to_write);

    printf("Successfully wrote %d bytes to %s\n", ftell(output_file), args.out_file);

    fclose(input_file);
    logv("closed input_file\n");
    fclose(output_file);
    logv("closed output_file\n");

    return 0;
}
