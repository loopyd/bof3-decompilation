#include "bof3/battle/battle15_internal.h"

/* State byte runs starting at record +0x08: one 0x140-byte party row per
 * battler (records above 0x80145FB8) and one 0x118-byte reserve row per
 * battler (records above 0x801EB734). Each run base keeps its own symbol
 * because the original materialises it with a `lui %hi`/`addiu %lo` pair into
 * a register and indexes it, as it does for the byte symbols above the same
 * records. */
extern u8 battlePartyStateBytesAt80145FBC[][0x140]; /* @source 0x80145FBC @kind table */
extern u8 battleEnemyStateBytesAt801EB738[][0x118]; /* @source 0x801EB738 @kind table */

/**
 * @source 0x800A17A4
 * @behavior Runs selection pass func_800A3F28, then clears the selected
 *   battler's record state. For a selected byte below three the record is the
 *   0x140-stride party record above 0x80145FB8, otherwise the 0x118-stride
 *   reserve record above 0x801EB734 at index minus three. It zeroes the eight
 *   state bytes at record +0x08, clears bits 0x8000 and 0x10000 of the word at
 *   +0x00, and clears bits 0x40, 0x80, 0x100, 0x200, 0x800, 0x1000, 0x2000 and
 *   0x40000 of the word at +0x04; every index use re-reads the selected byte
 *   D_80146394. It then calls func_800A9BD8 with the selected byte. The roles
 *   of the cleared state bytes and flag bits are unknown; the flag bits are the
 *   ones the neighbouring selection-pass handlers set.
 * @status exact
 * @match 100.00
 * @residual none
 */
void runSelectionPassClearSelectedRecordStateThenCallA9BD8(void)
{
  /* The cleared-run index is read through a pointer so the original's
   * register-held address reproduces; the cell is volatile so each use
   * reloads it after the record stores in between. */
  volatile u8 *sel;
  /* Reserve-record row index: the byte minus three, kept in a local so the
   * 0x118 scale is applied to the subtracted value rather than folded into
   * the store displacement. */
  u32 offset;
  /* Clear mask of the word being cleared; a local keeps the original's
   * pointer/mask register pair. */
  u32 clearMask;
  u8 i;

  func_800A3F28();
  if (((u8 *)&D_80146394)[0] < 3) {
    i = 0;
    do {
      battlePartyStateBytesAt80145FBC[((u8 *)&D_80146394)[0]][i] = 0;
      i++;
    } while (i < 8);
    sel = &D_80146394;
    clearMask = 0xFFFE7FFF;
    D_80145FB4[sel[0]].unk_00 &= clearMask;
    clearMask = 0xFFFBC43F;
    D_80145FB8[sel[0]].flags_00 &= clearMask;
  } else {
    i = 0;
    do {
      offset = ((u8 *)&D_80146394)[0] - 3;
      battleEnemyStateBytesAt801EB738[offset][i] = 0;
      i++;
    } while (i < 8);
    sel = &D_80146394;
    clearMask = 0xFFFE7FFF;
    D_801EB730[sel[0] - 3].unk_00 &= clearMask;
    clearMask = 0xFFFBC43F;
    D_801EB734[sel[0] - 3].word_00 &= clearMask;
  }
  func_800A9BD8(sel[0]);
}
