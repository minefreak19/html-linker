/*
Created 28 December 2021
 */

#ifndef HTMLL_H_
#define HTMLL_H_

#include <stdbool.h>

#include "util.h"
#include "tmp_buf.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct HTML_Linker_Args {
    Cstr program_name;
    Cstr input_file;
    Cstr output_file;
    bool mention_source;
    bool ignore_whitespace;
};

#define DEFAULT_HTML_LINKER_ARGS ((struct HTML_Linker_Args) {  \
    .program_name      = NULL,                                 \
    .input_file        = NULL,                                 \
    .output_file       = "./out.html",                         \
    .mention_source    = true,                                 \
    .ignore_whitespace = false,                                \
  })

void htmll(const struct HTML_Linker_Args *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HTMLL_H_ */
