#ifndef RETAINS_VARS_H_
#define RETAINS_VARS_H_
#include <gctypes.h>
#include "utils/FileReplacer.h"
extern char ipFromSd[16];
extern u8 hasReadIP;
extern volatile u8 gSDInitDone;
extern FileReplacer* replacer;
#endif // RETAINS_VARS_H_
