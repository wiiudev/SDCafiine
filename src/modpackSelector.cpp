#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include "modpackSelector.h"
#include "common/common.h"

#include "dynamic_libs/os_functions.h"
#include "dynamic_libs/vpad_functions.h"
#include "utils/logger.h"
#include "fs/fs_utils.h"
#include "fs/CFile.hpp"
#include "fs/DirList.h"
#include "common/retain_vars.h"

#define TEXT_SEL(x, text1, text2)           ((x) ? (text1) : (text2))

void HandleMultiModPacks() {
	sprintf(selectedMultiModPackFolder, "/");

    char TitleIDString[FS_MAX_FULLPATH_SIZE];
    sprintf(TitleIDString,"%016llX",OSGetTitleID());
    std::string modTitleIDPath = std::string(SD_PATH) + GAME_MOD_FOLDER + "/" + TitleIDString;

    DirList modTitleDirList(modTitleIDPath.c_str(), NULL, DirList::Dirs);
    modTitleDirList.SortList();

    //4 pages of up to 12 modpacks each not counting the default mod option if it exists
    char modpackList[4][12][FS_MAX_ENTNAME_SIZE];
    int modpackPageEntryCount[4] = {0,0,0,0};
    int modpackPageCount = 0;
    int modpackTotalCount = 0;
    bool hasDefault = false;

    for(int i = 0; i < modTitleDirList.GetFilecount(); i++) {
        if(strcmp(modTitleDirList.GetFilename(i), ".") == 0 || strcmp(modTitleDirList.GetFilename(i), "..") == 0)
            continue;

        if(strcmp(modTitleDirList.GetFilename(i), "content") == 0 || strcmp(modTitleDirList.GetFilename(i), "aoc") == 0) {
            hasDefault = true;
            modpackTotalCount++;
            break;
        }
    }

    for(int i = 0; i < modTitleDirList.GetFilecount() && modpackPageCount < 4; i++) {
        if(strcmp(modTitleDirList.GetFilename(i), ".") == 0 || strcmp(modTitleDirList.GetFilename(i), "..") == 0 ||
           //blacklisted modpack names
           strcmp(modTitleDirList.GetFilename(i), "content") == 0 || strcmp(modTitleDirList.GetFilename(i), "aoc") == 0 ||
           strcmp(modTitleDirList.GetFilename(i), "default") == 0) {
            continue;
        }

        DirList currentModTitleDirList((modTitleIDPath + "/" + modTitleDirList.GetFilename(i)).c_str(), NULL, DirList::Dirs);
        currentModTitleDirList.SortList();

        for(int j = 0; j < currentModTitleDirList.GetFilecount(); j++) {
            if(strcmp(currentModTitleDirList.GetFilename(j), ".") == 0 || strcmp(currentModTitleDirList.GetFilename(j), "..") == 0)
                continue;

            if(strcmp(currentModTitleDirList.GetFilename(j), "content") == 0 || strcmp(currentModTitleDirList.GetFilename(j), "aoc") == 0) {
                strcpy(modpackList[modpackPageCount][modpackPageEntryCount[modpackPageCount]], modTitleDirList.GetFilename(i));

                modpackPageEntryCount[modpackPageCount]++;
                if(modpackPageEntryCount[modpackPageCount] == 12) {
                    modpackPageCount++;
                }
                modpackTotalCount++;
                break;
            }
        }

    }

    if(modpackTotalCount > 1/*if we have more than one modpack display this ui*/) {
        // Init screen and screen buffers
        OSScreenInit();
        u32 screen_buf0_size = OSScreenGetBufferSizeEx(0);
        u32 screen_buf1_size = OSScreenGetBufferSizeEx(1);
        u32 * screenbuffers = (u32*)memalign(0x100, screen_buf0_size + screen_buf1_size);
        OSScreenSetBufferEx(0, (void *)screenbuffers);
        OSScreenSetBufferEx(1, (void *)(screenbuffers + screen_buf0_size));

        OSScreenEnableEx(0, 1);
        OSScreenEnableEx(1, 1);

        // Clear screens
        OSScreenClearBufferEx(0, 0);
        OSScreenClearBufferEx(1, 0);

        // Flip buffers
        OSScreenFlipBuffersEx(0);
        OSScreenFlipBuffersEx(1);

        VPADData vpad;
        s32 vpadError;
        int x_offset = -2;
        int initScreen = 1;
        int selected = 0;
        if (hasDefault) selected--;
        int page = 0;

        int max_pages = modpackPageCount + 1;
        int max_entries[4];
        for(int i = 0; i < 4;i++) {
            max_entries[i] = modpackPageEntryCount[i];
        }

        while(1)
        {
            //! update only at 50 Hz, thats more than enough
            vpadError = -1;
            VPADRead(0, &vpad, 1, &vpadError);

            if(vpadError == 0) {
                if(vpad.btns_d & VPAD_BUTTON_A) {
                    break;
                } else if(vpad.btns_d & VPAD_BUTTON_DOWN) {
                    selected++;
                    if(hasDefault) {
                        if(selected >= max_entries[page]) {
                            selected = -1;
                        }
                    } else {
                        if(selected >= max_entries[page]) {
                            selected = 0;
                        }
                    }

                    initScreen = 1;
                } else if(vpad.btns_d & VPAD_BUTTON_UP) {
                    selected--;
                    if(hasDefault) {
                        if(selected < -1) {
                            selected = max_entries[page] - 1;
                        }
                    } else {
                        if(selected < 0) {
                            selected = max_entries[page] - 1;
                        }
                    }

                    initScreen = 1;
                } else if(vpad.btns_d & VPAD_BUTTON_L) {
                    if(max_pages > 1) {
                        page--;
                        if(page < 0)
                            page = max_pages - 1;
                        
                        selected = 0;
                        if (hasDefault) selected--;
                        initScreen = 1;
                    }
                } else if(vpad.btns_d & VPAD_BUTTON_R) {
                    if(max_pages > 1) {
                        page++;
                        if(page >= max_pages)
                            page = 0;

                        selected = 0;
                        if (hasDefault) selected--;
                        initScreen = 1;
                    }
                }
            }

            if(initScreen) {
                OSScreenClearBufferEx(0, 0);
                OSScreenClearBufferEx(1, 0);

                console_print_pos(x_offset, -1, "                  -- SDCafiine %s by Maschell --",VERSION);

                console_print_pos(x_offset, 1, "Select your options and press A to launch.");
                int y_offset = 3;

                if(hasDefault) {
                    console_print_pos(x_offset, y_offset++, "%s %s", TEXT_SEL((selected == -1), "--->", "    "), "Default");
                }

                for(int idx = 0;idx < max_entries[page]; idx++)
                {
                    console_print_pos(x_offset, y_offset++, "%s %s", TEXT_SEL((selected == idx), "--->", "    "), modpackList[page][idx]);
                }

                if(max_pages > 0) {
                    console_print_pos(x_offset, 17, "Page %d/%d. Press L/R to change page.", page + 1,max_pages);
                }
                // Flip buffers
                OSScreenFlipBuffersEx(0);
                OSScreenFlipBuffersEx(1);

                initScreen = 0;
            }

            os_usleep(20000);
        }
        OSScreenClearBufferEx(0, 0);
        OSScreenClearBufferEx(1, 0);

        // Flip buffers
        OSScreenFlipBuffersEx(0);
        OSScreenFlipBuffersEx(1);

        if(selected == -1) {
        	sprintf(selectedMultiModPackFolder, "/");
        } else {
        	sprintf(selectedMultiModPackFolder, "/%s", modpackList[page][selected]);
        }
        free(screenbuffers);
    } else {
        //handle single modpack selection
        if(hasDefault) {
        	sprintf(selectedMultiModPackFolder, "/");
        } else {
        	sprintf(selectedMultiModPackFolder, "/%s", modpackList[0][0]);
        }
    } 
}

void console_print_pos(int x, int y, const char *format, ...)
{
    char * tmp = NULL;

    va_list va;
    va_start(va, format);
    if((vasprintf(&tmp, format, va) >= 0) && tmp)
    {
        if(strlen(tmp) > 79)
            tmp[79] = 0;

        OSScreenPutFontEx(0, x, y, tmp);
        OSScreenPutFontEx(1, x, y, tmp);

    }
    va_end(va);

    if(tmp)
        free(tmp);
}