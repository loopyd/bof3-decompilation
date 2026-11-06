#include "bof3/ui/shop00_internal.h"

/* @source 0x801DEE64
 * @behavior shop slot-list renderer called with no arguments by the phase step
 *           func_801DE45C: it reads the visible-list length through
 *           func_801BDB7C(0) and, while the slot index stays below it, draws
 *           each visible slot through the field-list emitter func_801D826C
 *           with the panel-relative x (0x11 - phaseTimer * 40) masked to
 *           0xFFFF, the even row offset (slot * 54 + 0x3E) & 0xFFFE, the
 *           field-record index D_80181B10[D_80144F5A[slot]] and bit 1 of that
 *           record's flag byte D_8014496F as the fourth argument; it then
 *           formats the one-based slot ordinal through sprintf into the shared
 *           text buffer D_80145AD4 with the EMI-local "%d" format string
 *           D_801D0E60 and draws it through the main-exe text emitter
 *           func_80150098 at (0x1A - phaseTimer * 40) by the sign-extended row
 *           y slot * 54 + 0x43 with the alignment constant 0. Afterwards it
 *           emits the slidable off-screen slot region through func_801DF00C at
 *           (0x96 + phaseTimer * 45) masked to 0xFFFF with the y constant 0x3B
 *           and the shop panel strip through func_801DAB90 with the fixed panel
 *           arguments 0x14, (0x12 - phaseTimer * 8) & 0xFFFE, 0x118 and 0x13
 *           plus the main-RAM CLUT-bank byte D_80144952.
 * @status exact
 * @match 100.00
 * @residual none
 */
/* The frame timer is volatile, so every read must survive as its own `lbu`,
 * while the original holds one materialized address across the calls of the
 * slot loop and materializes a second one after the loop for the two tail
 * reads: two pointer groups, each with its own variable, which is why the
 * second group is not folded into the first. The `slot = 0` initializer sits
 * before the entry guard, so the entry branch keeps it in its delay slot; the
 * guard itself is required because the visible count is an unrestricted u8 and
 * the do/while body runs at least once for every nonzero count. */
void drawNumberedShopSlotListThenPanelStrip(void) {
  u8 count;
  u8 slot;
  volatile u8* timer;
  volatile u8* tailTimer;

  count = func_801BDB7C(0);
  slot = 0;
  if (count != 0) {
    timer = &phaseTimer;
    do {
      u8 record;
      s32 offset;

      record = D_80181B10[D_80144F5A[slot]];
      offset = record * 164;
      func_801D826C((u16)(0x11 - *timer * 40),
                    (u32)(slot * 54 + 0x3E) & 0xFFFE, record,
                    D_8014496F[offset] & 2, 0);
      sprintf((char*)D_80145AD4, D_801D0E60, slot + 1);
      func_80150098(0x1A - *timer * 40, (s16)(slot * 54 + 0x43), 0,
                    D_80145AD4);
      slot++;
    } while (slot < count);
  }
  tailTimer = &phaseTimer;
  func_801DF00C((u16)(*tailTimer * 45 + 0x96), 0x3B);
  func_801DAB90(0x14, (0x12 - *tailTimer * 8) & 0xFFFE, 0x118, 0x13,
                D_80144952);
}
