#include "bof3/ui/shop00_internal.h"

/* @source 0x801E05E0
 * @behavior note-list sub-step handler reached as entry 4 of the sub-step
 *           table D_801E5CFC, which func_801DFDB8 dispatches with the UI
 *           sub-step byte D_80148652 as index: it first republishes the cursor
 *           state (D_80148624 = 1, the x halfword D_80148628 =
 *           D_801483C4 + 7, the y halfword D_8014862A = D_801483C6 + 13 *
 *           (D_801483CB - D_801483CA) + 0x1A, and the ability-record halfword
 *           D_80148340 = the +0x12 field of the 0x14-byte ability record
 *           selected by the note-bank byte D_8014546C[D_801483CB]), then
 *           dispatches the direction returned by func_8014E640 over the mask
 *           0x500C. Bit 0x1000 steps the flat cursor index D_801483CB down
 *           unless it is already zero and arms the scroll byte D_801483CD with
 *           0xF0 when the cursor ends up above the origin D_801483CA; bit
 *           0x4000 steps the cursor up where it stays below 0x7F and arms the
 *           same byte with 0x10 once the cursor reaches nine past the origin.
 *           Bit 0x4 moves the cursor and the origin one column back: to 0 when
 *           the origin is already zero, back by the origin when the origin is
 *           still below nine, and back by nine from both cells otherwise. Bit
 *           0x8 is the mirror move forward: with the origin at the last column
 *           0x77 it puts the cursor at 0x7F, with the origin at or past 0x6F it
 *           puts the cursor at the row position of that last column and sets
 *           the origin to 0x77, and otherwise it steps both cells forward by
 *           nine. It then plays cue 0x100 when the cursor changed, and returns
 *           while the scroll byte is nonzero. Otherwise, when the pad state
 *           shares the confirm mask D_80145AC2 it exchanges the note-bank byte
 *           D_8014546C[D_801483CB] with the byte at D_801449E2 + 164 *
 *           D_801483A6 + D_801483A7 and plays cue 0x103, or plays cue 0x107
 *           without touching the sub-step when both bytes are zero; when the
 *           pad state shares the cancel mask D_80145AC4 instead it plays cue
 *           0x106. Both of those paths then step the UI sub-step byte
 *           D_80148652 down by one.
 * @status partial
 * @match 94.31
 * @residual two allocation-only hunks against an exact 844-byte body and the
 *           original 24-byte frame: the right-hand pad arm keeps the raw and
 *           the narrowed view of D_801483CA in exchanged scratch registers and
 *           folds (start + 0x77) - column into start - (column - 0x77); the
 *           confirm arm loads D_801483A7 ahead of the 164 * D_801483A6 product
 *           and adds the D_801449E2 base after the D_801483A7 term instead of
 *           before it. Spelled alternatives for both hunks (per-arm local split,
 *           wide snapshot types, signed casts, named intermediate, and three
 *           slot-base orderings) measured neutral or worse.
 */
void stepNoteListCursorThenSwapSelectedNote(void) {
  s32 direction;
  u16 skillValue;
  u8  start;
  u8  column;
  u8  value;
  u8  swapped;
  u8* slot;
  u8* cursor;

  skillValue = *(u16*)abilityObjects[D_8014546C[D_801483CB]].control_12;
  D_80148624 = 1;
  D_80148628 = D_801483C4 + 7;
  D_80148340 = skillValue;
  D_8014862A = D_801483C6 + ((D_801483CB - D_801483CA) * 13 + 0x1A);

  direction = func_8014E640(D_80145AA8 & 0x500C);
  start = D_801483CB;

  if (direction & 0x1000) {
    if (start != 0) {
      D_801483CB = start - 1;
    }
    if (D_801483CB < D_801483CA) {
      D_801483CD = 0xF0;
    }
  } else if (direction & 0x4000) {
    if (start < 0x7F) {
      D_801483CB = start + 1;
    }
    if (D_801483CB >= D_801483CA + 9) {
      D_801483CD = 0x10;
    }
  } else if (direction & 0x4) {
    column = D_801483CA;
    if (column == 0) {
      D_801483CB = 0;
    } else if (column < 9) {
      D_801483CB = start - column;
      D_801483CA = 0;
    } else {
      D_801483CB = start - 9;
      D_801483CA = column - 9;
    }
  } else if (direction & 0x8) {
    column = D_801483CA;
    if (column == 0x77) {
      D_801483CB = 0x7F;
    } else if (column >= 0x6F) {
      D_801483CB = (start + 0x77) - column;
      D_801483CA = 0x77;
    } else {
      D_801483CB = start + 9;
      D_801483CA = column + 9;
    }
  }

  cursor = &D_801483CB;
  if (start != *cursor) {
    func_8015DF18(0x100);
  }
  if (D_801483CD != 0) {
    return;
  }

  if (D_80145AA8 & D_80145AC2) {
    slot = &D_801449E2[D_801483A6 * 164 + D_801483A7];
    value = D_8014546C[*cursor];
    if (value != 0 || *slot != 0) {
      swapped = *slot;
      *slot = value;
      D_8014546C[*cursor] = swapped;
      func_8015DF18(0x103);
    } else {
      func_8015DF18(0x107);
      return;
    }
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
  } else {
    return;
  }

  {
    u8* step = &D_80148652;
    *step = *step - 1;
  }
}
