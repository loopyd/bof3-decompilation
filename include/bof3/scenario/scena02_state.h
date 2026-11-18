#ifndef BOF3_SCENARIO_SCENA02_STATE_H
#define BOF3_SCENARIO_SCENA02_STATE_H

#include "bof3/bof3.h"

/* Shared progress byte and the sequence of state bytes this body clears after
 * it, then the requested primary-state byte at the end of the run.
 * @source 0x80146864 @kind data
 */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind data */
extern u8 D_80146865;
/* @source 0x80146866 @kind data */
extern u8 D_80146866;
/* @source 0x80146867 @kind data */
extern u8 D_80146867;
/* Shared sub-state byte and the shared primary-state byte after it.
 * @source 0x80146875 @kind data
 */
extern u8 D_80146875;
/* @source 0x80146874 @kind data */
extern u8 D_80146874;

/* Shared EXE-side front-end startup helper. */
void func_8015C088(void);

/* Arguments are integer constants; the expression result is unused. */
#define SCENA02_START_AND_REQUEST(progress, primary_state) \
    (func_8015C088(), \
     g_ScenarioProgress = (progress), \
     D_80146865 = 0, \
     D_80146866 = 0, \
     D_80146867 = 0, \
     D_80146875 = 0, \
     D_80146874 = (primary_state))

#endif
