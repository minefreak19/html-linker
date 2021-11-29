#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// define it up here so logv can access it
typedef struct arguments arguments;
struct arguments
{
    char *input_file;
    char *out_file;
    bool verbose;
    size_t tmp_mem;
};

arguments args;

// utils
#ifdef _WIN32
#define PATH_SEPARATOR ('\\')
#else
#define PATH_SEPARATOR ('/')
#endif // __win32

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
    fprintf(stderr, "ERROR: %s\n", message);
    exit(1);
}

bool str_arr_contains(char *str, int arrlen, char *arr[])
{
    for (int i = 0; i < arrlen; i++)
    {
        if (streq(str, arr[i]))
            return true;
    }

    return false;
}

// to be safe when using malloc/chkmem(calloc in case they return null
void *chkmem(void *ptr)
{
    if (ptr == NULL)
    {
        fprintf(stderr, "MEMORY ERROR: Pointer allocated was NULL. Possibly out of memory.");
        exit(2);
    }
    else
        return ptr;
}

#define VERBOSE_DEFAULT false

void print_args(arguments *args)
{
    printf("Arguments parsed: \n");
    printf("\tInput file: %s\n", args->input_file);
    printf("\tOutput file: %s\n", args->out_file);

    printf("\n\tFLAGS:\n");
    printf("\tverbose: %s\n", args->verbose ? "true" : "false");

    printf("\n");
}

void print_usage(char *executable_name)
{
    // start immediately after the last slash to get only the executable name
    printf("USAGE: %s [flags] <inputFile> -o <outputFile>\n", strrchr(executable_name, PATH_SEPARATOR) + 1);
    printf("FLAGS:\n");
    printf("\t-v | --verbose: Enables verbose mode for debugging.\n");
}

arguments parse_args(int argc, char *args[])
{
    arguments res;

    res.verbose = VERBOSE_DEFAULT;

    if (argc <= 3)
    {
        print_usage(args[0]);
        error("Too few arguments.");
    }
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
    logv("opening file %s\n", path);
    FILE *file = fopen(path, MODE_READ);

    if (!file)
    {
        fprintf(stderr, "error: could not find file at %s\n", path);
        exit(1);
    }
    char *result;

    // go to the end, and then find out where the cursor is
    fseek(file, 0l, SEEK_END);
    long len = ftell(file);

    // go back to file start
    fseek(file, 0l, SEEK_SET);

    // leave space for ze null byte
    result = chkmem(calloc(len + 1, sizeof(char)));

    // actually read the file
    fread(result, sizeof(char), len, file);

    fclose(file);

    logv("read: \n%s\n", result);
    return result;
}

// buffer to be used for string concatenation, etc
//  simpler than dealing with malloc() and free() for this use case

// definitely aren't going to be writing 1MB of html anytime soon
// #define DEF_TMP_BUF_CAP (1048576)
#define DEF_TMP_BUF_CAP (1024)
size_t tmp_buf_cap = DEF_TMP_BUF_CAP;
char *tmp_buf = NULL;
size_t tmp_buf_sz = 0;

void tmp_init()
{
    tmp_buf = malloc(tmp_buf_cap * sizeof(char));
}

char *tmp_end(void)
{
    return tmp_buf + tmp_buf_sz;
}

void tmp_ensure(size_t size)
{
    size_t req_sz = size + tmp_buf_sz;
    if (req_sz > tmp_buf_cap)
    {
        tmp_buf_cap = req_sz;
        tmp_buf = chkmem(realloc(tmp_buf, req_sz));
        // the casting makes the compiler happy
        // i'm sure theres a fix but this works
        printf("reallocating tmp buffer to size %u\n", (unsigned)req_sz);
    }
}

char *tmp_alloc(size_t size)
{
    tmp_ensure(size);
    char *result = tmp_end();
    tmp_buf_sz += size;
    return result;
}

char *tmp_append_char(char c)
{
    *(tmp_alloc(1)) = c;
    return tmp_end();
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

void tmp_rewind(size_t old_sz)
{
    tmp_buf_sz = old_sz;
}

void tmpcpy(char *to)
{
    memcpy(to, tmp_buf, tmp_buf_sz);
}

char *refactor_relative_path(const char *path, const char *relative_to)
{
    logv("refactoring path \"%s\" relative to \"%s\"\n", path, relative_to);
    logv("current size of tmp: %I32u\n", tmp_buf_sz);
    size_t rewind = tmp_buf_sz;

    char *ret;

    // not using PATH_SEPARATOR because we want to support forward-slash paths on windows too
    char *last_slash = strrchr(relative_to, '\\');
    logv("last_slash = %p\n", last_slash);
    if (!last_slash)
    {
        last_slash = strrchr(relative_to, '/');
        logv("last_slash = %p\n", last_slash);
        if (!last_slash)
        {
            error("no file separators found in argument relative_to");
        }
    }

    // we need the char after the slash
    last_slash++;
    logv("last_slash = %p\n", last_slash);

    tmp_append_str(relative_to, last_slash - relative_to);
    tmp_append_cstr(path);
    tmp_append_char(0); // terminate string

    ret = chkmem(malloc(tmp_buf_sz - rewind));
    memcpy(ret, tmp_buf + rewind, tmp_buf_sz - rewind);

    tmp_rewind(rewind);

    logv("refactored relative path: %s\n", ret);
    return ret;
}

char *inline_scripts_in_html(long len, char *source)
{
    size_t rewind = tmp_buf_sz;

    char *ret;
    int *token_indexes = NULL, index_count = 0, cursor = 0;

    char c_val, skip_until = '\0';

    do
    {
        c_val = source[cursor];
        if (c_val == skip_until)
        {
            skip_until = '\0';
        }

        if (skip_until != '\0')
        {
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
                token_indexes = chkmem(malloc(++index_count * sizeof(int)));
            else
                token_indexes = chkmem(realloc(token_indexes, ++index_count * sizeof(token_indexes[0])));
            token_indexes[index_count - 1] = cursor;
        }

    cont:
        cursor++;
    } while (c_val != '\0');

    // add the string length as a token index
    //  so it stops reading tokens when it gets to the end
    token_indexes = chkmem(realloc(token_indexes, (index_count + 1) * sizeof(token_indexes[0])));
    token_indexes[index_count] = len;

    logv("index_count = %d\n", index_count);

    const char word_delim[] = "> ";

    size_t after_body_len = 0;
    char **after_body = chkmem(malloc(after_body_len * sizeof(after_body[0])));

    // should the current token be appended to the final file?
    char *append = NULL;
    size_t append_len = 0;

    // definitely terrible practice but
    bool prev_script_src = false;
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
                char *script_tag = chkmem(malloc(tok_len * sizeof(char)));
                memcpy(script_tag, token, tok_len);
                script_tag[tok_len] = 0;

                char *word = strtok(script_tag, word_delim);
                while (word)
                {
                    logv("currently looking at word: %s\n", word);
                    if (str_startswith(word, "src="))
                    {
                        logv("this word starts with \"src=\"\n");
                        logv("word = 0x%p\n", word);
                        logv("script_src_path = %p\n", script_src_path);
                        logv("strlen(word) = %d\n", strlen(word));
                        script_src_path = chkmem(malloc((strlen(word) - strlen("src=\"\"")) * sizeof(char)));

                        logv("script_src_path = %p\n", script_src_path);
                        logv("*script_src_path = %s\n", script_src_path);
                        sscanf(word, "src=\"%s", script_src_path);
                        logv("the source path for this script is %s\n", script_src_path);
                        script_src_path[strlen(script_src_path) - 1] = 0;
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

                if (defer)
                    logv("this tag is deferred\n");
                if (ext_src)
                {
                    logv("script_src_path = %s\n", script_src_path);
                    logv("args.input_file = %s\n", args.input_file);

                    char *real_src_path = refactor_relative_path(script_src_path, args.input_file);
                    char *script = read_file(real_src_path);

                    logv("javascript source is:\n%s\n", script);

                    size_t script_len = strlen(script) + strlen("<script>") + strlen("</script>");
                    char *to_append;

                    to_append = chkmem(calloc(script_len, sizeof(char)));
                    logv("`to_append` = %p\n", to_append);

                    strcat(to_append, "<script>");
                    strcat(to_append, script);
                    strcat(to_append, "</script>");

                    if (!defer)
                    {
                        logv("setting `append` to:\n%s\n", to_append);
                        append = to_append;
                        append_len = script_len;
                    }
                    else
                    {
                        after_body = chkmem(realloc(after_body, ++after_body_len * sizeof(after_body[0])));
                        after_body[after_body_len - 1] = to_append;
                        append = "";
                        append_len = 0;
                    }
                }
                else
                {
                    append = NULL; // continue as-is
                }
            }
            else
            {
                prev_script_src = false; // if no attributes, it's not a source script
            }
            logv("this script tag %s an external source\n", ext_src ? "is" : "is not");
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

            bool stylesheet = false;
            char *stylesheet_src_path;

            char *link_tag = chkmem(malloc(tok_len));
            memcpy(link_tag, token, tok_len);

            char *word = strtok(link_tag, word_delim);
            while (word)
            {
                logv("currently looking at word: %s\n", word);
                if (str_startswith(word, "rel=\"stylesheet\""))
                {
                    stylesheet = true;
                }
                if (str_startswith(word, "href=\""))
                {
                    logv("strlen(word) = %d\n", strlen(word));
                    // logv("strlen(word) - strlen(\"src=\\\"\\\"\") = ", strlen(word) - strlen("src=\"\""));
                    stylesheet_src_path = chkmem(calloc((strlen(word) - strlen("href=\"\"")), sizeof(char)));
                    logv("stylesheet_src_path = %p\n", stylesheet_src_path);
                    logv("*stylesheet_src_path = %s\n", stylesheet_src_path);
                    sscanf(word, "href=\"%s", stylesheet_src_path);
                    logv("the source path for this script is %s\n", stylesheet_src_path);
                    stylesheet_src_path[strlen(stylesheet_src_path) - 1] = 0;
                }
                word = strtok(NULL, word_delim);
            }

            if (stylesheet)
            {
                logv("this links to a stylesheet\n");

                logv("stylesheet_src_path = %s\n", stylesheet_src_path);
                logv("args.input_file = %s\n", args.input_file);

                if (!(*stylesheet_src_path))
                {
                    error("no href provided in stylesheet link");
                }

                char *real_src_path = refactor_relative_path(stylesheet_src_path, args.input_file);
                char *style = read_file(real_src_path);

                logv("style source is:\n%s\n", style);

                size_t style_len = strlen(style) + strlen("<style>") + strlen("</style>");
                char *to_append;

                to_append = chkmem(calloc(style_len, sizeof(char)));
                logv("`to_append` = %p\n", to_append);

                strcat(to_append, "<style>");
                strcat(to_append, style);
                strcat(to_append, "</style>");

                logv("setting `append` to:\n%s\n", to_append);
                append = to_append;
                append_len = style_len;
            }
            else
            {
                append = NULL;
            }
        }

        if (str_startswith(token, "</body>"))
        {
            logv("detected end body tag\n");
            append = "";
            append_len = 0;

            tmp_append_cstr("</body>");
            for (size_t i = 0; i < after_body_len; i++)
            {
                tmp_append_cstr(after_body[i]);
            }
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

    ret = chkmem(malloc(tmp_buf_sz));
    memcpy(ret, tmp_buf + rewind, tmp_buf_sz - rewind);

    tmp_rewind(rewind);

    return ret;
}

void html_linker(void)
{
    FILE *input_file, *output_file;
    char *contents;
    long int bytes_input;

    // read input file
    logv("Opening input file...\n");
    logv("args.input_file = %s\n", args.input_file);
    input_file = fopen(args.input_file, MODE_READ);
    logv("input_file points to %p\n", input_file);

    fseek(input_file, 0l, SEEK_END); // go to end of input_file
    bytes_input = ftell(input_file); // figure out file size
    logv("Input file is %d bytes long.\n", bytes_input);

    fseek(input_file, 0l, SEEK_SET); // go to beginning of input_file

    contents = chkmem(calloc(bytes_input, sizeof(char)));
    logv("Allocated ptr 0x%p to contents\n", contents);

    if (contents == NULL)
        error("Memory error: chkmem(calloc() returned NULL");

    logv("Reading input file...\n");
    fread(contents, sizeof(char), bytes_input, input_file);
    logv("Read: \n%s\n\n", contents);

    char *to_parse = chkmem(malloc(bytes_input * sizeof(char)));
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

    printf("Successfully wrote %ld bytes to %s\n", ftell(output_file), args.out_file);

    fclose(input_file);
    logv("closed input_file\n");
    fclose(output_file);
    logv("closed output_file\n");
}

int main(int argc, char *argv[])
{
    tmp_init();

    args = parse_args(argc, argv);

    if (args.verbose)
        puts("Verbose mode ENABLED");

    if (args.verbose)
        print_args(&args);

    html_linker();
}
