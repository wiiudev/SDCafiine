#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "main.h"
#include "common/common.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "patcher/fs_function_patcher.h"
#include "utils/function_patcher.h"
#include "kernel/kernel_functions.h"
#include "utils/logger.h"
#include "fs/fs_utils.h"
#include "common/retain_vars.h"

u8 isFirstBoot __attribute__((section(".data"))) = 1;

/* Entry point */
extern "C" int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! aquire every rpl we want to patch

    InitOSFunctionPointers();
    InitSocketFunctionPointers(); //For logging

    InitSysFunctionPointers(); // For SYSLaunchMenu()
    InitFSFunctionPointers();

    SetupKernelCallback();

    Init_Log();

    //Reset everything when were going back to the Mii Maker
    if(!isFirstBoot && isInMiiMakerHBL()){
        log_print("Returing to the Homebrew Launcher!\n");
        isFirstBoot = 0;
        deInit();
        return EXIT_SUCCESS;
    }

    //!*******************************************************************
    //!                        Patching functions                        *
    //!*******************************************************************
    log_print("Patching functions\n");
    ApplyPatches();

    if(!isInMiiMakerHBL()){ //Starting the application
        return EXIT_RELAUNCH_ON_LOAD;
    }

    if(isFirstBoot){ // First boot back to SysMenu
        isFirstBoot = 0;
        SYSLaunchMenu();
        return EXIT_RELAUNCH_ON_LOAD;
    }

    deInit();
    return EXIT_SUCCESS;
}

/*
    Patching all the functions!!!
*/
void ApplyPatches(){
    PatchInvidualMethodHooks(method_hooks_fs,           method_hooks_size_fs,           method_calls_fs);
}

/*
    Restoring everything!!
*/

void RestorePatches(){
    RestoreInvidualInstructions(method_hooks_fs,        method_hooks_size_fs);
    KernelRestoreInstructions();
}

void deInit(){
    RestorePatches();
    log_deinit();
}

s32 isInMiiMakerHBL(){
    if (OSGetTitleID != 0 && (
            OSGetTitleID() == 0x000500101004A200 || // mii maker eur
            OSGetTitleID() == 0x000500101004A100 || // mii maker usa
            OSGetTitleID() == 0x000500101004A000 ||// mii maker jpn
            OSGetTitleID() == 0x0005000013374842))
        {
            return 1;
    }
    return 0;
}

void Init_Log() {
    log_init("192.168.1.50");
    if(!hasReadIP) {
        log_printf("Reading ip from sd card\n");
        hasReadIP = 1;
        
        FSStat stats;
        // get command and client
        void* pClient = malloc(FS_CLIENT_SIZE);
        void* pCmd = malloc(FS_CMD_BLOCK_SIZE);
        char * mountPath = NULL;

        if(!pClient || !pCmd) {
            // just in case free if not 0
            if(pClient)
                free(pClient);
            if(pCmd)
                free(pCmd);
            return;
        }

        FSInit();
        FSInitCmdBlock(pCmd);
        FSAddClientEx(pClient, 0, -1);
        
        if(MountFS(pClient,pCmd,&mountPath) == 0) {
            log_printf("Successfully mounted sd card!\n");
            
            char path[250];
            sprintf(path,"%s%s/ip.txt",CAFE_OS_SD_PATH,WIIU_PATH);
            
            int status = -1;
            s32 handle;
            if((status = FSGetStat(pClient,pCmd,path,&stats,-1)) == 0){
                log_printf("File found at (%s)\n",path);
                if(stats.size > 15){
                    log_printf("filesize is to big!\n");
                    FSDelClient(pClient);
                    free(pClient);
                    free(pCmd);
                    return;
                }
                char * file  = (char *) malloc((sizeof(char)*stats.size)+1);
                if(!file){
                    log_printf("Unable to malloc enough space to store contents of file\n");
                    FSDelClient(pClient);
                    free(pClient);
                    free(pCmd);
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
                    log_printf("Unable to open (%s)\n",path);
                    FSDelClient(pClient);
                    free(pClient);
                    free(pCmd);
                    free(file);
                    return;
                }
                FSCloseFile(pClient,pCmd,handle,-1);
                UmountFS(pClient,pCmd,mountPath);
                FSDelClient(pClient);
                free(pClient);
                free(pCmd);
                free(file);
                
                
                strcpy(ipFromSd,file);
                log_printf("Successfully read ip from sd! ip is: %s\n",ipFromSd);
            }else{
                log_printf("%s was not found, unable to load ip from sd\n",path);
            }
            
        } else {
            log_printf("Unable to mount sd card\n");
        }
    }
    if(strlen(ipFromSd) > 0) {
        log_init(ipFromSd);
    }
}

