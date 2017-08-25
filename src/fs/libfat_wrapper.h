#ifndef __LIBFAT_WRAPPER_H_
#define __LIBFAT_WRAPPER_H_

#include "utils/FileReplacerUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/fs_functions.h"
#include <unistd.h>


int libfat_FSCloseFile(int handle);

int libfat_FSGetPosFile(int handle,int * pos);

int libfat_FSGetStat(const char * path, FSStat * stats);

int libfat_FSGetStatFile(int handle, FSStat * stats);

int libfat_FSIsEof(int handle);

int libfat_FSOpenFile(const char * path, int * handle);

int libfat_FSReadFile(int handle,void *buffer,size_t size,size_t count);

int libfat_FSReadFileWithPos(int handle,u32 pos, void *buffer,size_t size,size_t count);

int libfat_FSSetPosFile(int handle,u32 pos);

#ifdef __cplusplus
}
#endif

#endif // __LIBFAT_WRAPPER_H_
