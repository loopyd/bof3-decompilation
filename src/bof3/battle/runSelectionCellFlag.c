#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A1704
 * @behavior Runs selection pass stateRefreshFlagStatus, then sets flag 0x100 in
 *   the word at selected local record +0x128 for indices below three,
 *   or selected reserve record +0x104 at index minus three otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSelectionCellFlag(void)
{
  u32 index;

  stateRefreshFlagStatus();
  index = *(u8 *)&D_80146374;
  if (index < 3) {
    D_80145FB8[index].flags_00 |= 0x100;
  } else {
    D_801EB734[index - 3].word_00 |= 0x100;
  }
}
