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

    if(strncmp(new_path,"/vol/content",12) == 0) return 1;
    //dlc support
    if(strncmp(new_path,"/vol/aoc",8) == 0) return 2;

    return 0;
}


char * getNewPath(void *pClient, void *pCmd, const char *path){
    if(DEBUG_LOG) log_printf("getNewPath %s\n", path); 
    int gameFile = is_gamefile(path);
    if(gameFile && (int)bss_ptr != 0x0A000000) {
        int path_offset = 0;

        // In case the path starts by "//" and not "/" (some games do that ... ...)
        if (path[0] == '/' && path[1] == '/')
            path = &path[1];

        // In case the path does not start with "/" set an offset for all the accesses
        if(path[0] != '/')
            path_offset = -1;

        char * newPath;

        if(gameFile == 1) {
            //content

            // some games are doing /vol/content/./....
            if(path[13 + path_offset] == '.' && path[14 + path_offset] == '/') {
                path_offset += 2;
            }
            char * pathfoo = (char*)path + 13 + path_offset;
            if(pathfoo[0] == '/') pathfoo++; //Skip double slash

            newPath = (char*)malloc(sizeof(char) * (strlen(bss.content_mount_base) + strlen(pathfoo) + 2));
            if(newPath == NULL){
                if(DEBUG_LOG) log_printf("malloc failed\n");
                return NULL;
            }
            sprintf(newPath,"%s/%s",bss.content_mount_base,pathfoo);
        } else if (gameFile == 2) {
            //aoc
            int aocFolderLength = 1;
            char * aocFolderLengthCheck = (char*)path + 5 + path_offset;
            while (aocFolderLengthCheck[0] != '/') {
                aocFolderLength++;
                aocFolderLengthCheck++;
            }
            char * pathfoo = (char*)path + 5 + aocFolderLength + path_offset;
            if(pathfoo[0] == '/') pathfoo++; //Skip double slash

            newPath = (char*)malloc(sizeof(char) * (strlen(bss.aoc_mount_base) + strlen(pathfoo) + 2));
            if(newPath == NULL){
                if(DEBUG_LOG) log_printf("malloc failed\n");
                return NULL;
            }
            sprintf(newPath,"%s/%s",bss.aoc_mount_base,pathfoo);
        }

        if(getClientAndInitSD(pClient,pCmd) != -1){
            FSStat tmp_stats;
            if (FSGetStat(pClient, pCmd, newPath, &tmp_stats, -1) == 0) {
                log_printf("using new path -> %s\n",newPath);
                return newPath;
            }else{
                log_printf("using old path -> %s\n",path);
            }
        }
    }  
    return NULL;
}