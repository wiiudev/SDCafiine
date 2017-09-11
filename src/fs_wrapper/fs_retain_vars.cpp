#include <gctypes.h>
#include "fs_retain_vars.h"

OSMessageQueue fsFSQueue __attribute__((section(".data")));
OSMessage fsFSQueueMessages[FS_QUEUE_MESSAGE_COUNT] __attribute__((section(".data")));

FSAsyncResult fsAsyncResultCache[ASYNC_RESULT_CACHE_SIZE] __attribute__((section(".data")));
u8 fsAsyncResultCacheLock __attribute__((section(".data"))) = 0;
u8 fsAsyncResultCacheCur __attribute__((section(".data"))) = 0;
