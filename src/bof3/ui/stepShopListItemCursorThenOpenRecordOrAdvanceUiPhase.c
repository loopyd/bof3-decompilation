#include "bof3/ui/shop00_internal.h"


/* Representation notes for the two sites that needed a probe before the first
 * exact measurement. The column-origin local is declared once per pad arm: one
 * shared `column` scored 194/201 instructions with the left arm's origin load
 * landing in a0 instead of the original's v1, and the per-arm locals scored
 * 201/201. The forward arm's new row is spelled through a one-use s32
 * intermediate, because the flat `(start + 8) - column` was reassociated into
 * `start - (column - 8)` and scored 199/201, while `start - column + 8` also
 * scored 199/201 with the same first difference.
 */
/* Shop state byte cleared on every call to this handler. No other source of
 * this target reads it and its role is not recovered, so it stays
 * address-anchored; the generated undefined-symbol list carries the address,
 * so no target-map row is added.
 * @source 0x8014845A @kind unknown */
extern u8 D_8014845A;
extern void func_801E2724(void); /* @source 0x801E2724 */

/* @source 0x801E2150
 * @behavior shop sub-step handler reached as entry 0 of the three-entry
 *           sub-step table D_801E5D50 (the shipped word at 0x801E5D50 is this
 *           function), which the dispatcher func_801E2114 indexes with the UI
 *           sub-step byte D_80148652. Every call first republishes the list
 *           cursor: it arms the s16 panel-state cell D_80148340 with 0x32,
 *           raises the cursor-visible flag D_80148624, clears the state byte
 *           D_8014845A, publishes the x halfword D_80148628 as the panel x
 *           origin D_80148358 plus 4 and the y halfword D_8014862A as the
 *           panel y origin D_8014835A plus 13 times the row offset
 *           D_8014835F - D_8014835E plus 0x1A. It then
 *           reads the pad direction func_8014E640 returns for the mask 0xF00C
 *           of the pad state D_80145AA8: bit 0x1000 decrements the cursor row
 *           unless it is already zero and arms the scroll byte D_8014835C with
 *           0xF0 when the row ends up above the column origin D_8014835E; bit
 *           0x4000 increments the row while it stays below 16 and arms the
 *           scroll byte with 0x10 once the row reaches nine past that origin;
 *           bit 0x4 moves the row and the origin one column back (to row 0 when
 *           the origin is already zero, back by the origin when it is still
 *           below nine, and back by nine from both cells otherwise); bit 0x8 is
 *           the mirror move one column forward (row 16 when the origin sits at
 *           8, otherwise the row position of that column with the origin set to
 *           8). It plays cue 0x100 when the row changed and returns while the
 *           scroll byte is nonzero. Otherwise, when the pad state carries the
 *           confirm mask D_80145AC2 it rejects an empty selection (the
 *           selection byte D_80148361 already at 0xFF) with cue 0x107 and
 *           returns; a real selection instead plays the cues 0x103 and 0x102,
 *           runs the panel opener func_801E2724, arms the panel step byte
 *           D_80148357 with 3, drops the cursor-visible flag, re-arms the frame
 *           timer phaseTimer with 8 and advances the UI sub-step byte by one.
 *           When the pad state carries the cancel mask D_80145AC4 instead it
 *           plays the cues 0x106 and 0x102, arms the shop mode byte D_80148333
 *           with 1 and the panel step byte with 1, drops the cursor-visible
 *           flag, re-arms phaseTimer with 10, clears the UI sub-step byte and
 *           advances the UI phase byte D_80148651 by one.
 * @status exact
 * @match 100.00
 * @residual none
 */
void stepShopListItemCursorThenOpenRecordOrAdvanceUiPhase(void) {
  s32 direction;
  u8  start;

  D_80148340 = 0x32;
  D_80148624 = 1;
  D_8014845A = 0;
  D_80148628 = D_80148358 + 4;
  D_8014862A = D_8014835A + (D_8014835F - D_8014835E) * 13 + 0x1A;

  direction = func_8014E640(D_80145AA8 & 0xF00C);
  start = D_8014835F;

  if (direction & 0x1000) {
    if (start != 0) {
      D_8014835F = start - 1;
    }
    if (D_8014835F < D_8014835E) {
      D_8014835C = 0xF0;
    }
  } else if (direction & 0x4000) {
    if (start < 0x10) {
      D_8014835F = start + 1;
    }
    if (D_8014835F >= D_8014835E + 9) {
      D_8014835C = 0x10;
    }
  } else if (direction & 0x4) {
    u8 column = D_8014835E;

    if (column == 0) {
      D_8014835F = 0;
    } else if (column < 9) {
      D_8014835F = start - column;
      D_8014835E = 0;
    } else {
      D_8014835F = start - 9;
      D_8014835E = column - 9;
    }
  } else if (direction & 0x8) {
    u8  column = D_8014835E;
    s32 next;

    if (column == 8) {
      D_8014835F = 0x10;
    } else {
      next = start + 8;
      D_8014835F = next - column;
      D_8014835E = 8;
    }
  }

  if (start != D_8014835F) {
    func_8015DF18(0x100);
  }
  if (D_8014835C != 0) {
    return;
  }

  if (D_80145AA8 & D_80145AC2) {
    if (D_80148361 != 0xFF) {
      func_8015DF18(0x103);
      func_8015DF18(0x102);
      func_801E2724();
      D_80148357 = 3;
      D_80148624 = 0;
      phaseTimer = 8;
      D_80148652 += 1;
    } else {
      func_8015DF18(0x107);
    }
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    func_8015DF18(0x102);
    D_80148333 = 1;
    D_80148357 = 1;
    D_80148624 = 0;
    phaseTimer = 10;
    D_80148652 = 0;
    D_80148651 += 1;
  }
}
