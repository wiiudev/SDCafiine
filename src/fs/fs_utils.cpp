#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "fs_utils.h"
#include "common/common.h"
#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/fs_functions.h"
#include "utils/logger.h"
#include "utils/StringTools.h"
#include "common/retain_vars.h"

int MountFS(void *pClient, void *pCmd, char **mount_path){
    int result = -1;

    void *mountSrc = malloc(FS_MOUNT_SOURCE_SIZE);
    if(!mountSrc)
        return -3;

    char* mountPath = (char*) malloc(FS_MAX_MOUNTPATH_SIZE);
    if(!mountPath) {
        free(mountSrc);
        return -4;
    }

    memset(mountSrc, 0, FS_MOUNT_SOURCE_SIZE);
    memset(mountPath, 0, FS_MAX_MOUNTPATH_SIZE);

    // Mount sdcard
    if (FSGetMountSource(pClient, pCmd, FS_SOURCETYPE_EXTERNAL, mountSrc, -1) == 0)
    {
        result = FSMount(pClient, pCmd, mountSrc, mountPath, FS_MAX_MOUNTPATH_SIZE, -1);
        if((result == 0) && mount_path) {
            *mount_path = (char*)malloc(strlen(mountPath) + 1);
            if(*mount_path)
                strcpy(*mount_path, mountPath);
        }
    }

    free(mountPath);
    free(mountSrc);
    return result;
}

int UmountFS(void *pClient, void *pCmd, const char *mountPath){
    int result = -1;
    result = FSUnmount(pClient, pCmd, mountPath, -1);

    return result;
}

int LoadFileToMem(const char *filepath, u8 **inbuffer, u32 *size){
    //! always initialze input
	*inbuffer = NULL;
    if(size)
        *size = 0;

    int iFd = open(filepath, O_RDONLY);
	if (iFd < 0)
		return -1;

	u32 filesize = lseek(iFd, 0, SEEK_END);
    lseek(iFd, 0, SEEK_SET);

	u8 *buffer = (u8 *) malloc(filesize);
	if (buffer == NULL)
	{
        close(iFd);
		return -2;
	}

    u32 blocksize = 0x4000;
    u32 done = 0;
    int readBytes = 0;

	while(done < filesize)
    {
        if(done + blocksize > filesize) {
            blocksize = filesize - done;
        }
        readBytes = read(iFd, buffer + done, blocksize);
        if(readBytes <= 0)
            break;
        done += readBytes;
    }

    close(iFd);

	if (done != filesize)
	{
		free(buffer);
		return -3;
	}

	*inbuffer = buffer;

    //! sign is optional input
    if(size){
        *size = filesize;
    }
	return filesize;
}

int CheckFile(const char * filepath){
	if(!filepath)
		return 0;

	struct stat filestat;

	char dirnoslash[strlen(filepath)+2];
	snprintf(dirnoslash, sizeof(dirnoslash), "%s", filepath);

	while(dirnoslash[strlen(dirnoslash)-1] == '/')
		dirnoslash[strlen(dirnoslash)-1] = '\0';

	char * notRoot = strrchr(dirnoslash, '/');
	if(!notRoot)
	{
		strcat(dirnoslash, "/");
	}

	if (stat(dirnoslash, &filestat) == 0)
		return 1;

	return 0;
}

int CreateSubfolder(const char * fullpath){
	if(!fullpath)
		return 0;

	int result = 0;

	char dirnoslash[strlen(fullpath)+1];
	strcpy(dirnoslash, fullpath);

	int pos = strlen(dirnoslash)-1;
	while(dirnoslash[pos] == '/')
	{
		dirnoslash[pos] = '\0';
		pos--;
	}

	if(CheckFile(dirnoslash))
	{
		return 1;
	}
	else
	{
		char parentpath[strlen(dirnoslash)+2];
		strcpy(parentpath, dirnoslash);
		char * ptr = strrchr(parentpath, '/');

		if(!ptr)
		{
			//!Device root directory (must be with '/')
			strcat(parentpath, "/");
			struct stat filestat;
			if (stat(parentpath, &filestat) == 0)
				return 1;

			return 0;
		}

		ptr++;
		ptr[0] = '\0';

		result = CreateSubfolder(parentpath);
	}

	if(!result)
		return 0;

	if (mkdir(dirnoslash, 0777) == -1)
	{
		return 0;
	}

	return 1;
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

    if(strncmp(new_path,"/vol/content",12) == 0) return GAME_PATH_TYPE_CONTENT;
    //dlc support
    if(strncmp(new_path,"/vol/aoc",8) == 0) return GAME_PATH_TYPE_AOC;

    return 0;
}
char * getPathWithNewBase(const char * inPath, const char * newBase){
    if(inPath == NULL || newBase == NULL || !(strlen(newBase) > 0)) return NULL;
    char * relativePath = getRelativePath(inPath);
    char * result = NULL;


    if(relativePath == NULL){
        /*
        if(strlen(gModFolder) > 0){
            int file = 0;
            if(endsWith(inPath,BOOT_TV_TEX_TGA,-1,-1)){ file = 1;}
            else if(endsWith(inPath,BOOT_DRC_TEX_TGA,-1,-1)){ file = 2;}
            else if(endsWith(inPath,BOOT_SOUND_BTSND,-1,-1)){ file = 3;}
            if(file == 0){ return NULL;}

            result = (char*)malloc((sizeof(char) * (strlen(newBase) +1 + strlen(META_FOLDER) + 1 + 20) + 1));
            if(result == NULL){
                DEBUG_FUNCTION_LINE("malloc for result failed.\n");
                return NULL;
            }
            if(file == 1){sprintf(result,"%s/%s/%s",newBase,META_FOLDER,BOOT_TV_TEX_TGA);}
            if(file == 2){sprintf(result,"%s/%s/%s",newBase,META_FOLDER,BOOT_DRC_TEX_TGA);}
            if(file == 3){sprintf(result,"%s/%s/%s",newBase,META_FOLDER,BOOT_SOUND_BTSND);}
            return result;
        }*/

        return NULL;
    }

    result = (char*)malloc((sizeof(char) * (strlen(newBase) +1+ strlen(relativePath))) + 1);
    if(result == NULL){
        DEBUG_FUNCTION_LINE("malloc for result failed.\n");
        return NULL;
    }
    sprintf(result,"%s/%s",newBase,relativePath);

    free(relativePath);
    relativePath = NULL;

    return result;
}

char * getRelativePath(const char *path){
    if(path == NULL) return NULL;
    char * pathForCheck = NULL;

    int gameFile = is_gamefile(path);
    if(gameFile > 0) {
        //if(DEBUG_LOG) log_printf("getNewPath %s\n", path);
        int path_offset = 0;

        // In case the path starts by "//" and not "/" (some games do that ... ...)
        if (path[0] == '/' && path[1] == '/')
            path = &path[1];

        // In case the path does not start with "/" set an offset for all the accesses
        if(path[0] != '/')
            path_offset = -1;


        if(gameFile == GAME_PATH_TYPE_CONTENT) { // is content
            //content

            // some games are doing /vol/content/./....
            if(path[13 + path_offset] == '.' && path[14 + path_offset] == '/') {
                path_offset += 2;
            }
            char * pathForCheckInternal = (char*)path + 13 + path_offset;
            if(pathForCheckInternal[0] == '/') pathForCheckInternal++; //Skip double slash

            pathForCheck = (char*)malloc(sizeof(CONTENT_FOLDER) + 1 + (sizeof(char) * (strlen(pathForCheckInternal) + 1)));
            if(pathForCheck == NULL){
                DEBUG_FUNCTION_LINE("malloc failed\n");
                return NULL;
            }
            sprintf(pathForCheck,"%s/%s",CONTENT_FOLDER,pathForCheckInternal);
        } else if (gameFile == GAME_PATH_TYPE_AOC) { // is aoc
            //aoc
            int aocFolderLength = 1;
            char * aocFolderLengthCheck = (char*)path + 5 + path_offset;
            while (aocFolderLengthCheck[0] != '/') {
                aocFolderLength++;
                aocFolderLengthCheck++;
            }
            char * pathForCheckInternal = (char*)path + 5 + aocFolderLength + path_offset;
            if(pathForCheckInternal[0] == '/') pathForCheckInternal++; //Skip double slash

            pathForCheck = (char*)malloc(sizeof(AOC_FOLDER) + 1 + (sizeof(char) * (strlen(pathForCheckInternal) + 1)));
            if(pathForCheck == NULL){
                DEBUG_FUNCTION_LINE("malloc failed\n");
                return NULL;
            }
            sprintf(pathForCheck,"%s/%s",AOC_FOLDER,pathForCheckInternal);
        }
    }

    return pathForCheck;
}
