#include "bof3/battle/battle15_internal.h"

/* @source 0x8009E07C
 * @behavior Reads the active battler index byte D_80146394 and takes the 0x4000
 *   bit of that battler's work-record halfword: the party work record halfword
 *   at 0x80145F10 (stride 0x140) for an index below three, otherwise the enemy
 *   work record halfword at 0x801EB6B2 (stride 0x118) selected by the index
 *   rebased by -3. When the bit is set it stores -1 in the signed halfword at
 *   byte +0x04 of the selection record pointer D_8014639C.record, clears that
 *   0x4000 bit from the battler through func_800A36F0 and advances the band's
 *   counter byte: D_801462F1 for a party index, D_801462F3 for an enemy index.
 *   When the bit is clear it stores zero in that record halfword.
 * @status exact
 * @match 100.00
 * @residual none
 */
void setRecordField4NegOneWhenBattlerFlag4000(void) {
  u8* state;
  u32 index;
  u16 flag;

  /* Non-volatile wide read of the active battler index: through the header's
   * volatile u8 cell every use of the local carries an `andi 0xff` the
   * original does not issue, while this view keeps the folded lui/lbu pair. */
  index = *(u8*)&D_80146394;
  if (index < 3) {
    flag = *(u16*)(D_80145F10 + index * 0x140) & 0x4000;
  } else {
    index -= 3;
    flag = *(u16*)(D_801EB6B2 + index * 0x118) & 0x4000;
  }
  if (flag != 0) {
    D_8014639C.record[2] = -1;
    state = (u8*)&D_80146394;
    func_800A36F0(*(volatile u8*)state, 0x4000);
    if (*state < 3) {
      D_801462F1++;
    } else {
      D_801462F3++;
    }
  } else {
    D_8014639C.record[2] = 0;
  }
}
