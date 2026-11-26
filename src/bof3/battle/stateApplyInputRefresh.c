#include "bof3/battle/battle15_internal.h"

/* Byte +0x0F of each 0x140-stride party record above 0x80145FB8, addressed as
 * its own symbol like battlePartyByteAt80145. The original materialises it
 * with its own `lui %hi(D_80145FC7)` pair on the scaled index, so the role is
 * unknown here; battle03's copies of the same block read it as the record's
 * countdown byte. */
extern u8 battlePartyByteAt801452[]; /* @source 0x80145FC7 @kind unknown */

/* @source 0x800A02E8
 * @behavior Applies the 0x800 input mask to the battle selection for the
 *   selected battler byte D_80146394 when that battler's flag word at
 *   0x80145FB8 (stride 0x140) has neither low bit set. On that path it first
 *   clears byte +0x0F of the record when the selection byte is below three
 *   (party side only). querySelectionApplyInput reports whether the mask was
 *   applied; when it was not, the handler runs the selection refresh pass
 *   stateRefreshFlagStatus and returns. Otherwise, and whenever the flag word carries a
 *   low bit, it stores 1 in byte +0x08 of the selection record pointer
 *   D_8014639C.record.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stateApplyInputRefresh(void)
{
  u32 index;

  /* Non-volatile view: widening the volatile cell costs an extra `andi` on
   * the load the original does not have. */
  index = *(u8 *)&D_80146394;
  if ((D_80145FB8[index].flags_00 & 3) == 0) {
    if (index < 3) {
      battlePartyByteAt801452[index * 0x140] = 0;
    }
    if ((u8)querySelectionApplyInput(0x800) == 0) {
      stateRefreshFlagStatus();
      return;
    }
  }
  ((u8 *)D_8014639C.record)[8] = 1;
}
