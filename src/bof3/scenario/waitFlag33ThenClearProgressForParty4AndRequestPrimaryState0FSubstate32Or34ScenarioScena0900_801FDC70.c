#include "bof3/bof3.h"

/* @source 0x8014686C @kind unknown */
extern u32 D_8014686C;
/* @source 0x80146874 @kind unknown */
extern u8 D_80146874;
/* @source 0x80146875 @kind unknown */
extern u8 D_80146875;
/* @source 0x80146864 @kind unknown */
extern u8 g_ScenarioProgress;
/* @source 0x80145F09 @kind unknown — party byte (+0x79) of the 0x140-byte
 * stride local work records based at 0x80145E90, so this reads record 0. */
extern u8 D_80145F09;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FDC70
 * @behavior Overlay handler reached only indirectly: the address is word 13
 * (the word stored at 0x801FE4F0) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that dispatchCoordinateLookupHandlerScenarioScena0900_801FD280
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 0x0E) and calls with jalr, and that dispatcher returns this
 * handler's signed low byte, so no in-image jal targets this address. It first
 * takes the two-stage wait func_8015B5D4 on the shared flag word D_8014686C
 * (0x8014686C, read with lui/lw into $a0) for wait id 0x33; a non-zero return
 * means the wait is still running and the handler answers -1, materialised as
 * addiu $v0,$zero,-1 in the bnez delay slot, which is the early return to the
 * shared epilogue. A zero return means the wait is done, so it branches on the
 * party byte of local work record 0 (D_80145F09, 0x80145F09): when that byte
 * holds 4 it first clears the shared progress byte g_ScenarioProgress
 * (0x80146864), then runs the shared front-end startup helper func_8015C088
 * and arms sub-state 0x32; on every other party it runs func_8015C088 and arms
 * sub-state 0x34 instead. Both paths then store that sub-state in the shared
 * sub-state byte D_80146875 and request primary scene state 0x0F in the shared
 * state byte D_80146874, answering 1. It takes no arguments, reads no state
 * beyond the wait word and that one byte, and its 0x18-byte frame only keeps
 * $ra across the calls; each of the three byte stores materialises the 0x8014
 * page in $at on its own and the wait's constant argument is materialised in
 * the jal delay slot. A bounded scan of the generated overlay asm found no
 * other block combining wait id 0x33 with this 0x32/0x34 sub-state pair, so no
 * clone candidate exists; the neighbouring table words 11 and 14 to 22
 * (0x801FDBA8 waiting on id 0x3D and the armEffectGroupThenInvokeThreeHelpers
 * run) keep only the wait-then-request shape, which is shape evidence and not
 * clone proof.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* Matching note: the original falls into the party-4 arm and branches to the
 * 0x34 arm, so the guard here is the negated test with the 0x34 arm spelled as
 * the trailing block (measured 33/33 at 132 bytes). The two sub-state-local
 * forms built to 128 bytes and diverged at the first wait branch (25/33 for a
 * `!= 4` guard, 28/33 for `== 4` if/else), and `== 4` with the tail written as
 * a trailing block kept the 132-byte size but inverted the arm order (27/33,
 * the party test already differing at 0x2C). */
s32 waitFlag33ThenClearProgressForParty4AndRequestPrimaryState0FSubstate32Or34ScenarioScena0900_801FDC70(void) {
  if (func_8015B5D4(D_8014686C, 0x33) != 0) {
    return -1;
  }

  if (D_80145F09 != 4) {
    func_8015C088();
    D_80146875 = 0x34;
    D_80146874 = 0x0F;
    return 1;
  }

  g_ScenarioProgress = 0;
  func_8015C088();
  D_80146875 = 0x32;
  D_80146874 = 0x0F;
  return 1;
}
