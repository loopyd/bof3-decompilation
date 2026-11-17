#ifndef BOF3_SCENARIO_SCENA09_STATE_H
#define BOF3_SCENARIO_SCENA09_STATE_H

#include "bof3/bof3.h"

/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;

void func_8015C088(void);

/* substate is one simple integer constant; the expression result is unused. */
#define SCENA09_START_AND_REQUEST_STATE6(substate) \
    (func_8015C088(), \
     D_80146875 = (substate), \
     D_80146874 = 6)

#endif
