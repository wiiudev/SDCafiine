#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "common/common.h"
#include "common/fs_defs.h"
#include "common/loader_defs.h"
#include "dynamic_libs/aoc_functions.h"
#include "dynamic_libs/ax_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/padscore_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/acp_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "kernel/kernel_functions.h"
#include "system/exception_handler.h"
#include "function_hooks.h"
#include "fs/fs_utils.h"
#include "utils/logger.h"
#include "system/memory.h"
#include "common/retain_vars.h"


#define LIB_CODE_RW_BASE_OFFSET                         0xC1000000
#define CODE_RW_BASE_OFFSET                             0x00000000
#define DEBUG_LOG_DYN                                   0

#define USE_EXTRA_LOG_FUNCTIONS   0


#define DEBUG_LOG 0

#define DECL(res, name, ...) \
        res (* real_ ## name)(__VA_ARGS__) __attribute__((section(".data"))); \
        res my_ ## name(__VA_ARGS__)




static int client_num_alloc(void *pClient) {
    int i;

    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == 0) {
            bss.pClient_fs[i] = (int)pClient;
            return i;
        }
    return -1;
}
static void client_num_free(int client) {
    bss.pClient_fs[client] = 0;
    bss.sd_mount[client] = 0;
}
static int client_num(void *pClient) {
    int i;
    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == (int)pClient)
            return i;
    return -1;
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

static int is_gamefile(const char *path) {
    if(strncmp(path,"/vol/content",12) == 0) return 1;

    return 0;
}

DECL(int, FSInit, void) {
    if(DEBUG_LOG) log_print("FSInit\n");
    if ((int)bss_ptr == 0x0a000000)
    {
        gReadIPfromSD = 0;
         // allocate memory for our stuff
        void *mem_ptr = memalign(0x40, sizeof(struct bss_t));
        if(!mem_ptr)
            return real_FSInit();
        // copy pointer
        bss_ptr = mem_ptr;
        memset(bss_ptr, 0, sizeof(struct bss_t));

        // setup exceptions, has to be done once per core
        setup_os_exceptions();


        sprintf(bss.mount_base,"%s/%016llX",CAFE_OS_SD_PATH,OSGetTitleID());
        if(DEBUG_LOG) log_printf("bss.mount_base: %s\n",bss.mount_base);
    }
    return real_FSInit();
}

DECL(int, FSShutdown, void) {
    if(DEBUG_LOG) log_print("FSShutdown\n");
    return real_FSShutdown();
}

static void readIPfromSD(void *pClient, void *pCmd){
    if(!gReadIPfromSD){
        log_print("Trying to read IP from SD\n");
        gReadIPfromSD = 1;
        FSStat stats;
        char path[250];
        sprintf(path,"%s%s/ip.txt",CAFE_OS_SD_PATH,WIIU_PATH);
        int status = -1;
        int handle;
        if((status = FSGetStat(pClient,pCmd,path,&stats,-1)) == 0){
            if(stats.size > 15){
                return;
            }
            char * file  = (char *) malloc((sizeof(char)*stats.size)+1);
            if(!file){
                log_print("Failed to allocate space for reading the file\n");
                return;
            }
            file[stats.size] = '\0';
            if((status = FSOpenFile(pClient,pCmd,path,"r",&handle,-1)) == 0){
                int total_read = 0;
                int ret2 = 0;
                while ((ret2 = FSReadFile(pClient,  pCmd, file+total_read, 1, stats.size-total_read, handle, 0, -1)) > 0){
                    total_read += ret2;
                }
            }else{
                log_printf("(FSOpenFile) Couldn't open file (%s), error: %d",path,status);
                free(file);
                file=NULL;
                return;
            }
            FSCloseFile(pClient,pCmd,handle,-1);
            log_init(file);
            log_printf("Now trying to use the IP: %s\n",file);
            free(file);
            file = NULL;
        }else{
            log_printf("File not found %s\n",path);
        }
    }
}

static int getClientAndInitSD(void *pClient, void *pCmd){
    if(DEBUG_LOG) log_print("getClientAndInitSD\n");
    if ((int)bss_ptr != 0x0A000000) {
        int client = client_num(pClient);
        if (client < MAX_CLIENT && client >= 0) {
            if(DEBUG_LOG) log_printf("found client: %d\n",client);
            if (!bss.sd_mount[client]){
                if(DEBUG_LOG) log_printf("SD is not mounted for this client\n");
                if(MountFS(pClient, pCmd,0) == 0){
                    log_printf("SD mounting was successful\n");
                    readIPfromSD(pClient, pCmd);

                    bss.sd_mount[client] = 1;
                }else{
                    bss.sd_mount[client] = 2;
                    log_printf("SD mounting failed\n");
                }
            }
            return client;
        }
    }
    return -1;
}



char * getNewPath(void *pClient, void *pCmd, const char *path);

// Async
typedef void (*FSAsyncCallback)(void *pClient, void *pCmd, int result, void *context);
typedef struct
{
    FSAsyncCallback userCallback;
    void            *userContext;
    void            *ioMsgQueue;
} FSAsyncParams;

DECL(int, FSGetStat, void *pClient, void *pCmd, const char *path, void *stats, int error) {
    if(DEBUG_LOG) log_print("FSGetStat\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSGetStat(pClient, pCmd, newPath, stats, error);
    free(newPath); newPath = NULL;
    return res;
}

char * getNewPath(void *pClient, void *pCmd, const char *path){
    if(DEBUG_LOG) log_print("getNewPath\n");
    char * newPath;
    int useOld = 1;

    if ((is_gamefile(path)) && (int)bss_ptr != 0x0A000000) {
        newPath = malloc(sizeof(char) * (strlen(path) + strlen(bss.mount_base) + 1));
        if(newPath == NULL){
            if(DEBUG_LOG) log_print("malloc failed\n");
        }
        int client = -1;

        if((client = getClientAndInitSD(pClient,pCmd)) != -1){
            sprintf(newPath,"%s%s",bss.mount_base,(path+12));

            int tmp_stats[25];
            if (real_FSGetStat(pClient, pCmd, newPath, &tmp_stats, -1) == 0) {
                log_printf("new path -> %s\n",newPath);
                useOld = 0;
            }else{
                log_printf("    path -> %s\n",newPath);
            }
        }

    }else{
         newPath = malloc(sizeof(char) * (strlen(path) + 1));
    }
    if(useOld){
            sprintf(newPath,"%s",path);
    }
    return newPath;
}

DECL(int, FSGetStatAsync, void *pClient, void *pCmd, const char *path, void *stats, int error, FSAsyncParams *asyncParams) {
    if(DEBUG_LOG) log_print("FSGetStatAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSGetStatAsync(pClient, pCmd, newPath, stats, error, asyncParams);
    free(newPath); newPath = NULL;
    return res;
}

DECL(int, FSOpenFile, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error) {
    if(DEBUG_LOG) log_print("FSOpenFile\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSOpenFile(pClient, pCmd, newPath, mode, handle, error);
    free(newPath); newPath = NULL;
    return res;
}

DECL(int, FSOpenFileAsync, void *pClient, void *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams) {
    if(DEBUG_LOG) log_print("FSOpenFileAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSOpenFileAsync(pClient, pCmd, newPath, mode, handle, error, asyncParams);
    free(newPath); newPath = NULL;
    return res;
}


DECL(int, FSOpenDir, void *pClient, void* pCmd, const char *path, int *handle, int error) {
    if(DEBUG_LOG) log_print("FSOpenDir\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSOpenDir(pClient, pCmd, newPath, handle, error);
    free(newPath); newPath = NULL;
    return res;
}

DECL(int, FSOpenDirAsync, void *pClient, void* pCmd, const char *path, int *handle, int error, FSAsyncParams *asyncParams) {
    if(DEBUG_LOG) log_print("FSOpenDirAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSOpenDirAsync(pClient, pCmd, newPath, handle, error, asyncParams);
    free(newPath); newPath = NULL;
    return res;
}

DECL(int, FSChangeDir, void *pClient, void *pCmd, const char *path, int error) {
    if(DEBUG_LOG) log_print("FSChangeDir\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSChangeDir(pClient, pCmd, newPath, error);
    free(newPath); newPath = NULL;
    return res;
}

DECL(int, FSChangeDirAsync, void *pClient, void *pCmd, const char *path, int error, FSAsyncParams *asyncParams) {
    if(DEBUG_LOG) log_print("FSChangeDirAsync\n");
    char * newPath = getNewPath(pClient,pCmd,path);
    int res = real_FSChangeDirAsync(pClient, pCmd, newPath, error, asyncParams);
    free(newPath); newPath = NULL;
    return res;
}

/* *****************************************************************************
 * Creates function pointer array
 * ****************************************************************************/
#define MAKE_MAGIC(x, lib,functionType) { (unsigned int) my_ ## x, (unsigned int) &real_ ## x, lib, # x,0,0,functionType,0}

static struct hooks_magic_t {
    const unsigned int replaceAddr;
    const unsigned int replaceCall;
    const unsigned int library;
    const char functionName[50];
    unsigned int realAddr;
    unsigned int restoreInstruction;
    unsigned char functionType;
    unsigned char alreadyPatched;
} method_hooks[] = {
     // Common FS functions
    MAKE_MAGIC(FSInit,              LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSShutdown,          LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSAddClientEx,       LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSDelClient,         LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStat,           LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSGetStatAsync,      LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFile,          LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenFileAsync,     LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenDir,           LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSOpenDirAsync,      LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSChangeDir,         LIB_FS, STATIC_FUNCTION),
    MAKE_MAGIC(FSChangeDirAsync,    LIB_FS, STATIC_FUNCTION),

};

//! buffer to store our 7 instructions needed for our replacements
//! the code will be placed in the address of that buffer - CODE_RW_BASE_OFFSET
//! avoid this buffer to be placed in BSS and reset on start up
volatile unsigned int dynamic_method_calls[sizeof(method_hooks) / sizeof(struct hooks_magic_t) * 7] __attribute__((section(".data")));

/*
*Patches a function that is loaded at the start of each application. Its not required to restore, at least when they are really dynamic.
* "normal" functions should be patch with the normal patcher.
*/
void PatchMethodHooks(void)
{
    /* Patch branches to it.  */
    volatile unsigned int *space = &dynamic_method_calls[0];

    int method_hooks_count = sizeof(method_hooks) / sizeof(struct hooks_magic_t);

    u32 skip_instr = 1;
    u32 my_instr_len = 6;
    u32 instr_len = my_instr_len + skip_instr;
    u32 flush_len = 4*instr_len;
    for(int i = 0; i < method_hooks_count; i++)
    {
        if(method_hooks[i].functionType == STATIC_FUNCTION && method_hooks[i].alreadyPatched == 1){
            if(isDynamicFunction((u32)OSEffectiveToPhysical((void*)method_hooks[i].realAddr))){
                if(DEBUG_LOG) log_printf("The function %s is a dynamic function. Please fix that <3\n", method_hooks[i].functionName);
                method_hooks[i].functionType = DYNAMIC_FUNCTION;
            }else{
                if(DEBUG_LOG) log_printf("Skipping %s, its already patched\n", method_hooks[i].functionName);
                space += instr_len;
                continue;
            }
        }

        u32 physical = 0;
        unsigned int repl_addr = (unsigned int)method_hooks[i].replaceAddr;
        unsigned int call_addr = (unsigned int)method_hooks[i].replaceCall;

        unsigned int real_addr = GetAddressOfFunction(method_hooks[i].functionName,method_hooks[i].library);

        if(!real_addr){
            if(DEBUG_LOG) log_printf("OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
            space += instr_len;
            continue;
        }

        if(DEBUG_LOG_DYN)if(DEBUG_LOG) log_printf("%s is located at %08X!\n", method_hooks[i].functionName,real_addr);

        physical = (u32)OSEffectiveToPhysical((void*)real_addr);
        if(!physical){
             if(DEBUG_LOG) log_printf("Something is wrong with the physical address\n");
             space += instr_len;
             continue;
        }

        if(DEBUG_LOG_DYN)if(DEBUG_LOG) log_printf("%s physical is located at %08X!\n", method_hooks[i].functionName,physical);

        bat_table_t my_dbat_table;
        if(DEBUG_LOG_DYN)if(DEBUG_LOG) log_printf("Setting up DBAT\n");
        KernelSetDBATsForDynamicFuction(&my_dbat_table,physical);

        //if(DEBUG_LOG) log_printf("Setting call_addr to %08X\n",(unsigned int)(space) - CODE_RW_BASE_OFFSET);
        *(volatile unsigned int *)(call_addr) = (unsigned int)(space) - CODE_RW_BASE_OFFSET;

        // copy instructions from real function.
        u32 offset_ptr = 0;
        for(offset_ptr = 0;offset_ptr<skip_instr*4;offset_ptr +=4){
             if(DEBUG_LOG_DYN)if(DEBUG_LOG) log_printf("(real_)%08X = %08X\n",space,*(volatile unsigned int*)(physical+offset_ptr));
            *space = *(volatile unsigned int*)(physical+offset_ptr);
            space++;
        }

        //Only works if skip_instr == 1
        if(skip_instr == 1){
            // fill the restore instruction section
            method_hooks[i].realAddr = real_addr;
            method_hooks[i].restoreInstruction = *(volatile unsigned int*)(physical);
        }else{
            if(DEBUG_LOG) log_printf("Can't save %s for restoring!\n", method_hooks[i].functionName);
        }

        //adding jump to real function
        /*
            90 61 ff e0     stw     r3,-32(r1)
            3c 60 12 34     lis     r3,4660
            60 63 56 78     ori     r3,r3,22136
            7c 69 03 a6     mtctr   r3
            80 61 ff e0     lwz     r3,-32(r1)
            4e 80 04 20     bctr*/
        *space = 0x9061FFE0;
        space++;
        *space = 0x3C600000 | (((real_addr + (skip_instr * 4)) >> 16) & 0x0000FFFF); // lis r3, real_addr@h
        space++;
        *space = 0x60630000 |  ((real_addr + (skip_instr * 4)) & 0x0000ffff); // ori r3, r3, real_addr@l
        space++;
        *space = 0x7C6903A6; // mtctr   r3
        space++;
        *space = 0x8061FFE0; // lwz     r3,-32(r1)
        space++;
        *space = 0x4E800420; // bctr
        space++;
        DCFlushRange((void*)(space - instr_len), flush_len);
        ICInvalidateRange((unsigned char*)(space - instr_len), flush_len);

        //setting jump back
        unsigned int replace_instr = 0x48000002 | (repl_addr & 0x03fffffc);
        *(volatile unsigned int *)(physical) = replace_instr;
        ICInvalidateRange((void*)(real_addr), 4);

        //restore my dbat stuff
        KernelRestoreDBATs(&my_dbat_table);

        method_hooks[i].alreadyPatched = 1;
    }
    if(DEBUG_LOG) log_print("Done with patching all functions!\n");
}

/* ****************************************************************** */
/*                  RESTORE ORIGINAL INSTRUCTIONS                     */
/* ****************************************************************** */
void RestoreInstructions(void)
{
    bat_table_t table;
    if(DEBUG_LOG) log_printf("Restore functions!\n");
    int method_hooks_count = sizeof(method_hooks) / sizeof(struct hooks_magic_t);
    for(int i = 0; i < method_hooks_count; i++)
    {
        if(method_hooks[i].restoreInstruction == 0 || method_hooks[i].realAddr == 0){
            if(DEBUG_LOG) log_printf("I dont have the information for the restore =( skip\n");
            continue;
        }

        unsigned int real_addr = GetAddressOfFunction(method_hooks[i].functionName,method_hooks[i].library);

        if(!real_addr){
            if(DEBUG_LOG) log_printf("OSDynLoad_FindExport failed for %s\n", method_hooks[i].functionName);
            continue;
        }

        u32 physical = (u32)OSEffectiveToPhysical((void*)real_addr);
        if(!physical){
            if(DEBUG_LOG) log_printf("Something is wrong with the physical address\n");
            continue;
        }

        if(isDynamicFunction(physical)){
             if(DEBUG_LOG) log_printf("Its a dynamic function. We don't need to restore it! %s\n",method_hooks[i].functionName);
        }else{
            KernelSetDBATs(&table);

            *(volatile unsigned int *)(LIB_CODE_RW_BASE_OFFSET + method_hooks[i].realAddr) = method_hooks[i].restoreInstruction;
            DCFlushRange((void*)(LIB_CODE_RW_BASE_OFFSET + method_hooks[i].realAddr), 4);
            ICInvalidateRange((void*)method_hooks[i].realAddr, 4);
            if(DEBUG_LOG) log_printf("Restored %s\n",method_hooks[i].functionName);
            KernelRestoreDBATs(&table);
        }
        method_hooks[i].alreadyPatched = 0; // In case a
    }
    KernelRestoreInstructions();
    if(DEBUG_LOG) log_print("Done with restoring all functions!\n");
}

int isDynamicFunction(unsigned int physicalAddress){
    if((physicalAddress & 0x80000000) == 0x80000000){
        return 1;
    }
    return 0;
}

unsigned int GetAddressOfFunction(const char * functionName,unsigned int library){
    unsigned int real_addr = 0;

    unsigned int rpl_handle = 0;
    if(library == LIB_CORE_INIT){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_CORE_INIT\n", functionName);
        if(coreinit_handle == 0){if(DEBUG_LOG) log_print("LIB_CORE_INIT not aquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_NSYSNET){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_NSYSNET\n", functionName);
        if(nsysnet_handle == 0){if(DEBUG_LOG) log_print("LIB_NSYSNET not aquired\n"); return 0;}
        rpl_handle = nsysnet_handle;
    }
    else if(library == LIB_GX2){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_GX2\n", functionName);
        if(gx2_handle == 0){if(DEBUG_LOG) log_print("LIB_GX2 not aquired\n"); return 0;}
        rpl_handle = gx2_handle;
    }
    else if(library == LIB_AOC){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_AOC\n", functionName);
        if(aoc_handle == 0){if(DEBUG_LOG) log_print("LIB_AOC not aquired\n"); return 0;}
        rpl_handle = aoc_handle;
    }
    else if(library == LIB_AX){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_AX\n", functionName);
        if(sound_handle == 0){if(DEBUG_LOG) log_print("LIB_AX not aquired\n"); return 0;}
        rpl_handle = sound_handle;
    }
    else if(library == LIB_FS){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_FS\n", functionName);
        if(coreinit_handle == 0){if(DEBUG_LOG) log_print("LIB_FS not aquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_OS){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_OS\n", functionName);
        if(coreinit_handle == 0){if(DEBUG_LOG) log_print("LIB_OS not aquired\n"); return 0;}
        rpl_handle = coreinit_handle;
    }
    else if(library == LIB_PADSCORE){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_PADSCORE\n", functionName);
        if(padscore_handle == 0){if(DEBUG_LOG) log_print("LIB_PADSCORE not aquired\n"); return 0;}
        rpl_handle = padscore_handle;
    }
    else if(library == LIB_SOCKET){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_SOCKET\n", functionName);
        if(nsysnet_handle == 0){if(DEBUG_LOG) log_print("LIB_SOCKET not aquired\n"); return 0;}
        rpl_handle = nsysnet_handle;
    }
    else if(library == LIB_SYS){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_SYS\n", functionName);
        if(sysapp_handle == 0){if(DEBUG_LOG) log_print("LIB_SYS not aquired\n"); return 0;}
        rpl_handle = sysapp_handle;
    }
    else if(library == LIB_VPAD){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_VPAD\n", functionName);
        if(vpad_handle == 0){if(DEBUG_LOG) log_print("LIB_VPAD not aquired\n"); return 0;}
        rpl_handle = vpad_handle;
    }
    else if(library == LIB_NN_ACP){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_NN_ACP\n", functionName);
        if(acp_handle == 0){if(DEBUG_LOG) log_print("LIB_NN_ACP not aquired\n"); return 0;}
        rpl_handle = acp_handle;
    }
    else if(library == LIB_SYSHID){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_SYSHID\n", functionName);
        if(syshid_handle == 0){if(DEBUG_LOG) log_print("LIB_SYSHID not aquired\n"); return 0;}
        rpl_handle = syshid_handle;
    }
    else if(library == LIB_VPADBASE){
        if(DEBUG_LOG) log_printf("FindExport of %s! From LIB_VPADBASE\n", functionName);
        if(vpadbase_handle == 0){if(DEBUG_LOG) log_print("LIB_VPADBASE not aquired\n"); return 0;}
        rpl_handle = vpadbase_handle;
    }

    if(!rpl_handle){
        if(DEBUG_LOG) log_printf("Failed to find the RPL handle for %s\n", functionName);
        return 0;
    }

    OSDynLoad_FindExport(rpl_handle, 0, functionName, &real_addr);

    if(!real_addr){
        if(DEBUG_LOG) log_printf("OSDynLoad_FindExport failed for %s\n", functionName);
        return 0;
    }

    if((u32)(*(volatile unsigned int*)(real_addr) & 0xFF000000) == 0x48000000){
        real_addr += (u32)(*(volatile unsigned int*)(real_addr) & 0x0000FFFF);
        if((u32)(*(volatile unsigned int*)(real_addr) & 0xFF000000) == 0x48000000){
            return 0;
        }
    }

    return real_addr;
}
