

#ifndef __FILE_REPLACER_UTILS_H_
#define __FILE_REPLACER_UTILS_H_

#include <vector>
#include <algorithm>

#include "common/retain_vars.h"
#include "system/OSThread.h"
#include "dynamic_libs/fs_defs.h"

typedef struct CustomAsyncParam_{
    FSClient *pClient;
    FSCmdBlock *pCmd;
    const char *path;
    const char *newPath;
    const char *mode;
    FSStat * stats;
    void * buffer;
    int handle;
    int size;
    int count;
    int pos;
    int flag;
    int *handlePtr;
    int *posPtr;
    int error;
    FSAsyncParams *asyncParams;
    void* realFunction;
} CustomAsyncParam;

class FileReplacerUtils{
    public:
        static FileReplacerUtils * getInstance() {
            if(!instance){
                instance = new FileReplacerUtils();
            }

            return instance;
        }

        static void destroyInstance() {
            if(instance){
                instance->StopAsyncThread();
                while(!instance->serverHasStopped){
                    os_usleep(1000);
                }
                os_usleep(10000);
                delete instance;
                instance = NULL;
            }
        }

        static void addHandle(int handle){
            getInstance()->addHandleInternal(handle);
        }

        static void removeHandle(int handle){
            getInstance()->removeHandleInternal(handle);
        }

        static bool hasHandle(int handle){
            return getInstance()->hasHandleInternal(handle);
        }

        static void sendAsyncCommand(FSClient * client, FSCmdBlock * cmd,FSAsyncParams* asyncParams,int status);

        void StartAsyncThread();

        void StopAsyncThread(){
            DEBUG_FUNCTION_LINE("StopAsyncThread\n");
            OSMessage message;
            message.message = 0xDEADBEEF;
            while(!OSSendMessage(&gFSQueue,&message,OS_MESSAGE_NOBLOCK));
        }

	private:
	    FileReplacerUtils(){
            OSInitMessageQueue(&gFSQueue, gFSQueueMessages, FS_QUEUE_MESSAGE_COUNT);
            DEBUG_FUNCTION_LINE("Init queue done! \n");
	    }

        void addHandleInternal(int handle){
            handles.push_back(handle);
        }

        void removeHandleInternal(int handle){
            handles.erase(std::remove(handles.begin(), handles.end(), handle), handles.end());
        }

        bool hasHandleInternal(int handle){
            if(std::find(handles.begin(), handles.end(), handle) != handles.end()) {
                return true;
            } else {
                return false;
            }
        }

        static void DoAsyncThread(CThread *thread, void *arg);

        static FSAsyncResult * getNewAsyncParamPointer();

        void DoAsyncThreadInternal();

        CThread *pThread;

        volatile int serverHasStopped = 1;

	    std::vector<int> handles;
        static FileReplacerUtils * instance;
};
#endif // __FILE_REPLACER_UTILS_H_
