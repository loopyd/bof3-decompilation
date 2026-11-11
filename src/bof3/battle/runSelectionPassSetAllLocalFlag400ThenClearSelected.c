#include "bof3/battle/battle15_internal.h"

/**
 * @source 0x800A1CB4
 * @behavior Runs the battle selection refresh pass func_800A3F28 and then,
 *   while the selected battler byte at 0x80146394 is below three, raises flag
 *   0x400 in the flag word of every local battler record below the volatile
 *   count byte at 0x801462F0 (records at 0x80145FB8, stride 0x140) and clears
 *   flag 0x400 in the selected battler's own record. Byte indices of three and
 *   above leave every record untouched.
 * @status exact
 * @match 100.00
 * @residual none
 * The name follows the pass-then-flag family already exact in this target
 * (runSelectionPassThenSetSelectedRecordFlag100 at 0x800A1704,
 * runSelectionPassSetSelectedFlag1000ThenCallA9BD8 at 0x800A2120): this body
 * raises the flag on every local battler and then clears it on the selected
 * one. Flag 0x400's role is unestablished here - battle/03's testFlag400WhenEligible
 * only re-reads it - so the name states the flag and the pass, nothing more.
 * The selected byte is read through a plain view because the original's two
 * reads carry no widening `andi`; the count's address is built from the
 * selected byte's base register (`addiu $a1, $v1, -0xA4`), the same relation
 * 0x801462F0 has to 0x80146394, while the entry test reads the named cell.
 */
void runSelectionPassSetAllLocalFlag400ThenClearSelected(void)
{
  u32 index;
  u8 *selected;
  volatile u8 *count;

  func_800A3F28();
  selected = (u8 *)&D_80146394;
  if (*selected < 3) {
    index = 0;
    if (localBattlerCount != 0) {
      count = (volatile u8 *)selected - 0xA4;
      do {
        D_80145FB8[(u8)index].flags_00 |= 0x400;
        index += 1;
      } while ((u8)index < *count);
    }
    D_80145FB8[*(u8 *)&D_80146394].flags_00 &= ~0x400;
  }
}
