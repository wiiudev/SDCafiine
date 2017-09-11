#ifndef FS_RETAINS_VARS_H_
#define FS_RETAINS_VARS_H_
#include <gctypes.h>
#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/os_types.h"

#define ASYNC_RESULT_CACHE_SIZE     50
#define FS_QUEUE_MESSAGE_COUNT      5

extern OSMessageQueue fsFSQueue __attribute__((section(".data")));
extern OSMessage fsFSQueueMessages[FS_QUEUE_MESSAGE_COUNT] __attribute__((section(".data")));

extern FSAsyncResult fsAsyncResultCache[ASYNC_RESULT_CACHE_SIZE];

extern u8 fsAsyncResultCacheLock;
extern u8 fsAsyncResultCacheCur;

#endif // FS_RETAINS_VARS_H_
