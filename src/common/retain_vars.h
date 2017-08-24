#ifndef RETAINS_VARS_H_
#define RETAINS_VARS_H_
#include <gctypes.h>
#include "utils/FileReplacer.h"
#include "patcher/fs_function_patcher.h"
#include "dynamic_libs/fs_defs.h"
<<<<<<< HEAD
=======
extern char ipFromSd[16];
extern u8 hasReadIP;
extern u8 gUsingLibIOSUHAX;
<<<<<<< HEAD
>>>>>>> 358e54a4c1d3c609394a2207e9c380431f3c10e3
=======
>>>>>>> 358e54a4c1d3c609394a2207e9c380431f3c10e3
extern u8 gAppStatus;
extern volatile u8 gSDInitDone;
extern FileReplacer* replacer;
extern char selectedMultiModPackFolder[FS_MAX_ENTNAME_SIZE];
extern struct fs_patcher_utils fspatchervars __attribute__((section(".data")));

#endif // RETAINS_VARS_H_
