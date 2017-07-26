#include <string>
#include "Directory.h"
#include <sys/dirent.h>

#ifndef __FILE_REPLACER_H_
#define __FILE_REPLACER_H_

extern "C" {
  #include "fs/sd_fat_devoptab.h"
  #include "dynamic_libs/fs_functions.h"
}

class FileReplacer{
   public:
      FileReplacer(std::string path);
	  ~FileReplacer();
	  bool read_dir(const std::string  & path, Directory * dir,int * entries);
	  bool isFileExisting(std::string param);
	  int getSize();
	private:
    Directory * dir_all;
    Directory * dir_content;
    Directory * dir_aoc;

      //FileReplacer(std::string path,std::string content,std::string filename,void * pClient,void * pCmd);
      //std::string getFileListAsString();
      //bool readFromFile(void *pClient, void *pCmd, const std::string & path , Directory* dir);
};
#endif // __FILE_REPLACER_H_
