#include "bof3/battle/battle15_internal.h"

void func_800AAC88(s16 arg0, u8 arg1); /* @source 0x800AAC88 */

/* @source 0x800A0564
 * @behavior Runs the battle selection roll func_800A3A10 for the acting
 *   battler byte D_80146374 and selected battler byte D_80146394. A nonzero
 *   roll stores 1 in byte +0x08 of the selection record pointer
 *   D_8014639C.record and calls func_800AAC88 with effect kind 0 and the
 *   selected battler byte. A zero roll instead raises bit 0x2000 in the flag
 *   word at 0x80145FB8 (stride 0x140) when the selected battler index is
 *   below three, or at 0x801EB734 (stride 0x118) rebased by -3 otherwise, and
 *   then calls func_800A9BD8 with the selected battler byte. Both paths finish
 *   with the selection refresh pass func_800A3F28.
 * @status partial
 * @match 93.65
 * @residual The byte-store arm allocates the stored constant to v1 and the
 *   record base it loads from 0x801463A0 to v0; the original holds the
 *   constant in v0 (`addiu v0,zero,1`) and the base in v1. Four instructions
 *   inside that one block differ, size is already equal, and the flag arm,
 *   both calls and the shared tail match.
 */
void runSelectionRollSetRecordByte8OrFlag2000ThenRefresh(void)
{
  u32 index;

  if ((u8)func_800A3A10(D_80146374, D_80146394) == 0) {
    /* Non-volatile view: the roll call above already read the cell and
     * widening the volatile u8 costs an extra `andi` on the load, the same
     * remedy storeSelectedValueSetRecordByte8To1WhenRollRejected uses. */
    index = *(u8 *)&D_80146394;
    if (index < 3) {
      D_80145FB8[index].flags_00 |= 0x2000;
    } else {
      D_801EB734[index - 3].word_00 |= 0x2000;
    }
    func_800A9BD8(D_80146394);
  } else {
    ((u8 *)D_8014639C.record)[8] = 1;
    func_800AAC88(0, D_80146394);
  }
  func_800A3F28();
}
