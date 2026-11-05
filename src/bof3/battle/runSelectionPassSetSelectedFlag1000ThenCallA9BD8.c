#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A2120
 * @behavior Runs selection pass func_800A3F28, then sets flag 0x1000 in
 *   the selected local record word at 0x80145FB8 (stride 0x140) for indices
 *   below three, or the reserve record word at 0x801EB734 (stride 0x118)
 *   at index minus three otherwise. Calls func_800A9BD8 with D_80146394.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSelectionPassSetSelectedFlag1000ThenCallA9BD8(void)
{
  u32 index;

  func_800A3F28();
  index = *(u8 *)&D_80146374;
  if (index < 3) {
    D_80145FB8[index].flags_00 |= 0x1000;
  } else {
    D_801EB734[index - 3].word_00 |= 0x1000;
  }
  func_800A9BD8(D_80146394);
}
