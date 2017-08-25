#include "libfat_wrapper.h"
#include "common/common.h"
#include "common/retain_vars.h"
#include "fs_utils.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "utils/logger.h"

int libfat_FSCloseFile(int handle){
    if(FileReplacerUtils::hasHandle(handle)){
        FileReplacerUtils::removeHandle(handle);
        close(handle);
        if(DEBUG_LOG){log_printf("libfat_FSCloseFile: closed %d\n",handle);}
        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

int libfat_FSGetPosFile(int handle,int * pos){
    if(FileReplacerUtils::hasHandle(handle)){
        int currentPos = lseek(handle, (size_t)0, SEEK_CUR);
        *pos = currentPos;

        if(DEBUG_LOG){log_printf("libfat_FSGetPosFile: pos %08X\n",*pos);}

        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

int libfat_FSGetStat(const char * path, FSStat * stats){
    int result = USE_OS_FS_FUNCTION;
    char * newPath = getPathWithNewBase(path,gModFolder);
    if(newPath != NULL){
        struct stat path_stat;
        if(stat(newPath, &path_stat) < 0){
            result = -1;
            log_printf("libfat_FSGetStat: failed for path %s\n",newPath);
        }else{
            stats->flag = 0;
            if(S_ISDIR(path_stat.st_mode)){
                stats->flag |= 0;
            }

            stats->size = path_stat.st_size;

            //Values copied from my console while loading SSBU. Maybe doesn't work with all games/consoles/regions?
            stats->permission = 0x00000444;
            stats->owner_id = 0x10053000;

            if(DEBUG_LOG){log_printf("libfat_FSGetStat: success! size: %08X for path %s\n",stats->size,newPath);}

            result = FS_STATUS_OK;
        }
        free(newPath);
        newPath = NULL;
    }
    return result;
}

int libfat_FSGetStatFile(int handle, FSStat * stats){
    if(FileReplacerUtils::hasHandle(handle)){
        struct stat path_stat;
        if(fstat(handle, &path_stat) < 0){
            log_printf("libfat_FSGetStatFile failed! handle: %08X\n",handle);
            return -1;
        }

        stats->size = path_stat.st_size;
        stats->flag = 0;
        //Values copied from my console while loading SSBU. Maybe doesn't work with all games/consoles/regions?
        stats->permission = 0x00000444;
        stats->owner_id = 0x10053000;

        if(DEBUG_LOG){log_printf("libfat_FSGetStatFile success! handle: %08X size: %08X\n",handle,stats->size);}

        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

int libfat_FSIsEof(int handle){
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        off_t currentPos = lseek(handle, (off_t) 0, SEEK_CUR);
        off_t endPos = lseek(handle, (off_t) 0, SEEK_END);

        if(currentPos == endPos){
            result = FS_STATUS_EOF;
        }else{
            lseek(handle, currentPos, SEEK_CUR);
            result = FS_STATUS_OK;
        }
        if(DEBUG_LOG){log_printf("libfat_FSIsEof: handle: %08X result: %08X\n",handle,result);}
    }
    return result;
}

int libfat_FSOpenFile(const char * path, int * handle){
    int result = USE_OS_FS_FUNCTION;
    char * newPath = getPathWithNewBase(path,gModFolder);
    if(newPath != NULL){
        result = USE_OS_FS_FUNCTION;
        if(DEBUG_LOG){log_printf("libfat_FSOpenFile: open path: %s\n",newPath);}
        int fd = open(newPath,O_RDONLY); //TODO: remove hardcoded mode.
        if(fd != -1){
            FileReplacerUtils::addHandle(fd);
            *handle = fd;
            result = FS_STATUS_OK;
            if(DEBUG_LOG){log_printf("libfat_FSOpenFile: opened path: %s handle: %08X\n",newPath,handle);}
        }else{
            if(DEBUG_LOG){log_printf("libfat_FSOpenFile: failed path: %s\n",newPath);}
        }
        free(newPath);
        newPath = NULL;
    }

    return result;
}
int libfat_FSReadFile(int handle,void *buffer,size_t size, size_t count){
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        result = read(handle, buffer,size*count);
        if(DEBUG_LOG){log_printf("libfat_FSReadFile: Tried %08X bytes from handle %08X. result %08X \n",size,handle,result);}
    }
    return result;
}

int libfat_FSReadFileWithPos(int handle,u32 pos, void *buffer,size_t size, size_t count){
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        lseek(handle, pos, SEEK_SET);//TODO check for lseek result.
        result = read(handle, buffer,size*count);
        if(DEBUG_LOG){log_printf("libfat_FSReadFileWithPos: Tried %08X bytes from handle %08X at pos %08X. result %08X \n",size,handle,pos,result);}
    }
    return result;
}

int libfat_FSSetPosFile(int handle,u32 pos){
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        off_t newOffset = -1;
        result = -1;
        newOffset = lseek(handle, (off_t)pos, SEEK_SET);
        if(newOffset == (off_t)pos){
            result = FS_STATUS_OK;
            if(DEBUG_LOG){log_printf("libfat_FSSetPosFile: Set position to %08X for handle %08X\n",pos,handle);}
        }else{
            log_printf("libfat_FSSetPosFile: Failed set position to %08X for handle %08X\n",pos,handle);
        }

    }
    return result;
}
