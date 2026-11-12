#include "bof3/ui/shop00_internal.h"

/* Note-cursor block bytes armed by the confirm arm below: D_801483C0,
 * D_801483C1 and D_801483C2 take 1, 7 and 0xB there and are read back by the
 * note-cursor sub-step handler, so only the store position is recovered and the
 * three cells stay address-anchored at the original byte width. Not owned by
 * the shared map; the still-assembled sibling func_801DF9EC carries the same
 * addresses, so the generated undefined-symbol list keeps them and no
 * target-map row is added. */
extern u8 D_801483C0; /* @source 0x801483C0 @kind data */
extern u8 D_801483C1; /* @source 0x801483C1 @kind data */
extern u8 D_801483C2; /* @source 0x801483C2 @kind data */

/* @source 0x801E006C
 * @behavior note-list sub-step handler reached as entry 2 of the sub-step
 *           table D_801E5CFC, which func_801DFDB8 dispatches with the UI
 *           sub-step byte D_80148652 as index: it submits the 164-byte field
 *           record selected by the rebuilt list entry
 *           D_801E6264[D_801E625C] through the main-exe icon-record helper
 *           func_8016620C with the constants 0 and 5, publishes the halfword
 *           D_80148340 = 0x75, and, while the panel-state byte D_8014839F is
 *           zero, sets the visibility byte D_80148624 = 1 and publishes the
 *           cursor cells D_80148628 = D_801483A0 + 0x2A and
 *           D_8014862A = D_801483A2 + 7; it clears that visibility byte when
 *           the panel-state byte is nonzero.
 *           It then decodes the pad state D_80145AA8 over the direction mask
 *           0xF000 through func_8014E640. Bit 0x8000 plays cue 0x101, arms
 *           D_801483A9 with 0x32 and steps the list cursor D_801E625C down by
 *           one, wrapping it to D_801E6258 - 1 once the byte turns negative;
 *           bit 0x2000 plays the same cue, arms D_801483A9 with 0x31 and steps
 *           the cursor up by one, wrapping it to zero once it passes
 *           D_801E6258 - 1. It republishes the selected entry
 *           D_801E6264[D_801E625C] into both D_80148382 and the selected-record
 *           byte D_801483A6 and then, when the pad state shares the confirm
 *           mask D_80145AC2, plays cues 0x103 and 0x102, initializes the
 *           note-cursor block (D_8014839F = 3, D_801483C1 = 7, D_801483C2 =
 *           0xB, D_801483C3 = 2, D_801483A8 = 0xFF, D_801483CC = 0xFF,
 *           D_8014837B = 1, D_801483C0 = 1, D_801483CA = 0, D_801483CB = 0,
 *           D_801483C4 = 0x140, D_801483C6 = 0x3E, D_801483CD = 0) and
 *           advances the UI sub-step byte D_80148652 by one. When the pad state
 *           shares the cancel mask D_80145AC4 instead it plays cue 0x106, stores
 *           1 into both D_8014837B and D_8014839F and steps the UI phase byte
 *           D_80148651 down by one.
 * @status partial
 * @match 95.45
 * @residual The confirm arm's constant materialisation differs: the original
 *           loads 1 and 0xFF into $v1 and $a0 at the head of the arm and
 *           materialises 0x140 before the arming stores, while this spelling
 *           materialises each literal inline in $v0. Eight of the original's
 *           176 instructions differ at the same 704-byte size; the note-cursor
 *           stores, both cue arms and the D_80148652 increment are exact.
 */
void stepNoteListSelectionThenArmNoteCursorSubstep(void) {
  s32 direction;
  u8  index;
  u8  next;
  u8* step;
  u8* phase;

  func_8016620C(0, 5, (u32)(D_80144968 + 164 * D_801E6264[D_801E625C]));
  D_80148340 = 0x75;
  if (D_8014839F == 0) {
    D_80148624 = 1;
    D_80148628 = D_801483A0 + 0x2A;
    D_8014862A = D_801483A2 + 7;
  } else {
    D_80148624 = 0;
  }

  direction = func_8014E640(D_80145AA8 & 0xF000);
  if (direction & 0x8000) {
    func_8015DF18(0x101);
    D_801483A9 = 0x32;
    index = D_801E625C - 1;
    D_801E625C = index;
    if ((s8)index < 0) {
      D_801E625C = D_801E6258 - 1;
    }
  } else if (direction & 0x2000) {
    func_8015DF18(0x101);
    D_801483A9 = 0x31;
    D_801E625C = D_801E625C + 1;
    if (D_801E625C > D_801E6258 - 1) {
      D_801E625C = 0;
    }
  }

  D_80148382 = D_801E6264[D_801E625C];
  D_801483A6 = D_801E6264[D_801E625C];

  if (D_80145AA8 & D_80145AC2) {
    func_8015DF18(0x103);
    func_8015DF18(0x102);
    D_8014839F = 3;
    D_801483C1 = 7;
    D_801483C2 = 0xB;
    D_801483C3 = 2;
    D_801483C4 = 0x140;
    D_801483A8 = 0xFF;
    D_801483CC = 0xFF;
    step = &D_80148652;
    D_8014837B = 1;
    D_801483C0 = 1;
    D_801483CA = 0;
    D_801483CB = 0;
    D_801483C6 = 0x3E;
    D_801483CD = 0;
    *step = *step + 1;
  } else if (D_80145AA8 & D_80145AC4) {
    func_8015DF18(0x106);
    phase = &D_80148651;
    D_8014837B = 1;
    D_8014839F = 1;
    *phase = *phase - 1;
  }
}
