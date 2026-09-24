#include "bof3/ui/shop00_internal.h"

/* @source 0x801D7B74
 * @behavior shop panel phase step sharing the entry sequence of the sibling
 *           steps func_801D7CC8 and func_801D7D48: calls func_801D7E1C with
 *           the work-block index byte D_80148656[0], 0x98, 0x3F, 0 and the
 *           main-RAM byte D_8014865D as the fifth argument, then buildSlotList
 *           with 0, then forwards the same work-block index byte read signed to
 *           the shop panel helper func_801D8E2C as the even value
 *           (index * 54 + 0x3E) & 0xFFFE alongside 0x11, 0x80, 0x34, 0 and 6;
 *           when the byte counter func_801D81B4(&D_80148656[5], 0xF) returns a
 *           nonzero low byte it publishes 1 << (s8)D_8014865D into the byte at
 *           D_80144981 + 164 * D_80145FCC[(s8)D_80148656[0] * 0x140], rebuilds
 *           that same 164-byte field record through func_80164A44 and rolls the
 *           UI phase byte D_80148651 back by four.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801D7B74(void) {
  u8* work;
  s32 offset;
  s32 record_offset;

  work = D_80148656;
  func_801D7E1C(work[0], 0x98, 0x3F, 0, D_8014865D);
  buildSlotList(0);
  func_801D8E2C(0x11, ((s8)work[0] * 54 + 0x3E) & 0xFFFE, 0x80, 0x34, 0, 6);
  if ((u8)func_801D81B4(work + 5, 0xF)) {
    offset = (s8)work[0] * 0x140;
    record_offset = D_80145FCC[offset] * 164;
    D_80144981[record_offset] = 1 << (s8)D_8014865D;
    offset = (s8)work[0] * 0x140;
    record_offset = D_80145FCC[offset] * 164;
    func_80164A44(D_80144968 + record_offset);
    D_80148651 -= 4;
  }
}
