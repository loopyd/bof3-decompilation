#include "bof3/bof3.h"

extern u8 g_ScenarioProgress;
extern u8 D_80146865;
extern u8 D_80146866;
extern u8 D_80146874;
extern u8 D_80146875;

void func_8015C088(void);

/* @source 0x801FD2E0
 * @behavior Overlay record callback at index 11 of the local callback table
 * D_801FE304 (pointer word at 0x801FE330), invoked as (record, work) by
 * dispatchRecordCallbackByByte7AScenarioScena0200_801FD084; it reads neither
 * argument. It runs the shared front-end startup helper func_8015C088
 * (0x8015C088), sets the shared progress byte g_ScenarioProgress (0x80146864)
 * to 1, clears the shared bytes 0x80146865 and 0x80146866, clears the shared
 * sub-state byte D_80146875 and requests primary scene state 7 through
 * D_80146874. The 0x18-byte frame only keeps $ra across the call, and every
 * byte store materialises the 0x8014 page in $at on its own. The constant 1 is
 * materialised in $v0 straight after the call and the constant 7 in the same
 * register before the three clearing stores, so the store order 0x80146864,
 * 0x80146865, 0x80146866, D_80146875, D_80146874 is the source statement
 * order. Its 80 bytes are the exact neighbours'
 * clearSharedBytes6566AndRequestPrimaryState0BScenarioScena0200_801FD330
 * (0x801FD330) and
 * clearSharedBytes6566AndRequestPrimaryState0FScenarioScena0200_801FD40C
 * (0x801FD40C) shape plus the progress store; those siblings differ only in
 * the stored state constant. The address anchor keeps the name
 * target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setProgress1ClearSharedBytes6566AndRequestPrimaryState7ScenarioScena0200_801FD2E0(void) {
  func_8015C088();
  g_ScenarioProgress = 1;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146875 = 0;
  D_80146874 = 7;
}
