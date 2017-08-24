#ifndef _MAIN_H_
#define _MAIN_H_

#include "common/types.h"
#include "dynamic_libs/os_functions.h"

/* Main */
#ifdef __cplusplus
extern "C" {
#endif

//! C wrapper for our C++ functions
int Menu_Main(void);
void deInit(void);
void ApplyPatches(void);
void RestorePatches();
void Init_SD(void);

s32 isInMiiMakerHBL();

#ifdef __cplusplus
}
#endif

#endif
