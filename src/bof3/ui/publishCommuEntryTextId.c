#include "bof3/bof3.h"

/* Shared commu list cursor. moveCommuRowCursorByPadDirection moves it by the
 * pad direction with a press and repeat state, and the row drawing code
 * drawRowMarkerOutlineCommu0107_801D36FC indexes the twelve-entry layout table at 0x801D4C44 with it, so
 * it is the row of this overlay's list that its handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Eight stride-8 local record slots: byte 0 of a slot is its record kind and
 * byte 1 the variant byte of the same slot. */
extern u8 D_801457A8[]; /* @source 0x801457A8 @kind data */
extern u8 D_801457A9[]; /* @source 0x801457A9 @kind data */

/* Nine halfword values the overlay keeps in its own header data, ordered by
 * record kind; this function reads them through a stack copy. */
extern u16 D_801D0C18[]; /* @source 0x801D0C18 @kind data */

/* Result halfword of the current commu request, consumed as a text id: the
 * drawing handler drawCommuEntryListScreen indexes the executable's string-offset table at
 * 0x80010000 with it. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* @source 0x801D49C4
 * @behavior Publishes the text id of the commu list row at the shared cursor
 * 0x801448ED into the request result halfword 0x801D4E0C. Row 0 takes 0x6A and
 * rows 9 through 0xB take twice the row plus 0x2C. Rows 1 through 8 select the
 * local record slot at (row - 1) * 8: an empty slot takes 0x6A, kind 4 takes the
 * slot's variant byte at 0x801457A9 plus 0x4A, and every other kind indexes the
 * nine-halfword table at 0x801D0C18 with kind - 5. The table is read through a
 * stack copy, so the value survives the arm that follows the slot lookup.
 * @status exact
 * @match 100.00
 * @residual none
 * Two source shapes are load-bearing. The cursor local is wide, so the masked
 * `row & 0xFF` comparisons and the unmasked `row * 2` stay the two distinct
 * forms the original emits. The slot arm fills its own halfword local and hands
 * it to `value` once, because assigning `value` inside the arm leaves both 0x6A
 * arms as identical blocks and the late jump passes then merge them, which drops
 * the row-0 arm's own store path and two instructions. The hand-over copy
 * coalesces away in registration.
 */
void publishCommuEntryTextId(void) {
  u16 values[9];
  u32 row;
  u8 kind;
  s32 slot;
  u8 index;
  u16 value;
  u16 slotValue;

  __builtin_memcpy(values, D_801D0C18, sizeof(values));
  row = D_801448ED;

  if ((row & 0xFF) == 0) {
    value = 0x6A;
  } else if ((row & 0xFF) >= 9) {
    value = row * 2 + 0x2C;
  } else {
    slot = ((row & 0xFF) - 1) * 8;
    kind = D_801457A8[slot];
    if (kind == 0) {
      slotValue = 0x6A;
    } else {
      index = (kind - 4) & 0xFF;
      if (index == 0) {
        slotValue = D_801457A9[slot] + 0x4A;
      } else {
        slotValue = values[index - 1];
      }
    }
    value = slotValue;
  }

  D_801D4E0C = value;
}
