/*
Created 28 December 2021
 */

#ifndef HTMLL_H_
#define HTMLL_H_

#include "util.h"
#include "tmp_buf.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

struct Arguments {
    Cstr program_name;
    Cstr input_file;
    Cstr output_file;
};

void htmll(const struct Arguments *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HTMLL_H_ */
