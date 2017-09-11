#ifndef __FS_ASYNC_WRAPPER_H_
#define __FS_ASYNC_WRAPPER_H_

#include "FileReplacerUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "fs_sync_wrapper.h"
#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/fs_functions.h"
#include <unistd.h>

int fs_os_wrapper_FSCloseFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams, void * realFunction);

int fs_os_wrapper_FSGetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams *asyncParams, void * realFunction);

int fs_os_wrapper_FSGetStatAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams *asyncParams, void * realFunction);

int fs_os_wrapper_FSGetStatFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, FSStat * stats, int error, FSAsyncParams * asyncParams, void * realFunction);

int fs_os_wrapper_FSIsEofAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, int error, FSAsyncParams *asyncParams, void * realFunction);

int fs_os_wrapper_FSOpenFileAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams, void * realFunction);int libfat_FSOpenFile(const char * path, const char * mode, int * handle);

int fs_os_wrapper_FSReadFileAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction);int libfat_FSReadFile(int handle,void *buffer,size_t size,size_t count);

int fs_os_wrapper_FSReadFileWithPosAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction);int libfat_FSReadFileWithPos(void *buffer, int size, int count, u32 pos, int handle);

int fs_os_wrapper_FSSetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, u32 pos, int error, FSAsyncParams *asyncParams, void * realFunction);

#ifdef __cplusplus
}
#endif

#endif // __FS_ASYNC_WRAPPER_H_
