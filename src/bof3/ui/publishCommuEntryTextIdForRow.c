#include "bof3/bof3.h"

/* Shared commu list cursor. moveCommuRowCursorByPadDirection moves it by the
 * pad direction with a press and repeat state, and the row drawing code
 * func_801D36FC indexes the twelve-entry layout table at 0x801D4C44 with it, so
 * it is the row of this overlay's list that its handlers act on. */
extern u8 D_801448ED; /* @source 0x801448ED @kind data */

/* Pending world id; the commu entry count of a world is one byte of the table at
 * 0x801F286D indexed by it (the same pair func_801D482C bounds its own entry
 * index with). */
extern u16 D_80143F00; /* @source 0x80143F00 @kind data */
extern u8 D_801F286D[]; /* @source 0x801F286D @kind data */

/* Eight stride-8 local record slots: byte 0 of a slot is its record kind and
 * byte 1 the variant byte of the same slot. */
extern u8 D_801457A8[]; /* @source 0x801457A8 @kind data */

/* Halfword text ids of this overlay's own data, ordered by the commu list row
 * for rows 9 and above. */
extern u16 D_801D4DDA[]; /* @source 0x801D4DDA @kind data */

/* Halfword text ids ordered by the record kind byte of a local slot. */
extern u16 D_801D4BB0[]; /* @source 0x801D4BB0 @kind data */

/* Result halfword of the current commu request, consumed as a text id: the
 * drawing handler func_801D3038 indexes the executable's string-offset table at
 * 0x80010000 with it. */
extern u16 D_801D4E0C; /* @source 0x801D4E0C @kind data */

/* @source 0x801D4AAC
 * @behavior Publishes the text id of the commu list row that rowPtr addresses
 * into the request result halfword 0x801D4E0C. A row equal to the shared cursor
 * 0x801448ED takes 0x44 and row 0 takes 0x69. Rows 9 and above index the
 * halfword table at 0x801D4DDA with the row. Rows 1 through 8 fail with 0x44
 * when the entry count of the pending world (0x801F286D indexed by 0x80143F00)
 * is below the row, take 0x69 when the record slot at (row - 1) * 8 is empty,
 * and otherwise index the halfword table at 0x801D4BB0 with that slot's kind
 * byte. Both callers pass the request row byte 0x801D4E28 and ignore the result.
 * @status exact
 * @match 100.00
 * @residual none
 * The row arm writes the local rowValue and hands it to value once, as this
 * function's shape sibling publishCommuEntryTextId does for its own arm. The
 * first seed assigned value in both 0x44 arms and the late jump passes merged
 * them, so this arm's `beq` jumped to a shared block with an unfilled delay slot
 * instead of branching to the store with 0x44 in it (35/43, first difference at
 * instruction 4). The hand-over copy coalesces away in registration.
 */
void publishCommuEntryTextIdForRow(u8 *rowPtr) {
  u32 row;
  u32 limit;
  u8 slotKind;
  s32 slot;
  u16 value;
  u16 rowValue;

  row = *rowPtr;

  if (row == D_801448ED) {
    value = 0x44;
  } else if (row == 0) {
    value = 0x69;
  } else if (row >= 9) {
    value = D_801D4DDA[row];
  } else {
    limit = D_801F286D[D_80143F00];
    if (limit < row) {
      rowValue = 0x44;
    } else {
      slot = (row - 1) * 8;
      slotKind = D_801457A8[slot];
      if (slotKind == 0) {
        rowValue = 0x69;
      } else {
        rowValue = D_801D4BB0[slotKind];
      }
    }
    value = rowValue;
  }

  D_801D4E0C = value;
}
