#include <malloc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <fat.h>
#include <iosuhax.h>
#include <iosuhax_devoptab.h>
#include <iosuhax_disc_interface.h>
#include <utils/logger.h>
#include <system/exception_handler.h>
#include <utils/StringTools.h>
#include <fs/sd_fat_devoptab.h>
#include <fswrapper/FileReplacerUtils.h>
#include <fswrapper/fs_default_os_wrapper.h>
#include <dynamic_libs/fs_defs.h>

#include "myfs/fs_utils.h"
#include "../main.h"
#include "fs_function_patcher.h"
#include "../modpackSelector.h"
#include "common/retain_vars.h"
#include "common/common.h"


void deInitApplication(){
    //FileReplacerUtils::getInstance()->StopAsyncThread();
    FileReplacerUtils::destroyInstance();
    deInit_SD_USB();
}

DECL(void, __PPCExit, void){
    DEBUG_FUNCTION_LINE("__PPCExit\n");

    deInitApplication();

    real___PPCExit();
}

DECL(u32, ProcUIProcessMessages, u32 u){
    u32 res = real_ProcUIProcessMessages(u);
    if(res != gAppStatus){
        DEBUG_FUNCTION_LINE("App status changed from %d to %d \n",gAppStatus,res);
        gAppStatus = res;
    }

    return res;
}

DECL(int, FSCloseFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE){ return real_FSCloseFile(pClient, pCmd, fd, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSCloseFile(fd)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSCloseFile(pClient, pCmd, fd, setErrorFlag(error));
}

DECL(int, FSGetPosFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE){ return real_FSGetPosFile(pClient, pCmd, fd, pos, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSGetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
        return result;
    }

	return real_FSGetPosFile(pClient, pCmd, fd, pos, setErrorFlag(error));
}

DECL(int, FSGetStat, FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE){ return real_FSGetStat(pClient, pCmd, path, stats, error); }

    int result = USE_OS_FS_FUNCTION;

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("for path %s\n",path); }
    char * newPath = getPathWithNewBase(path,gModFolder);
    if(newPath != NULL){
        if((result = fs_wrapper_FSGetStat(newPath,stats)) != USE_OS_FS_FUNCTION){
            if(newPath){ free(newPath); newPath = NULL;}
            return result;
        }
        if(newPath){ free(newPath); newPath = NULL;} //Should be dead code...
    }

    return real_FSGetStat(pClient, pCmd, path, stats, setErrorFlag(error));
}

DECL(int, FSGetStatFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, FSStat * stats, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE){ return real_FSGetStatFile(pClient, pCmd, fd, stats, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSGetStatFile(fd,stats)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSGetStatFile(pClient, pCmd, fd, stats, setErrorFlag(error));
}

DECL(int, FSIsEof, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE) return real_FSIsEof(pClient, pCmd, fd, error);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSIsEof(fd)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSIsEof(pClient, pCmd, fd, setErrorFlag(error));
}

DECL(int, FSOpenFile, FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE){ return real_FSOpenFile(pClient, pCmd, path, mode, handle, error); }

    /*if(endsWith(path,BOOT_TV_TEX_TGA,-1,-1)){ //Mario Party 10 crashes when pressing the home button.
        if(startsWith("/vol/storage_mlc01/usr/title/",path)){
            u64 tID = getTitleIDFromPath(path);
            HandleMultiModPacks(tID,false);
        }else if(strlen(gLastMetaPath) > 0){
            DEBUG_FUNCTION_LINE("gLastMetaPath %s\n",gLastMetaPath);
            if(startsWith("/vol/storage_usb01/usr/title/",gLastMetaPath) ||
               startsWith("/vol/storage_mlc01/usr/title/",gLastMetaPath)
               ){
                u64 tID = getTitleIDFromPath(gLastMetaPath);
                HandleMultiModPacks(tID,false);
            }
        }
    }*/

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("for path %s\n",path); }
    char * newPath = getPathWithNewBase(path,gModFolder);
    int result = USE_OS_FS_FUNCTION;

    if(newPath != NULL){
        if((result = fs_wrapper_FSOpenFile(newPath,mode,handle)) != USE_OS_FS_FUNCTION){
        if(newPath){ free(newPath); newPath = NULL;}
        return result;
        }
        if(newPath){ free(newPath); newPath = NULL;} //Should be dead code...
    }

    return real_FSOpenFile(pClient, pCmd, path, mode, handle, setErrorFlag(error));
}

DECL(int, FSReadFile, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int handle, int flag, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE){ return real_FSReadFile(pClient, pCmd, buffer, size, count, handle, flag, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSReadFile(handle,buffer,size,count)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSReadFile(pClient, pCmd, buffer, size, count, handle, flag, setErrorFlag(error));
}

DECL(int, FSSetPosFile, FSClient *pClient, FSCmdBlock *pCmd, int fd, u32 pos, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE) return real_FSSetPosFile(pClient, pCmd, fd, pos, error);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSSetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
        return result;
    }

	return real_FSSetPosFile(pClient, pCmd, fd, pos, setErrorFlag(error));
}

DECL(int, FSReadFileWithPos, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int fd, int flag, int error) {
    if(gAppStatus == 2 || gSDInitDone <= SDUSB_MOUNTED_FAKE){ return real_FSReadFileWithPos(pClient, pCmd, buffer, size, count, pos, fd, flag, error); }

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSReadFileWithPos(buffer,size,count,pos,fd)) != USE_OS_FS_FUNCTION){
        return result;
    }

    return real_FSReadFileWithPos(pClient, pCmd, buffer, size, count, pos, fd, flag, setErrorFlag(error));
}


/**
In theory it would be enough just to patch the "async" versions of the function.
The non-async functions internally use the async function anyway.
However this my be a bit faster/robust, when we handle the async functions async.
**/

DECL(int, FSCloseFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSCloseFileAsync(pClient, pCmd, fd, error, asyncParams);
    }

    return fs_default_os_wrapper_FSCloseFileAsync(pClient, pCmd, fd, error, asyncParams,(void*) real_FSCloseFileAsync);
}

DECL(int, FSGetPosFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSGetPosFileAsync(pClient, pCmd, fd, pos, error, asyncParams);
    }

    return fs_default_os_wrapper_FSGetPosFileAsync(pClient, pCmd, fd, pos, error, asyncParams,(void*) real_FSGetPosFileAsync);
}

DECL(int, FSGetStatAsync, FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSGetStatAsync(pClient, pCmd, path, stats, error, asyncParams);
    }

    return fs_default_os_wrapper_FSGetStatAsync(getPathWithNewBase((char*)path,gModFolder),pClient, pCmd, path, stats, error, asyncParams,(void*) real_FSGetStatAsync);
}

DECL(int, FSGetStatFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, FSStat * stats, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSGetStatFileAsync(pClient, pCmd, fd, stats, error, asyncParams);
    }

    return fs_default_os_wrapper_FSGetStatFileAsync(pClient, pCmd, fd, stats, error, asyncParams,(void*) real_FSGetStatFileAsync);
}

DECL(int, FSIsEofAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSIsEofAsync(pClient, pCmd, fd, error,asyncParams);
    }

    return fs_default_os_wrapper_FSIsEofAsync(pClient, pCmd, fd, error,asyncParams,(void*) real_FSIsEofAsync);
}
/*
u64 getTitleIDFromPath(const char * path){
    if(path == NULL || strlen(path) < 46) return 0;
    char titleID[0x11];
    char titleIDHigh[0x09];
    char titleIDLow[0x09];
    char * test = (char * )&path[29];
    snprintf(titleIDHigh,0x09,"%s",test);
    test = (char * ) &path[38];
    snprintf(titleIDLow,0x09,"%s",test);
    snprintf(titleID,0x11,"%s%s",titleIDHigh,titleIDLow);
    u64 tID = strtoll(titleID, NULL, 16);
    tID &= ~ 0x0000000E00000000; // remove update flag
    return tID;
}*/

DECL(int, FSOpenFileAsync, FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle,error, asyncParams);
    }

    return fs_default_os_wrapper_FSOpenFileAsync(getPathWithNewBase((char*)path,gModFolder), pClient, pCmd, path, mode, handle,error, asyncParams,(void*) real_FSOpenFileAsync);
}

DECL(int, FSReadFileAsync, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int fd, int flag, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSReadFileAsync(pClient, pCmd, buffer, size, count, fd, flag, error, asyncParams);
    }

    return fs_default_os_wrapper_FSReadFileAsync(pClient, pCmd, buffer, size, count, fd, flag, error, asyncParams, (void*) real_FSReadFileAsync);
}

DECL(int, FSReadFileWithPosAsync, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int fd, int flag, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, count, pos, fd, flag, error, asyncParams);
    }

    return fs_default_os_wrapper_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, count, pos, fd, flag, error, asyncParams, (void*) real_FSReadFileWithPosAsync);
}

DECL(int, FSSetPosFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int handle, u32 pos, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2 || checkErrorFlag(&error) || gSDInitDone <= SDUSB_MOUNTED_FAKE){ // Use the real implementation if the app is not in foreground or we already checked it.
        return real_FSSetPosFileAsync(pClient, pCmd, handle, pos, error,asyncParams);
    }

    return fs_default_os_wrapper_FSSetPosFileAsync(pClient, pCmd, handle, pos, error,asyncParams, (void*) real_FSSetPosFileAsync);
}

/*
DECL(int, FSBindMount, void *pClient, void *pCmd, char *source, char *target, int error){
    if(gAppStatus == 2) return real_FSBindMount(pClient,pCmd,source,target,error);
    memcpy(gLastMetaPath,source,strlen(source) + 1);
    return real_FSBindMount(pClient,pCmd,source,target,error);
}

DECL(int, FSBindUnmount, void *pClient, void *pCmd, char *target, int error){
    if(gAppStatus == 2) real_FSBindUnmount(pClient,pCmd,target,error);
    gLastMetaPath[0] = 0;
    return real_FSBindUnmount(pClient,pCmd,target,error);
}*/

hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSCloseFile,                 LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetPosFile,                LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStat,                   LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatFile,               LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSIsEof,                     LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFile,                  LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFile,                  LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileWithPos,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSSetPosFile,                LIB_CORE_INIT,  STATIC_FUNCTION),

    MAKE_MAGIC(FSCloseFileAsync,            LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetPosFileAsync,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatAsync,              LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatFileAsync,          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSIsEofAsync,                LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFileAsync,             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileAsync,             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileWithPosAsync,      LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSSetPosFileAsync,           LIB_CORE_INIT,  STATIC_FUNCTION),

    //MAKE_MAGIC(FSBindMount,                 LIB_CORE_INIT,  STATIC_FUNCTION),
    //MAKE_MAGIC(FSBindUnmount,               LIB_CORE_INIT,  STATIC_FUNCTION),

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

