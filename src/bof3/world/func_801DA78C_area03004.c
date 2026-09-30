#include "bof3/world/area03004_internal.h"

/* @behavior AREA030 front-end state step: when the shared mode byte
 * D_80143FC9 holds 3 it stores 3 in byte 2 of the scratch work record published
 * at the cursor 0x1F800044 and clears that record's byte 3; otherwise, while
 * the shared word D_80143FDC is clear, the shared pad state D_80145AA8 carries
 * 0x40 and the shared position D_80143FD8 sits above 0x110000, it steps that
 * position back by 0x4000. It then, while scratch record byte 0x0B is clear,
 * either latches the shared pad mask D_80145AA4/0xA000 into the record word at
 * 0xC together with a -0x800/0x800 step word at 0x80143FD4 and arms record byte
 * 0x07 (when record byte 0x07 is clear), or, once that latched mask no longer
 * intersects D_80145AA4 or the word at 0x80143FFC falls past 0xEFFFE after a
 * -0x60001 adjustment, clears the step word and arms record byte 0x0B; the
 * shared helper func_8014D978 runs last.
 * @source 0x801DA78C
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801DA78C(void) {
  u32* table = (u32*)D_80143FC8;
  u8* work;

  if (D_80143FC9 == 3) {
    D_1F800044[2] = 3;
    D_1F800044[3] = 0;
  } else {
    if (D_80143FDC == 0 && (D_80145AA8 & 0x40) != 0 && D_80143FD8 > 0x110000) {
      D_80143FD8 -= 0x4000;
    }
    work = D_1F800044;
    if (work[0x0B] == 0) {
      if (work[0x07] == 0) {
        u32 mask = D_80145AA4 & 0xA000;
        if (mask != 0) {
          *(u32*)(work + 0x0C) = mask;
          table[3] = (mask == 0x8000) ? -0x800 : 0x800;
          D_1F800044[7] = 1;
        }
      } else {
        u32 active = D_80145AA4 & *(u32*)(work + 0x0C);
        if (active == 0 || table[0x0D] - 0x60001u > 0xEFFFEu) {
          table[3] = 0;
          work[0x0B] = 1;
        }
      }
    }
  }
  func_8014D978();
}
