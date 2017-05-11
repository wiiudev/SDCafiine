#include "FileReplacer.h"
#include "utils/logger.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <malloc.h>
#include "common/common.h"
#include "fs/fs_utils.c"

/*
FileReplacer::FileReplacer(std::string path,std::string filename,void * pClient,void * pCmd){
	bool result = false;
	dir_all = new Directory("content");
    std::string filepath = path + std::string("/") + filename;
    log_printf("Read from file for replacement: %s\n",filename.c_str());
    result = this->readFromFile(pClient,pCmd,filepath,dir_all);
    if(!result){
        log_print("Error!\n");
    }else{
        //dir_all->printFolderRecursive("");
    }
}*/

FileReplacer::FileReplacer(std::string path){
    int entries = 0;

    dir_all = new Directory("");
    read_dir(path, dir_all, &entries);
    log_printf("Found %d entries\n",entries);
    dir_all->printFolderRecursive("");
}

std::string FileReplacer::getFileListAsString(){
	return dir_all->getFileList();
}

int FileReplacer::getSize()
{
	int size = sizeof(FileReplacer);
	size += dir_all->getSize();
	return size;
}

FileReplacer::~FileReplacer()
{
	delete dir_all;
}

/* with untested changes
bool FileReplacer::readFromFile(void *pClient, void *pCmd, const std::string & path , Directory* dir){
	s32 handle = 0;
	FSStat stats;
	s32 ret = -1;

    CFile file(filepath, CFile::ReadOnly);
    if (!file.isOpen()){
        log_printf("File not found: %s\n",filepath.c_str());
        return;
    }

    std::string strBuffer;
    strBuffer.resize(file.size());
    file.read((u8 *) &strBuffer[0], strBuffer.size());

    Directory * dir_cur = dir;
    char * ptr;
    char delimiter[] = { '\n', '\r' };
    ptr = strtok (strBuffer,delimiter);
    while (ptr != NULL)
    {
        std::string dirname(ptr);
        if(dirname.compare(PARENT_DIR_IDENTIFY) == 0){ // back dir
            dir_cur = dir_cur->getParent();
            if(dir_cur == NULL){
                log_print("Something went wrong. Try to delete the filelist.txt\n");

                return false;
            }
        }else if(dirname.substr(0,1).compare(DIR_IDENTIFY) == 0){ //DIR
            dirname = dirname.substr(1);
            Directory * dir_new = new Directory(dirname);
            dir_cur->addFolder(dir_new);
            dir_new->setParent(dir_cur);
            dir_cur = dir_new;
        }else {
            dir_cur->addFile(dirname);
        }
        ptr = strtok(NULL, delimiter);
    }
    return true;
}*/

int FileReplacer::read_dir(const std::string & path , Directory* dir, int * entries){
	std::vector<std::string *> dirlist;

    struct dirent *dirent = NULL;
	DIR *dir_ = opendir(path.c_str());
	if (dir_ == NULL){
        log_printf("dir not found..%s\n",path.c_str());
		return -1;
	}
	while ((dirent = readdir(dir_)) != 0){
		bool isDir = dirent->d_type & DT_DIR;
		const char *filename = dirent->d_name;

        if(isDir){
            dirlist.push_back(new std::string(filename));
        }else{
            (*entries)++;
            dir->addFile(filename);
        }
    }
    closedir(dir_);

    for(u32 i = 0; i < dirlist.size(); i++){
        Directory * dir_new = new Directory(*dirlist[i]);
        dir->addFolder(dir_new);
        read_dir((path + "/"+ *dirlist[i]),dir_new,entries);
    }

    for(u32 i = 0; i < dirlist.size(); i++){
        delete dirlist[i];
    }

	return 0;
}

int FileReplacer::isFileExisting(std::string param){
    log_printf("Checking if %s exists.\n",param.c_str());
	if(dir_all != 0){
		if(dir_all->isInFolder(param)){
			return 0;
		}
	}
	return -1;
}
