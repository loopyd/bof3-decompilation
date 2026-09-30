#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA8F4
 * @behavior Drains the queued UI ring entry at the consumer index: when the entry's
 * flag byte has bit 0 set and the gate halfword 0x80145AA8 is nonzero, and
 * advanceUiRingCheck() reports that the consumer index has reached the producer,
 * it sets panel state byte +3 to 3; when the flag byte has bit 1 set it decrements
 * the entry's countdown byte +1 and, once that byte reaches zero, performs the same
 * advance-and-raise test.
 * @status partial
 * @match 96.97
 * @residual 64/66 instructions, 264/264 bytes, live byte-match DIFFER; first mismatch
 * +0x009C is the placement of the compare constant: the original holds `li v0,255` in the
 * entry-branch delay slot with a `nop` in the `lbu` load-delay slot, this candidate emits
 * the `nop` in the branch delay slot and `li v0,255` in the load-delay slot; the
 * 66-instruction multiset and every register/operand are otherwise identical. Decisive
 * clean-C lever: keeping the loaded byte alive and deferring the decrement to its own
 * local (`next = count - 1;`) instead of the in-place `count = count - 1;` form
 * (54/66 -> 64/66). Tried and rejected with measured deltas: duplicated `count - 1`
 * expression (54/66), `--count` (54/66), constant kept in a `u8` local as a body
 * assignment / block-local initializer / function-scope initializer (64/66 / 64/66 /
 * 50/66), masked word locals (53/67, 64/66), unsigned literal compares (64/66), and
 * computing the decrement before the compare (56/67). Smallest missing evidence: a
 * clean-C statement order that materializes the compare constant before the `lbu` so the
 * delay-slot filler can sink it into the entry branch slot; profile/permuter rungs were
 * not authorized for this mission.
 */
void func_801EA8F4(void) {
  u8 head;
  u8 count;
  u8 next;

  func_801EAAB8();
  head = uiRingHead;
  if ((uiRingEntries[head].unk_00 & 1) != 0) {
    if (D_80145AA8 != 0) {
      if ((advanceUiRingCheck() & 0xff) != 0) {
        D_80148648[3] = 3;
      }
    }
  }
  head = uiRingHead;
  if ((uiRingEntries[head].unk_00 & 2) != 0) {
    count = uiRingEntries[head].unk_01;
    if ((count & 0xff) != 0xff) {
      next = count - 1;
      uiRingEntries[head].unk_01 = next;
      if ((next & 0xff) == 0) {
        if ((advanceUiRingCheck() & 0xff) != 0) {
          D_80148648[3] = 3;
        }
      }
    }
  }
}
