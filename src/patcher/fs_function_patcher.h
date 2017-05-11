#ifndef _FS_FUNCTION_PATCHER_H
#define _FS_FUNCTION_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utils/function_patcher.h"

/* Forward declarations */
#define MAX_CLIENT 32
#define DEBUG_LOG 0

struct bss_t {
    char content_mount_base[255];
    char aoc_mount_base[255];
    char replacer_path[255];
    int sd_mount[MAX_CLIENT];
    int pClient_fs[MAX_CLIENT];
};

#define bss_ptr (*(struct bss_t **)0x100000e4)
#define bss (*bss_ptr)

extern hooks_magic_t method_hooks_fs[];
extern u32 method_hooks_size_fs;
extern volatile unsigned int method_calls_fs[];

#ifdef __cplusplus
}
#endif

#endif /* _FS_FUNCTION_PATCHER_H */
