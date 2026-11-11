#include "bof3/battle/battle15_internal.h"

/* Plain byte view of the volatile selection cell at 0x80146374: the original
 * materializes that cell's address once and reads the plain byte through it
 * both before and after the flag store. */
typedef struct BattleSelectionCell {
  u8 index;
} BattleSelectionCell;

/* Record byte raised by this pass, one byte below the 0x80145FC9 counter
 * byte that applyPendingBattleDamage consumes. */
extern u8 battlePartyByteAt80145FC8[]; /* @source 0x80145FC8 @kind unknown */

/**
 * @source 0x800A14B4
 * @behavior Runs selection pass func_800A3F28, raises flag 0x40 in the flag
 *   word of the selected record - the 0x140-stride party record at 0x80145FB8
 *   for indices below three, the 0x118-stride reserve record at 0x801EB734 at
 *   index minus three otherwise - then reloads the selection and advances that
 *   record's byte at +0x10, which stops at 2.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSelectionPassSetSelectedFlag40ThenAdvanceByte10(void)
{
  u32 index;
  u32 current;
  u32 offset;
  BattleSelectionCell *selected;
  u8 advance;

  func_800A3F28();
  selected = (BattleSelectionCell *)&D_80146374;
  index = selected->index;
  if (index < 3) {
    D_80145FB8[index].flags_00 |= 0x40;
    current = selected->index;
    offset = current * 0x140;
    advance = battlePartyByteAt80145FC8[offset];
    if (advance < 2) {
      ((u8 *)D_80145FB8)[offset + 0x10] = (u8)(advance + 1);
    }
  } else {
    D_801EB734[index - 3].word_00 |= 0x40;
    current = selected->index;
    offset = (current - 3) * 0x118;
    advance = D_801EB744[offset];
    if (advance < 2) {
      ((u8 *)D_801EB734)[offset + 0x10] = (u8)(advance + 1);
    }
  }
}
