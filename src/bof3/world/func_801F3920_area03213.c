#include "bof3/world/area03213_internal.h"

/* @source 0x801F3920
 * @behavior Advances the signed level bytes 0x5D, 0x5E and 0x5F of the scratch
 * cursor record at 0x1F800044 by 4 while each stays below -0x40; when the word
 * at 0x5C carries the 0xC0C0C000 colour marker it hands the work-record effect
 * byte at 0x93 to func_80196670 and resets the record (clears flag bit 0x20, the
 * 0x5C byte, the 0x5D-0x5F level triple and byte 4, and writes 2 into byte 0x78
 * of the work record), otherwise it advances the record 0x34 accumulator by its
 * 0x0C step and consumes two units of the work-record countdown word at 0x7E.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801F3920(void) {
  u8* record;
  u8* cursor;
  u8* work;

  if (((s8*)D_1F800044)[0x5d] < -0x40) {
    ((s8*)D_1F800044)[0x5d] += 4;
  }
  if (((s8*)D_1F800044)[0x5e] < -0x40) {
    ((s8*)D_1F800044)[0x5e] += 4;
  }
  if (((s8*)D_1F800044)[0x5f] < -0x40) {
    ((s8*)D_1F800044)[0x5f] += 4;
  }

  record = D_1F800044;
  if ((*(u32*)(record + 0x5c) & 0xffffff00) == 0xc0c0c000) {
    func_80196670(D_80146884[0x93]);
    D_1F800044[0] &= 0xdf;
    D_1F800044[0x5c] = 0;
    cursor = D_1F800044;
    cursor[0x5f] = 0;
    cursor[0x5e] = 0;
    cursor[0x5d] = 0;
    D_1F800044[4] = 0;
    D_80146884[0x78] = 2;
  } else {
    work = D_80146884;
    *(s32*)(record + 0x34) += *(s32*)(record + 0x0c);
    *(u16*)(work + 0x7e) -= 2;
  }
}
