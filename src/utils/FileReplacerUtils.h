#include <vector>
#include <algorithm>

#ifndef __FILE_REPLACER_UTILS_H_
#define __FILE_REPLACER_UTILS_H_

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

	private:
	    FileReplacerUtils(){
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

	    std::vector<int> handles;
        static FileReplacerUtils * instance;

};
#endif // __FILE_REPLACER_UTILS_H_
