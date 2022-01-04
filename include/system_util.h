/*
Created 04 January 2022
 */

#ifndef FILE_UTIL_H_
#define FILE_UTIL_H_

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
typedef ULONGLONG filetime_t;
#define FILETIME_FMT "%I64u"
#elif defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
typedef time_t filetime_t;
#define FILETIME_FMT "%lu"
#else // !(defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)))

#error I need Windows or Unix!

#endif // _WIN32


#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

filetime_t get_file_modified_time(const char *path);
void sys_sleep(int seconds);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* FILE_UTIL_H_ */
