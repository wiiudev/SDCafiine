#include <vector>
#include "FileReplacerUtils.h"

FileReplacerUtils *FileReplacerUtils::instance = NULL;

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
    log_printf("DoAsyncThreadInternal()[LINE %d]: FSAsyncServer started! \n",__LINE__);

    OSInitMessageQueue(&gFSQueue, gFSQueueMessages, FS_QUEUE_MESSAGE_COUNT);

    log_printf("DoAsyncThreadInternal()[LINE %d]: Init queue done! \n",__LINE__);

    OSMessage message;
    void (*callback)(CustomAsyncParam *);
    while(true){
        if(DEBUG_LOG){log_printf("DoAsyncThreadInternal()[LINE %d]: Waiting for message\n",__LINE__);}
        if(!OSReceiveMessage(&gFSQueue,&message,OS_MESSAGE_BLOCK)){
            //os_usleep(1000*100);
            continue;
        }
        if(DEBUG_LOG){log_printf("DoAsyncThreadInternal()[LINE %d]: Received message %08X\n",__LINE__,message.message);}
        if(message.message == 0xDEADBEEF){
            log_printf("DoAsyncThreadInternal()[LINE %d]: We should stop the server\n",__LINE__);
            break;
        }

        callback = (void(*)(CustomAsyncParam *))message.message;
        CustomAsyncParam * param = (CustomAsyncParam *)message.data0;

        if(DEBUG_LOG){log_printf("DoAsyncThreadInternal()[LINE %d]: Calling callback at %08X, with %08X\n",__LINE__,callback,param);}
        callback(param);
        free(param);
    }
    serverHasStopped = 1;
}

FSAsyncResult * FileReplacerUtils::getNewAsyncParamPointer(){
    while(gAsyncResultCacheLock){
        os_usleep(100);
    }
    gAsyncResultCacheLock = 1;

    if(gAsyncResultCacheCur >= ASYNC_RESULT_CACHE_SIZE){
        gAsyncResultCacheCur = 0;
    }

    FSAsyncResult *result = &gAsyncResultCache[gAsyncResultCacheCur++];

    gAsyncResultCacheLock = 0;
    return result;
}

void FileReplacerUtils::sendAsyncCommand(FSClient * client, FSCmdBlock * cmd,FSAsyncParams* asyncParams,int status){
    if(asyncParams != NULL){
        if(asyncParams->userCallback != NULL){ //Using the userCallback
            asyncParams->userCallback(client,cmd,status,asyncParams->userContext);
            return;
        }else{
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
