#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A99AC
 * @behavior Reports whether any battle participant is both armed and carrying a
 * status word: the three party records based at D_80145E90 (0x140-byte stride)
 * are tested first, then the eight reserve records based at D_801EB630
 * (0x118-byte stride, record index i - 3). A participant counts when bit 0 of
 * its leading status byte is set and the high nibble of its flag word
 * (party +0x124, reserve +0x100) is non-zero. Returns 1 on the first such
 * participant and 0 when the scan completes.
 * @status exact
 * @match 100.00
 * @residual none
 */
u32 func_800A99AC(void)
{
  u8 i;

  for (i = 0; i < 3; i++) {
    if ((D_80145E90[i].unk_00 & 1u) != 0u &&
        (D_80145E90[i].unk_124 & 0xF0u) != 0u) {
      return 1u;
    }
  }

  for (i = 3; i < 11; i++) {
    if ((D_801EB630[i - 3u].unk_00 & 1u) != 0u &&
        (D_801EB630[i - 3u].unk_100 & 0xF0u) != 0u) {
      return 1u;
    }
  }

  return 0u;
}
