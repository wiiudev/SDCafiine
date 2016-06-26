#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/gx2_functions.h"
#include "dynamic_libs/syshid_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/fs_functions.h"
#include "system/memory.h"
#include "utils/logger.h"
#include "common/common.h"
#include "start.h"
#include "patcher/function_hooks.h"
#include "kernel/kernel_functions.h"
#include "system/exception_handler.h"


/* Entry point */
extern "C" int Menu_Main(void)
{
    //!*******************************************************************
    //!                   Initialize function pointers                   *
    //!*******************************************************************
    //! do OS (for acquire) and sockets first so we got logging
    InitOSFunctionPointers();
    InitSocketFunctionPointers();
    InitGX2FunctionPointers();
    InitSysFunctionPointers();
    InitFSFunctionPointers();


    log_init("192.168.0.182");

    SetupKernelCallback();
    log_printf("Started %s\n", cosAppXmlInfoStruct.rpx_name);

    PatchMethodHooks();
    if(strlen(cosAppXmlInfoStruct.rpx_name) > 0 && strcasecmp("ffl_app.rpx", cosAppXmlInfoStruct.rpx_name) != 0)
    {
        return EXIT_RELAUNCH_ON_LOAD;
    }



    if(strlen(cosAppXmlInfoStruct.rpx_name) <= 0){ // First boot back to SysMenu
        log_printf("First boot back to SysMenu\n");
        SYSLaunchMenu();
        return EXIT_RELAUNCH_ON_LOAD;
    }


    RestoreInstructions();



    log_deinit();


    return EXIT_SUCCESS;
}

