/*
Created 28 December 2021
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stddef.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#define PATH_SEPARATOR "/"

typedef const char *Cstr;

#ifndef SIZE_T_MODIFIER
# ifdef _WIN32
#  define SIZE_T_MODIFIER "I"
# else 
#  define SIZE_T_MODIFIER "z"
# endif // _WIN32
#endif // !defined(SIZE_T_MODIFIER)

# if defined(__GNUC__) || defined(__clang__)

#  ifdef _WIN32
#   define PRINTF_FORMAT(fmt_idx, chk_idx) __attribute__ ((format (ms_printf, fmt_idx, chk_idx)))
#  else
#   define PRINTF_FORMAT(fmt_idx, chk_idx) __attribute__ ((format (printf, fmt_idx, chk_idx)))
#  endif // _WIN32

# else /* defined(__GNUC__) || defined(__clang__) */
#  define PRINTF_FORMAT(...)
# endif /* defined(__GNUC__) || defined(__clang__) */

# ifdef _DEBUG
#  define NOTNULL(ptr) notnull_debug(__FILE__, __LINE__, __func__, #ptr, ptr)
#  define PANIC(...) panic_debug(__FILE__, __LINE__, __func__, __VA_ARGS__)
# else 
#  define NOTNULL(ptr) notnull_impl(ptr)
#  define PANIC(...) panic_impl(__VA_ARGS__)
#endif

void panic_debug(Cstr file, size_t line, Cstr func, Cstr message, ...) PRINTF_FORMAT(4, 5);
void panic_impl(Cstr message, ...) PRINTF_FORMAT(1, 2);

void *notnull_debug(Cstr file, size_t line, Cstr func, Cstr var_name, void *ptr);
void *notnull_impl(void *ptr);

Cstr shift_arg(int *argc, Cstr *argv[]);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UTIL_H_ */
