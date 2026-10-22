#include "bof3/battle/battle03_internal.h"

/* @source 0x801EA8F4
 * @behavior Drains the queued UI ring entry at the consumer index: when the entry's
 * flag byte has bit 0 set and the gate halfword 0x80145AA8 is nonzero, and
 * advanceUiRingCheck() reports that the consumer index has reached the producer,
 * it sets panel state byte +3 to 3; when the flag byte has bit 1 set it decrements
 * the entry's countdown byte +1 and, once that byte reaches zero, performs the same
 * advance-and-raise test.
 * @status exact
 * @match 100.00
 * @residual none
 * The countdown read is a deliberate plain view of the volatile ring cell: the
 * original holds `li v0,255` in the entry branch delay slot and the load-delay
 * `nop` after the `lbu`, while a volatile read schedules the constant after the
 * load instead. The store, the ring head and both flag loads keep their volatile
 * declarations.
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
    /* Plain view of the volatile ring cell: the original schedule hoists the
     * compare constant above the count load into the entry branch delay slot;
     * a volatile read schedules the constant after the load instead. */
    count = *(u8 *)&uiRingEntries[head].unk_01;
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
