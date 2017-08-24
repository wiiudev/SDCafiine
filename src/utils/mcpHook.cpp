#include "mcpHook.h"
#include <iosuhax.h>
#include "dynamic_libs/os_functions.h"

//just to be able to call async
void someFunc(void *arg){
    (void)arg;
}

static int mcp_hook_fd  __attribute__((section(".data"))) = -1;
int MCPHookOpen(){
    //take over mcp thread
    mcp_hook_fd = MCP_Open();
    if(mcp_hook_fd < 0)
        return -1;
    IOS_IoctlAsync(mcp_hook_fd, 0x62, (void*)0, 0, (void*)0, 0, (void*)someFunc, (void*)0);
    //let wupserver start up
    os_sleep(1);
    if(IOSUHAX_Open("/dev/mcp") < 0)
    {
        MCP_Close(mcp_hook_fd);
        mcp_hook_fd = -1;
        return -1;
    }
    return 0;
}

void MCPHookClose(){
    if(mcp_hook_fd < 0)
        return;
    //close down wupserver, return control to mcp
    IOSUHAX_Close();
    //wait for mcp to return
    os_sleep(1);
    MCP_Close(mcp_hook_fd);
    mcp_hook_fd = -1;
}
