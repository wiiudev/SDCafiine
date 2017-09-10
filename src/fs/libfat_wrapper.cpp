#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libfat_wrapper.h"
#include "common/common.h"
#include "common/retain_vars.h"
#include "fs_utils.h"
#include "utils/logger.h"

//Wii U fails to allocate memory if we do the functions async. =/
#define DO_REAL_ASYNC       0

void libfat_FSCloseFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSCloseFile(params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSCloseFileAsyncCallback)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
    real_FSCloseFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSCloseFileAsyncCallback(params->pClient, params->pCmd, params->handle, params->error,&my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSCloseFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSCloseFile(fd)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        int (*real_FSCloseFileAsyncCallback)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
        real_FSCloseFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) realFunction;
        return real_FSCloseFileAsyncCallback(pClient, pCmd, fd, error ,asyncParams);
    }
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called!\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->handle = fd;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSCloseFileAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSCloseFile(int handle){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! for handle: %08X \n",handle); }
    if(FileReplacerUtils::hasHandle(handle)){
        FileReplacerUtils::removeHandle(handle);
        close(handle);
        DEBUG_FUNCTION_LINE("closed handle %08X\n",handle);
        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

void libfat_FSGetPosFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetPosFile(params->handle,params->posPtr)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *);
    real_FSGetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetPosFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->posPtr, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSGetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSGetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        int (*real_FSGetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *);
        real_FSGetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *)) realFunction;
        return real_FSGetPosFileAsyncFunc(pClient, pCmd, fd, pos, error, asyncParams);
    }
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->handle = fd;
    params->posPtr = pos;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSGetPosFileAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSGetPosFile(int handle,int * pos){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! for handle: %08X \n",handle); }
    if(FileReplacerUtils::hasHandle(handle)){
        int currentPos = lseek(handle, (size_t)0, SEEK_CUR);
        *pos = currentPos;

        DEBUG_FUNCTION_LINE("pos %08X for handle %08X\n",*pos,handle);

        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

void libfat_FSGetStatAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    char path[FS_MAX_ENTNAME_SIZE];
    snprintf(path,FS_MAX_ENTNAME_SIZE,"%s",params->path);
    free((void*)params->path);

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStat(path,params->stats)) != USE_OS_FS_FUNCTION){
        DEBUG_FUNCTION_LINE("success. for params %08X \n",params);
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetStatAsyncFunc)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *);
    real_FSGetStatAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetStatAsyncFunc(params->pClient, params->pCmd, path, params->stats, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSGetStatAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSGetStat(path,stats)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }
        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSGetStatAsyncFunc)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *);
        real_FSGetStatAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *)) realFunction;
        return real_FSGetStatAsyncFunc(pClient, pCmd, path, stats, error, asyncParams);
    }
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    char * pathCopy = (char *) malloc(sizeof(char) * strlen(path)+1);
    if(pathCopy == NULL){
        free(params);
        free(new_asyncParams);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the path param!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(pathCopy,path,strlen(path)+1);

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->path = pathCopy;
    params->stats = stats;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSGetStatAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSGetStat(const char * path, FSStat * stats){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called!\n"); }
    if(gSDInitDone <= SDUSB_MOUNTED_FAKE){
        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("SD/USB not initialized.\n"); }
        return USE_OS_FS_FUNCTION;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("for path \"%s\"\n",path); }
    int result = USE_OS_FS_FUNCTION;
    char * newPath = getPathWithNewBase(path,gModFolder);
    if(newPath != NULL){
        DEBUG_FUNCTION_LINE("Searching for path \"%s\"\n",newPath);
        struct stat path_stat;
        if(stat(newPath, &path_stat) < 0){
            result = USE_OS_FS_FUNCTION;
            if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("failed for path %s\n",newPath); }
        }else{
            DEBUG_FUNCTION_LINE("success! path %s\n",newPath);
            stats->flag = 0;
            if(S_ISDIR(path_stat.st_mode)){
                stats->flag |= 0;
            }

            stats->size = path_stat.st_size;

            //Values copied from my console while loading SSBU. Maybe doesn't work with all games/consoles/regions?
            stats->permission = 0x00000444;
            stats->owner_id = 0x10053000;

            result = FS_STATUS_OK;
        }
        free(newPath);
        newPath = NULL;
    }
    return result;
}

void libfat_FSGetStatFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStatFile(params->handle,params->stats)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function.\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetStatFileAsyncFunc)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *);
    real_FSGetStatFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetStatFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->stats, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSGetStatFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, FSStat * stats, int error, FSAsyncParams * asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSGetStatFile(handle,stats)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function.\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSGetStatFileAsyncFunc)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *);
        real_FSGetStatFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *)) realFunction;
        return real_FSGetStatFileAsyncFunc(pClient, pCmd, handle, stats, error, asyncParams);
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->handle = handle;
    params->stats = stats;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSGetStatFileAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSGetStatFile(int handle, FSStat * stats){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! for handle: %08X\n",handle); }
    if(FileReplacerUtils::hasHandle(handle)){
        struct stat path_stat;
        if(fstat(handle, &path_stat) < 0){
            DEBUG_FUNCTION_LINE("failed! handle: %08X\n",handle);
            return -1;
        }

        stats->size = path_stat.st_size;
        stats->flag = 0;
        //Values copied from my console while loading SSBU. Maybe doesn't work with all games/consoles/regions?
        stats->permission = 0x00000444;
        stats->owner_id = 0x10053000;

        DEBUG_FUNCTION_LINE("success! handle: %08X size: %08X\n",handle,stats->size);

        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

void libfat_FSIsEofAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSIsEof(params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSIsEofAsyncFunc)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
    real_FSIsEofAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSIsEofAsyncFunc(params->pClient, params->pCmd, params->handle, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSIsEofAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSIsEof(handle)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSIsEofAsyncFunc)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
        real_FSIsEofAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) realFunction;
        return real_FSIsEofAsyncFunc(pClient, pCmd, handle, error, asyncParams);
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->handle = handle;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSIsEofAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSIsEof(int handle){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! handle: %08X\n",handle); }
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
        DEBUG_FUNCTION_LINE("handle: %08X result: %08X\n",handle,result);
    }
    return result;
}

void libfat_FSOpenFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    char path[FS_MAX_ENTNAME_SIZE];
    snprintf(path,FS_MAX_ENTNAME_SIZE,"%s",params->path);
    free((void*)params->path);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSOpenFile(path,params->mode,params->handlePtr)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function. Path %s \n",params,path); }

    //If we fail, we fall back to the OS function.
    int (*real_FSOpenFileAsyncCallback)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *);
    real_FSOpenFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSOpenFileAsyncCallback(params->pClient, params->pCmd, path, params->mode, params->handlePtr, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSOpenFileAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSOpenFile(path,mode,handle)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function.\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSOpenFileAsyncCallback)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *);
        real_FSOpenFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *)) realFunction;
        return real_FSOpenFileAsyncCallback(pClient, pCmd, path, mode, handle, error, asyncParams);
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    char * pathCopy = (char *) malloc(sizeof(char) * strlen(path)+1);
    if(pathCopy == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the path param!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(pathCopy,path,strlen(path)+1);

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->path = pathCopy;
    params->mode = mode;
    params->handlePtr = handle;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSOpenFileAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSOpenFile(const char * path, const char * mode, int * handle){
    if(gSDInitDone <= SDUSB_MOUNTED_FAKE){
        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! SD/USB not initialized.\n"); }
        return USE_OS_FS_FUNCTION;
    }
    int result = USE_OS_FS_FUNCTION;
    char * newPath = getPathWithNewBase(path,gModFolder);
    if(newPath != NULL){
        result = USE_OS_FS_FUNCTION;
        DEBUG_FUNCTION_LINE("Searching for path %s\n",newPath);
        int fd = open(newPath,O_RDONLY); //TODO: remove hardcoded mode.
        if(fd != -1){
            DEBUG_FUNCTION_LINE("opened path: %s handle: %08X\n",newPath,fd);
            FileReplacerUtils::addHandle(fd);
            *handle = fd;
            result = FS_STATUS_OK;

        }else{
            if(DEBUG_LOG){DEBUG_FUNCTION_LINE("failed path: %s\n",newPath);}
        }
        free(newPath);
        newPath = NULL;
    }

    return result;
}

void libfat_FSReadFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFile(params->handle,params->buffer,params->size,params->count)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSReadFileAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *);
    real_FSReadFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSReadFileAsyncFunc(params->pClient, params->pCmd, params->buffer, params->size, params->count, params->handle, params->flag, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSReadFileAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSReadFile(handle,buffer,size,count)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSReadFileAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *);
        real_FSReadFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *)) realFunction;
        return real_FSReadFileAsyncFunc(pClient, pCmd, buffer, size, count, handle, flag, error, asyncParams);
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->buffer = buffer;
    params->size = size;
    params->count = count;
    params->handle = handle;
    params->flag = flag;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSReadFileAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSReadFile(int handle,void *buffer,size_t size, size_t count){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! for handle: %08X \n",handle); }
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        result = read(handle, buffer,size*count);
        DEBUG_FUNCTION_LINE("Reading %08X bytes from handle %08X. result %08X \n",size,handle,result);
    }
    return result;
}

void libfat_FSReadFileWithPosAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFileWithPos(params->buffer,params->size,params->count,params->pos,params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSReadFileWithPosAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *);
    real_FSReadFileWithPosAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSReadFileWithPosAsyncFunc(params->pClient, params->pCmd, params->buffer, params->size, params->count, params->pos, params->handle, params->flag, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSReadFileWithPosAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSReadFileWithPos(buffer, size, count, pos, handle)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSReadFileWithPosAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *);
        real_FSReadFileWithPosAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *)) realFunction;
        return real_FSReadFileWithPosAsyncFunc(pClient, pCmd, buffer, size, count, pos, handle, flag, error, asyncParams);
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->buffer = buffer;
    params->size = size;
    params->count = count;
    params->pos = pos;
    params->handle = handle;
    params->flag = flag;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSReadFileWithPosAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSReadFileWithPos(void *buffer, int size, int count, u32 pos, int handle){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        lseek(handle, pos, SEEK_SET);//TODO check for lseek result.
        result = read(handle, buffer,size*count);
        DEBUG_FUNCTION_LINE("Reading %08X bytes from handle %08X at pos %08X. result %08X \n",size,handle,pos,result);
    }
    return result;
}

void libfat_FSSetPosFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSSetPosFile(params->handle,params->pos)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSSetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *);
    real_FSSetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSSetPosFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->pos, params->error, &my_asyncparams);
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int libfat_FSSetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, u32 pos, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = libfat_FSSetPosFile(handle,pos)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSSetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *);
        real_FSSetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *)) realFunction;
        return real_FSSetPosFileAsyncFunc(pClient, pCmd, handle, pos, error, asyncParams);
    }

    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!Failed to allocate memory!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        free(params);
        DEBUG_FUNCTION_LINE("!!!Failed to allocate enough space to copy the FSAsyncParams!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    params->pClient = pClient;
    params->pCmd = pCmd;
    params->handle = handle;
    params->pos = pos;
    params->error = error;
    params->asyncParams = new_asyncParams;
    params->realFunction = realFunction;

    OSMessage message;
    message.message = (u32) libfat_FSSetPosFileAsyncCallback;
    message.data0 = (u32) params;

    OSSendMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK);

    return FS_STATUS_OK;
}

int libfat_FSSetPosFile(int handle,u32 pos){
    if(DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        off_t newOffset = -1;
        result = -1;
        newOffset = lseek(handle, (off_t)pos, SEEK_SET);
        if(newOffset == (off_t)pos){
            result = FS_STATUS_OK;
            DEBUG_FUNCTION_LINE("Set position to %08X for handle %08X\n",pos,handle);
        }else{
            DEBUG_FUNCTION_LINE("Failed set position to %08X for handle %08X\n",pos,handle);
        }

    }
    return result;
}
