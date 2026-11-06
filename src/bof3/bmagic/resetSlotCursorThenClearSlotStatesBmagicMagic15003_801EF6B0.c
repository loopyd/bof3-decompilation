#include "bof3/bof3.h"

/* Eight-entry table of 8-byte slots at 0x801EFAB4, inside this overlay's own
 * image. Each slot holds a leading state halfword, a countdown halfword, an
 * angle halfword and a life halfword. The table is addressed rather than typed
 * by the original, so it stays a halfword view anchored at its own address. */
extern u16 D_801EFAB4[]; /* @source 0x801EFAB4 @kind data */

/* Cursor cell into that table, at 0x801EFAF4. The original re-materializes the
 * cell address for every access (lui $at,0x801F before each sw, lui $v0,0x801F
 * before the lw of each iteration), so the cell is kept as the named symbol
 * instead of a pointer local. */
extern u16 *D_801EFAF4; /* @source 0x801EFAF4 @kind data */

/* @source 0x801EF6B0
 * @behavior Resets the overlay's eight-slot table cursor and empties the slots:
 *           it publishes the table base 0x801EFAB4 into the cursor cell
 *           0x801EFAF4, then walks the eight 8-byte slots with an unsigned byte
 *           counter in a bottom-tested loop, clearing each slot's leading state
 *           halfword (sh $zero) through the pointer it re-reads from the cell
 *           and publishing the advanced cursor back into the cell after every
 *           slot. The counter increments before the clear and the loop test
 *           masks it to 0xFF before the unsigned compare with 8, so slots 0
 *           through 7 are emptied. The function takes no arguments, has no
 *           frame, makes no call, returns nothing and reads no state other than
 *           the cursor cell. The two payload neighbours that use the same table
 *           corroborate the shape: 0x801EF6F8 fills only slots whose state
 *           halfword is zero with {1, 0x300, rand() << 4, 8}, and 0x801EF8EC
 *           walks the same eight entries through this cursor to draw and
 *           advance them.
 * @status exact
 * @match 100.00
 * @residual none
 */
void resetSlotCursorThenClearSlotStatesBmagicMagic15003_801EF6B0(void) {
  u8 i;
  u16 *slot;

  D_801EFAF4 = D_801EFAB4;
  i = 0u;
  do {
    slot = D_801EFAF4;
    i += 1u;
    *slot = 0;
    D_801EFAF4 = slot + 4;
  } while (i < 8u);
}
