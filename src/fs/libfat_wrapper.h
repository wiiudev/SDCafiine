#ifndef __LIBFAT_WRAPPER_H_
#define __LIBFAT_WRAPPER_H_

#include "utils/FileReplacerUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/fs_functions.h"
#include <unistd.h>

int libfat_FSCloseFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSCloseFile(int handle);

int libfat_FSGetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSGetPosFile(int handle,int * pos);

int libfat_FSGetStatAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSGetStat(const char * path, FSStat * stats);

int libfat_FSGetStatFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, FSStat * stats, int error, FSAsyncParams * asyncParams, void * realFunction);
int libfat_FSGetStatFile(int handle, FSStat * stats);

int libfat_FSIsEofAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSIsEof(int handle);

int libfat_FSOpenFileAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSOpenFile(const char * path, const char * mode, int * handle);

int libfat_FSReadFileAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSReadFile(int handle,void *buffer,size_t size,size_t count);

int libfat_FSReadFileWithPosAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSReadFileWithPos(void *buffer, int size, int count, u32 pos, int handle);

int libfat_FSSetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, u32 pos, int error, FSAsyncParams *asyncParams, void * realFunction);
int libfat_FSSetPosFile(int handle,u32 pos);

#ifdef __cplusplus
}
#endif

#endif // __LIBFAT_WRAPPER_H_
