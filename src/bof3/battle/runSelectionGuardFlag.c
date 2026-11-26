#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A1A30
 * @behavior Runs selection pass stateRefreshFlagStatus, then sets flag 0x10000 in
 *   the guard word of the selected local record for indices below three,
 *   or the selected reserve record at index minus three otherwise.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSelectionGuardFlag(void)
{
  u32 index;

  stateRefreshFlagStatus();
  index = *(u8 *)&D_80146374;
  if (index < 3) {
    D_80145FB4[index].unk_00 |= 0x10000;
  } else {
    D_801EB730[index - 3].unk_00 |= 0x10000;
  }
}
