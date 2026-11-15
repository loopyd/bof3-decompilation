#ifndef BOF3_SCENARIO_SCENA06_STATE_H
#define BOF3_SCENARIO_SCENA06_STATE_H

#include "bof3/bof3.h"

/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;

void func_8015C088(void);

/* primary_state is one simple integer constant; the expression result is unused. */
#define SCENA06_RESET_PROGRESS_AND_REQUEST(primary_state) \
    (func_8015C088(), \
     g_ScenarioProgress = 0, \
     D_80146865 = 0, \
     D_80146866 = 0, \
     D_80146867 = 0, \
     D_80146875 = 0, \
     D_80146874 = (primary_state))

#endif
