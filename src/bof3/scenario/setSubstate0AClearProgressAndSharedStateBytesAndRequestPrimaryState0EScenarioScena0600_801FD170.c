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

/* @source 0x801FD170
 * @behavior Word 11 of the overlay-local record callback table D_801FE42C
 * (0x801FE42C, pointer word at 0x801FE458), reached as (record, D_8014686C) by
 * dispatchRecordCallbackScenarioScena0600_801FCDD0 (0x801FCDD0); it reads
 * neither argument. It runs the shared front-end startup helper func_8015C088
 * (0x8015C088), requests secondary sub-state 0xA in the shared sub-state byte
 * D_80146875, clears the shared progress byte g_ScenarioProgress (0x80146864)
 * and the shared bytes 0x80146865, 0x80146866 and 0x80146867, then requests
 * primary scene state 0xE through D_80146874. Takes no arguments and returns
 * nothing. The 0x18-byte frame only keeps $ra across the call, and every byte
 * store materialises the 0x8014 page in $at on its own, which is how the psyq
 * compiler emits separate shared-byte stores. The sub-state constant is
 * materialised in $v0 straight after the call and the primary-state constant
 * reaches $v0 before the four clearing stores, so the store order D_80146875,
 * g_ScenarioProgress, 0x80146865, 0x80146866, 0x80146867, D_80146874 is the
 * source statement order. No in-image jal targets the address. Its 0x58 bytes
 * are instruction-for-instruction identical to the exact sibling
 * setSubstate14ClearProgressAndSharedStateBytesAndRequestPrimaryState4ScenarioScena0600_801FD014
 * (0x801FD014 of this target), which differs only in the two stored constants.
 * The address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate0AClearProgressAndSharedStateBytesAndRequestPrimaryState0EScenarioScena0600_801FD170(void) {
  func_8015C088();
  D_80146875 = 0xA;
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 0xE;
}
