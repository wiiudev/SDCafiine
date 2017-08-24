#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include "main.h"
#include "modpackSelector.h"
#include "common/common.h"
#include <fat.h>
#include <iosuhax.h>
#include <iosuhax_devoptab.h>
#include <iosuhax_disc_interface.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/proc_ui_functions.h"
#include "patcher/fs_function_patcher.h"
#include "utils/function_patcher.h"
#include "kernel/kernel_functions.h"
#include "utils/FileReplacer.h"
#include "utils/logger.h"
#include "fs/fs_utils.h"
#include "fs/sd_fat_devoptab.h"
#include "fs/CFile.hpp"
#include "fs/DirList.h"
#include "common/retain_vars.h"
#include "system/exception_handler.h"
#include "utils/mcpHook.h"

u8 isFirstBoot __attribute__((section(".data"))) = 1;

/* Entry point */

extern "C" int Menu_Main(void)
{
    if(gAppStatus == 2){
        //"No, we don't want to patch stuff again.");
        return EXIT_RELAUNCH_ON_LOAD;
    }
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! aquire every rpl we want to patch

    InitOSFunctionPointers();
    InitSocketFunctionPointers(); //For logging

    InitSysFunctionPointers(); // For SYSLaunchMenu()
    InitProcUIFunctionPointers(); // For SYSLaunchMenu()
    InitFSFunctionPointers();
    InitVPadFunctionPointers();

    log_init();

    setup_os_exceptions();

    log_printf("Mount SD partition\n");
    Init_SD();

    SetupKernelCallback();

    memset(&fspatchervars,0,sizeof(fspatchervars));

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

    HandleMultiModPacks();

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
    unmount_sd_fat("sd");
    unmount_fake();
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
    delete replacer;
    replacer = NULL;
    gSDInitDone = 0;
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

void Init_SD() {
    int res = IOSUHAX_Open(NULL); //This is not working properly..
    if(res < 0){
        res = MCPHookOpen();
        gUsingLibIOSUHAX = 2;
    }
    if(res < 0){
        gUsingLibIOSUHAX = 0;
        log_printf("IOSUHAX_open failed\n");
        mount_fake();
        if((res = mount_sd_fat("sd")) >= 0){
            log_printf("mount_sd_fat success\n");
            gSDInitDone = 1;
        }else{
            log_printf("mount_sd_fat failed %d\n",res);
        }
    }else{
        if(gUsingLibIOSUHAX != 2){
            log_printf("Using IOSUHAX for (some) sd access\n");
            gUsingLibIOSUHAX = 1;
        }else{
            log_printf("Using IOSUHAX (MCPHOOK) for (some) sd access\n");
        }

        if((res = fatInitDefault()) >= 0){
            log_printf("fatInitDefault success\n");
            gSDInitDone = 1;
        }else{
            log_printf("fatInitDefault failed %d\n",res);
        }
    }
}