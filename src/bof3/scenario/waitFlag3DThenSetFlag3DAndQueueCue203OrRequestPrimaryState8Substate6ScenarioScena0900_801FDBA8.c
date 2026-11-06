#include "bof3/bof3.h"

extern u32 D_8014686C;
extern u8 D_80146874;
extern u8 D_80146875;

s32 func_8015B5D4(u32 arg0, s32 arg1);
void func_8015B580(u32 arg0, s32 arg1);
void func_8015C088(void);
void func_8015DF18(u16 arg0);

/* @source 0x801FDBA8
 * @behavior Overlay handler reached only indirectly: the address is word 11
 * (the word stored at 0x801FE4E8) of the 38-entry in-image code-pointer table
 * at 0x801FE4BC that dispatchCoordinateLookupHandlerScenarioScena0900_801FD280
 * (0x801FD280) indexes with the signed byte returned by func_801A7AD8
 * (0x801FE474, 14) and calls with jalr, and that dispatcher returns this
 * handler's signed low byte, so no in-image jal targets this address. It first
 * takes the two-stage wait func_8015B5D4 on the shared flag word D_8014686C
 * (0x8014686C, read with lui/lw into $a0) for wait id 0x3D. A non-zero return
 * runs the shared front-end startup helper func_8015C088 and then requests
 * primary scene state 8 with secondary sub-state 6, storing 6 in the shared
 * sub-state byte D_80146875 and then 8 in the shared state byte D_80146874 and
 * answering 1; both constants are materialised in $v1 and the answer 1 fills
 * the j delay slot of the jump to the shared epilogue. A zero return arms that
 * same wait instead: it sets flag bit 0x3D of the shared word through
 * func_8015B580(D_8014686C, 0x3D), queues the front-end cue 0x203 through
 * func_8015DF18(0x203) and answers 1 by falling into the epilogue, without
 * reading or writing the state or sub-state byte. It takes no arguments, reads
 * no other state and writes no other byte, and its 0x18-byte frame only keeps
 * $ra across the calls; the wait's constant argument is materialised in the jal
 * delay slot and each byte store materialises the 0x8014 page in $at on its
 * own. The arm taken on a non-zero return is the state-8 request, the opposite
 * polarity to the next table word
 * waitFlag2CThenRequestPrimaryState9Substate14ScenarioScena0900_801FDC1C
 * (0x801FDC1C), which answers -1 while func_8015B5D4 still returns non-zero.
 * Its neighbours keep the same wait-then-request shape with their own wait ids
 * and stored constants: words 9 and 10 (0x801FDA90, 0x801FDB1C) test the same
 * id 0x3D and arm state 8 with sub-states 3 and 5 or 4 and 5. That is shape
 * evidence only; no shipped overlay payload holds a byte-identical copy of its
 * 116 bytes, so no clone candidate exists. The zero-return arm is the guarded
 * early return and the state request follows it: writing the request first as
 * `if (wait != 0) { request; return 1; }` makes the psyq compiler invert the
 * test to bnez, place the request out of line and hoist its constant 1 ahead of
 * the two stores (measured 19/29 instructions), while this order keeps the beqz
 * on the zero arm, the request as the fall-through and its 1 in the j delay slot.
 * @status exact
 * @match 100.00
 * @residual none
 */
s32 waitFlag3DThenSetFlag3DAndQueueCue203OrRequestPrimaryState8Substate6ScenarioScena0900_801FDBA8(void) {
  if (func_8015B5D4(D_8014686C, 0x3D) == 0) {
    func_8015B580(D_8014686C, 0x3D);
    func_8015DF18(0x203);
    return 1;
  }

  func_8015C088();
  D_80146875 = 6;
  D_80146874 = 8;
  return 1;
}
