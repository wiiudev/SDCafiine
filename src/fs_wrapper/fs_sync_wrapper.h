#ifndef __FS_SYNC_WRAPPER_H_
#define __FS_SYNC_WRAPPER_H_

#include "FileReplacerUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/fs_functions.h"
#include <unistd.h>

int fs_wrapper_FSCloseFile(int handle);

int fs_wrapper_FSGetPosFile(int handle,int * pos);

int fs_wrapper_FSGetStat(const char * path, FSStat * stats);

int fs_wrapper_FSGetStatFile(int handle, FSStat * stats);

int fs_wrapper_FSIsEof(int handle);

int fs_wrapper_FSOpenFile(const char * path, const char * mode, int * handle);

int fs_wrapper_FSReadFile(int handle,void *buffer,size_t size,size_t count);

int fs_wrapper_FSReadFileWithPos(void *buffer, int size, int count, u32 pos, int handle);

int fs_wrapper_FSSetPosFile(int handle,u32 pos);

#ifdef __cplusplus
}
#endif

#endif // __FS_SYNC_WRAPPER_H_
