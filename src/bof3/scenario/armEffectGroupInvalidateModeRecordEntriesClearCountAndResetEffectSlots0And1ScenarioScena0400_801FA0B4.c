#include "bof3/bof3.h"

extern u8 D_80144F5A;
extern u8 D_80144F5B;
extern u8 D_80144F5C;
extern u8 D_80144F5D;
extern u8 D_80144F5E;
extern u8 D_80144F5F;
extern u8 D_80146254;

void func_80166E88(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_801C187C(s32 arg0);

/* @source 0x801FA0B4
 * @behavior Entry 1 of the four-word primary-state request handler run at
 * 0x801FA230 (0x801FA074, 0x801FA0B4, 0x801FA12C, 0x801FA178) that
 * dispatchRoute30CoordinateLookupHandlerScenarioScena0400_801FA010
 * (0x801FA010) indexes with the sign-extended byte func_801A7AD8(&D_801FA228,
 * 1, ...) returns; no in-image `jal` targets it and its only image word is
 * 0x801FA0B4 at 0x801FA234. It first re-arms the overlay effect group for the
 * ids 0, 1 and 5 through func_80166E88(0, 1, 5, 0) - a0 zeroed, a1=1 and a2=5
 * materialised before the jal and a3 zeroed in that call's delay slot - then
 * writes 0xFF into the six consecutive shared bytes D_80144F5A through
 * D_80144F5F, both three-byte entries of the per-mode record run at 0x80144F5A
 * that the shop and game targets index as D_80144F5A[], clears the shared
 * live work-record count byte D_80146254, and finally runs the per-id effect
 * teardown func_801C187C for the ids 0 and 1 in that order. It reads no memory
 * of its own, takes no arguments, consumes no result and nothing sets $v0; the
 * 0x18-byte frame only keeps $ra at 0x10 across the three calls, the single
 * 0xFF is materialised once in $v0 and reused by all six stores, each of those
 * stores materialises the 0x8014 page in $at on its own, the zero store uses
 * $zero directly, and each teardown id is materialised in $a0 inside its own
 * jal delay slot. Its immediate neighbour 0x801FA12C is the same
 * arm-clear-teardown shape without the six-byte invalidation, and the family
 * spelling - arm the group, clear the count, reset the slots - came from the
 * sibling armEffectGroupThenClearWorkRecordsAndResetEffectSlotsScenarioScena1400_801FBD2C
 * (0x801FBD2C, recorded exact), whose body differs in the arm ids, in the ids
 * it resets and in the missing mode-record run; no extracted payload holds a
 * byte-identical copy of this 120-byte span.
 * @status exact
 * @match 100.00
 * @residual none
 */
void armEffectGroupInvalidateModeRecordEntriesClearCountAndResetEffectSlots0And1ScenarioScena0400_801FA0B4(void) {
  func_80166E88(0, 1, 5, 0);
  D_80144F5A = 0xFF;
  D_80144F5B = 0xFF;
  D_80144F5C = 0xFF;
  D_80144F5D = 0xFF;
  D_80144F5E = 0xFF;
  D_80144F5F = 0xFF;
  D_80146254 = 0;
  func_801C187C(0);
  func_801C187C(1);
}
