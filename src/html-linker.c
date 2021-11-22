#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// define it up here so util functions can access it
typedef struct arguments arguments;
struct arguments
{
    char *input_file;
    char *out_file;
    bool verbose;
};

arguments args;

// utils
#define streq(a, b) (strcmp(a, b) == 0)
#define strcontains(a, b, idx) (strstr(a + idx, b) != NULL)
#define str_startswith(a, b) ((strncmp(a, b, strlen(b))) == 0)
#define str_endswith(a, b) ((strncmp(a + (strlen(a) - strlen(b)), b, strlen(b))) == 0)

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

#define MODE_READ "r"
#define MODE_WRITE "w"

char *read_file(const char *path)
{
    FILE *file = fopen(path, MODE_READ);
    char *result;

    // go to the end, and then find out where the cursor is
    fseek(file, 0l, SEEK_END);
    long len = ftell(file);

    // go back to file start
    fseek(file, 0l, SEEK_SET);

    // leave space for ze null byte
    result = calloc(len, sizeof(char));

    // actually read the file
    fread(result, sizeof(char), len, file);

    fclose(file);
    return result;
}

// buffer to be used for string concatenation, etc
//  simpler than dealing with malloc() and free() for this use case

// definitely aren't going to be writing 8KB of html anytime soon
#define TMP_BUFFER_CAP (8192)
char tmp_buf[TMP_BUFFER_CAP];
size_t tmp_buf_sz;

char *tmp_end(void)
{
    return tmp_buf + tmp_buf_sz;
}

char *tmp_alloc(size_t size)
{
    assert(size + tmp_buf_sz < TMP_BUFFER_CAP);
    char *result = tmp_end();
    tmp_buf_sz += size;
    return result;
}

char *tmp_append_char(char c)
{
    *(tmp_alloc(1)) = c;
}

char *tmp_append_str(const char *str, size_t len)
{
    char *append = tmp_alloc(len);
    memcpy(append, str, len);
    return append;
}

char *tmp_append_cstr(const char *cstr)
{
    return tmp_append_str(cstr, strlen(cstr));
}

void tmp_clean(void) { tmp_buf_sz = 0; }

void tmp_rewind(const char *end)
{
    tmp_buf_sz = end - tmp_buf;
}

void tmpcpy(char *to)
{
    memcpy(to, tmp_buf, tmp_buf_sz);
}

char *refactor_relative_path(const char *path, const char *relative_to)
{
    char *rewind = tmp_end();

    char *ret;

    char *last_slash = strrchr(relative_to, '\\');
    if (!last_slash)
    {
        last_slash = strrchr(relative_to, '/');
        if (!last_slash)
        {
            error("no file separators found in argument relative_to");
        }
    }

    // we need the char after the slash
    last_slash++;

    tmp_append_str(relative_to, last_slash - relative_to);
    tmp_append_cstr(path);
    tmp_append_char(0); // terminate string

    ret = malloc(tmp_end() - rewind);
    memcpy(ret, tmp_buf, tmp_end() - rewind);

    tmp_rewind(rewind);

    return ret;
}

char *inline_scripts_in_html(long len, char *source)
{
    char *ret;
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

    // add the string length as a token index
    //  so it stops reading tokens when it gets to the end
    token_indexes = realloc(token_indexes, (index_count + 1) * sizeof(token_indexes[0]));
    token_indexes[index_count] = len;

    logv("index_count = %d\n", index_count);

    const char word_delim[2] = " ";

    size_t after_body_len = 0;
    char **after_body = (char **)malloc(after_body_len * sizeof(after_body[0]));

    // should the current token be appended to the final file?
    char *append = NULL;
    size_t append_len = 0;

    // definitely terrible practice but
    bool prev_script_src = false, prev_style_src = false;
    for (int i = 0; i < index_count; i++)
    {
        logv("Token %d: \n", i);
        cursor = token_indexes[i];
        char *token = source + cursor;
        unsigned int tok_len = token_indexes[i + 1] - cursor;
        logv("Cursor: %d\n", cursor);
        logv("next token index: %d\n", token_indexes[i + 1]);

        // token_indexes[i] is the location where the current token starts
        //  useful if we ever move code around, as `cursor` is modified

        // this should select all script tags
        if (str_startswith(token, "<script"))
        {
            logv("detected script tag\n");
            bool ext_src = false;
            bool defer = false;

            if (!(str_startswith(token, "<script>"))) // if there are attributes, parse them
            {
                char *script_src_path;

                // to duplicate string
                char *script_tag = malloc(tok_len * sizeof(char));
                memcpy(script_tag, token, tok_len);

                char *word = strtok(script_tag, word_delim);
                while (word)
                {
                    logv("currently looking at word: %s\n", word);
                    if (str_startswith(word, "src="))
                    {
                        sscanf(word, "src=\"%s\"", &script_src_path);
                        logv("this word starts with \"src=\"\n");
                        logv("the source path for this script is %s\n", script_src_path);
                        ext_src = true;
                    }
                    if (str_startswith(word, "defer"))
                    {
                        logv("this word starts with \"defer\"\n");
                        defer = true;
                    }
                    word = strtok(NULL, word_delim);
                }
                prev_script_src = ext_src;

                if (ext_src)
                {
                }
            }
            else
            {
                prev_script_src = false; // if no attributes, it's not a source script
            }
            logv("this script tag %s an external source\n", ext_src ? "is" : "is not");
            if (defer)
                logv("this tag is deferred\n");
            if (ext_src)
            {
                append = "";
                append_len = 0;
                // TODO: read file, edit append, check for defer
            }
            else
            {
                append = NULL; // continue as-is
            }
        }
        else if (str_startswith(token, "</script"))
        {
            logv("encountered end script tag\n");

            // exclude this if it ends a script tag with external src
            if (prev_script_src)
            {
                append = "";
                append_len = 0;
            }
            else
            {
                append = NULL;
            }
        }

        // stylesheets?
        if (str_startswith(token, "<link"))
        {
            logv("link tag detected\n");
            append = "";
            append_len = 0;
        }

        if (append)
        {
        }
        else
        {
            append = token;
            append_len = tok_len;
        }
        tmp_append_str(append, append_len);

        // set it back to NULL so we don't keep concatenating the same string
        append = NULL;

        if (args.verbose)
        {
            do
            {
                c_val = source[cursor++];
                printf("%c", c_val);
            } while (cursor < token_indexes[i + 1]);
            printf("\n");

            puts("\n"); // \n\n
        }
    }

    ret = malloc(tmp_buf_sz);
    memcpy(ret, tmp_buf, tmp_buf_sz);

    return ret;
}

int main(int argc, char *cmdargs[])
{
    args = parse_args(argc, cmdargs);
    FILE *input_file, *output_file;
    char *contents;
    long int bytes_input, bytes_output;

    if (args.verbose)
        puts("Verbose mode ENABLED");

    if (args.verbose)
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

    contents = calloc(bytes_input, sizeof(char));
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

    char *to_write = inline_scripts_in_html(bytes_input, to_parse);

    // write to output file
    logv("opening output file in write mode...\n");
    output_file = fopen(args.out_file, MODE_WRITE);

    fseek(output_file, 0l, SEEK_SET);

    // add a trailing newline if there isn't one already
    if (!(str_endswith(to_write, "\n")))
        fprintf(output_file, "%s\n", to_write);
    else
        fprintf(output_file, "%s", to_write);

    printf("Successfully wrote %d bytes to %s\n", ftell(output_file), args.out_file);

    fclose(input_file);
    logv("closed input_file\n");
    fclose(output_file);
    logv("closed output_file\n");

    return 0;
}
