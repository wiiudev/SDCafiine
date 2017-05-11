#include <gctypes.h>
#include "retain_vars.h"
char ipFromSd[16] __attribute__((section(".data")));
u8 hasReadIP __attribute__((section(".data"))) = 0;
u8 gSDInitDone __attribute__((section(".data"))) = 0;
FileReplacer * replacer __attribute__((section(".data"))) = NULL;
