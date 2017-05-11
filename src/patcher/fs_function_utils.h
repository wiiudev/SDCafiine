#ifndef _FS_FUNCTION_UTILS_H
#define _FS_FUNCTION_UTILS_H


#ifdef __cplusplus
extern "C" {
#endif

int client_num_alloc(void *pClient);
void client_num_free(int client);
int client_num(void *pClient);
int getClientAndInitSD(void *pClient, void *pCmd);
int is_gamefile(const char *path);
char * getNewPath(void *pClient, void *pCmd, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* _FS_FUNCTION_PATCHER_H */
