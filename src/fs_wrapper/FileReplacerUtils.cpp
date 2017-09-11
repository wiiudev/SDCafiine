#include <vector>
#include "FileReplacerUtils.h"

OSMessageQueue lfFSQueue __attribute__((section(".data")));
OSMessage lfFSQueueMessages[FS_QUEUE_MESSAGE_COUNT] __attribute__((section(".data")));

FSAsyncResult lfAsyncResultCache[ASYNC_RESULT_CACHE_SIZE] __attribute__((section(".data")));
u8 lfAsyncResultCacheLock __attribute__((section(".data"))) = 0;
u8 lfAsyncResultCacheCur __attribute__((section(".data"))) = 0;

FileReplacerUtils *FileReplacerUtils::instance = NULL;

int setErrorFlag(int error){
    int result = error;
    if(error == -1){
        result = CHECKED_WITH_ALL_ERRORS;
    }else{
        result |= CHECKED_MASK;
    }
    return result;
}

int checkErrorFlag(int * error){
    if(*error == CHECKED_WITH_ALL_ERRORS){
        *error = -1;
        return true;
    }else if ((*error & CHECKED_MASK) == CHECKED_MASK){
        *error &= ~CHECKED_MASK;
        return true;
    }
    return false;
}

void FileReplacerUtils::StartAsyncThread(){
    s32 priority = 15;
    this->pThread = CThread::create(DoAsyncThread, this , CThread::eAttributeAffCore2,priority);
    this->pThread->resumeThread();
}

void FileReplacerUtils::DoAsyncThread(CThread *thread, void *arg){
    FileReplacerUtils * arg_instance = (FileReplacerUtils *) arg;
    return arg_instance->DoAsyncThreadInternal();
}

void FileReplacerUtils::DoAsyncThreadInternal(){
    serverHasStopped = 0;
    DEBUG_FUNCTION_LINE("FSAsyncServer started! \n");

    OSMessage message;
    void (*callback)(CustomAsyncParam *);
    CustomAsyncParam cParam;
    while(true){
        if(FS_WRAPPER_DEBUG_LOG){DEBUG_FUNCTION_LINE("Waiting for message\n");}
        if(!OSReceiveMessage(&fsFSQueue,&message,OS_MESSAGE_BLOCK)){
            os_usleep(1000);
            continue;
        }
        if(FS_WRAPPER_DEBUG_LOG){DEBUG_FUNCTION_LINE("Received message %08X\n",message.message);}
        if(message.message == 0xDEADBEEF){
            DEBUG_FUNCTION_LINE("We should stop the server\n");
            break;
        }

        callback = (void(*)(CustomAsyncParam *))message.message;
        CustomAsyncParam * param = (CustomAsyncParam *)message.data0;
        memcpy(&cParam,param,sizeof(CustomAsyncParam));
        free(param);
        if(FS_WRAPPER_DEBUG_LOG){DEBUG_FUNCTION_LINE("Calling callback at %08X, with %08X\n",callback,&cParam);}
        callback(&cParam);
    }
    serverHasStopped = 1;
}

FSAsyncResult * FileReplacerUtils::getNewAsyncParamPointer(){
    while(fsAsyncResultCacheLock){
        os_usleep(100);
    }
    fsAsyncResultCacheLock = 1;

    if(fsAsyncResultCacheCur >= ASYNC_RESULT_CACHE_SIZE){
        fsAsyncResultCacheCur = 0;
    }

    FSAsyncResult *result = &fsAsyncResultCache[fsAsyncResultCacheCur++];

    fsAsyncResultCacheLock = 0;
    return result;
}

void FileReplacerUtils::sendAsyncCommand(FSClient * client, FSCmdBlock * cmd,FSAsyncParams* asyncParams,int status){
    if(asyncParams != NULL){
        if(asyncParams->userCallback != NULL){ //Using the userCallback
            if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("userCallback %08X userContext %08X\n",asyncParams->userCallback,asyncParams->userContext); }
            asyncParams->userCallback(client,cmd,status,asyncParams->userContext);
            return;
        }else{
            if(FS_WRAPPER_DEBUG_LOG){ DEBUG_FUNCTION_LINE("asyncParams->ioMsgQueue %08X \n",asyncParams->ioMsgQueue); }
            FSAsyncResult * result = FileReplacerUtils::getNewAsyncParamPointer();
            FSMessage message;
            result->userParams.userCallback = asyncParams->userCallback;
            result->userParams.userContext = asyncParams->userContext;
            result->userParams.ioMsgQueue = asyncParams->ioMsgQueue;
            result->ioMsg.data = &result;
            result->ioMsg.unkwn1 = 0;
            result->ioMsg.unkwn2 = 0;
            result->ioMsg.unkwn3 = 0x08;
            result->client = client;
            result->block = cmd;
            result->result = status;

            message.data = (void*)result;
            message.unkwn1 = (u32)0;
            message.unkwn2 = (u32)0;
            message.unkwn3 = (u32)0x08;

            OSSendMessage(asyncParams->ioMsgQueue,(OSMessage*)&message,OS_MESSAGE_BLOCK);
        }
    }
}
