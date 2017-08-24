#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "fs_function_patcher.h"
#include "fs_function_utils.h"
#include <fat.h>
#include <iosuhax.h>
#include <iosuhax_devoptab.h>
#include <iosuhax_disc_interface.h>
#include "utils/logger.h"
#include "common/retain_vars.h"
#include "system/exception_handler.h"
#include "utils/FileReplacer.h"
#include "utils/StringTools.h"
#include "utils/mcpHook.h"

DECL(int, FSInit, void) {
    if(gAppStatus == 2) return real_FSInit();
    if(DEBUG_LOG) log_print("FSInit\n");

    // setup exceptions, has to be done once per core
    setup_os_exceptions();

    sprintf(fspatchervars.content_mount_base,"%s%s/%016llX%s/content",CAFE_OS_SD_PATH,GAME_MOD_FOLDER,OSGetTitleID(),selectedMultiModPackFolder);
    sprintf(fspatchervars.aoc_mount_base,"%s%s/%016llX%s/aoc",CAFE_OS_SD_PATH,GAME_MOD_FOLDER,OSGetTitleID(),selectedMultiModPackFolder);
    
    if(DEBUG_LOG) log_printf("bss.content_mount_base: %s\n",fspatchervars.content_mount_base);
    if(DEBUG_LOG) log_printf("bss.aoc_mount_base: %s\n",fspatchervars.aoc_mount_base);

    return real_FSInit();
}

DECL(int, FSAddClientEx, void *pClient, s32 unk_zero_param, s32 errHandling) {
    int res = real_FSAddClientEx(pClient, unk_zero_param, errHandling);
    if(gAppStatus == 2) return res;

    if (res >= 0) {
        client_num_alloc(pClient);
    }

    return res;
}

DECL(int, FSDelClient, void *pClient) {
    if(gAppStatus == 2) return real_FSDelClient(pClient);
    int client = client_num(pClient);
    if (client < MAX_CLIENT && client >= 0) {
        client_num_free(client);
    }
    return real_FSDelClient(pClient);
}

DECL(int, FSGetStatAsync, void *pClient, void *pCmd, const char *path, void *stats, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSGetStatAsync(pClient, pCmd, path, stats, error, asyncParams);
    if(DEBUG_LOG) log_printf("FSGetStatAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    if(DEBUG_LOG) log_printf("after getNewPath FSGetStatAsync\n");
    const char * use_path = path;
    if(newPath != NULL){
        use_path = newPath;
    }
    if(DEBUG_LOG) log_printf("before real_FSOpenFileAsync\n");
    int res = real_FSGetStatAsync(pClient, pCmd, use_path, stats, error, asyncParams);
    if(DEBUG_LOG) log_printf("after real_FSOpenFileAsync\n");
    if(newPath != NULL){free(newPath); newPath = NULL;}
    return res;
}

DECL(int, FSOpenFileAsync, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSOpenFileAsync(pClient, pCmd, path, mode, handle, error, asyncParams);
    if(DEBUG_LOG) log_printf("FSOpenFileAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    if(DEBUG_LOG) log_printf("after getNewPath FSOpenFileAsync\n");
    const char * use_path = path;
    if(newPath != NULL){
        use_path = newPath;
    }
    int res = real_FSOpenFileAsync(pClient, pCmd, use_path, mode, handle, error, asyncParams);
    if(newPath != NULL){free(newPath); newPath = NULL;}
    return res;
}

DECL(int, FSOpenDirAsync, void *pClient, void* pCmd, const char *path, int *handle, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSOpenDirAsync(pClient, pCmd, path, handle, error, asyncParams);
    if(DEBUG_LOG) log_printf("FSOpenDirAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    if(DEBUG_LOG) log_printf("after getNewPath FSOpenDirAsync\n");
    const char * use_path = path;
    if(newPath != NULL){
        use_path = newPath;
    }
    int res = real_FSOpenDirAsync(pClient, pCmd, use_path, handle, error, asyncParams);
    if(newPath != NULL){free(newPath); newPath = NULL;}
    return res;
}

DECL(int, FSChangeDirAsync, void *pClient, void *pCmd, const char *path, int error, FSAsyncParams *asyncParams) {
    if(gAppStatus == 2) return real_FSChangeDirAsync(pClient, pCmd, path, error, asyncParams);
    if(DEBUG_LOG) log_printf("FSChangeDirAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    if(DEBUG_LOG) log_printf("after getNewPath FSChangeDirAsync\n");
    const char * use_path = path;
    if(newPath != NULL){
        use_path = newPath;
    }
    int res = real_FSChangeDirAsync(pClient, pCmd, use_path, error, asyncParams);
    if(newPath != NULL){free(newPath); newPath = NULL;}
    return res;
}

DECL(void, __PPCExit, void){
    log_printf("__PPCExit\n");
    delete replacer;
    replacer = NULL;
    sprintf(selectedMultiModPackFolder, "/");
    gSDInitDone = 0;

    if(gUsingLibIOSUHAX != 0){
        fatUnmount("sd");
         if(gUsingLibIOSUHAX == 1){
            log_printf("close IOSUHAX_Close\n");
            IOSUHAX_Close();
        }else{
            log_printf("close MCPHookClose\n");
            MCPHookClose();
        }
    }else{
        unmount_sd_fat("sd");
        unmount_fake();
    }

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

hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSInit,                  LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSAddClientEx,           LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSDelClient,             LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatAsync,          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFileAsync,         LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenDirAsync,          LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSChangeDirAsync,        LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(__PPCExit,               LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(ProcUIProcessMessages,   LIB_PROC_UI,    DYNAMIC_FUNCTION),
};

u32 method_hooks_size_fs __attribute__((section(".data"))) = sizeof(method_hooks_fs) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile u32 method_calls_fs[sizeof(method_hooks_fs) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

