#include "bof3/bof3.h"

/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80146865 @kind unknown */
extern u8 D_80146865;
/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80146874 @kind unknown */
extern s8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;

void func_8015C088(void);
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801FD1C8
 * @behavior Word 12 of the overlay-local per-record callback table D_801FE42C
 * (0x801FE42C, word at 0x801FE45C), reached only through
 * dispatchRecordCallbackScenarioScena0600_801FCDD0 (0x801FCDD0) as
 * (record, D_8014686C); it reads neither argument and no in-image jal targets
 * the address. It runs the shared front-end startup helper func_8015C088
 * (0x8015C088), clears the shared progress byte g_ScenarioProgress (0x80146864)
 * and the shared byte D_80146865 (0x80146865) in that order, then takes the
 * two-stage wait func_8015B5D4 (0x8015B5D4) on the shared scenario flag word
 * D_8014686C (0x8014686C, read with lui/lw into $a0) for flag 0x22, which is
 * materialised in the jal delay slot. A zero wait result leaves the shared
 * sub-state byte D_80146875 (0x80146875) at zero and a non-zero one arms 0x14,
 * and the function then requests primary scene state 0x10 in the shared state
 * byte D_80146874 (0x80146874). Takes no arguments and returns nothing. Its
 * 0x18-byte frame only keeps $ra across the two calls, the flag-word load is
 * scheduled above the two clearing byte stores, and each byte store
 * materialises the 0x8014 page in $at on its own. The following table word
 * clearProgressAndSharedByteThenWaitFlag23Substate0AOr14AndRequestPrimaryState10ScenarioScena0600_801FD234
 * (0x801FD234) keeps the same clear-then-wait shape with flag 0x23 and selects a
 * non-zero sub-state 0x0A for its zero-return arm, so it is shape evidence and
 * not clone proof; 28 of its 96 bytes differ.
 * @status exact
 * @match 100.00
 * @residual none
 */
void clearProgressAndSharedByteThenWaitFlag22SubstateZeroOr14AndRequestPrimaryState10ScenarioScena0600_801FD1C8(void) {
  s32 substate;

  func_8015C088();
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  substate = func_8015B5D4(D_8014686C, 0x22);
  if (substate == 0) {
    D_80146875 = 0;
  } else {
    D_80146875 = 0x14;
  }
  D_80146874 = 0x10;
}
