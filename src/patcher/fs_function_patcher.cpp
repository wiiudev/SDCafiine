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

FSAsyncResult * getNewAsyncParamPointer(){
    while(gAsyncResultCacheLock){
        os_usleep(100);
    }
    gAsyncResultCacheLock = 1;

    if(gAsyncResultCacheCur >= ASYNC_RESULT_CACHE_SIZE){
        gAsyncResultCacheCur = 0;
    }

    FSAsyncResult *result = &gAsyncResultCache[gAsyncResultCacheCur++];

    gAsyncResultCacheLock = 0;
    return result;
}

void sendAsyncCommand(FSClient * client, FSCmdBlock * cmd,FSAsyncParams* asyncParams,int status){
    if(asyncParams != NULL){
        if(asyncParams->userCallback != NULL){ //Using the userCallback
            if(DEBUG_LOG){log_printf("asyncParams->userCallback %08X\n",asyncParams);}
            asyncParams->userCallback(client,cmd,status,asyncParams->userContext);
            return;
        }else{
            FSAsyncResult * result = getNewAsyncParamPointer();
            FSMessage message;
            result->userParams.userCallback = asyncParams->userCallback;
            result->userParams.userContext = asyncParams->userContext;
            result->userParams.ioMsgQueue = asyncParams->ioMsgQueue;
            result->ioMsg.data = &result;
            result->ioMsg.unkwn1 = 0;
            result->ioMsg.unkwn2 = 0;
            result->ioMsg.unkwn3 = 0x08;
            result->client = client;
            result->block = cmd;
            result->result = status;

            message.data = (void*)result;
            message.unkwn1 = (u32)0;
            message.unkwn2 = (u32)0;
            message.unkwn3 = (u32)0x08;

            OSSendMessage(asyncParams->ioMsgQueue,(OSMessage*)&message,OS_MESSAGE_BLOCK);
        }
    }
}

DECL(void, __PPCExit, void){
    log_printf("__PPCExit\n");

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

DECL(int, FSCloseFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSCloseFileAsync(pClient, pCmd, fd, error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSCloseFile(fd)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSCloseFileAsync(pClient, pCmd, fd, error,asyncParams);
}


DECL(int, FSGetPosFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSGetPosFileAsync(pClient, pCmd, fd, pos, error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSGetPosFileAsync(pClient, pCmd, fd, pos, error,asyncParams);
}

DECL(int, FSGetStatAsync, FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSGetStatAsync(pClient, pCmd, path, stats, error, asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStat(path,stats)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

    return real_FSGetStatAsync(pClient, pCmd, path, stats, error, asyncParams);
}

DECL(int, FSGetStatFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, FSStat * stats, int error, FSAsyncParams * asyncParams) {
    if(gAppStatus == 2) return real_FSGetStatFileAsync(pClient, pCmd, fd, stats, error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStatFile(fd,stats)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSGetStatFileAsync(pClient, pCmd, fd, stats, error,asyncParams);
}

DECL(int, FSIsEofAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSIsEofAsync(pClient, pCmd, fd, error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSIsEof(fd)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSIsEofAsync(pClient, pCmd, fd, error,asyncParams);
}

DECL(int, FSOpenFileAsync, FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle,error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSOpenFile(path,handle)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle, error,asyncParams);
}

DECL(int, FSReadFileAsync, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int fd, int flag, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSReadFileAsync(pClient, pCmd, buffer, size, count, fd, flag, error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFile(fd,buffer,size,count)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSReadFileAsync(pClient, pCmd, buffer, size, count, fd, flag, error,asyncParams);
}

DECL(int, FSReadFileWithPosAsync, FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int fd, int flag, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, count, pos, fd, flag, error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFileWithPos(fd,pos,buffer,size,count)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSReadFileWithPosAsync(pClient, pCmd, buffer, size, count, pos, fd, flag, error,asyncParams);
}

DECL(int, FSSetPosFileAsync, FSClient *pClient, FSCmdBlock *pCmd, int fd, u32 pos, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSSetPosFileAsync(pClient, pCmd, fd, pos, error,asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSSetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
        sendAsyncCommand(pClient, pCmd,asyncParams,result);
        return FS_STATUS_OK;
    }

	return real_FSSetPosFileAsync(pClient, pCmd, fd, pos, error,asyncParams);
}

hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSCloseFileAsync,            LIB_CORE_INIT,  STATIC_FUNCTION),

    MAKE_MAGIC(FSGetPosFileAsync,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatAsync,              LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatFileAsync,          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSIsEofAsync,                LIB_CORE_INIT,  STATIC_FUNCTION),

    MAKE_MAGIC(FSOpenFileAsync,             LIB_CORE_INIT,  STATIC_FUNCTION),

    MAKE_MAGIC(FSReadFileAsync,             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSReadFileWithPosAsync,      LIB_CORE_INIT,  STATIC_FUNCTION),

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

