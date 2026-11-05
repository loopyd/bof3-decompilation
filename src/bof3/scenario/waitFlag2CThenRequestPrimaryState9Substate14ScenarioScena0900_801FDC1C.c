#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015C088(void);

/* @source 0x801FDC1C
 * @behavior Overlay handler reached only indirectly: the address is word 12
 * (the word stored at 0x801FE4EC) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that dispatchCoordinateLookupHandlerScenarioScena0900_801FD280
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 14) and calls with jalr, and that dispatcher returns this
 * handler's signed low byte, so no in-image jal targets this address. It first
 * takes the two-stage wait func_8015B5D4 on the shared flag word D_8014686C
 * (0x8014686C, read with lui/lw into $a0) for wait id 0x2C: a non-zero return
 * means the wait is still running and the handler answers -1, materialised as
 * addiu $v0,$zero,-1 in the bnez delay slot, which is the early return jumping
 * to the shared epilogue. A zero return means the wait is done, so it runs the
 * shared front-end startup helper func_8015C088 and then requests primary scene
 * state 9 with secondary sub-state 0x14 by storing 0x14 in the shared sub-state
 * byte D_80146875 and then 9 in the shared state byte D_80146874, answering 1.
 * It takes no arguments, reads no other state and writes no other byte; the
 * 0x18-byte frame only keeps $ra across the calls, the wait's constant argument
 * is materialised in the jal delay slot, and each of the two byte stores
 * materialises the 0x8014 page in $at on its own. No shipped overlay payload
 * holds a byte-identical copy of its 84 bytes; the two neighbouring table words
 * 11 and 13 (0x801FDBA8 waiting on id 0x3D and 0x801FDC70 waiting on id 0x33)
 * show the same wait-then-request shape with their own constants, which is shape
 * evidence only. The wait arm returns before the request block instead of
 * carrying a result local across the calls: a single `result = -1` local forces
 * the psyq compiler to hold it in $s0 (measured sw $s0,16($sp), 96 bytes, 11/21
 * instructions matching), while the two-return form matches all 21.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitFlag2CThenRequestPrimaryState9Substate14ScenarioScena0900_801FDC1C(void) {
  if (func_8015B5D4(D_8014686C, 0x2C) == 0) {
    func_8015C088();
    D_80146875 = 0x14;
    D_80146874 = 9;
    return 1;
  }

  return -1;
}
