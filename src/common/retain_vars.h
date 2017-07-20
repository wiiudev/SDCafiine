#ifndef RETAINS_VARS_H_
#define RETAINS_VARS_H_
#include <gctypes.h>
#include "utils/FileReplacer.h"
#include "patcher/fs_function_patcher.h"
extern char ipFromSd[16];
extern u8 hasReadIP;
extern u8 gAppStatus;
extern volatile u8 gSDInitDone;
extern FileReplacer* replacer;
extern struct fs_patcher_utils fspatchervars __attribute__((section(".data")));

#endif // RETAINS_VARS_H_
