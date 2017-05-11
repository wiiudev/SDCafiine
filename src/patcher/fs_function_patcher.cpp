#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "fs_function_patcher.h"
#include "fs_function_utils.h"

#include "utils/logger.h"
#include "common/retain_vars.h"
#include "system/exception_handler.h"
#include "utils/FileReplacer.h"
#include "utils/StringTools.h"

DECL(int, FSInit, void) {
    if(DEBUG_LOG) log_print("FSInit\n");
    if ((int)bss_ptr == 0x0a000000) {
         // allocate memory for our stuff
        void *mem_ptr = memalign(0x40, sizeof(struct bss_t));
        if(!mem_ptr)
            return real_FSInit();
        // copy pointer
        bss_ptr = (bss_t*)mem_ptr;
        memset(bss_ptr, 0, sizeof(struct bss_t));

        // setup exceptions, has to be done once per core
        setup_os_exceptions();

        sprintf(bss.content_mount_base,"%s%s/%016llX/content",CAFE_OS_SD_PATH,GAME_MOD_FOLDER,OSGetTitleID());
        sprintf(bss.aoc_mount_base,"%s%s/%016llX/aoc",CAFE_OS_SD_PATH,GAME_MOD_FOLDER,OSGetTitleID());
        if(DEBUG_LOG) log_printf("bss.content_mount_base: %s\n",bss.content_mount_base);
        if(DEBUG_LOG) log_printf("bss.aoc_mount_base: %s\n",bss.aoc_mount_base);
    }
    return real_FSInit();
}

DECL(int, FSAddClientEx, void *r3, void *r4, void *r5) {
    int res = real_FSAddClientEx(r3, r4, r5);

    if ((int)bss_ptr != 0x0A000000 && res >= 0) {
        client_num_alloc(r3);
    }

    return res;
}

DECL(int, FSDelClient, void *pClient) {
    if ((int)bss_ptr != 0x0A000000) {
        int client = client_num(pClient);
        if (client < MAX_CLIENT && client >= 0) {
            client_num_free(client);
        }
    }
    return real_FSDelClient(pClient);
}

DECL(int, FSGetStatAsync, void *pClient, void *pCmd, const char *path, void *stats, int error, FSAsyncParams *asyncParams) {
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
    delete replacer;
    replacer = NULL;
    if((int)bss_ptr != 0x0A000000){
        free(bss_ptr);
    }
    gSDInitDone = 0;
    log_printf("__PPCExit\n");
    real___PPCExit();
}

hooks_magic_t method_hooks_fs[] __attribute__((section(".data"))) = {
    MAKE_MAGIC(FSInit,              LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSAddClientEx,       LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSDelClient,         LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatAsync,      LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFileAsync,     LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenDirAsync,      LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(FSChangeDirAsync,    LIB_CORE_INIT,  STATIC_FUNCTION),
    MAKE_MAGIC(__PPCExit,           LIB_CORE_INIT,  STATIC_FUNCTION),
};

u32 method_hooks_size_fs __attribute__((section(".data"))) = sizeof(method_hooks_fs) / sizeof(hooks_magic_t);

//! buffer to store our instructions needed for our replacements
volatile unsigned int method_calls_fs[sizeof(method_hooks_fs) / sizeof(hooks_magic_t) * FUNCTION_PATCHER_METHOD_STORE_SIZE] __attribute__((section(".data")));

