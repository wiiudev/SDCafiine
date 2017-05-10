#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "common/retain_vars.h"
#include "fs_function_patcher.h"
#include "utils/logger.h"
#include "fs_function_utils.h"
#include "fs/fs_utils.h"

int client_num_alloc(void *pClient) {
    int i;

    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == 0) {
            bss.pClient_fs[i] = (int)pClient;
            return i;
        }
    return -1;
}

void client_num_free(int client) {
    bss.pClient_fs[client] = 0;
    bss.sd_mount[client] = 0;
}

int client_num(void *pClient) {
    int i;
    for (i = 0; i < MAX_CLIENT; i++)
        if (bss.pClient_fs[i] == (int)pClient)
            return i;
    return -1;
}

int getClientAndInitSD(void *pClient, void *pCmd){
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

int is_gamefile(const char *path) {
     // In case the path starts by "//" and not "/" (some games do that ... ...)
    if (path[0] == '/' && path[1] == '/')
        path = &path[1];

    // In case the path does not start with "/" (some games do that too ...)
    int len = 0;
    char new_path[16];
    if(path[0] != '/') {
        new_path[0] = '/';
        len++;
    }

    while(*path && len < (int)(sizeof(new_path) - 1)) {
        new_path[len++] = *path++;
    }
    new_path[len++] = 0;

    if(strncmp(path,"/vol/content",12) == 0) return 1;

    return 0;
}


char * getNewPath(void *pClient, void *pCmd, const char *path){
    char * newPath;
    int useOld = 1;
    if ((is_gamefile(path)) && (int)bss_ptr != 0x0A000000) {
        if(DEBUG_LOG) log_print("getNewPath\n");

        s32 i = 0;
        s32 len = 0;
        s32 path_offset = 0;

        // In case the path starts by "//" and not "/" (some games do that ... ...)
        if (path[0] == '/' && path[1] == '/')
            path = &path[1];

        // In case the path does not start with "/" set an offset for all the accesses
        if(path[0] != '/')
            path_offset = -1;

        // some games are doing /vol/content/./....
        if(path[13 + path_offset] == '.' && path[14 + path_offset] == '/') {
            path_offset += 2;
        }

        newPath = (char*)malloc(sizeof(char) * (strlen(path) + strlen(bss.mount_base) + 1));
        if(newPath == NULL){
            if(DEBUG_LOG) log_print("malloc failed\n");
            return NULL;
        }
        int client = -1;

        if((client = getClientAndInitSD(pClient,pCmd)) != -1){
            s32 n = strlcpy(newPath, bss.mount_base, sizeof(bss.mount_base));

            // copy the content file path with slash at the beginning
            for (i = 0; i < (len - 12 - path_offset); i++) {
                char cChar = path[12 + i + path_offset];
                // skip double slashes
                if((newPath[n-1] == '/') && (cChar == '/')) {
                    continue;
                }
                newPath[n++] = cChar;
                newPath[n++] = '\0';
            }

            FSStat tmp_stats;
            if (FSGetStat(pClient, pCmd, newPath, &tmp_stats, -1) == 0) {
                log_printf("using new path -> %s\n",newPath);
                useOld = 0;
            }else{
                log_printf("using old path -> %s\n",newPath);
            }
        }

    }else{
         newPath = (char*)malloc(sizeof(char) * (strlen(path) + 1));
         if(newPath == NULL) return NULL;
    }
    if(useOld){
            sprintf(newPath,"%s",path);
    }
    return newPath;
}


void readIPfromSD(void *pClient, void *pCmd){
    if(!gReadIPfromSD){
        log_print("Trying to read IP from SD\n");
        gReadIPfromSD = 1;
        FSStat stats;
        char path[250];
        sprintf(path,"%s%s/ip.txt",CAFE_OS_SD_PATH,WIIU_PATH);
        s32 status = -1;
        s32 handle;
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
                s32 total_read = 0;
                s32 ret2 = 0;
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
