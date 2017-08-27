#ifndef RETAINS_VARS_H_
#define RETAINS_VARS_H_
#include <gctypes.h>
#include "patcher/fs_function_patcher.h"
#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/os_types.h"

#define ASYNC_RESULT_CACHE_SIZE     250
#define FS_QUEUE_MESSAGE_COUNT      50

extern u8 gUsingLibIOSUHAX;
extern u8 gAppStatus;
extern volatile u8 gSDInitDone;

extern char gModFolder[FS_MAX_ENTNAME_SIZE];

extern OSMessageQueue gFSQueue __attribute__((section(".data")));
extern OSMessage gFSQueueMessages[FS_QUEUE_MESSAGE_COUNT] __attribute__((section(".data")));

extern struct fs_patcher_utils fspatchervars __attribute__((section(".data")));
extern FSAsyncResult gAsyncResultCache[ASYNC_RESULT_CACHE_SIZE];

extern u8 gAsyncResultCacheLock;
extern u8 gAsyncResultCacheCur;

#endif // RETAINS_VARS_H_
