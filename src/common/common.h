#ifndef COMMON_H
#define    COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "os_defs.h"

#define CAFE_OS_SD_PATH             "/vol/external01"
#define GAME_MOD_FOLDER             "/sdcafiine"
#define SD_PATH                     "sd:"
#define USB_PATH                    "usb:"
#define DEFAULT_NAME_PACKAGE        "DEFAULT"
#define NAME_PREFIX_SD              "sd: "
#define NAME_PREFIX_USB             "usb:"
#define WIIU_PATH                   "/wiiu"
#define IP_TXT                      "ip.txt"
#define VERSION                     "1.5b (experimental)"

#define CONTENT_FOLDER              "content"
#define AOC_FOLDER                  "aoc"
#define META_FOLDER                 "meta"

#define BOOT_TV_TEX_TGA             "bootTvTex.tga"
#define BOOT_DRC_TEX_TGA            "bootDrcTex.tga"
#define BOOT_SOUND_BTSND            "bootSound.btsnd"

#define GAME_PATH_TYPE_CONTENT      1
#define GAME_PATH_TYPE_AOC          2

#define USE_OS_FS_FUNCTION          -1337

#define SDUSB_MOUNTED_NONE          0
#define SDUSB_MOUNTED_FAKE          (1<<0)
#define SDUSB_MOUNTED_OS_SD         (1<<1)
#define SDUSB_MOUNTED_LIBIOSUHAX    (1<<2)

#define FILELIST_NAME               "filelist.txt"
#define DIR_IDENTIFY                "?"  /* maximum length = 1*/
#define PARENT_DIR_IDENTIFY         "?.."

/* Macros for libs */
#define LIB_CORE_INIT           0
#define LIB_NSYSNET             1
#define LIB_GX2                 2
#define LIB_AOC                 3
#define LIB_AX                  4
#define LIB_FS                  5
#define LIB_OS                  6
#define LIB_PADSCORE            7
#define LIB_SOCKET              8
#define LIB_SYS                 9
#define LIB_VPAD                10
#define LIB_NN_ACP              11
#define LIB_SYSHID              12
#define LIB_VPADBASE            13
#define LIB_AX_OLD              14
#define LIB_PROC_UI             15

// functions types
#define STATIC_FUNCTION         0
#define DYNAMIC_FUNCTION        1

// none dynamic libs
#define LIB_LOADER              0x1001

#ifndef MEM_BASE
#define MEM_BASE                (0x00800000)
#endif

#define ELF_DATA_ADDR               (*(volatile unsigned int*)(MEM_BASE + 0x1300 + 0x00))
#define ELF_DATA_SIZE               (*(volatile unsigned int*)(MEM_BASE + 0x1300 + 0x04))
#define MAIN_ENTRY_ADDR             (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x00))
#define OS_FIRMWARE                 (*(volatile unsigned int*)(MEM_BASE + 0x1400 + 0x04))

#define OS_SPECIFICS                ((OsSpecifics*)(MEM_BASE + 0x1500))

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS                0
#endif
#define EXIT_HBL_EXIT               0xFFFFFFFE
#define EXIT_RELAUNCH_ON_LOAD       0xFFFFFFFD

#ifdef __cplusplus
}
#endif

#endif    /* COMMON_H */

