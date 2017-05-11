#include <string>
#include "Directory.h"
#include <sys/dirent.h>

#ifndef __FILE_REPLACER_H_
#define __FILE_REPLACER_H_

extern "C" {
  #include "fs/sd_fat_devoptab.h"
  #include "dynamic_libs/fs_functions.h"
}

class FileReplacer
{
   public:
	  //FileReplacer(std::string path,std::string content,std::string filename,void * pClient,void * pCmd);
      FileReplacer(std::string path);
	  ~FileReplacer();
	  bool readFromFile(void *pClient, void *pCmd, const std::string & path , Directory* dir);
	  int read_dir(const std::string  & path, Directory * dir,int * entries);
	  int isFileExisting(std::string param);
	  int getSize();
	  std::string getFileListAsString();
	private:
	  Directory * dir_all;
};
#endif // __FILE_REPLACER_H_
