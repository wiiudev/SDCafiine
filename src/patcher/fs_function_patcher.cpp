#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "dynamic_libs/fs_defs.h"
#include "fs_function_patcher.h"
#include "fs_function_utils.h"

#include <fat.h>
#include <iosuhax.h>
#include <iosuhax_devoptab.h>
#include <iosuhax_disc_interface.h>
#include "utils/logger.h"
#include "utils/FileReplacerUtils.h"
#include "common/retain_vars.h"
#include "system/exception_handler.h"
#include "utils/StringTools.h"
#include "utils/mcpHook.h"
#include "fs/sd_fat_devoptab.h"
#include "fs/libfat_wrapper.h"

DECL(void, __PPCExit, void){
    log_printf("__PPCExit\n");

    FileReplacerUtils::getInstance()->StopAsyncThread();


    FileReplacerUtils::destroyInstance();

    gSDInitDone = 0;

    if(gUsingLibIOSUHAX != 0){
        fatUnmount("sd");
        fatUnmount("usb");
         if(gUsingLibIOSUHAX == 1){
            log_printf("close IOSUHAX_Close\n");
            IOSUHAX_Close();
        }else{
            log_printf("close MCPHookClose\n");
            MCPHookClose();
        }
    }else{
        unmount_sd_fat("sd");
    }
    unmount_fake();
    deleteDevTabsNames();

    real___PPCExit();
}

DECL(u32, ProcUIProcessMessages, u32 u){
    u32 res = real_ProcUIProcessMessages(u);
    if(res != gAppStatus){
        log_printf("App status changed from %d to %d \n",gAppStatus,res);
        gAppStatus = res;
    }

    return res;
}

#define CHECKED_WITH_ALL_ERRORS     0x10000
#define CHECKED_MASK                0x01000

/**
Returns a modified error flag.
This will be used to save the information if a file/handle was already
tried to be patched.
The non-async function internally call the async functions, and this way
we avoid testing it twice.
If the result contains our mask, we just straight to the OS functions.
**/
int setErrorFlag(int error){
    int result = error;
    if(error == -1){
        result = CHECKED_WITH_ALL_ERRORS;
    }else{
        result |= CHECKED_MASK;
    }
    return result;
}

/**
Check if we already checked the file/handle.
Returns true if it was already checked (+ revert the error)
Return false if it should be (tried) to be patched.
**/
int checkErrorFlag(int * error){
    if(*error == CHECKED_WITH_ALL_ERRORS){
        *error = -1;
        return true;
    }else if ((*error & CHECKED_MASK) == CHECKED_MASK){
        *error &= ~CHECKED_MASK;
        return true;
    }
    return false;
}

/**
In theory it would be enough just to patch the "Async" versions of the function.
The non-async functions internally use the async function anyway.
However this my be a bit faster/robust, when we handle th async functions async.
**/

DECL(int, FSCloseFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error) {
    if(gAppStatus == 2){ return real_FSCloseFile(pClient, pCmd, fd, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSCloseFile(fd)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSCloseFile(pClient, pCmd, fd, setErrorFlag(error));
}

DECL(int, FSCloseFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSCloseFileAsync(pClient, pCmd, fd, error, asyncParams);
    }

    return libfat_FSCloseFileAsync(pClient, pCmd, fd, error, asyncParams,(void*)real_FSCloseFileAsync);
}

DECL(int, FSGetPosFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error) {
    if(gAppStatus == 2){ return real_FSGetPosFile(pClient, pCmd, fd, pos, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
        return result;
    }

	return real_FSGetPosFile(pClient, pCmd, fd, pos, setErrorFlag(error));
}

DECL(int, FSGetPosFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSGetPosFileAsync(pClient, pCmd, fd, pos, error, asyncParams);
    }

	return libfat_FSGetPosFileAsync(pClient, pCmd, fd, pos, error, asyncParams,(void*) real_FSGetPosFileAsync);
}

DECL(int, FSGetStat, FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error) {
    if(gAppStatus == 2){ return real_FSGetStat(pClient, pCmd, path, stats, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStat(path,stats)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSGetStat(pClient, pCmd, path, stats, setErrorFlag(error));
}

DECL(int, FSGetStatAsync, FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSGetStatAsync(pClient, pCmd, path, stats, error, asyncParams);
    }

    return libfat_FSGetStatAsync(pClient, pCmd, path, stats, error, asyncParams,(void*) real_FSGetStatAsync);
}

DECL(int, FSGetStatFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, FSStat * stats, int error) {
    if(gAppStatus == 2){ return real_FSGetStatFile(pClient, pCmd, fd, stats, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStatFile(fd,stats)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSGetStatFile(pClient, pCmd, fd, stats, setErrorFlag(error));
}

DECL(int, FSGetStatFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, FSStat * stats, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSGetStatFileAsync(pClient, pCmd, fd, stats, error, asyncParams);
    }

	return libfat_FSGetStatFileAsync(pClient, pCmd, fd, stats, error, asyncParams,(void *) real_FSGetStatFileAsync);
}
DECL(int, FSIsEof, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error) {
    if(gAppStatus == 2) return real_FSIsEof(pClient, pCmd, fd, error);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSIsEof(fd)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSIsEof(pClient, pCmd, fd, setErrorFlag(error));
}

DECL(int, FSIsEofAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSIsEofAsync(pClient, pCmd, fd, error,asyncParams);
    }

	return libfat_FSIsEofAsync(pClient, pCmd, fd, error, asyncParams, (void *) real_FSIsEofAsync);
}

DECL(int, FSOpenFile, FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error) {
    if(gAppStatus == 2){ return real_FSOpenFile(pClient, pCmd, path, mode, handle, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSOpenFile(path,mode,handle)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSOpenFile(pClient, pCmd, path, mode, handle, setErrorFlag(error));
}

DECL(int, FSOpenFileAsync, FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle,error, asyncParams);
    }

    return libfat_FSOpenFileAsync(pClient, pCmd, path, mode, handle,error,asyncParams,(void*)real_FSOpenFileAsync);
}
DECL(int, FSReadFile, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int handle, int flag, int error) {
    if(gAppStatus == 2){ return real_FSReadFile(pClient, pCmd, buffer, size, count, handle, flag, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFile(handle,buffer,size,count)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSReadFile(pClient, pCmd, buffer, size, count, handle, flag, setErrorFlag(error));
}


DECL(int, FSReadFileAsync, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int fd, int flag, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSReadFileAsync(pClient, pCmd, buffer, size, count, fd, flag, error, asyncParams);
    }

    return libfat_FSReadFileAsync(pClient, pCmd, buffer, size, count, fd, flag, error, asyncParams, (void*)real_FSReadFileAsync);
}

DECL(int, FSReadFileWithPos, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int fd, int flag, int error) {
    if(gAppStatus == 2){ return real_FSReadFileWithPos(pClient, pCmd, buffer, size, count, pos, fd, flag, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFileWithPos(buffer,size,count,pos,fd)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSReadFileWithPos(pClient, pCmd, buffer, size, count, pos, fd, flag, setErrorFlag(error));
}

DECL(int, FSReadFileWithPosAsync, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int fd, int flag, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, count, pos, fd, flag, error, asyncParams);
    }

    return libfat_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, count, pos, fd, flag, error, asyncParams, (void*)real_FSReadFileWithPosAsync);
}

DECL(int, FSSetPosFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, u32 pos, int error) {
    if(gAppStatus == 2) return real_FSSetPosFile(pClient, pCmd, fd, pos, error);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSSetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
        return result;
    }

	return real_FSSetPosFile(pClient, pCmd, fd, pos, setErrorFlag(error));
}

DECL(int, FSSetPosFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int handle, u32 pos, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error)){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSSetPosFileAsync(pClient, pCmd, handle, pos, error,asyncParams);
    }

	return libfat_FSSetPosFileAsync(pClient, pCmd, handle, pos, error, asyncParams, (void*)real_FSSetPosFileAsync);
}

hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSCloseFile,                 LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSCloseFileAsync,            LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetPosFile,                LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetPosFileAsync,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStat,                   LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatAsync,              LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatFile,               LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatFileAsync,          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSIsEof,                     LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSIsEofAsync,                LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFile,                  LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFileAsync,             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFile,                  LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileAsync,             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileWithPos,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileWithPosAsync,      LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSSetPosFile,                LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSSetPosFileAsync,           LIB_CORE_INIT,  STATIC_FUNCTION),

    //MAKE_MAGIC(FSOpenDirAsync,              LIB_CORE_INIT,  STATIC_FUNCTION),
    //MAKE_MAGIC(FSReadDirAsync,              LIB_CORE_INIT,  STATIC_FUNCTION),
    //MAKE_MAGIC(FSRewindDirAsync,            LIB_CORE_INIT,  STATIC_FUNCTION),
    //MAKE_MAGIC(FSCloseDirAsync,             LIB_CORE_INIT,  STATIC_FUNCTION),

    //MAKE_MAGIC(FSGetAsyncResult,             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(__PPCExit,               LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(ProcUIProcessMessages,   LIB_PROC_UI,    DYNAMIC_FUNCTION),
};

u32 method_hooks_size_fs __attribute__((section(".data"))) = sizeof(method_hooks_fs) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile u32 method_calls_fs[sizeof(method_hooks_fs) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

