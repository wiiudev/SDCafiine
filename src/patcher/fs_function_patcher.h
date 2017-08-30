#ifndef _FS_FUNCTION_PATCHER_H
#define _FS_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

/* Forward declarations */
#define DEBUG_LOG 0

extern hooks_magic_t method_hooks_fs[];
extern u32 method_hooks_size_fs;
extern volatile u32 method_calls_fs[];

#ifdef __cplusplus
}
#endif

#endif /* _FS_FUNCTION_PATCHER_H */
