#include "bof3/battle/battle03_internal.h"

/* @behavior Scans the first 0x30 entries of the D_801EC330 queued slot store for
 * the first entry whose byte +0x00 has bit 0 clear, marks it active by setting
 * that bit and stores the two caller mode bytes at +0x06 (arg0) and +0x05
 * (arg1), then returns the entry index; returns 0xFF when every entry is active.
 * @source 0x801E590C
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 func_801E590C(u32 arg0, u32 arg1) {
  u32 index;
  u8  slot;
  u8  flags;

  index = 0u;
  do {
    slot = (u8)index;
    flags = D_801EC330[slot].unk_00;
    if ((flags & 1u) != 0u) {
      index += 1u;
    } else {
      D_801EC330[slot].unk_00 = (u8)(flags | 1u);
      D_801EC330[slot].unk_06 = (u8)arg0;
      D_801EC330[slot].unk_05 = (u8)arg1;
      return slot;
    }
  } while ((u8)index < 0x30u);

  return 0xffu;
}
