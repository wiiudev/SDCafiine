#include <gctypes.h>
#include "retain_vars.h"

u8 gUsingLibIOSUHAX __attribute__((section(".data"))) = 0;
u8 gAppStatus __attribute__((section(".data"))) = 0;
volatile u8 gSDInitDone __attribute__((section(".data"))) = 0;

char gModFolder[FS_MAX_ENTNAME_SIZE] __attribute__((section(".data")));
//char gLastMetaPath[FS_MAX_ENTNAME_SIZE] __attribute__((section(".data")));

OSMessageQueue gFSQueue __attribute__((section(".data")));
OSMessage gFSQueueMessages[FS_QUEUE_MESSAGE_COUNT] __attribute__((section(".data")));

FSAsyncResult gAsyncResultCache[ASYNC_RESULT_CACHE_SIZE] __attribute__((section(".data")));
u8 gAsyncResultCacheLock __attribute__((section(".data"))) = 0;
u8 gAsyncResultCacheCur __attribute__((section(".data"))) = 0;

void * ntfs_mounts __attribute__((section(".data"))) = NULL;
int ntfs_mount_count __attribute__((section(".data"))) = 0;
