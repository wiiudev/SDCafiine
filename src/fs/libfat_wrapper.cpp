#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "libfat_wrapper.h"
#include "common/common.h"
#include "common/retain_vars.h"
#include "fs_utils.h"
#include "utils/logger.h"

void libfat_FSCloseFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSCloseFileAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSCloseFileAsyncCallback was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSCloseFile(params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ log_printf("libfat_FSCloseFileAsyncCallback: params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSCloseFileAsyncCallback)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
    real_FSCloseFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSCloseFileAsyncCallback(params->pClient, params->pCmd, params->handle, params->error,&my_asyncparams);
    if(DEBUG_LOG){ log_printf("libfat_FSCloseFileAsyncCallback: result real function %d\n",res); }
}

int libfat_FSCloseFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSCloseFileAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSCloseFileAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSCloseFile: for handle: %08X \n",handle); }
    if(FileReplacerUtils::hasHandle(handle)){
        FileReplacerUtils::removeHandle(handle);
        close(handle);
        if(DEBUG_LOG){log_printf("libfat_FSCloseFile: closed %d\n",handle);}
        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

void libfat_FSGetPosFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSGetPosFileAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSGetPosFileAsyncCallback was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetPosFile(params->handle,params->posPtr)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ log_printf("libfat_FSGetPosFileAsyncCallback: params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *);
    real_FSGetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetPosFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->posPtr, params->error, &my_asyncparams);
    if(DEBUG_LOG){ log_printf("libfat_FSGetPosFileAsyncCallback: result real function %d\n",res); }
}

int libfat_FSGetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSGetPosFileAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSGetPosFileAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSGetPosFile: for handle: %08X \n",handle); }
    if(FileReplacerUtils::hasHandle(handle)){
        int currentPos = lseek(handle, (size_t)0, SEEK_CUR);
        *pos = currentPos;

        if(DEBUG_LOG){log_printf("libfat_FSGetPosFile: pos %08X\n",*pos);}

        return FS_STATUS_OK;
    }
    return USE_OS_FS_FUNCTION;
}

void libfat_FSGetStatAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSGetStatAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSGetStatAsyncCallback was NULL\n");
    }

    char path[FS_MAX_ENTNAME_SIZE];
    snprintf(path,FS_MAX_ENTNAME_SIZE,"%s",params->path);
    free((void*)params->path);

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStat(path,params->stats)) != USE_OS_FS_FUNCTION){
        log_printf("async libfat_FSGetStat success. for params %08X \n",params);
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }
    if(DEBUG_LOG){ log_printf("libfat_FSGetStatAsyncCallback: params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetStatAsyncFunc)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *);
    real_FSGetStatAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetStatAsyncFunc(params->pClient, params->pCmd, path, params->stats, params->error, &my_asyncparams);
    if(DEBUG_LOG){ log_printf("libfat_FSGetStatAsyncCallback: result real function %d\n",res); }
}

int libfat_FSGetStatAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSGetStatAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSGetStatAsync!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    char * pathCopy = (char *) malloc(sizeof(char) * strlen(path)+1);
    if(pathCopy == NULL){
        log_printf("!!!Failed to allocate enough space to copy the path param for libfat_FSGetStatAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSGetStat: for path %s\n",path); }
    int result = USE_OS_FS_FUNCTION;
    char * newPath = getPathWithNewBase(path,gModFolder);
    if(DEBUG_LOG){ log_printf("libfat_FSGetStat: new path %s\n",newPath); }
    if(newPath != NULL){
        struct stat path_stat;
        if(stat(newPath, &path_stat) < 0){
            result = USE_OS_FS_FUNCTION;
            if(DEBUG_LOG){ log_printf("libfat_FSGetStat: failed for path %s\n",newPath); }
        }else{
            if(DEBUG_LOG){ log_printf("libfat_FSGetStat: success! size: %08X for path %s\n",path_stat.st_size,newPath);}
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
    if(DEBUG_LOG){ log_printf("libfat_FSGetStatFileAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSGetStatFileAsyncCallback was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSGetStatFile(params->handle,params->stats)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ log_printf("libfat_FSGetStatFileAsyncCallback: params: %08X. We need to use the OS Function.\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetStatFileAsyncFunc)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *);
    real_FSGetStatFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetStatFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->stats, params->error, &my_asyncparams);
    if(DEBUG_LOG){ log_printf("libfat_FSCloseFileAsyncCallback: result real function %d\n",res); }
}

int libfat_FSGetStatFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, FSStat * stats, int error, FSAsyncParams * asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSGetStatFileAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSGetStatFileAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSGetStatFile for handle: %08X\n",handle); }
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

void libfat_FSIsEofAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSIsEofAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSIsEofAsyncCallback was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSIsEof(params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ log_printf("libfat_FSIsEofAsyncCallback: params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSIsEofAsyncFunc)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
    real_FSIsEofAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSIsEofAsyncFunc(params->pClient, params->pCmd, params->handle, params->error, &my_asyncparams);
    if(DEBUG_LOG){ log_printf("libfat_FSIsEofAsyncCallback: result real function %d\n",res); }
}

int libfat_FSIsEofAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSIsEofAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSIsEofAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSIsEof for handle: %08X\n",handle); }
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

void libfat_FSOpenFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSOpenFileAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSOpenFileAsyncCallback was NULL\n");
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

    if(DEBUG_LOG){ log_printf("libfat_FSOpenFileAsyncCallback: params: %08X. We need to use the OS Function. Path %s \n",params,path); }

    //If we fail, we fall back to the OS function.
    int (*real_FSOpenFileAsyncCallback)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *);
    real_FSOpenFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSOpenFileAsyncCallback(params->pClient, params->pCmd, path, params->mode, params->handlePtr, params->error, &my_asyncparams);
    if(DEBUG_LOG){ log_printf("libfat_FSOpenFileAsyncCallback: result real function %d\n",res); }
}

int libfat_FSOpenFileAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSOpenFileAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSOpenFileAsync!!!\n");
        return FS_STATUS_FATAL_ERROR;
    }
    memcpy(new_asyncParams,asyncParams,sizeof(FSAsyncParams));

    char * pathCopy = (char *) malloc(sizeof(char) * strlen(path)+1);
    if(pathCopy == NULL){
        log_printf("!!!Failed to allocate enough space to copy the path param for libfat_FSOpenFileAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSOpenFile for path: %s and mode %s\n",path,mode); }
    int result = USE_OS_FS_FUNCTION;
    char * newPath = getPathWithNewBase(path,gModFolder);
    if(newPath != NULL){
        result = USE_OS_FS_FUNCTION;
        if(DEBUG_LOG){log_printf("libfat_FSOpenFile: open path: %s\n",newPath);}
        int fd = open(newPath,O_RDONLY); //TODO: remove hardcoded mode.
        if(fd != -1){
            if(DEBUG_LOG){log_printf("libfat_FSOpenFile: opened path: %s handle: %08X\n",newPath,fd);}
            FileReplacerUtils::addHandle(fd);
            *handle = fd;
            result = FS_STATUS_OK;

        }else{
            if(DEBUG_LOG){log_printf("libfat_FSOpenFile: failed path: %s\n",newPath);}
        }
        free(newPath);
        newPath = NULL;
    }

    return result;
}

void libfat_FSReadFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSReadFileAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSOpenFileAsyncCallback was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFile(params->handle,params->buffer,params->size,params->count)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ log_printf("libfat_FSReadFileAsyncCallback: params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSReadFileAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *);
    real_FSReadFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *)) params->realFunction;
    real_FSReadFileAsyncFunc(params->pClient, params->pCmd, params->buffer, params->size, params->count, params->handle, params->flag, params->error, &my_asyncparams);
}

int libfat_FSReadFileAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSReadFileAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSReadFileAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSReadFile: for handle: %08X \n",handle); }
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        result = read(handle, buffer,size*count);
        if(DEBUG_LOG){log_printf("libfat_FSReadFile: Tried %08X bytes from handle %08X. result %08X \n",size,handle,result);}
    }
    return result;
}

void libfat_FSReadFileWithPosAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSReadFileWithPosAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSReadFileWithPosAsyncCallback was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSReadFileWithPos(params->buffer,params->size,params->count,params->pos,params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ log_printf("libfat_FSReadFileWithPosAsyncCallback: params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSReadFileWithPosAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *);
    real_FSReadFileWithPosAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *)) params->realFunction;
    real_FSReadFileWithPosAsyncFunc(params->pClient, params->pCmd, params->buffer, params->size, params->count, params->pos, params->handle, params->flag, params->error, &my_asyncparams);
}

int libfat_FSReadFileWithPosAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSReadFileWithPosAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSReadFileWithPosAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSReadFileWithPos: for handle: %08X \n",handle); }
    int result = USE_OS_FS_FUNCTION;
    if(FileReplacerUtils::hasHandle(handle)){
        lseek(handle, pos, SEEK_SET);//TODO check for lseek result.
        result = read(handle, buffer,size*count);
        if(DEBUG_LOG){log_printf("libfat_FSReadFileWithPos: Tried %08X bytes from handle %08X at pos %08X. result %08X \n",size,handle,pos,result);}
    }
    return result;
}

void libfat_FSSetPosFileAsyncCallback(CustomAsyncParam * params){
    if(DEBUG_LOG){ log_printf("libfat_FSSetPosFileAsyncCallback: params: %08X \n",params); }
    if(params == NULL){
        log_printf("!!!WARNING: Given parameter for libfat_FSSetPosFileAsyncCallback was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = libfat_FSSetPosFile(params->handle,params->pos)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(DEBUG_LOG){ log_printf("libfat_FSSetPosFileAsyncCallback: params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSSetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *);
    real_FSSetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *)) params->realFunction;
    real_FSSetPosFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->pos, params->error, &my_asyncparams);
}

int libfat_FSSetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, u32 pos, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(DEBUG_LOG){ log_printf("libfat_FSSetPosFileAsync\n"); }
    CustomAsyncParam *  params = (CustomAsyncParam *) malloc(sizeof(CustomAsyncParam));

    FSAsyncParams * new_asyncParams = (FSAsyncParams *) malloc(sizeof(FSAsyncParams));
    if(new_asyncParams == NULL){
        log_printf("!!!Failed to allocate enough space to copy the FSAsyncParams for libfat_FSSetPosFileAsync!!!\n");
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
    if(DEBUG_LOG){ log_printf("libfat_FSSetPosFile: for handle: %08X \n",handle); }
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
