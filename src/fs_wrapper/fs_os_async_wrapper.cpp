#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "fs_os_async_wrapper.h"
#include "utils/logger.h"

//Wii U fails to allocate memory if we do the functions async. =/
#define DO_REAL_ASYNC       0

void fs_os_wrapper_FSCloseFileAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSCloseFile(params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSCloseFileAsyncCallback)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
    real_FSCloseFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSCloseFileAsyncCallback(params->pClient, params->pCmd, params->handle, params->error,&my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSCloseFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSCloseFile(fd)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        int (*real_FSCloseFileAsyncCallback)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
        real_FSCloseFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) realFunction;
        return real_FSCloseFileAsyncCallback(pClient, pCmd, fd, error ,asyncParams);
    }
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called!\n"); }
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
    message.message = (u32) fs_os_wrapper_FSCloseFileAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSGetPosFileAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSGetPosFile(params->handle,params->posPtr)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *);
    real_FSGetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetPosFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->posPtr, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSGetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int fd, int *pos, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSGetPosFile(fd,pos)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        int (*real_FSGetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *);
        real_FSGetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int *, int, FSAsyncParams *)) realFunction;
        return real_FSGetPosFileAsyncFunc(pClient, pCmd, fd, pos, error, asyncParams);
    }
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSGetPosFileAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSGetStatAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
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
    if((result = fs_wrapper_FSGetStat(path,params->stats)) != USE_OS_FS_FUNCTION){
        DEBUG_FUNCTION_LINE("success. for params %08X \n",params);
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetStatAsyncFunc)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *);
    real_FSGetStatAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetStatAsyncFunc(params->pClient, params->pCmd, path, params->stats, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSGetStatAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, FSStat *stats, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSGetStat(path,stats)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }
        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSGetStatAsyncFunc)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *);
        real_FSGetStatAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, const char *, FSStat *, int, FSAsyncParams *)) realFunction;
        return real_FSGetStatAsyncFunc(pClient, pCmd, path, stats, error, asyncParams);
    }
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSGetStatAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSGetStatFileAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSGetStatFile(params->handle,params->stats)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function.\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSGetStatFileAsyncFunc)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *);
    real_FSGetStatFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSGetStatFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->stats, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSGetStatFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, FSStat * stats, int error, FSAsyncParams * asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSGetStatFile(handle,stats)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function.\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSGetStatFileAsyncFunc)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *);
        real_FSGetStatFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, FSStat *, int, FSAsyncParams *)) realFunction;
        return real_FSGetStatFileAsyncFunc(pClient, pCmd, handle, stats, error, asyncParams);
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSGetStatFileAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSIsEofAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSIsEof(params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSIsEofAsyncFunc)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
    real_FSIsEofAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSIsEofAsyncFunc(params->pClient, params->pCmd, params->handle, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSIsEofAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSIsEof(handle)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSIsEofAsyncFunc)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *);
        real_FSIsEofAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, int, FSAsyncParams *)) realFunction;
        return real_FSIsEofAsyncFunc(pClient, pCmd, handle, error, asyncParams);
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSIsEofAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSOpenFileAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
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
    if((result = fs_wrapper_FSOpenFile(path,params->mode,params->handlePtr)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function. Path %s \n",params,path); }

    //If we fail, we fall back to the OS function.
    int (*real_FSOpenFileAsyncCallback)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *);
    real_FSOpenFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSOpenFileAsyncCallback(params->pClient, params->pCmd, path, params->mode, params->handlePtr, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSOpenFileAsync(FSClient *pClient, FSCmdBlock *pCmd, const char *path, const char *mode, int *handle, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSOpenFile(path,mode,handle)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function.\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSOpenFileAsyncCallback)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *);
        real_FSOpenFileAsyncCallback = (int(*)(FSClient *, FSCmdBlock *, const char *, const char *, int *, int, FSAsyncParams *)) realFunction;
        return real_FSOpenFileAsyncCallback(pClient, pCmd, path, mode, handle, error, asyncParams);
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSOpenFileAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSReadFileAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSReadFile(params->handle,params->buffer,params->size,params->count)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSReadFileAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *);
    real_FSReadFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSReadFileAsyncFunc(params->pClient, params->pCmd, params->buffer, params->size, params->count, params->handle, params->flag, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSReadFileAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSReadFile(handle,buffer,size,count)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSReadFileAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *);
        real_FSReadFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, int, int, int, FSAsyncParams *)) realFunction;
        return real_FSReadFileAsyncFunc(pClient, pCmd, buffer, size, count, handle, flag, error, asyncParams);
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSReadFileAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSReadFileWithPosAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSReadFileWithPos(params->buffer,params->size,params->count,params->pos,params->handle)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSReadFileWithPosAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *);
    real_FSReadFileWithPosAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSReadFileWithPosAsyncFunc(params->pClient, params->pCmd, params->buffer, params->size, params->count, params->pos, params->handle, params->flag, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSReadFileWithPosAsync(FSClient *pClient, FSCmdBlock *pCmd, void *buffer, int size, int count, u32 pos, int handle, int flag, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSReadFileWithPos(buffer, size, count, pos, handle)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSReadFileWithPosAsyncFunc)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *);
        real_FSReadFileWithPosAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, void *, int, int, u32, int, int, int, FSAsyncParams *)) realFunction;
        return real_FSReadFileWithPosAsyncFunc(pClient, pCmd, buffer, size, count, pos, handle, flag, error, asyncParams);
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSReadFileWithPosAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}

void fs_os_wrapper_FSSetPosFileAsyncCallback(CustomAsyncParam * params){
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! params: %08X \n",params); }
    if(params == NULL){
        DEBUG_FUNCTION_LINE("!!!WARNING: Given parameter was NULL\n");
    }

    FSAsyncParams my_asyncparams;
    memcpy(&my_asyncparams,params->asyncParams,sizeof(FSAsyncParams));
    free(params->asyncParams);

    int result = USE_OS_FS_FUNCTION;
    if((result = fs_wrapper_FSSetPosFile(params->handle,params->pos)) != USE_OS_FS_FUNCTION){
        FileReplacerUtils::sendAsyncCommand(params->pClient, params->pCmd, &my_asyncparams, result);
        return;
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("params: %08X. We need to use the OS Function\n",params); }

    //If we fail, we fall back to the OS function.
    int (*real_FSSetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *);
    real_FSSetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *)) params->realFunction;
    int res = real_FSSetPosFileAsyncFunc(params->pClient, params->pCmd, params->handle, params->pos, params->error, &my_asyncparams);
    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("result real function %d\n",res); }
}

int fs_os_wrapper_FSSetPosFileAsync(FSClient *pClient, FSCmdBlock *pCmd, int handle, u32 pos, int error, FSAsyncParams *asyncParams, void * realFunction){
    if(!DO_REAL_ASYNC){
        int result = USE_OS_FS_FUNCTION;
        if((result = fs_wrapper_FSSetPosFile(handle,pos)) != USE_OS_FS_FUNCTION){
            FileReplacerUtils::sendAsyncCommand(pClient, pCmd, asyncParams, result);
            return FS_STATUS_OK;
        }

        if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("We need to use the OS Function\n"); }

        //If we fail, we fall back to the OS function.
        int (*real_FSSetPosFileAsyncFunc)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *);
        real_FSSetPosFileAsyncFunc = (int(*)(FSClient *, FSCmdBlock *, int, u32, int, FSAsyncParams *)) realFunction;
        return real_FSSetPosFileAsyncFunc(pClient, pCmd, handle, pos, error, asyncParams);
    }

    if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("Called! \n"); }
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
    message.message = (u32) fs_os_wrapper_FSSetPosFileAsyncCallback;
    message.data0 = (u32) params;

    FileReplacerUtils::addFSQueueMSG(&message);

    return FS_STATUS_OK;
}
