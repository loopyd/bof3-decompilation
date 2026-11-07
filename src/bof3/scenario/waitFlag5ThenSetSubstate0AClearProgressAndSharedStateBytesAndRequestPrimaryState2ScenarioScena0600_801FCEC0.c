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
s32 func_8015B5D4(u32 arg0, s32 arg1);

/* @source 0x801FCEC0
 * @behavior Word 3 of the overlay-local per-record callback table D_801FE42C
 * (0x801FE42C, word at 0x801FE438), reached only through
 * dispatchRecordCallbackScenarioScena0600_801FCDD0 (0x801FCDD0) as
 * (record, D_8014686C); it reads neither argument and no in-image jal targets
 * the address. It first takes the two-stage wait func_8015B5D4 (0x8015B5D4) on
 * the shared scenario flag word D_8014686C (0x8014686C, read with lui/lw into
 * $a0 above the frame's $ra save) for flag 0x05, which is materialised in the
 * jal delay slot, and returns immediately when that wait reads non-zero. A zero
 * wait result runs the shared front-end startup helper func_8015C088
 * (0x8015C088), arms the shared sub-state byte D_80146875 (0x80146875) with
 * 0x0A, clears the shared progress byte g_ScenarioProgress (0x80146864) and the
 * shared bytes D_80146865..D_80146867 (0x80146865..0x80146867) in that order,
 * then requests primary scene state 0x02 in the shared state byte D_80146874
 * (0x80146874). Takes no arguments and returns nothing. Its 0x18-byte frame
 * only keeps $ra across the two calls, and each byte store materialises the
 * 0x8014 page in $at on its own. The two table neighbours
 * clearProgressAndSharedStateBytesThenRequestPrimaryState2ScenarioScena0600_801FCE6C
 * (word 2) and
 * clearProgressAndSharedStateBytesThenRequestPrimaryState4ScenarioScena0600_801FCFC0
 * (word 6) keep the same clear-then-request shape for primary states 0x02 and
 * 0x04 but run the helper unconditionally and arm D_80146875 with zero, so the
 * flag-0x05 wait and the 0x0A sub-state are this word's own shape and the
 * neighbours are shape evidence, not clone proof.
 * @status exact
 * @match 100.00
 * @residual none
 */
void waitFlag5ThenSetSubstate0AClearProgressAndSharedStateBytesAndRequestPrimaryState2ScenarioScena0600_801FCEC0(void) {
  if (func_8015B5D4(D_8014686C, 0x5) != 0) {
    return;
  }
  func_8015C088();
  D_80146875 = 0xA;
  g_ScenarioProgress = 0;
  D_80146865 = 0;
  D_80146866 = 0;
  D_80146867 = 0;
  D_80146874 = 2;
}
