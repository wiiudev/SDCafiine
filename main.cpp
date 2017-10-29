#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include "main.h"
#include "modpackSelector.h"
#include "common/common.h"
#include <iosuhax.h>
#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "dynamic_libs/socket_functions.h"
#include "dynamic_libs/sys_functions.h"
#include "dynamic_libs/proc_ui_functions.h"
#include "patcher/fs_function_patcher.h"
#include "utils/function_patcher.h"
#include "kernel/kernel_functions.h"
#include <fswrapper/FileReplacerUtils.h>
#include "utils/logger.h"
#include <fs/FSUtils.h>
#include <utils/StringTools.h>
#include "fs/sd_fat_devoptab.h"
#include "fs/CFile.hpp"
#include "fs/DirList.h"
#include "common/retain_vars.h"
#include "system/exception_handler.h"
#include "system/CThread.h"
#include "myutils/mocha.h"
#include "myutils/libfat.h"
#include "myutils/libntfs.h"

#include <sys/types.h>
#include <dirent.h>

bool printdir(std::string path){
    log_printf(       "open dir                         %s\n",path.c_str());
    struct dirent *dirent = NULL;
	DIR *dir_ = opendir(path.c_str());
	if (dir_ == NULL){
		return false;
	}
	while ((dirent = readdir(dir_)) != 0){
		bool isDir = dirent->d_type & DT_DIR;
		bool isFile = dirent->d_type & DT_REG;
		const char *filename = dirent->d_name;

		if(strcmp(filename,".") == 0 ||
            strcmp(filename,"..") == 0){
            continue;
        }

        std::string newPath = path + "/" + std::string(filename);

        log_printf(    "flags             %08X path: %s\n",dirent->d_type,filename);
        if(isDir){
            printdir(newPath);
        }else if(isFile){
            int fd = open(newPath.c_str(),O_RDONLY);
            log_printf("open file  fd:    %08X path: %s\n",fd,filename);
            log_printf("close file result:%08X path: %s\n",close(fd),filename);
        }else{
            log_printf("unkown filetype %08X %s\n",dirent->d_type,filename);
        }
    }
    log_printf(        "close dir                        %s\n",path.c_str());
    //closedir(dir_);

}

u8 isFirstBoot __attribute__((section(".data"))) = 1;

/* Entry point */
extern "C" int Menu_Main(void){
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

    gSDInitDone = 0;

    DEBUG_FUNCTION_LINE("Mount SD partition\n");
    Init_SD_USB();

    //printdir("ntfs0:/sdcafiine");

    /*

    DEBUG_FUNCTION_LINE("test1 open\n");
    FILE * file1 = fopen("ntfs0:/test1.txt","wb+");
    DEBUG_FUNCTION_LINE("test2 open\n");
    FILE * file2 = fopen("ntfs0:/test2.txt","wb+");
    DEBUG_FUNCTION_LINE("test3 open\n");
    FILE * file3 = fopen("ntfs0:/test3.txt","wb+");

    if(file1 != NULL && file2 != NULL && file3 != NULL){
        DEBUG_FUNCTION_LINE("Creating all file was successful\n");
        void *  data = malloc(sizeof(char) * 0x8000*100);
        memset(data,0x37,sizeof(char) * 0x8000*100);
        DEBUG_FUNCTION_LINE("Write 2\n");
        fwrite (data , 1, sizeof(char) * 0x8000*100, file2);
        DEBUG_FUNCTION_LINE("Write 1\n");
        fwrite (data , 1, sizeof(char) * 0x8000*100, file1);
        DEBUG_FUNCTION_LINE("Write 2\n");
        fwrite (data , 1, sizeof(char) * 0x8000*100, file2);
        DEBUG_FUNCTION_LINE("Write 3\n");
        fwrite (data , 1, sizeof(char) * 0x8000*100, file3);
        DEBUG_FUNCTION_LINE("Write 2\n");
        fwrite (data , 1, sizeof(char) * 0x8000*100, file2);
        DEBUG_FUNCTION_LINE("Write 1\n");
        fwrite (data , 1, sizeof(char) * 0x8000*100, file1);
        DEBUG_FUNCTION_LINE("Write 1\n");
        fwrite (data , 1, sizeof(char) * 0x8000*100, file1);

        DEBUG_FUNCTION_LINE("test1 close\n");
        fclose(file1);
        DEBUG_FUNCTION_LINE("test2 close\n");
        fclose(file2);
        DEBUG_FUNCTION_LINE("test3 close\n");
        fclose(file3);
    }*/

    //deInit();
    //return EXIT_SUCCESS;


    SetupKernelCallback();
    //!*******************************************************************
    //!                        Patching functions                        *
    //!*******************************************************************
    DEBUG_FUNCTION_LINE("Patching functions\n");
    ApplyPatches();

    //FileReplacerUtils::getInstance()->StartAsyncThread();

    //gLastMetaPath[0] = 0;

    //Reset everything when were going back to the Mii Maker
    if(!isFirstBoot && isInMiiMakerHBL()){
        DEBUG_FUNCTION_LINE("Returing to the Homebrew Launcher!\n");
        isFirstBoot = 0;
        deInit();
        return EXIT_SUCCESS;
    }

    if(!isInMiiMakerHBL()){ //Starting the application
        HandleMultiModPacks(OSGetTitleID());
        return EXIT_RELAUNCH_ON_LOAD;
    }

    if(isFirstBoot){ // First boot back to SysMenu
        DEBUG_FUNCTION_LINE("Loading the System Menu\n");
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
    //KernelRestoreInstructions();
}

void deInit(){
    RestorePatches();
    //FileReplacerUtils::getInstance()->StopAsyncThread();
    FileReplacerUtils::destroyInstance();
    log_deinit();

    deInit_SD_USB();
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

void Init_SD_USB() {
    int res = IOSUHAX_Open(NULL);
    if(res < 0){
        ExecuteIOSExploitWithDefaultConfig();
        return;
    }
    deleteDevTabsNames();
    mount_fake();
    gSDInitDone |= SDUSB_MOUNTED_FAKE;

    if(res < 0){
        DEBUG_FUNCTION_LINE("IOSUHAX_open failed\n");
        if((res = mount_sd_fat("sd")) >= 0){
            DEBUG_FUNCTION_LINE("mount_sd_fat success\n");
            gSDInitDone |= SDUSB_MOUNTED_OS_SD;
        }else{
            DEBUG_FUNCTION_LINE("mount_sd_fat failed %d\n",res);
        }
    }else{
        DEBUG_FUNCTION_LINE("Using IOSUHAX for SD/USB access\n");
        gSDInitDone |= SDUSB_LIBIOSU_LOADED;
        int ntfs_mounts = mountAllNTFS();
        if(ntfs_mounts > 0){
            gSDInitDone |= USB_MOUNTED_LIBNTFS;
        }

        if(mount_libfatAll() == 0){
            gSDInitDone |= SD_MOUNTED_LIBFAT;
            gSDInitDone |= USB_MOUNTED_LIBFAT;
        }
    }
    DEBUG_FUNCTION_LINE("%08X\n",gSDInitDone);
}

void deInit_SD_USB(){
    DEBUG_FUNCTION_LINE("Called this function.\n");

    if(gSDInitDone & SDUSB_MOUNTED_FAKE){
       DEBUG_FUNCTION_LINE("Unmounting fake\n");
       unmount_fake();
       gSDInitDone &= ~SDUSB_MOUNTED_FAKE;
    }
    if(gSDInitDone & SDUSB_MOUNTED_OS_SD){
        DEBUG_FUNCTION_LINE("Unmounting OS SD\n");
        unmount_sd_fat("sd");
        gSDInitDone &= ~SDUSB_MOUNTED_OS_SD;
    }

    if(gSDInitDone & SD_MOUNTED_LIBFAT){
        DEBUG_FUNCTION_LINE("Unmounting LIBFAT SD\n");
        unmount_libfat("sd");
        gSDInitDone &= ~SD_MOUNTED_LIBFAT;
    }

    if(gSDInitDone & USB_MOUNTED_LIBFAT){
        DEBUG_FUNCTION_LINE("Unmounting LIBFAT USB\n");
        unmount_libfat("usb");
        gSDInitDone &= ~USB_MOUNTED_LIBFAT;
    }

    if(gSDInitDone & USB_MOUNTED_LIBNTFS){
        DEBUG_FUNCTION_LINE("Unmounting LIBNTFS USB\n");
        unmountAllNTFS();
        gSDInitDone &= ~USB_MOUNTED_LIBNTFS;
    }

    if(gSDInitDone & SDUSB_LIBIOSU_LOADED){
        DEBUG_FUNCTION_LINE("Calling IOSUHAX_Close\n");
        IOSUHAX_Close();
        gSDInitDone &= ~SDUSB_LIBIOSU_LOADED;

    }
    deleteDevTabsNames();
    if(gSDInitDone != SDUSB_MOUNTED_NONE){
        DEBUG_FUNCTION_LINE("WARNING. Some devices are still mounted.\n");
    }
    DEBUG_FUNCTION_LINE("Function end.\n");
}
