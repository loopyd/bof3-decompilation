#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146867;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD5A8
 * @behavior Overlay record callback at index 22 of the local callback table
 * D_801FE304 (pointer word at 0x801FE35C), invoked as (record, work) by
 * dispatchRecordCallbackByByte7AScenarioScena0200_801FD084 (0x801FD084); it
 * reads neither argument. It runs the shared front-end startup helper
 * func_8015C088 (0x8015C088), clears the shared progress byte
 * g_ScenarioProgress (0x80146864), clears the shared bytes 0x80146865,
 * 0x80146866 and 0x80146867, clears the shared sub-state byte D_80146875 and
 * then requests primary scene state 0x19 through D_80146874, which selects
 * word 0x19 of the primary-state table D_801FE294 - the pointer 0x801FB6C4 at
 * 0x801FE2F8 - in dispatchPrimaryStateHandlerScenarioScena0200_801F7A60
 * (0x801F7A60). The 0x18-byte frame only keeps $ra across the call, and every
 * byte store materialises the 0x8014 page in $at on its own. The constant
 * 0x19 is materialised in $v0 straight after the call and survives the five
 * clearing stores, so the store order 0x80146864, 0x80146865, 0x80146866,
 * 0x80146867, D_80146875, D_80146874 is the source statement order. Its 84
 * bytes are instruction-for-instruction identical to the exact same-target
 * sibling clearProgressAndSharedStateBytesThenRequestPrimaryState12ScenarioScena0200_801FD490
 * (0x801FD490), differing only in the requested primary-state constant.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedStateBytesThenRequestPrimaryState19ScenarioScena0200_801FD5A8(void) {
  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146875 = 0;
  D_80146874 = 0x19;
}
