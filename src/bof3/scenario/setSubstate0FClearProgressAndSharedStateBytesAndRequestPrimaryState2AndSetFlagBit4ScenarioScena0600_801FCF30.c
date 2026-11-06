#include "bof3/bof3.h"

/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x80146866 @kind unknown */
extern u8 D_80146866;
/* @source 0x80146867 @kind unknown */
extern u8 D_80146867;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;

void func_8015C088(void);
void func_8015B580(u32 arg0, s32 arg1);

/* @source 0x801FCF30
 * @behavior Word 4 of the overlay-local record callback table D_801FE42C
 * (0x801FE42C, pointer word at 0x801FE43C), reached as (record, D_8014686C) by
 * dispatchRecordCallbackScenarioScena0600_801FCDD0 (0x801FCDD0); it reads
 * neither argument. It runs the shared front-end startup helper func_8015C088
 * (0x8015C088), requests secondary sub-state 0x0F in the shared sub-state byte
 * D_80146875, clears the shared progress byte g_ScenarioProgress (0x80146864)
 * and the shared bytes 0x80146865, 0x80146866 and 0x80146867, then requests
 * primary scene state 2 through D_80146874 and sets scenario flag bit 4 of the
 * shared flag word D_8014686C through the shared bit-set helper func_8015B580
 * (0x8015B580), which receives the flag word loaded with `lui`/`lw` and the
 * flag index 4. Takes no arguments and returns nothing. The 0x18-byte frame
 * only keeps $ra across the two calls, and every byte store materialises the
 * 0x8014 page in $at on its own. The constant 0x0F is materialised in $v0
 * straight after the helper call and the constant 2 reaches $v0 before the four
 * clearing stores, while the flag-word load is scheduled between that constant
 * and the first clearing store, so the store order D_80146875,
 * g_ScenarioProgress, 0x80146865, 0x80146866, 0x80146867, D_80146874 is the
 * source statement order. No in-image jal targets the address. Its 0x68 bytes
 * keep the sub-state-then-clear-then-request shape of this target's own
 * setSubstate14ClearProgressAndSharedStateBytesAndRequestPrimaryState4ScenarioScena0600_801FD014
 * (0x801FD014), which differs in the sub-state and primary state constants and
 * carries no flag call. The address anchor keeps the name target-qualified.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setSubstate0FClearProgressAndSharedStateBytesAndRequestPrimaryState2AndSetFlagBit4ScenarioScena0600_801FCF30(void) {
  func_8015C088();
  D_80146875 = 0x0F;
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 2;
  func_8015B580(D_8014686C, 4);
}
