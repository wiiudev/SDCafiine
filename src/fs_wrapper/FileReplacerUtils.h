#ifndef __FILE_REPLACER_UTILS_H_
#define __FILE_REPLACER_UTILS_H_

#include <vector>
#include <algorithm>

#include "fs_retain_vars.h"
#include "system/OSThread.h"
#include "dynamic_libs/fs_defs.h"
#include "dynamic_libs/os_types.h"

#define CHECKED_WITH_ALL_ERRORS     0x10000
#define CHECKED_MASK                0x01000

#define FS_WRAPPER_DEBUG_LOG        0

#define USE_OS_FS_FUNCTION          -1337

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

/**
Returns a modified error flag.
This will be used to save the information if a file/handle was already
tried to be patched.
The non-async function internally call the async functions, and this way
we avoid testing it twice.
If the result contains our mask, we just straight to the OS functions.
**/
int setErrorFlag(int error);

/**
Check if we already checked the file/handle.
Returns true if it was already checked (+ revert the error)
Return false if it should be (tried) to be patched.
**/
int checkErrorFlag(int * error);

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

        static void addFileHandle(int handle){
            getInstance()->addFileHandleInternal(handle);
        }

        static void removeFileHandle(int handle){
            getInstance()->removeFileHandleInternal(handle);
        }

        static bool hasFileHandle(int handle){
            return getInstance()->hasFileHandleInternal(handle);
        }

        static void addDirHandle(int handle){
            getInstance()->addDirHandleInternal(handle);
        }

        static void removeDirHandle(int handle){
            getInstance()->removeDirHandleInternal(handle);
        }

        static bool hasDirHandle(int handle){
            return getInstance()->hasDirHandleInternal(handle);
        }

        static bool addFSQueueMSG(OSMessage* message){
            OSSendMessage(&fsFSQueue,message,OS_MESSAGE_BLOCK);
            return true;
        }

        static void sendAsyncCommand(FSClient * client, FSCmdBlock * cmd,FSAsyncParams* asyncParams,int status);

        void StartAsyncThread();

        void StopAsyncThread(){
            DEBUG_FUNCTION_LINE("StopAsyncThread\n");
            OSMessage message;
            message.message = 0xDEADBEEF;
            while(!OSSendMessage(&fsFSQueue,&message,OS_MESSAGE_NOBLOCK));
        }

	private:
	    FileReplacerUtils(){
            OSInitMessageQueue(&fsFSQueue, fsFSQueueMessages, FS_QUEUE_MESSAGE_COUNT);
            DEBUG_FUNCTION_LINE("Init queue done! \n");
	    }

        void addFileHandleInternal(int handle){
            filehandles.push_back(handle);
        }

        void removeFileHandleInternal(int handle){
            filehandles.erase(std::remove(filehandles.begin(), filehandles.end(), handle), filehandles.end());
        }

        bool hasFileHandleInternal(int handle){
            if(std::find(filehandles.begin(), filehandles.end(), handle) != filehandles.end()) {
                return true;
            } else {
                return false;
            }
        }

        void addDirHandleInternal(int handle){
            dirhandles.push_back(handle);
        }

        void removeDirHandleInternal(int handle){
            dirhandles.erase(std::remove(dirhandles.begin(), dirhandles.end(), handle), dirhandles.end());
        }

        bool hasDirHandleInternal(int handle){
            if(std::find(dirhandles.begin(), dirhandles.end(), handle) != dirhandles.end()) {
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

	    std::vector<int> filehandles;
	    std::vector<int> dirhandles;
        static FileReplacerUtils * instance;
};
#endif // __FILE_REPLACER_UTILS_H_
