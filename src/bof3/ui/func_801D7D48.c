#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7D48
 * @behavior shop panel phase step sharing the entry sequence of the sibling
 *           steps func_801D7B74 and func_801D7CC8: calls func_801D7E1C with
 *           the work-block index byte D_80148656[0], 0x98, 0x3F, 0 and 0xFF,
 *           then buildSlotList with 0, then forwards the same work-block index
 *           byte (read signed) to the shop panel helper func_801D8E2C as the
 *           even value (index * 54 + 0x3E) & 0xFFFE alongside 0x11, 0x80, 0x34,
 *           1 and 6; finally tests the byte counter func_801D81B4(&D_80148656[5],
 *           D_8014865F) and, when that counter returns a nonzero low byte, rolls
 *           the UI phase byte D_80148651 back by six.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7D48(void) {
  u8* work;

  work = D_80148656;
  func_801D7E1C(work[0], 0x98, 0x3F, 0, 0xFF);
  buildSlotList(0);
  func_801D8E2C(0x11, ((s8)work[0] * 54 + 0x3E) & 0xFFFE, 0x80, 0x34, 1, 6);
  if ((u8)func_801D81B4(work + 5, D_8014865F)) {
    D_80148651 -= 6;
  }
}
