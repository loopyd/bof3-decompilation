#include "bof3/ui/shop00_internal.h"

/* @source 0x801E0BAC
 * @behavior note-list sub-step handler reached as entry 7 of the sub-step table
 *           D_801E5CFC, which func_801DFDB8 dispatches with the UI sub-step byte
 *           D_80148652 as index: it first republishes the cursor state
 *           (D_80148624 = 1, the x halfword D_80148628 = D_801483C4, the y
 *           halfword D_8014862A = D_801483C6 + 13 * (D_801483CB -
 *           D_801483CA) + 0x1A, and the ability-record halfword D_80148340 =
 *           the +0x12 field of the 0x14-byte ability record selected by the
 *           note-bank byte D_8014546C[D_801483CB]), then dispatches the
 *           direction returned by func_8014E640 over the mask 0x500C. Bit
 *           0x1000 steps the flat cursor index D_801483CB down unless it is
 *           already zero and arms the scroll byte D_801483CD with 0xF0 when the
 *           cursor ends up above the row origin D_801483CA; bit 0x4000 steps
 *           the cursor up where it stays below 0x7F and arms the same byte with
 *           0x10 once the cursor reaches nine past the origin. Bit 0x4 moves the
 *           cursor and the origin one column back: to 0 when the origin is
 *           already zero, back by the origin when the origin is still below
 *           nine, and back by nine from both cells otherwise. Bit 0x8 is the
 *           mirror move forward: with the origin at the last column 0x77 it puts
 *           the cursor at 0x7F, with the origin at or past 0x6F it puts the
 *           cursor at the row position of that last column and sets the origin
 *           to 0x77, and otherwise it steps both cells forward by nine. It then
 *           plays cue 0x100 when the cursor changed, and returns while the
 *           scroll byte is nonzero. Otherwise, when the pad state D_80145AA8
 *           shares the confirm mask D_80145AC2, it records the note-bank slot
 *           under the cursor in the pending-slot byte D_801483CC and plays cue
 *           0x103 while that bank byte is nonzero, plays cue 0x107 and records
 *           nothing while it is zero, and with a slot already recorded it plays
 *           cue 0x103 and exchanges the bank bytes D_8014546C[D_801483CC] and
 *           D_8014546C[D_801483CB] before clearing the pending byte to 0xFF.
 *           When the pad state shares the cancel mask D_80145AC4 instead it
 *           plays cue 0x106 and, with no slot recorded, sets D_801483EF to 0xFF
 *           and steps the UI sub-step byte D_80148652 down by one, or otherwise
 *           just clears the pending byte.
 * @status partial
 * @match 93.30
 * @residual the confirm arm of the pad dispatch and the three pending-slot
 *           stores. The original reaches D_801483CC as the second byte of the
 *           cursor cell in that arm (`lbu`/`sb` at offset 1 of the register
 *           holding &D_801483CB, one instruction per access) and stores to it
 *           on each of its own paths, so each path ends `j` plus a store in
 *           the delay slot. Every clean-C spelling of that offset measured
 *           here folds the pointer addition into the constant address
 *           D_801483CB+1 and emits the absolute `lui`/`lbu` pair instead, which
 *           also lets the compiler cross-jump the three pending stores into one
 *           shared block: 209 of 224 instructions, 892 -> 896 bytes, first
 *           difference at 0x0240. Spelling probes that measured this way:
 *           cursor[1], `*(u8*)((u32)cursor + 1)`, a two-field struct view taken
 *           inline and through its own pointer, a `u8(*)[2]` cast, a second
 *           pointer assigned `&D_801483CC`, a `cursor + 1` pointer statement,
 *           and a volatile pointee cast. The 0x8 direction arm reaches the
 *           original's add-then-subtract order and its raw/masked register
 *           split only through the dedicated `last` local below; sharing the
 *           0x4 arm's `column` local folded that sum (202 of 224).
 */
void stepNoteListCursorThenPickOrSwapNote(void) {
  s32 direction;
  u16 skillValue;
  u8  start;
  u8  column;
  u8  value;
  u8  pending;
  u8* cursor;
  u8* subStep;
  s32 base;

  skillValue = *(u16*)abilityObjects[D_8014546C[D_801483CB]].control_12;
  D_80148624 = 1;
  D_80148628 = D_801483C4;
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
    u8 last = D_801483CA;
    if (last == 0x77) {
      D_801483CB = 0x7F;
    } else if (last >= 0x6F) {
      base = start + 0x77;
      D_801483CB = base - last;
      D_801483CA = 0x77;
    } else {
      D_801483CB = start + 9;
      D_801483CA = last + 9;
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
    if (D_801483CC == 0xFF) {
      value = D_8014546C[*cursor];
      if (value != 0) {
        func_8015DF18(0x103);
        D_801483CC = *cursor;
      } else {
        func_8015DF18(0x107);
      }
    } else {
      func_8015DF18(0x103);
      value = D_8014546C[*cursor];
      pending = D_8014546C[D_801483CC];
      D_8014546C[D_801483CC] = value;
      D_8014546C[*cursor] = pending;
      D_801483CC = 0xFF;
    }
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    if (D_801483CC == 0xFF) {
      D_801483EF = 0xFF;
      subStep = &D_80148652;
      *subStep -= 1;
    } else {
      D_801483CC = 0xFF;
    }
  }
}
