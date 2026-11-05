#include "bof3/battle/battle15_internal.h"

extern u8 battlePartyByteAt80145FC6[]; /* @source 0x80145FC6 @kind unknown */

/**
 * @source 0x800A22CC
 * @behavior Runs selection pass func_800A3F28, sets selected flag 0x4000
 *   at 0x80145FB8 (stride 0x140) for indices below three or 0x801EB734
 *   (stride 0x118) at index minus three otherwise, then reloads the selection
 *   and clears the corresponding byte at word +0x0E. Calls func_800A9BD8
 *   with D_80146394. The cleared byte's role is unknown.
 * @status partial
 * @match 84.62
 * @residual Redundant narrow-load masks and folded enemy byte displacement.
 */
void runSelectionPassSetSelectedFlag4000ClearByteThenCallA9BD8(void)
{
  u32 index;
  volatile u8 *selected;

  func_800A3F28();
  selected = &D_80146374;
  index = *selected;
  if (index < 3) {
    D_80145FB8[index].flags_00 |= 0x4000;
    index = *selected;
    battlePartyByteAt80145FC6[index * 0x140] = 0;
  } else {
    D_801EB734[index - 3].word_00 |= 0x4000;
    index = *selected;
    D_801EB742[(index - 3) * 0x118] = 0;
  }
  func_800A9BD8(D_80146394);
}
