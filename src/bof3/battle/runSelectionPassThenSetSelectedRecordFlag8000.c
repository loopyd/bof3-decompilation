#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A1414
 * @behavior Runs selection pass func_800A3F28, then sets flag 0x8000 in
 *   the record selected by D_80146374: local record for indices below three,
 *   reserve record at index minus three otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSelectionPassThenSetSelectedRecordFlag8000(void)
{
  u32 index;

  func_800A3F28();
  index = *(u8 *)&D_80146374;
  if (index < 3) {
    D_80145FB0[index].flags |= 0x8000;
  } else {
    D_801EB72C[index - 3].flags |= 0x8000;
  }
}
