#include "bof3/battle/battle03_internal.h"

/* @source 0x801E99AC
 * @behavior Selects the 0x118-byte record D_801EB630[panel byte +0x0A - 3]:
 * stores panel halfword +0x04 as that record's signed halfword +0x2E plus its
 * signed byte +0xDA, adding the module byte 0x801EAEA4 while the record byte
 * +0x08 is 0 or 3 and 0x801EAEA6 otherwise; then stores panel halfword +0x06 as
 * the record's unsigned halfword +0x30 plus its signed byte +0xDB plus 0xC and
 * forwards panel halfwords +0x04/+0x06 with panel byte +0x0A to func_801D8690.
 * Finally, while the selection gate byte 0x801462EF is set, a queued slot byte
 * D_801EB4D8 that has no 0xC0 bits and equals panel byte +0x0A, or that has
 * 0xC0 bits and 0x40 set, skips the queued panel task commit.
 * @status exact
 * @match 100.00
 * @residual none
 */
void func_801E99AC(void) {
  u8* panel;
  u8  index;
  u32 offset;
  s16 value;
  u8  flags;

  panel = D_80148648;
  index = panel[0xA] - 3;
  offset = (u32)index * 0x118;

  do {
    *(s16*)&panel[4] =
        *(s16*)&D_801EB65E[offset] + *(s8*)&D_801EB712[offset] +
        (D_801EB638[offset] == 0 || D_801EB638[offset] == 3 ? D_801EAEA4
                                                           : D_801EAEA6);
  } while (0);

  value = *(u16*)&D_801EB660[(u32)index * 0x118] +
          (s8)D_801EB713[(u32)index * 0x118] + 12;
  *(s16*)&D_80148648[6] = value;
  func_801D8690(*(s16*)&D_80148648[4], value, D_80148648[0xA]);

  if (D_801462EF != 0) {
    flags = *D_801EB4D8;
    if ((flags & 0xC0) == 0) {
      if (flags == D_80148648[0xA]) {
        return;
      }
    } else if ((flags & 0x40) != 0) {
      return;
    }
  }
  commitQueuedPanelTask();
}
