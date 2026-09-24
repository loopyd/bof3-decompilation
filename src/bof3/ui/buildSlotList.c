#include "bof3/ui/shop00_internal.h"

/* The row-offset mask is spelled through an unsigned operand: the masked value
 * is passed as a u16, and the signed spelling of `& 0xFFFE` makes gcc emit the
 * mask as an `and` against a register-held -2 (plus a redundant andi) instead
 * of the original's single `andi 0xFFFE`. The slot record offset is bound to a
 * local before the table read, which is what keeps the two table bases folded
 * into `lui at / addu at / lbu %lo(sym)(at)` instead of being hoisted into
 * callee-saved registers by the loop optimizer. */
/* @source 0x801D8050
 * @behavior shop slot-list builder reached from the panel phase steps
 *           func_801D7B74/func_801D7CC8/func_801D7D48 with the constant 0: its
 *           byte argument only selects whether the frame timer phaseTimer
 *           supplies the phase offset (nonzero) or the offset is zero. It emits
 *           the shop panel strip through the panel strip emitter func_801DAB90
 *           with the panel x constant 0x14, the phase-relative second argument
 *           (0x10 - phaseTimer * 10) masked with 0xFFFE, the height constant
 *           0x118, the constant 0x13 and the main-RAM CLUT-bank byte
 *           D_80144952 as the fifth argument; then walks the slots while the
 *           byte result of the mode scanner func_801BDB7C(0) has not been
 *           reached, reading the leading byte of the 0x140-byte-stride record
 *           D_80145FCC for each slot and drawing it through the field-list
 *           emitter func_801D826C with the even row offset
 *           (slot * 54 + 0x3E) & 0xFFFE, the record byte, the nonzero state of
 *           the bit-mask byte D_80144981[164 * record] and the constant 0;
 *           finally draws the shop panel through func_801DA2F4 at
 *           phaseTimer * 32 + 0xB4 with the y constant 0x26, the constant 0 and
 *           the main-RAM value D_80144F50.
 * @status partial
 * @match 96.63
 * @residual entry-block scheduling permutation: this candidate reshuffles only
 *           the first basic block (sw s2,0x20(sp) and the timer reset ahead of
 *           andi a0,a0,0xFF, sw s0,0x18(sp) deferred into the branch delay
 *           slot) where the original keeps the narrowing and the save order
 *           ra, s2, s1, s0 and puts the timer reset in the delay slot; the
 *           instruction multiset and every instruction from the first emitters
 *           on match.
 */
void buildSlotList(u8 arg0) {
  u8 timer = 0;
  u8 slot;

  if (arg0 != 0) {
    timer = phaseTimer;
  }
  func_801DAB90(0x14, (0x10 - timer * 10) & 0xFFFE, 0x118, 0x13, D_80144952);
  for (slot = 0; slot < func_801BDB7C(0); slot++) {
    s32 offset;
    s32 record_offset;
    u8 record;

    offset = slot * 0x140;
    record = D_80145FCC[offset];
    record_offset = record * 164;
    if (D_80144981[record_offset] != 0) {
      func_801D826C((u16)(0x11 - timer * 32), (u32)(slot * 54 + 0x3E) & 0xFFFE,
                    record, 1, 0);
    } else {
      func_801D826C((u16)(0x11 - timer * 32), (u32)(slot * 54 + 0x3E) & 0xFFFE,
                    record, 0, 0);
    }
  }
  func_801DA2F4(timer * 32 + 0xB4, 0x26, 0, D_80144F50);
}
